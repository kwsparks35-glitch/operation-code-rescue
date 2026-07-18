#include "CodeRunnerLibrary.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Internationalization/Regex.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

static FString QuoteArg(const FString& In)
{
    return FString::Printf(TEXT("\"%s\""), *In);
}

// Cached availability probes. Initialized to "unknown" on first query, then
// either true or false for the rest of the session. Avoids re-spawning a
// child process every time the player opens a terminal.
static TOptional<bool> GHasJava;
static TOptional<bool> GHasC;
static TOptional<bool> GHasCpp;
static TOptional<bool> GHasPython;
static TOptional<bool> GHasMATLAB;
static constexpr double GCodeValidationProcessTimeoutSeconds = 8.0;

static TAutoConsoleVariable<int32> CVarCodeRescueAllowExternalCodeValidation(
    TEXT("CodeRescue.AllowExternalCodeValidation"),
    0,
    TEXT("Allow Code Rescue terminals to compile/run local user code with OS toolchains. ")
    TEXT("0 keeps validation in-engine for public-release safety; 1 enables trusted local development validators."));

static FString ExternalValidationDisabledMessage()
{
    return TEXT("External compiler/interpreter validation is disabled by CodeRescue.AllowExternalCodeValidation=0. Using the in-engine validator for public-release safety. Trusted local QA can opt in with -AllowExternalCodeValidation.");
}

enum class ECodeChallengeKind
{
    Sum,
    Lock,
    Reverse,
    Palindrome,
    FizzBuzz,
    EvenFilter,
    LinkedListTraverse,
    BinarySearch,
    Unknown
};

static ECodeChallengeKind GetChallengeKind(const FChallengeSpec& Challenge)
{
    if (Challenge.Id.Contains(TEXT("sum")) || Challenge.Id.Contains(TEXT("generator")))
    {
        return ECodeChallengeKind::Sum;
    }
    if (Challenge.Id.Contains(TEXT("lock")))
    {
        return ECodeChallengeKind::Lock;
    }
    if (Challenge.Id.Contains(TEXT("reverse")))
    {
        return ECodeChallengeKind::Reverse;
    }
    if (Challenge.Id.Contains(TEXT("palindrome")))
    {
        return ECodeChallengeKind::Palindrome;
    }
    if (Challenge.Id.Contains(TEXT("fizzbuzz")))
    {
        return ECodeChallengeKind::FizzBuzz;
    }
    if (Challenge.Id.Contains(TEXT("filter")) || Challenge.Id.Contains(TEXT("even")))
    {
        return ECodeChallengeKind::EvenFilter;
    }
    if (Challenge.Id.Contains(TEXT("linkedlist")) || Challenge.Id.Contains(TEXT("linked_list")) || Challenge.Id.Contains(TEXT("traverse")))
    {
        return ECodeChallengeKind::LinkedListTraverse;
    }
    if (Challenge.Id.Contains(TEXT("binary_search")) || Challenge.Id.Contains(TEXT("binarysearch")) || Challenge.Id.Contains(TEXT("bsearch")))
    {
        return ECodeChallengeKind::BinarySearch;
    }
    return ECodeChallengeKind::Unknown;
}

static uint32 ChallengeSeed(const FChallengeSpec& Challenge)
{
    return GetTypeHash(Challenge.Id);
}

static FString BuildValidationSentinel(const FChallengeSpec& Challenge, const TCHAR* LanguageTag)
{
    return FString::Printf(TEXT("CODE_RESCUE_VALIDATION_OK_%08x_%s"), ChallengeSeed(Challenge), LanguageTag);
}

static FString PythonStringLiteral(const FString& In)
{
    FString Out = In;
    Out.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
    Out.ReplaceInline(TEXT("'"), TEXT("\\'"));
    return TEXT("'") + Out + TEXT("'");
}

static FString DoubleQuotedStringLiteral(const FString& In)
{
    FString Out = In;
    Out.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
    Out.ReplaceInline(TEXT("\""), TEXT("\\\""));
    Out.ReplaceInline(TEXT("\r"), TEXT("\\r"));
    Out.ReplaceInline(TEXT("\n"), TEXT("\\n"));
    Out.ReplaceInline(TEXT("\t"), TEXT("\\t"));
    return TEXT("\"") + Out + TEXT("\"");
}

static FString MatlabStringLiteral(const FString& In)
{
    FString Out = In;
    Out.ReplaceInline(TEXT("'"), TEXT("''"));
    return TEXT("'") + Out + TEXT("'");
}

static FString StripQuotedTestValue(FString Value)
{
    Value.TrimStartAndEndInline();
    if (Value.Len() >= 2)
    {
        const TCHAR First = Value[0];
        const TCHAR Last = Value[Value.Len() - 1];
        if ((First == TCHAR('\'') && Last == TCHAR('\'')) || (First == TCHAR('"') && Last == TCHAR('"')))
        {
            return Value.Mid(1, Value.Len() - 2);
        }
    }
    return Value;
}

static bool ParseBoolTestValue(FString Value)
{
    Value.TrimStartAndEndInline();
    Value = Value.ToLower();
    return Value == TEXT("true") || Value == TEXT("1");
}

static TArray<bool> ParseBoolTestArgs(const FString& Input)
{
    TArray<FString> Pieces;
    Input.ParseIntoArray(Pieces, TEXT(","), true);
    TArray<bool> Out;
    for (FString Piece : Pieces)
    {
        Out.Add(ParseBoolTestValue(Piece));
    }
    while (Out.Num() < 2)
    {
        Out.Add(false);
    }
    return Out;
}

static TArray<int32> ParseIntArrayTestValue(FString Value)
{
    Value.ReplaceInline(TEXT("["), TEXT(""));
    Value.ReplaceInline(TEXT("]"), TEXT(""));
    Value.TrimStartAndEndInline();

    TArray<int32> Out;
    if (Value.IsEmpty())
    {
        return Out;
    }

    TArray<FString> Pieces;
    Value.ParseIntoArray(Pieces, TEXT(","), true);
    for (FString Piece : Pieces)
    {
        Piece.TrimStartAndEndInline();
        if (!Piece.IsEmpty())
        {
            Out.Add(FCString::Atoi(*Piece));
        }
    }
    return Out;
}

// ---------------------------------------------------------------------------
// 2026-07-04 R1 — GENERIC data-driven validation.
//
// Before this pass, declarative curriculum tests only executed for the three
// hand-mapped validator kinds (Lock/Reverse/EvenFilter) with HARD-CODED
// function names. This section derives the target function name and argument/
// return types from the challenge's OWN starter code + test literals, so any
// curriculum entry that declares visible/hidden tests executes them — the
// unlock for scaling content without touching C++ (oversight rec R1).

// Array-literal renderers are defined further down this file (near the kind-based
// harness builders); forward-declare for the generic renderers above them.
static FString CArrayLiteral(const TArray<int32>& Values);
static FString JavaArrayLiteral(const TArray<int32>& Values);
static FString PythonListLiteral(const TArray<int32>& Values);
static FString MatlabVectorLiteral(const TArray<int32>& Values);

enum class EGenericLit : uint8 { Bool, Int, Float, Str, IntArray, Unsupported };

static EGenericLit ClassifyGenericLiteral(FString Raw)
{
    Raw.TrimStartAndEndInline();
    if (Raw.IsEmpty())
    {
        return EGenericLit::Unsupported;
    }
    const FString Lower = Raw.ToLower();
    if (Lower == TEXT("true") || Lower == TEXT("false"))
    {
        return EGenericLit::Bool;
    }
    if (Raw.StartsWith(TEXT("[")) && Raw.EndsWith(TEXT("]")))
    {
        // Only int arrays are supported generically (matches curriculum usage).
        FString Inner = Raw.Mid(1, Raw.Len() - 2);
        Inner.ReplaceInline(TEXT(","), TEXT(" "));
        TArray<FString> Parts;
        Inner.ParseIntoArray(Parts, TEXT(" "), true);
        for (const FString& PartRaw : Parts)
        {
            FString Part = PartRaw.TrimStartAndEnd();
            if (!Part.IsEmpty() && !Part.IsNumeric() && !(Part.StartsWith(TEXT("-")) && Part.Mid(1).IsNumeric()))
            {
                return EGenericLit::Unsupported;
            }
        }
        return EGenericLit::IntArray;
    }
    if ((Raw.StartsWith(TEXT("\"")) && Raw.EndsWith(TEXT("\""))) ||
        (Raw.StartsWith(TEXT("'")) && Raw.EndsWith(TEXT("'"))))
    {
        return EGenericLit::Str;
    }
    FString Numeric = Raw;
    if (Numeric.StartsWith(TEXT("-")))
    {
        Numeric = Numeric.Mid(1);
    }
    if (Numeric.IsNumeric())
    {
        return Numeric.Contains(TEXT(".")) ? EGenericLit::Float : EGenericLit::Int;
    }
    return EGenericLit::Unsupported;
}

/** Split "true, [1, 2], \"a,b\"" into top-level args, respecting quotes/brackets. */
static TArray<FString> SplitTopLevelTestArgs(const FString& Input)
{
    TArray<FString> Out;
    FString Current;
    int32 Depth = 0;
    TCHAR Quote = 0;
    for (int32 i = 0; i < Input.Len(); ++i)
    {
        const TCHAR C = Input[i];
        if (Quote != 0)
        {
            Current.AppendChar(C);
            if (C == Quote)
            {
                Quote = 0;
            }
            continue;
        }
        if (C == TCHAR('"') || C == TCHAR('\''))
        {
            Quote = C;
            Current.AppendChar(C);
        }
        else if (C == TCHAR('[') || C == TCHAR('('))
        {
            ++Depth;
            Current.AppendChar(C);
        }
        else if (C == TCHAR(']') || C == TCHAR(')'))
        {
            --Depth;
            Current.AppendChar(C);
        }
        else if (C == TCHAR(',') && Depth == 0)
        {
            Out.Add(Current.TrimStartAndEnd());
            Current.Reset();
        }
        else
        {
            Current.AppendChar(C);
        }
    }
    if (!Current.TrimStartAndEnd().IsEmpty())
    {
        Out.Add(Current.TrimStartAndEnd());
    }
    return Out;
}

static bool IsReservedCodeWord(const FString& Word)
{
    static const TCHAR* Reserved[] = {
        TEXT("if"), TEXT("for"), TEXT("while"), TEXT("switch"), TEXT("return"),
        TEXT("sizeof"), TEXT("printf"), TEXT("main"), TEXT("function"), TEXT("def"),
        TEXT("public"), TEXT("static"), TEXT("class"), TEXT("new"), TEXT("assert"),
    };
    for (const TCHAR* R : Reserved)
    {
        if (Word.Equals(R, ESearchCase::IgnoreCase))
        {
            return true;
        }
    }
    return false;
}

/** Pull the solution function's name out of the challenge's own starter code. */
static FString ExtractStarterFunctionName(ECodingLanguage Language, const FString& Starter)
{
    if (Starter.IsEmpty())
    {
        return FString();
    }
    if (Language == ECodingLanguage::Python)
    {
        FRegexPattern P(TEXT("def\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\("));
        FRegexMatcher M(P, Starter);
        return M.FindNext() ? M.GetCaptureGroup(1) : FString();
    }
    if (Language == ECodingLanguage::MATLAB)
    {
        FRegexPattern PAssign(TEXT("function\\s+[A-Za-z_][A-Za-z0-9_\\[\\]\\s,]*=\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*\\("));
        FRegexMatcher MAssign(PAssign, Starter);
        if (MAssign.FindNext())
        {
            return MAssign.GetCaptureGroup(1);
        }
        FRegexPattern PBare(TEXT("function\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\("));
        FRegexMatcher MBare(PBare, Starter);
        return MBare.FindNext() ? MBare.GetCaptureGroup(1) : FString();
    }
    // C-family / Java: first identifier followed by '(' that is not a keyword.
    FRegexPattern P(TEXT("([A-Za-z_][A-Za-z0-9_]*)\\s*\\("));
    FRegexMatcher M(P, Starter);
    while (M.FindNext())
    {
        const FString Name = M.GetCaptureGroup(1);
        if (!IsReservedCodeWord(Name))
        {
            return Name;
        }
    }
    return FString();
}

static FString SnakeToCamelName(const FString& In)
{
    FString Out;
    bool bUpNext = false;
    for (TCHAR C : In)
    {
        if (C == TCHAR('_'))
        {
            bUpNext = true;
        }
        else
        {
            Out.AppendChar(bUpNext ? FChar::ToUpper(C) : C);
            bUpNext = false;
        }
    }
    return Out;
}

static FString CamelToSnakeName(const FString& In)
{
    FString Out;
    for (TCHAR C : In)
    {
        if (FChar::IsUpper(C))
        {
            Out.AppendChar(TCHAR('_'));
            Out.AppendChar(FChar::ToLower(C));
        }
        else
        {
            Out.AppendChar(C);
        }
    }
    return Out;
}

/** True when every input arg + expected output of every test is a literal we can render. */
static bool GenericDeclarativeSupported(const FChallengeSpec& Challenge, bool bScalarOnly = false)
{
    if (Challenge.TestCases.Num() == 0)
    {
        return false;
    }
    if (ExtractStarterFunctionName(Challenge.Language, Challenge.StarterCode).IsEmpty())
    {
        return false;
    }
    for (const FChallengeTestCase& Test : Challenge.TestCases)
    {
        const EGenericLit ExpectedKind = ClassifyGenericLiteral(Test.ExpectedOutput);
        if (ExpectedKind == EGenericLit::Unsupported)
        {
            return false;
        }
        if (bScalarOnly && (ExpectedKind == EGenericLit::IntArray || ExpectedKind == EGenericLit::Str))
        {
            return false;
        }
        for (const FString& Arg : SplitTopLevelTestArgs(Test.Input))
        {
            const EGenericLit ArgKind = ClassifyGenericLiteral(Arg);
            if (ArgKind == EGenericLit::Unsupported)
            {
                return false;
            }
            if (bScalarOnly && ArgKind == EGenericLit::IntArray)
            {
                return false;
            }
        }
    }
    return true;
}

static FString RenderGenericLiteral(ECodingLanguage Language, const FString& Raw)
{
    const EGenericLit Kind = ClassifyGenericLiteral(Raw);
    switch (Kind)
    {
    case EGenericLit::Bool:
    {
        const bool bValue = ParseBoolTestValue(Raw);
        if (Language == ECodingLanguage::Python)
        {
            return bValue ? TEXT("True") : TEXT("False");
        }
        if (Language == ECodingLanguage::C)
        {
            // C starters use int flags (no stdbool dependency in the sandbox TU).
            return bValue ? TEXT("1") : TEXT("0");
        }
        return bValue ? TEXT("true") : TEXT("false");
    }
    case EGenericLit::Int:
    case EGenericLit::Float:
        return Raw.TrimStartAndEnd();
    case EGenericLit::Str:
    {
        const FString Inner = StripQuotedTestValue(Raw);
        if (Language == ECodingLanguage::Python)
        {
            return PythonStringLiteral(Inner);
        }
        if (Language == ECodingLanguage::MATLAB)
        {
            return MatlabStringLiteral(Inner);
        }
        return DoubleQuotedStringLiteral(Inner);
    }
    case EGenericLit::IntArray:
    {
        const TArray<int32> Values = ParseIntArrayTestValue(Raw);
        switch (Language)
        {
        case ECodingLanguage::Python:  return PythonListLiteral(Values);
        case ECodingLanguage::MATLAB:  return MatlabVectorLiteral(Values);
        case ECodingLanguage::Java:    return JavaArrayLiteral(Values);
        default:                       return TEXT("std::vector<int>") + CArrayLiteral(Values);
        }
    }
    default:
        return Raw;
    }
}

static FString RenderGenericArgList(ECodingLanguage Language, const FString& Input)
{
    TArray<FString> Rendered;
    for (const FString& Arg : SplitTopLevelTestArgs(Input))
    {
        Rendered.Add(RenderGenericLiteral(Language, Arg));
    }
    return FString::Join(Rendered, TEXT(", "));
}

static FString BuildGenericJavaHarness(const FChallengeSpec& Challenge)
{
    const FString Fn = ExtractStarterFunctionName(Challenge.Language, Challenge.StarterCode);
    FString Harness = TEXT("import java.util.Arrays; public class MissionHarness { public static void main(String[] args) { ");
    for (int32 Index = 0; Index < Challenge.TestCases.Num(); ++Index)
    {
        const FChallengeTestCase& Test = Challenge.TestCases[Index];
        const FString Args = RenderGenericArgList(ECodingLanguage::Java, Test.Input);
        const FString Failure = FString::Printf(TEXT("data test %d (%s) failed"), Index + 1, *Test.Label);
        switch (ClassifyGenericLiteral(Test.ExpectedOutput))
        {
        case EGenericLit::Str:
            Harness += FString::Printf(TEXT("if (!%s.equals(MissionSolution.%s(%s))) throw new RuntimeException(\"%s\"); "),
                *RenderGenericLiteral(ECodingLanguage::Java, Test.ExpectedOutput), *Fn, *Args, *Failure);
            break;
        case EGenericLit::IntArray:
            Harness += FString::Printf(TEXT("if (!Arrays.equals(MissionSolution.%s(%s), %s)) throw new RuntimeException(\"%s\"); "),
                *Fn, *Args, *RenderGenericLiteral(ECodingLanguage::Java, Test.ExpectedOutput), *Failure);
            break;
        case EGenericLit::Float:
            Harness += FString::Printf(TEXT("if (Math.abs(MissionSolution.%s(%s) - %s) > 1e-6) throw new RuntimeException(\"%s\"); "),
                *Fn, *Args, *Test.ExpectedOutput.TrimStartAndEnd(), *Failure);
            break;
        default:   // Bool / Int compare directly
            Harness += FString::Printf(TEXT("if (MissionSolution.%s(%s) != %s) throw new RuntimeException(\"%s\"); "),
                *Fn, *Args, *RenderGenericLiteral(ECodingLanguage::Java, Test.ExpectedOutput), *Failure);
            break;
        }
    }
    Harness += TEXT("System.out.println(\"ALL_TESTS_PASSED\"); } }");
    return Harness;
}

static FString BuildGenericCMain(const FChallengeSpec& Challenge, const FString& Sentinel)
{
    // C generic support is scalar-only (bool/int/float args + result); callers
    // must have vetted via GenericDeclarativeSupported(Challenge, /*bScalarOnly=*/true).
    const FString Fn = ExtractStarterFunctionName(Challenge.Language, Challenge.StarterCode);
    FString Source = TEXT("\nint main(void) {\n    int ok = 1;\n");
    for (const FChallengeTestCase& Test : Challenge.TestCases)
    {
        const FString Args = RenderGenericArgList(ECodingLanguage::C, Test.Input);
        switch (ClassifyGenericLiteral(Test.ExpectedOutput))
        {
        case EGenericLit::Float:
            Source += FString::Printf(TEXT("    { double r = (double)%s(%s); if (r - (%s) > 1e-6 || (%s) - r > 1e-6) ok = 0; }\n"),
                *Fn, *Args, *Test.ExpectedOutput.TrimStartAndEnd(), *Test.ExpectedOutput.TrimStartAndEnd());
            break;
        case EGenericLit::Bool:
            Source += FString::Printf(TEXT("    if ((%s(%s) ? 1 : 0) != %d) ok = 0;\n"),
                *Fn, *Args, ParseBoolTestValue(Test.ExpectedOutput) ? 1 : 0);
            break;
        default:   // Int
            Source += FString::Printf(TEXT("    if (%s(%s) != %s) ok = 0;\n"),
                *Fn, *Args, *Test.ExpectedOutput.TrimStartAndEnd());
            break;
        }
    }
    Source += FString::Printf(TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n    return 0;\n}\n"), *Sentinel);
    return Source;
}

static FString BuildGenericCppMain(const FChallengeSpec& Challenge, const FString& Sentinel)
{
    const FString Fn = ExtractStarterFunctionName(Challenge.Language, Challenge.StarterCode);
    FString Source = TEXT("int main() { bool ok = true; ");
    for (const FChallengeTestCase& Test : Challenge.TestCases)
    {
        const FString Args = RenderGenericArgList(Challenge.Language, Test.Input);
        switch (ClassifyGenericLiteral(Test.ExpectedOutput))
        {
        case EGenericLit::Float:
            Source += FString::Printf(TEXT("{ double r = (double)%s(%s); ok = ok && (r - (%s) < 1e-6) && ((%s) - r < 1e-6); } "),
                *Fn, *Args, *Test.ExpectedOutput.TrimStartAndEnd(), *Test.ExpectedOutput.TrimStartAndEnd());
            break;
        case EGenericLit::IntArray:
            Source += FString::Printf(TEXT("ok = ok && cr_equal(%s(%s), %s); "),
                *Fn, *Args, *RenderGenericLiteral(Challenge.Language, Test.ExpectedOutput));
            break;
        default:   // Bool / Int / Str (std::string ==)
            Source += FString::Printf(TEXT("ok = ok && (%s(%s) == %s); "),
                *Fn, *Args, *RenderGenericLiteral(Challenge.Language, Test.ExpectedOutput));
            break;
        }
    }
    Source += FString::Printf(TEXT("cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *Sentinel);
    return Source;
}

static FString BuildGenericPythonTestBlock(const FChallengeSpec& Challenge, const FString& SentinelLiteral)
{
    const FString Fn = ExtractStarterFunctionName(Challenge.Language, Challenge.StarterCode);
    FString Harness = FString::Printf(
        TEXT("_fn = _ns.get('%s') or _ns.get('%s') or _ns.get('%s')\nassert _fn is not None, 'missing function %s'\n"),
        *Fn, *SnakeToCamelName(Fn), *CamelToSnakeName(Fn), *Fn);
    for (const FChallengeTestCase& Test : Challenge.TestCases)
    {
        const FString Args = RenderGenericArgList(ECodingLanguage::Python, Test.Input);
        const FString Expected = RenderGenericLiteral(ECodingLanguage::Python, Test.ExpectedOutput);
        switch (ClassifyGenericLiteral(Test.ExpectedOutput))
        {
        case EGenericLit::Bool:
            Harness += FString::Printf(TEXT("assert _fn(%s) is %s\n"), *Args, *Expected);
            break;
        case EGenericLit::Float:
            Harness += FString::Printf(TEXT("assert abs(_fn(%s) - (%s)) < 1e-6\n"), *Args, *Expected);
            break;
        case EGenericLit::IntArray:
            Harness += FString::Printf(TEXT("assert list(_fn(%s)) == %s\n"), *Args, *Expected);
            break;
        default:
            Harness += FString::Printf(TEXT("assert _fn(%s) == %s\n"), *Args, *Expected);
            break;
        }
    }
    Harness += FString::Printf(TEXT("print(%s)\n"), *SentinelLiteral);
    return Harness;
}

static FString BuildGenericMatlabRunner(const FChallengeSpec& Challenge, const FString& SandboxDir)
{
    const FString Fn = ExtractStarterFunctionName(Challenge.Language, Challenge.StarterCode);
    FString SafeSandboxDir = SandboxDir;
    SafeSandboxDir.ReplaceInline(TEXT("\\"), TEXT("/"));
    FString Runner = FString::Printf(TEXT("addpath('%s'); "), *SafeSandboxDir);
    for (const FChallengeTestCase& Test : Challenge.TestCases)
    {
        const FString Args = RenderGenericArgList(ECodingLanguage::MATLAB, Test.Input);
        const FString Expected = RenderGenericLiteral(ECodingLanguage::MATLAB, Test.ExpectedOutput);
        switch (ClassifyGenericLiteral(Test.ExpectedOutput))
        {
        case EGenericLit::Str:
            Runner += FString::Printf(TEXT("assert(strcmp(%s(%s),%s)); "), *Fn, *Args, *Expected);
            break;
        case EGenericLit::Float:
            Runner += FString::Printf(TEXT("assert(abs(%s(%s) - (%s)) < 1e-6); "), *Fn, *Args, *Expected);
            break;
        case EGenericLit::IntArray:
            Runner += FString::Printf(TEXT("assert(isequal(%s(%s), %s)); "), *Fn, *Args, *Expected);
            break;
        default:
            Runner += FString::Printf(TEXT("assert(%s(%s)==%s); "), *Fn, *Args, *Expected);
            break;
        }
    }
    Runner += TEXT("disp('ALL_TESTS_PASSED');");
    return Runner;
}

static bool CanUseDeclarativeTests(const FChallengeSpec& Challenge, ECodeChallengeKind Kind)
{
    if (Challenge.TestCases.Num() == 0)
    {
        return false;
    }
    if (Kind == ECodeChallengeKind::Lock
        || Kind == ECodeChallengeKind::Reverse
        || Kind == ECodeChallengeKind::EvenFilter)
    {
        return true;
    }
    // 2026-07-04 R1: any other challenge with declared tests executes them
    // generically when the starter's function name + all literals are derivable.
    // C is scalar-only (no generic array/string return plumbing there).
    const bool bScalarOnly = (Challenge.Language == ECodingLanguage::C);
    return Kind == ECodeChallengeKind::Unknown && GenericDeclarativeSupported(Challenge, bScalarOnly);
}

static void ApplyDeclarativeTestCaseCounts(FCodeValidationResult& Result, const FChallengeSpec& Challenge)
{
    if (Challenge.TestCases.Num() > 0)
    {
        Result.TotalTestCases = Challenge.TestCases.Num();
        Result.PassedTestCases = Result.bSuccess ? Challenge.TestCases.Num() : 0;
    }
}

static FString CValidationMacroUndefs()
{
    return TEXT("\n")
        TEXT("#undef main\n")
        TEXT("#undef printf\n")
        TEXT("#undef strcmp\n")
        TEXT("#undef memcmp\n")
        TEXT("#undef totalPower\n")
        TEXT("#undef shouldUnlock\n")
        TEXT("#undef reverseString\n")
        TEXT("#undef isPalindrome\n")
        TEXT("#undef fizzBuzz\n")
        TEXT("#undef evenNumbers\n")
        TEXT("#undef countNodes\n")
        TEXT("#undef binarySearch\n");
}

static FString ReverseStringLiteral(const FString& In)
{
    FString Out;
    for (int32 i = In.Len() - 1; i >= 0; --i)
    {
        Out.AppendChar(In[i]);
    }
    return Out;
}

static FString HiddenReverseWord(const FChallengeSpec& Challenge)
{
    static const TCHAR* Words[] = {
        TEXT("signal"),
        TEXT("beacon"),
        TEXT("syntax"),
        TEXT("vector"),
        TEXT("module"),
        TEXT("rescuecode")
    };
    return Words[ChallengeSeed(Challenge) % UE_ARRAY_COUNT(Words)];
}

static FString HiddenPalindromeWord(const FChallengeSpec& Challenge)
{
    static const TCHAR* Words[] = {
        TEXT("level"),
        TEXT("civic"),
        TEXT("radar"),
        TEXT("refer"),
        TEXT("rotor")
    };
    return Words[ChallengeSeed(Challenge) % UE_ARRAY_COUNT(Words)];
}

static FString HiddenNonPalindromeWord(const FChallengeSpec& Challenge)
{
    static const TCHAR* Words[] = {
        TEXT("rescue"),
        TEXT("signal"),
        TEXT("module"),
        TEXT("syntax"),
        TEXT("compile")
    };
    return Words[(ChallengeSeed(Challenge) / 7) % UE_ARRAY_COUNT(Words)];
}

static void HiddenSumCase(const FChallengeSpec& Challenge, int32& A, int32& B, int32& C)
{
    const uint32 Seed = ChallengeSeed(Challenge);
    A = 2 + static_cast<int32>(Seed % 11);
    B = 3 + static_cast<int32>((Seed / 11) % 13);
    C = 4 + static_cast<int32>((Seed / 29) % 17);
}

static FString BuildFizzBuzzCsv(int32 N)
{
    FString Out;
    for (int32 i = 1; i <= N; ++i)
    {
        if (i > 1)
        {
            Out += TEXT(",");
        }
        if (i % 15 == 0)
        {
            Out += TEXT("FizzBuzz");
        }
        else if (i % 3 == 0)
        {
            Out += TEXT("Fizz");
        }
        else if (i % 5 == 0)
        {
            Out += TEXT("Buzz");
        }
        else
        {
            Out += FString::FromInt(i);
        }
    }
    return Out;
}

static int32 HiddenFizzBuzzN(const FChallengeSpec& Challenge)
{
    return 16 + static_cast<int32>(ChallengeSeed(Challenge) % 5);
}

static TArray<int32> HiddenEvenInput(const FChallengeSpec& Challenge)
{
    const int32 Base = 1 + static_cast<int32>(ChallengeSeed(Challenge) % 6);
    return { Base, Base + 1, Base + 2, Base + 3, Base + 4, Base + 5, Base + 6 };
}

static TArray<int32> HiddenLinkedNext(const FChallengeSpec& Challenge)
{
    const uint32 Seed = ChallengeSeed(Challenge);
    switch (Seed % 3)
    {
    case 0:
        return { 3, -1, 4, 1, 5, -1 };
    case 1:
        return { 2, 4, 3, -1, 5, -1 };
    default:
        return { 1, 3, -1, 4, 2, -1 };
    }
}

static int32 CountLinkedNodes(const TArray<int32>& Next, int32 Start)
{
    int32 Total = 0;
    int32 Current = Start;
    int32 Guard = 0;
    while (Next.IsValidIndex(Current) && Guard <= Next.Num())
    {
        ++Total;
        Current = Next[Current];
        ++Guard;
        if (Current < 0)
        {
            break;
        }
    }
    return Total;
}

static TArray<int32> HiddenSearchValues(const FChallengeSpec& Challenge)
{
    const int32 Base = 2 + static_cast<int32>(ChallengeSeed(Challenge) % 5);
    return { Base, Base + 3, Base + 8, Base + 13, Base + 21, Base + 34 };
}

static TArray<int32> FilterEvenValues(const TArray<int32>& Values)
{
    TArray<int32> Out;
    for (int32 Value : Values)
    {
        if (Value % 2 == 0)
        {
            Out.Add(Value);
        }
    }
    return Out;
}

static FString JoinInts(const TArray<int32>& Values, const FString& Separator)
{
    FString Out;
    for (int32 i = 0; i < Values.Num(); ++i)
    {
        if (i > 0)
        {
            Out += Separator;
        }
        Out += FString::FromInt(Values[i]);
    }
    return Out;
}

static FString CArrayLiteral(const TArray<int32>& Values)
{
    return TEXT("{") + JoinInts(Values, TEXT(",")) + TEXT("}");
}

static FString JavaArrayLiteral(const TArray<int32>& Values)
{
    return TEXT("new int[]{") + JoinInts(Values, TEXT(",")) + TEXT("}");
}

static FString PythonListLiteral(const TArray<int32>& Values)
{
    return TEXT("[") + JoinInts(Values, TEXT(",")) + TEXT("]");
}

static FString MatlabVectorLiteral(const TArray<int32>& Values)
{
    return TEXT("[") + JoinInts(Values, TEXT(" ")) + TEXT("]");
}

static FString MatlabLinkedVectorLiteral(const TArray<int32>& ZeroBasedNext)
{
    TArray<int32> OneBasedNext;
    OneBasedNext.Reserve(ZeroBasedNext.Num());
    for (int32 Value : ZeroBasedNext)
    {
        OneBasedNext.Add(Value < 0 ? 0 : Value + 1);
    }
    return MatlabVectorLiteral(OneBasedNext);
}

static FString CArrayInitializerOrZero(const TArray<int32>& Values)
{
    return Values.Num() > 0 ? CArrayLiteral(Values) : TEXT("{0}");
}

static FString BuildJavaDeclarativeHarness(const FChallengeSpec& Challenge, ECodeChallengeKind Kind)
{
    if (Kind == ECodeChallengeKind::Unknown)
    {
        return BuildGenericJavaHarness(Challenge);   // 2026-07-04 R1
    }
    FString Harness = Kind == ECodeChallengeKind::EvenFilter ? TEXT("import java.util.Arrays; ") : TEXT("");
    Harness += TEXT("public class MissionHarness { public static void main(String[] args) { ");
    for (int32 Index = 0; Index < Challenge.TestCases.Num(); ++Index)
    {
        const FChallengeTestCase& Test = Challenge.TestCases[Index];
        const FString Failure = FString::Printf(TEXT("data test %d failed"), Index + 1);
        if (Kind == ECodeChallengeKind::Lock)
        {
            const TArray<bool> Args = ParseBoolTestArgs(Test.Input);
            const bool bExpected = ParseBoolTestValue(Test.ExpectedOutput);
            Harness += FString::Printf(
                TEXT("if (MissionSolution.shouldUnlock(%s,%s) != %s) throw new RuntimeException(\"%s\"); "),
                Args[0] ? TEXT("true") : TEXT("false"),
                Args[1] ? TEXT("true") : TEXT("false"),
                bExpected ? TEXT("true") : TEXT("false"),
                *Failure);
        }
        else if (Kind == ECodeChallengeKind::Reverse)
        {
            Harness += FString::Printf(
                TEXT("if (!%s.equals(MissionSolution.reverseString(%s))) throw new RuntimeException(\"%s\"); "),
                *DoubleQuotedStringLiteral(StripQuotedTestValue(Test.ExpectedOutput)),
                *DoubleQuotedStringLiteral(StripQuotedTestValue(Test.Input)),
                *Failure);
        }
        else if (Kind == ECodeChallengeKind::EvenFilter)
        {
            Harness += FString::Printf(
                TEXT("if (!Arrays.equals(MissionSolution.evenNumbers(%s), %s)) throw new RuntimeException(\"%s\"); "),
                *JavaArrayLiteral(ParseIntArrayTestValue(Test.Input)),
                *JavaArrayLiteral(ParseIntArrayTestValue(Test.ExpectedOutput)),
                *Failure);
        }
    }
    Harness += TEXT("System.out.println(\"ALL_TESTS_PASSED\"); } }");
    return Harness;
}

static FString BuildCDeclarativeMain(const FChallengeSpec& Challenge, ECodeChallengeKind Kind, const FString& Sentinel)
{
    if (Kind == ECodeChallengeKind::Unknown)
    {
        return BuildGenericCMain(Challenge, Sentinel);   // 2026-07-04 R1 (scalar-only)
    }
    FString Source = TEXT("\nint main(void) {\n    int ok = 1;\n");
    for (int32 Index = 0; Index < Challenge.TestCases.Num(); ++Index)
    {
        const FChallengeTestCase& Test = Challenge.TestCases[Index];
        if (Kind == ECodeChallengeKind::Lock)
        {
            const TArray<bool> Args = ParseBoolTestArgs(Test.Input);
            const bool bExpected = ParseBoolTestValue(Test.ExpectedOutput);
            Source += FString::Printf(
                TEXT("    if ((shouldUnlock(%d, %d) ? 1 : 0) != %d) ok = 0;\n"),
                Args[0] ? 1 : 0,
                Args[1] ? 1 : 0,
                bExpected ? 1 : 0);
        }
        else if (Kind == ECodeChallengeKind::Reverse)
        {
            Source += FString::Printf(
                TEXT("    { char out[128]; reverseString(%s, out); if (strcmp(out, %s) != 0) ok = 0; }\n"),
                *DoubleQuotedStringLiteral(StripQuotedTestValue(Test.Input)),
                *DoubleQuotedStringLiteral(StripQuotedTestValue(Test.ExpectedOutput)));
        }
        else if (Kind == ECodeChallengeKind::EvenFilter)
        {
            const TArray<int32> Input = ParseIntArrayTestValue(Test.Input);
            const TArray<int32> Expected = ParseIntArrayTestValue(Test.ExpectedOutput);
            const int32 InputStorage = FMath::Max(1, Input.Num());
            const int32 ExpectedStorage = FMath::Max(1, Expected.Num());
            const int32 OutputStorage = FMath::Max(InputStorage, ExpectedStorage);
            Source += FString::Printf(
                TEXT("    { int input%d[%d] = %s; int expected%d[%d] = %s; int out%d[%d] = {0}; int count%d = evenNumbers(input%d, %d, out%d); if (count%d != %d) ok = 0; if (%d > 0 && memcmp(out%d, expected%d, sizeof(int) * %d) != 0) ok = 0; }\n"),
                Index, InputStorage, *CArrayInitializerOrZero(Input),
                Index, ExpectedStorage, *CArrayInitializerOrZero(Expected),
                Index, OutputStorage,
                Index, Index, Input.Num(), Index,
                Index, Expected.Num(),
                Expected.Num(), Index, Index, Expected.Num());
        }
    }
    Source += FString::Printf(
        TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n    return 0;\n}\n"),
        *Sentinel);
    return Source;
}

static FString BuildCppDeclarativeMain(const FChallengeSpec& Challenge, ECodeChallengeKind Kind, const FString& Sentinel)
{
    if (Kind == ECodeChallengeKind::Unknown)
    {
        return BuildGenericCppMain(Challenge, Sentinel);   // 2026-07-04 R1
    }
    FString Source = TEXT("int main() { bool ok = true; ");
    for (const FChallengeTestCase& Test : Challenge.TestCases)
    {
        if (Kind == ECodeChallengeKind::Lock)
        {
            const TArray<bool> Args = ParseBoolTestArgs(Test.Input);
            const bool bExpected = ParseBoolTestValue(Test.ExpectedOutput);
            Source += FString::Printf(
                TEXT("ok = ok && (shouldUnlock(%s, %s) == %s); "),
                Args[0] ? TEXT("true") : TEXT("false"),
                Args[1] ? TEXT("true") : TEXT("false"),
                bExpected ? TEXT("true") : TEXT("false"));
        }
        else if (Kind == ECodeChallengeKind::Reverse)
        {
            Source += FString::Printf(
                TEXT("ok = ok && reverseString(%s) == %s; "),
                *DoubleQuotedStringLiteral(StripQuotedTestValue(Test.Input)),
                *DoubleQuotedStringLiteral(StripQuotedTestValue(Test.ExpectedOutput)));
        }
        else if (Kind == ECodeChallengeKind::EvenFilter)
        {
            Source += FString::Printf(
                TEXT("ok = ok && cr_equal(evenNumbers(vector<int>%s), vector<int>%s); "),
                *CArrayLiteral(ParseIntArrayTestValue(Test.Input)),
                *CArrayLiteral(ParseIntArrayTestValue(Test.ExpectedOutput)));
        }
    }
    Source += FString::Printf(TEXT("cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *Sentinel);
    return Source;
}

static FString BuildPythonDeclarativeTestBlock(const FChallengeSpec& Challenge, ECodeChallengeKind Kind, const FString& SentinelLiteral)
{
    if (Kind == ECodeChallengeKind::Unknown)
    {
        return BuildGenericPythonTestBlock(Challenge, SentinelLiteral);   // 2026-07-04 R1
    }
    FString Harness;
    if (Kind == ECodeChallengeKind::Lock)
    {
        Harness += TEXT("_fn = _ns.get('should_unlock') or _ns.get('shouldUnlock')\nassert _fn is not None\n");
        for (const FChallengeTestCase& Test : Challenge.TestCases)
        {
            const TArray<bool> Args = ParseBoolTestArgs(Test.Input);
            const bool bExpected = ParseBoolTestValue(Test.ExpectedOutput);
            Harness += FString::Printf(
                TEXT("assert _fn(%s, %s) is %s\n"),
                Args[0] ? TEXT("True") : TEXT("False"),
                Args[1] ? TEXT("True") : TEXT("False"),
                bExpected ? TEXT("True") : TEXT("False"));
        }
    }
    else if (Kind == ECodeChallengeKind::Reverse)
    {
        Harness += TEXT("_fn = _ns.get('reverse_string') or _ns.get('reverseString') or _ns.get('reverse')\nassert _fn is not None\n");
        for (const FChallengeTestCase& Test : Challenge.TestCases)
        {
            Harness += FString::Printf(
                TEXT("assert _fn(%s) == %s\n"),
                *PythonStringLiteral(StripQuotedTestValue(Test.Input)),
                *PythonStringLiteral(StripQuotedTestValue(Test.ExpectedOutput)));
        }
    }
    else if (Kind == ECodeChallengeKind::EvenFilter)
    {
        Harness += TEXT("_fn = _ns.get('even_numbers') or _ns.get('evenNumbers') or _ns.get('evens') or _ns.get('filter_evens')\nassert _fn is not None\n");
        for (const FChallengeTestCase& Test : Challenge.TestCases)
        {
            Harness += FString::Printf(
                TEXT("assert list(_fn(%s)) == %s\n"),
                *PythonListLiteral(ParseIntArrayTestValue(Test.Input)),
                *PythonListLiteral(ParseIntArrayTestValue(Test.ExpectedOutput)));
        }
    }
    Harness += FString::Printf(TEXT("print(%s)\n"), *SentinelLiteral);
    return Harness;
}

static FString BuildMatlabDeclarativeRunner(const FChallengeSpec& Challenge, ECodeChallengeKind Kind, const FString& SandboxDir)
{
    if (Kind == ECodeChallengeKind::Unknown)
    {
        return BuildGenericMatlabRunner(Challenge, SandboxDir);   // 2026-07-04 R1
    }
    FString SafeSandboxDir = SandboxDir;
    SafeSandboxDir.ReplaceInline(TEXT("\\"), TEXT("/"));
    FString Runner = FString::Printf(TEXT("addpath('%s'); "), *SafeSandboxDir);
    for (const FChallengeTestCase& Test : Challenge.TestCases)
    {
        if (Kind == ECodeChallengeKind::Lock)
        {
            const TArray<bool> Args = ParseBoolTestArgs(Test.Input);
            const bool bExpected = ParseBoolTestValue(Test.ExpectedOutput);
            Runner += FString::Printf(
                TEXT("assert(should_unlock(%s,%s)==%s); "),
                Args[0] ? TEXT("true") : TEXT("false"),
                Args[1] ? TEXT("true") : TEXT("false"),
                bExpected ? TEXT("true") : TEXT("false"));
        }
        else if (Kind == ECodeChallengeKind::Reverse)
        {
            Runner += FString::Printf(
                TEXT("assert(strcmp(reverse_string(%s),%s)); "),
                *MatlabStringLiteral(StripQuotedTestValue(Test.Input)),
                *MatlabStringLiteral(StripQuotedTestValue(Test.ExpectedOutput)));
        }
        else if (Kind == ECodeChallengeKind::EvenFilter)
        {
            Runner += FString::Printf(
                TEXT("assert(isequal(even_numbers(%s), %s)); "),
                *MatlabVectorLiteral(ParseIntArrayTestValue(Test.Input)),
                *MatlabVectorLiteral(ParseIntArrayTestValue(Test.ExpectedOutput)));
        }
    }
    Runner += TEXT("disp('ALL_TESTS_PASSED');");
    return Runner;
}

bool UCodeRunnerLibrary::ProbeExecutable(const FString& Command, const FString& VersionArg)
{
    FString StdOut, StdErr;
    int32 Code = -1;
    // /usr/bin/env <cmd> <arg> succeeds (exit 0) if <cmd> is on PATH.
    const bool bOk = FPlatformProcess::ExecProcess(
        TEXT("/usr/bin/env"),
        *FString::Printf(TEXT("%s %s"), *Command, *VersionArg),
        &Code, &StdOut, &StdErr);
    return bOk && Code == 0;
}

bool UCodeRunnerLibrary::AreExternalValidatorsAllowed()
{
    if (FParse::Param(FCommandLine::Get(), TEXT("AllowExternalCodeValidation")))
    {
        return true;
    }
    return CVarCodeRescueAllowExternalCodeValidation.GetValueOnGameThread() != 0;
}

bool UCodeRunnerLibrary::IsLanguageAvailable(ECodingLanguage Language)
{
    if (!AreExternalValidatorsAllowed())
    {
        return false;
    }

    switch (Language)
    {
    case ECodingLanguage::Java:
        if (!GHasJava.IsSet())
        {
            GHasJava = ProbeExecutable(TEXT("javac"), TEXT("-version")) &&
                       ProbeExecutable(TEXT("java"), TEXT("-version"));
        }
        return GHasJava.GetValue();

    case ECodingLanguage::C:
        if (!GHasC.IsSet())
        {
            GHasC = ProbeExecutable(TEXT("clang"), TEXT("--version"));
        }
        return GHasC.GetValue();

    case ECodingLanguage::CPlus:
    case ECodingLanguage::Cpp:
        if (!GHasCpp.IsSet())
        {
            GHasCpp = ProbeExecutable(TEXT("clang++"), TEXT("--version"));
        }
        return GHasCpp.GetValue();

    case ECodingLanguage::Python:
        if (!GHasPython.IsSet())
        {
            GHasPython = ProbeExecutable(TEXT("python3"), TEXT("--version"));
        }
        return GHasPython.GetValue();

    case ECodingLanguage::MATLAB:
        if (!GHasMATLAB.IsSet())
        {
            const FString Bin = FindMATLABExecutable();
            GHasMATLAB = (!Bin.IsEmpty() && Bin != TEXT("matlab") && FPaths::FileExists(Bin)) ||
                         (Bin == TEXT("matlab") && ProbeExecutable(TEXT("matlab"), TEXT("-batch \"exit\"")));
        }
        return GHasMATLAB.GetValue();
    }
    return false;
}

FString UCodeRunnerLibrary::GetLanguageDependencyMessage(ECodingLanguage Language)
{
    if (!AreExternalValidatorsAllowed())
    {
        return ExternalValidationDisabledMessage();
    }

    const bool bAvailable = IsLanguageAvailable(Language);
    switch (Language)
    {
    case ECodingLanguage::Java:
        return bAvailable
            ? TEXT("Java toolchain detected (javac + java).")
            : TEXT("Java toolchain not found. Install JDK 17+ and ensure 'javac' and 'java' are on PATH. Falling back to in-engine validator.");
    case ECodingLanguage::C:
        return bAvailable
            ? TEXT("C toolchain detected (clang).")
            : TEXT("C toolchain not found. Install Xcode command-line tools (xcode-select --install) so 'clang' is on PATH. Falling back to in-engine validator.");
    case ECodingLanguage::CPlus:
        return bAvailable
            ? TEXT("C+ toolchain detected (clang++).")
            : TEXT("C+ toolchain not found. Install Xcode command-line tools (xcode-select --install) so 'clang++' is on PATH. Falling back to in-engine validator.");
    case ECodingLanguage::Cpp:
        return bAvailable
            ? TEXT("C++ toolchain detected (clang++).")
            : TEXT("C++ toolchain not found. Install Xcode command-line tools (xcode-select --install) so 'clang++' is on PATH. Falling back to in-engine validator.");
    case ECodingLanguage::Python:
        return bAvailable
            ? TEXT("Python toolchain detected (python3).")
            : TEXT("Python 3 not found. Install Python 3.10+ and ensure 'python3' is on PATH. Falling back to in-engine validator.");
    case ECodingLanguage::MATLAB:
        return bAvailable
            ? TEXT("MATLAB binary detected.")
            : TEXT("MATLAB not auto-detected. Set MATLAB_BIN, set MATLABROOT, install under /Applications, or ensure 'matlab' is on PATH. Falling back to in-engine validator.");
    }
    return TEXT("");
}

// ---- In-engine static-analysis fallback -------------------------------------
// Pattern-matches user code against the two known mission shapes. Returns a
// validation result that mirrors the external-tool path (passed/failed checks,
// score, summary), so the terminal widget renders identically either way.
FCodeValidationResult UCodeRunnerLibrary::ValidateInEngine(const FChallengeSpec& Challenge, const FString& UserCode)
{
    FCodeValidationResult Result;
    Result.StdOut = TEXT("(in-engine static-analysis validator — no external compiler used)\n");

    // Normalize the code so pattern matching is robust to formatting variation.
    FString Normalized = UserCode;
    Normalized.ReplaceInline(TEXT("\r"), TEXT(""));
    Normalized.ReplaceInline(TEXT("\t"), TEXT(" "));
    Normalized.ReplaceInline(TEXT("\n"), TEXT(" "));
    while (Normalized.Contains(TEXT("  ")))
    {
        Normalized.ReplaceInline(TEXT("  "), TEXT(" "));
    }

    // 2026-07-11 GENERALIZED ACCEPTANCE (Kenny's packaged report: two
    // functionally correct solutions rejected — a prefix-decrement !=
    // two-pointer palindrome and an overflow-safe lo+(hi-lo)/2 binary search).
    // Every archetype check below must accept every common functional FAMILY
    // of a correct solution, not one literal spelling. `Compact` (all
    // whitespace removed, lowercased) powers spelling-insensitive matching;
    // `NormalizedLower` keeps word boundaries for token searches.
    FString Compact = Normalized.ToLower();
    Compact.ReplaceInline(TEXT(" "), TEXT(""));
    const FString NormalizedLower = Normalized.ToLower();

    // Any explicit return/output form counts as "produces the value":
    // return / return( / print / printf / println / disp / cout /
    // console.log / assignment to a result-style variable.
    auto ProducesOutput = [&Compact]()
    {
        return Compact.Contains(TEXT("return"))
            || Compact.Contains(TEXT("disp("))
            || Compact.Contains(TEXT("print"))
            || Compact.Contains(TEXT("cout"))
            || Compact.Contains(TEXT("console.log"))
            || Compact.Contains(TEXT("result="))
            || Compact.Contains(TEXT("output="))
            || Compact.Contains(TEXT("out="));
    };

    const bool bSumChallenge       = Challenge.Id.Contains(TEXT("sum")) || Challenge.Id.Contains(TEXT("generator"));
    const bool bLockChallenge      = Challenge.Id.Contains(TEXT("lock"));
    // New shapes added in roadmap item 15 — see
    // Documentation/zombie_system/15_challenge_types.md for the prompt
    // language each `Id` substring matches.
    const bool bReverseChallenge   = Challenge.Id.Contains(TEXT("reverse"));
    const bool bPalindromeChallenge = Challenge.Id.Contains(TEXT("palindrome"));
    const bool bFizzBuzzChallenge  = Challenge.Id.Contains(TEXT("fizzbuzz"));
    const bool bFilterChallenge    = Challenge.Id.Contains(TEXT("filter")) || Challenge.Id.Contains(TEXT("even"));
    // Items added by improvement_pass_2026-05-03 (#13): linked-list traversal
    // and binary search. Each looks for the canonical algorithmic shape so
    // students can solve them in any of the four supported languages.
    const bool bLinkedListChallenge = Challenge.Id.Contains(TEXT("linkedlist"))
                                   || Challenge.Id.Contains(TEXT("linked_list"))
                                   || Challenge.Id.Contains(TEXT("traverse"));
    const bool bBinarySearchChallenge = Challenge.Id.Contains(TEXT("binary_search"))
                                     || Challenge.Id.Contains(TEXT("binarysearch"))
                                     || Challenge.Id.Contains(TEXT("bsearch"));

    // 2026-07-04 R1: GENERIC in-engine structural validation. Any data-driven
    // curriculum challenge (no hand-mapped archetype) with declared tests gets
    // an honest structural gate derived from its OWN starter: required function
    // present, produces a result, arity matches. Real test EXECUTION happens in
    // the external validators (CodeRescue.AllowExternalCodeValidation=1); this
    // fallback keeps offline play teachable for every entry in the curriculum.
    const bool bKnownArchetype = bSumChallenge || bLockChallenge || bReverseChallenge
        || bPalindromeChallenge || bFizzBuzzChallenge || bFilterChallenge
        || bLinkedListChallenge || bBinarySearchChallenge;
    if (!bKnownArchetype && Challenge.TestCases.Num() > 0)
    {
        const FString Fn = ExtractStarterFunctionName(Challenge.Language, Challenge.StarterCode);
        if (!Fn.IsEmpty())
        {
            const bool bMentionsFn = Normalized.Contains(Fn)
                || Normalized.Contains(SnakeToCamelName(Fn))
                || Normalized.Contains(CamelToSnakeName(Fn));
            AddCheck(Result, FString::Printf(TEXT("Defines the required function %s"), *Fn), bMentionsFn);

            // 2026-07-11: `return(x)`, print/cout output, and result-variable
            // assignment are as valid as `return ` with a trailing space.
            const bool bProducesResult = ProducesOutput()
                || Challenge.Language == ECodingLanguage::MATLAB;   // MATLAB outputs via assignment
            AddCheck(Result, TEXT("Produces a result (return / output assignment)"), bProducesResult);

            // Arity: compare the starter's parameter count with the user's definition.
            auto CountParams = [](const FString& Text, const FString& Name) -> int32
            {
                const int32 At = Text.Find(Name + TEXT("("));
                if (At == INDEX_NONE)
                {
                    return -1;
                }
                int32 Depth = 0;
                FString Params;
                for (int32 i = At + Name.Len(); i < Text.Len(); ++i)
                {
                    const TCHAR C = Text[i];
                    if (C == TCHAR('('))
                    {
                        ++Depth;
                        if (Depth == 1) { continue; }
                    }
                    if (C == TCHAR(')'))
                    {
                        --Depth;
                        if (Depth == 0) { break; }
                    }
                    if (Depth >= 1)
                    {
                        Params.AppendChar(C);
                    }
                }
                Params.TrimStartAndEndInline();
                if (Params.IsEmpty())
                {
                    return 0;
                }
                int32 Commas = 0;
                for (TCHAR C : Params)
                {
                    if (C == TCHAR(','))
                    {
                        ++Commas;
                    }
                }
                return Commas + 1;
            };
            const int32 StarterParams = CountParams(Challenge.StarterCode, Fn);
            const int32 UserParams = CountParams(Normalized, Fn);
            if (StarterParams >= 0 && UserParams >= 0)
            {
                AddCheck(Result, FString::Printf(TEXT("Keeps the required signature (%d parameter%s)"),
                    StarterParams, StarterParams == 1 ? TEXT("") : TEXT("s")), UserParams == StarterParams);
            }

            // 2026-07-05 first-level completion: ANTI-TRIVIAL gates. The unchanged
            // starter (or a body that still only returns the placeholder) must NEVER
            // validate — the very first lesson is writing the body yourself. External
            // validation doesn't need this (real tests fail trivial code naturally).
            auto NormalizeForCompare = [](FString In)
            {
                In.ReplaceInline(TEXT("\r"), TEXT(""));
                In.ReplaceInline(TEXT("\n"), TEXT(""));
                In.ReplaceInline(TEXT("\t"), TEXT(""));
                In.ReplaceInline(TEXT(" "), TEXT(""));
                return In;
            };
            const FString UserNorm = NormalizeForCompare(UserCode);
            const FString StarterNorm = NormalizeForCompare(Challenge.StarterCode);
            AddCheck(Result, TEXT("Starter code was actually modified (write the body!)"),
                !StarterNorm.IsEmpty() && UserNorm != StarterNorm);

            static const TCHAR* PlaceholderReturns[] = {
                TEXT("return0;"), TEXT("return0.0;"), TEXT("returnfalse;"), TEXT("returnFalse"),
                TEXT("return\"\";"), TEXT("return'';"), TEXT("return[];"), TEXT("out=0;"), TEXT("out=[];"), TEXT("out='';"),
            };
            bool bStillPlaceholder = false;
            for (const TCHAR* Placeholder : PlaceholderReturns)
            {
                if (StarterNorm.Contains(Placeholder) && UserNorm.Contains(Placeholder))
                {
                    bStillPlaceholder = true;
                    break;
                }
            }
            AddCheck(Result, TEXT("Returns a computed value (the placeholder return is gone)"),
                !bStillPlaceholder);

            Result.StdOut += FString::Printf(
                TEXT("Structural gate for data-driven challenge '%s' (%d declared test case%s).\n")
                TEXT("Enable CodeRescue.AllowExternalCodeValidation=1 to EXECUTE the declared tests.\n"),
                *Challenge.Id, Challenge.TestCases.Num(), Challenge.TestCases.Num() == 1 ? TEXT("") : TEXT("s"));

            Result.Score = FMath::RoundToInt(100.0f * Result.PassedChecks.Num()
                / FMath::Max(1, Result.PassedChecks.Num() + Result.FailedChecks.Num()));
            Result.bSuccess = Result.FailedChecks.Num() == 0 && Result.PassedChecks.Num() > 0;
            Result.Summary = Result.bSuccess
                ? TEXT("Structural checks passed (in-engine). External validation would execute the declared curriculum tests.")
                : TEXT("Structural checks failed. Match the starter's function name and signature, and return a value.");
            ApplyDeclarativeTestCaseCounts(Result, Challenge);
            return Result;
        }
    }

    // Common: must define *some* function for the expected name.
    auto MentionsFunction = [&Normalized](const TCHAR* Name)
    {
        return Normalized.Contains(Name);
    };

    // Heuristics by language. The concrete check is "does the body contain
    // the right operator pattern in the right shape?"
    if (bSumChallenge)
    {
        const bool bMentions = MentionsFunction(TEXT("totalPower")) || MentionsFunction(TEXT("total_power"));
        AddCheck(Result, TEXT("Defines a function named totalPower/total_power"), bMentions);

        // Must add at least three operands. Accepts a + b + c in one
        // expression, (a+b)+c, step-wise accumulation (t = a + b; t += c;),
        // and builtin aggregation (sum(...)). 2026-07-11: two separate '+'
        // operations are exactly as correct as one three-way expression.
        FRegexPattern SumPattern(TEXT("[A-Za-z_][A-Za-z0-9_]*\\s*\\+\\s*[A-Za-z_][A-Za-z0-9_]*\\s*\\+\\s*[A-Za-z_][A-Za-z0-9_]*"));
        FRegexMatcher SumMatcher(SumPattern, Normalized);
        FString CompactNoIncrement = Compact;
        CompactNoIncrement.ReplaceInline(TEXT("++"), TEXT(""));
        int32 PlusCount = 0;
        for (const TCHAR C : CompactNoIncrement)
        {
            PlusCount += (C == TCHAR('+')) ? 1 : 0;
        }
        const bool bHasThreeWayAdd = SumMatcher.FindNext()
            || Compact.Contains(TEXT("sum("))
            || PlusCount >= 2;
        AddCheck(Result, TEXT("Body adds three named operands together (a + b + c)"), bHasThreeWayAdd);

        AddCheck(Result, TEXT("Returns / outputs the computed value"), ProducesOutput());
    }
    else if (bLockChallenge)
    {
        const bool bMentions = MentionsFunction(TEXT("shouldUnlock")) || MentionsFunction(TEXT("should_unlock"));
        AddCheck(Result, TEXT("Defines a function named shouldUnlock/should_unlock"), bMentions);

        // Look for a conjunction of the two conditions. Accept C-family &&,
        // Python 'and', MATLAB &/&&, a bitwise & on booleans, NESTED IFs
        // (if (a) { if (b) return true; }), and a ternary (a ? b : false) —
        // all are functionally correct conjunctions. 2026-07-11 generalized.
        FRegexPattern AndPattern(TEXT("[A-Za-z_][A-Za-z0-9_]*\\s*(&&|\\band\\b|&)\\s*[A-Za-z_][A-Za-z0-9_]*"));
        FRegexMatcher AndMatcher(AndPattern, Normalized);
        int32 IfCount = 0;
        for (int32 SearchIndex = Compact.Find(TEXT("if"), ESearchCase::CaseSensitive, ESearchDir::FromStart, 0);
             SearchIndex != INDEX_NONE;
             SearchIndex = Compact.Find(TEXT("if"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchIndex + 2))
        {
            ++IfCount;
        }
        const bool bTernaryConjunction = Compact.Contains(TEXT("?")) && Compact.Contains(TEXT(":"));
        const bool bHasAnd = AndMatcher.FindNext()
            || Normalized.Contains(TEXT("&&"))
            || NormalizedLower.Contains(TEXT(" and "))
            || IfCount >= 2
            || bTernaryConjunction;
        AddCheck(Result, TEXT("Body uses boolean-AND between the two parameters"), bHasAnd);

        // Anti-pattern guard: reject obvious incorrect "always true" returns —
        // but only when there is NO conditional logic at all (a `return true`
        // nested inside an if-chain is a correct acceptance path).
        const bool bHasConditionalLogic = bHasAnd || IfCount >= 1 || Compact.Contains(TEXT("?"));
        const bool bAlwaysTrue = Compact.Contains(TEXT("returntrue")) && !bHasConditionalLogic;
        AddCheck(Result, TEXT("Does not unconditionally return true"), !bAlwaysTrue);
    }
    else if (bReverseChallenge)
    {
        // String/array reversal. Accepts either a built-in reverse
        // (Python `[::-1]`, Java `StringBuilder.reverse`, MATLAB `flip`,
        // C `for(i = n-1; ...)`) or a hand-written loop that decrements.
        const bool bMentions = MentionsFunction(TEXT("reverseString"))
                            || MentionsFunction(TEXT("reverse_string"))
                            || MentionsFunction(TEXT("reverse"));
        AddCheck(Result, TEXT("Defines a function named reverseString/reverse_string/reverse"), bMentions);

        const bool bUsesBuiltIn = Normalized.Contains(TEXT("[::-1]"))           // Python slice
                               || Normalized.Contains(TEXT(".reverse"))         // Java/JS
                               || Normalized.Contains(TEXT("reverse("))         // STL/Python
                               || Compact.Contains(TEXT("reversed("))           // Python builtin
                               || Normalized.Contains(TEXT("flip("))            // MATLAB
                               || Normalized.Contains(TEXT("fliplr("))          // MATLAB row-vector/string reverse
                               || Normalized.Contains(TEXT("flipud(")) ;        // MATLAB
        // Hand-written: any backwards-decrement loop pattern. Prefix decrement
        // (`--i`) is just as valid as postfix decrement (`i--`). The packaged
        // fallback previously recognized only the postfix form and rejected a
        // correct reverseString implementation captured in player telemetry.
        FRegexPattern DecPattern(TEXT("(--\\s*[A-Za-z_][A-Za-z0-9_]*|[A-Za-z_][A-Za-z0-9_]*\\s*(--|-=\\s*1))"));
        FRegexMatcher DecMatcher(DecPattern, Normalized);
        const bool bUsesLoop = DecMatcher.FindNext();
        const bool bUsesOutputBufferMirror = Normalized.Contains(TEXT("output["))
                                          && Normalized.Contains(TEXT("input["))
                                          && (Normalized.Contains(TEXT("- 1 -"))
                                           || Normalized.Contains(TEXT("-1-"))
                                           || Normalized.Contains(TEXT("length - i - 1"))
                                           || Normalized.Contains(TEXT("length-i-1")));
        // 2026-07-11: prepend accumulation (out = c + out) walks forward and
        // is exactly as correct as a backward loop.
        FRegexPattern PrependPattern(TEXT("=[a-z0-9_\\[\\]\\(\\)\\.]+\\+(out|result|rev|acc|output|res)"));
        FRegexMatcher PrependMatcher(PrependPattern, Compact);
        const bool bPrependAccumulation = PrependMatcher.FindNext();
        AddCheck(Result, TEXT("Reverses via a built-in, mirrored output buffer, or backward-iterating loop"),
            bUsesBuiltIn || bUsesLoop || bUsesOutputBufferMirror || bPrependAccumulation);

        const bool bHasReturn = ProducesOutput()
                             || (Normalized.Contains(TEXT("output[")) && Normalized.Contains(TEXT("'\\0'")));
        AddCheck(Result, TEXT("Returns / outputs the reversed value"), bHasReturn);
    }
    else if (bPalindromeChallenge)
    {
        // Compare original to reversed. Accepts an explicit reverse-then-equal
        // pattern, or a two-pointer i/j approach.
        const bool bMentions = MentionsFunction(TEXT("isPalindrome"))
                            || MentionsFunction(TEXT("is_palindrome"))
                            || MentionsFunction(TEXT("palindrome"));
        AddCheck(Result, TEXT("Defines a function named isPalindrome/is_palindrome"), bMentions);

        // Character comparison — 2026-07-11 generalized after the packaged
        // build rejected a CORRECT prefix-decrement != two-pointer solution:
        //   * ANY equality OR inequality test counts (== / != / ~= / .equals /
        //     .compare / strcmp / isequal). A mismatch early-return (!=) is
        //     functionally identical to an equality acceptance loop.
        //   * A two-pointer walk counts with ANY increment/decrement spelling
        //     (++i / i++ / += 1 anywhere, paired with --j / j-- / -= 1
        //     anywhere; the old regex demanded postfix on one line).
        //   * reverse-then-compare (s == reverse(s)) counts via its builtin.
        const bool bEquality = Normalized.Contains(TEXT("=="))
                            || Normalized.Contains(TEXT("!="))
                            || Normalized.Contains(TEXT("~="))         // MATLAB inequality
                            || Compact.Contains(TEXT(".equals("))
                            || Compact.Contains(TEXT(".equalsignorecase("))
                            || Compact.Contains(TEXT(".compare("))
                            || Compact.Contains(TEXT("strcmp("))
                            || Compact.Contains(TEXT("isequal("));
        const bool bHasIncrement = Compact.Contains(TEXT("++")) || Compact.Contains(TEXT("+=1"));
        const bool bHasDecrement = Compact.Contains(TEXT("--")) || Compact.Contains(TEXT("-=1"));
        const bool bTwoPointer = bHasIncrement && bHasDecrement;
        const bool bReverseCompare = Compact.Contains(TEXT("[::-1]"))
                                  || Compact.Contains(TEXT(".reverse"))
                                  || Compact.Contains(TEXT("reverse("))
                                  || Compact.Contains(TEXT("reversed("))
                                  || Compact.Contains(TEXT("flip("))
                                  || Compact.Contains(TEXT("fliplr("));
        AddCheck(Result, TEXT("Compares characters via equality OR two-pointer indices"),
            bEquality || bTwoPointer || bReverseCompare);

        AddCheck(Result, TEXT("Returns / outputs a boolean"), ProducesOutput());
    }
    else if (bFizzBuzzChallenge)
    {
        // Classic FizzBuzz: needs a modulo, and the literal "Fizz" + "Buzz"
        // strings present in the source somewhere.
        const bool bMentions = MentionsFunction(TEXT("fizzBuzz"))
                            || MentionsFunction(TEXT("fizz_buzz"))
                            || MentionsFunction(TEXT("fizzbuzz"));
        AddCheck(Result, TEXT("Defines a function named fizzBuzz/fizz_buzz"), bMentions);

        const bool bHasModulo = Normalized.Contains(TEXT("%"))
                             || Normalized.Contains(TEXT("mod(")) // MATLAB
                             || Normalized.Contains(TEXT("rem(")) // MATLAB remainder
                             || Normalized.Contains(TEXT("Math.floorMod"));
        AddCheck(Result, TEXT("Uses the modulo / remainder operator"), bHasModulo);

        // 2026-07-11: case-insensitive, quote-style-agnostic literal check
        // ('fizz' single-quoted Python is as valid as "Fizz").
        const bool bHasFizz = Compact.Contains(TEXT("fizz"));
        const bool bHasBuzz = Compact.Contains(TEXT("buzz"));
        AddCheck(Result, TEXT("Mentions both 'Fizz' and 'Buzz' literals"), bHasFizz && bHasBuzz);

        // Anti-pattern: hard-coded "FizzBuzz" without any modulo is a fake.
        const bool bFakeFizzBuzz = !bHasModulo && bHasFizz && bHasBuzz;
        AddCheck(Result, TEXT("Doesn't hardcode the output without computing it"), !bFakeFizzBuzz);
    }
    else if (bFilterChallenge)
    {
        // Filter / even-numbers / pick-by-predicate.
        const bool bMentions = MentionsFunction(TEXT("filter"))
                            || MentionsFunction(TEXT("evens"))
                            || MentionsFunction(TEXT("evenNumbers"))
                            || MentionsFunction(TEXT("even_numbers"));
        AddCheck(Result, TEXT("Defines a filter/evens function"), bMentions);

        // 2026-07-11: bitwise even-tests (x & 1) and MATLAB bitand/rem are
        // functionally correct predicates too.
        const bool bHasModulo = Normalized.Contains(TEXT("%"))
                             || Normalized.Contains(TEXT("mod("))  // MATLAB
                             || Normalized.Contains(TEXT("rem("))  // MATLAB
                             || Compact.Contains(TEXT("&1"))       // bitwise even test
                             || Compact.Contains(TEXT("bitand("));
        AddCheck(Result, TEXT("Uses modulo to test evenness / a predicate"), bHasModulo);

        // Some form of iteration: for/while loop (with or without a space
        // before '('), list comprehension, .filter()/.stream()/.Where(), or
        // vectorized logical indexing.
        const bool bIterates = Normalized.Contains(TEXT(" for "))
                            || Normalized.Contains(TEXT(" while "))
                            || Compact.Contains(TEXT("for("))
                            || Compact.Contains(TEXT("while("))
                            || Compact.Contains(TEXT("foreach"))
                            || Compact.Contains(TEXT(".filter("))
                            || Compact.Contains(TEXT(".stream("))
                            || Compact.Contains(TEXT(".where("))
                            || Normalized.Contains(TEXT("[ x for")) // py listcomp lazy
                            || Normalized.Contains(TEXT("[x for"))
                            || Compact.Contains(TEXT("forxin"))     // compacted py listcomp, any spacing
                            || (bHasModulo && Normalized.Contains(TEXT("values("))) // MATLAB logical indexing
                            || (bHasModulo && Normalized.Contains(TEXT("input(")));
        AddCheck(Result, TEXT("Iterates over the input collection or uses vectorized logical indexing"), bIterates);
    }
    else if (bLinkedListChallenge)
    {
        // Linked-list traversal: must walk a `node = node.next` (or `node->next`,
        // or `current.next`) pattern and accumulate / count / inspect along the way.
        const bool bMentions = MentionsFunction(TEXT("traverse"))
                            || MentionsFunction(TEXT("traverseList"))
                            || MentionsFunction(TEXT("traverse_list"))
                            || MentionsFunction(TEXT("countNodes"))
                            || MentionsFunction(TEXT("count_nodes"))
                            || MentionsFunction(TEXT("walk"));
        AddCheck(Result, TEXT("Defines a function named traverse/traverseList/countNodes/walk"), bMentions);

        // Look for the canonical "advance to next" pattern. Accepts object
        // links (.next / ->next) and the index-array version used by the
        // shipped starters (`current = next[current]` / next_indices).
        const bool bAdvances = Normalized.Contains(TEXT(".next"))
                            || Normalized.Contains(TEXT("->next"))
                            || Normalized.Contains(TEXT(".Next"))
                            || Normalized.Contains(TEXT("next["))
                            || Normalized.Contains(TEXT("next_indices"))
                            || Normalized.Contains(TEXT("next("));
        AddCheck(Result, TEXT("Advances along next links or next-index arrays"), bAdvances);

        // Loop construct: while/for/recursion all acceptable. 2026-07-11:
        // recursion is detected as the traversal function calling itself
        // (its name appearing at least twice).
        auto CountOccurrences = [&Compact](const TCHAR* Token)
        {
            int32 Count = 0;
            const FString TokenString(Token);
            for (int32 SearchIndex = Compact.Find(TokenString, ESearchCase::CaseSensitive, ESearchDir::FromStart, 0);
                 SearchIndex != INDEX_NONE;
                 SearchIndex = Compact.Find(TokenString, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchIndex + TokenString.Len()))
            {
                ++Count;
            }
            return Count;
        };
        const bool bRecursive = CountOccurrences(TEXT("traverse")) >= 2
                             || CountOccurrences(TEXT("countnodes")) >= 2
                             || CountOccurrences(TEXT("count_nodes")) >= 2
                             || CountOccurrences(TEXT("walk")) >= 2;
        const bool bLoops = Normalized.Contains(TEXT(" while "))
                         || Normalized.Contains(TEXT(" for "))
                         || Normalized.Contains(TEXT("while("))
                         || Normalized.Contains(TEXT("for("))
                         || bRecursive;
        AddCheck(Result, TEXT("Iterates with a while/for loop"), bLoops);

        // Anti-pattern: a constant return that doesn't actually walk the list.
        const bool bConstantReturn = Normalized.Contains(TEXT("return 0")) && !bAdvances;
        AddCheck(Result, TEXT("Doesn't return a constant without traversing"), !bConstantReturn);
    }
    else if (bBinarySearchChallenge)
    {
        // Binary search: must use lo/hi (or low/high, left/right) and a midpoint
        // calculation, plus a comparison that decides which half to keep.
        const bool bMentions = MentionsFunction(TEXT("binarySearch"))
                            || MentionsFunction(TEXT("binary_search"))
                            || MentionsFunction(TEXT("bsearch"));
        AddCheck(Result, TEXT("Defines a function named binarySearch/binary_search"), bMentions);

        const bool bHasLoHi = (Normalized.Contains(TEXT("lo")) && Normalized.Contains(TEXT("hi")))
                           || (Normalized.Contains(TEXT("low")) && Normalized.Contains(TEXT("high")))
                           || (Normalized.Contains(TEXT("left")) && Normalized.Contains(TEXT("right")))
                           || (Normalized.Contains(TEXT("start")) && Normalized.Contains(TEXT("end")));
        AddCheck(Result, TEXT("Tracks lo/hi (or low/high, left/right) bounds"), bHasLoHi);

        // Midpoint — 2026-07-11 generalized after the packaged build rejected
        // the CORRECT overflow-safe form `low + (high - low) / 2`. Every
        // functional halving family counts: (lo+hi)/2, (lo+hi)>>1, //2 floor
        // division, lo+(hi-lo)/2, len(arr)//2 slice recursion, and the
        // midpoint()/floor()/fix()/idivide helper spellings. On the compact
        // string "/2" also matches "//2" and "(hi-lo)/2".
        const bool bHasMid = Compact.Contains(TEXT("/2"))
                          || Compact.Contains(TEXT(">>1"))
                          || Compact.Contains(TEXT("midpoint"))
                          || Compact.Contains(TEXT("middle"))
                          || Compact.Contains(TEXT("floor("))
                          || Compact.Contains(TEXT("fix("))
                          || Compact.Contains(TEXT("idivide"))
                          || Compact.Contains(TEXT("median"));
        AddCheck(Result, TEXT("Computes a midpoint that halves the range (e.g. (lo+hi)/2 or lo+(hi-lo)/2)"), bHasMid);

        // Comparison + bound update: a comparison AND a re-assignment that
        // shrinks the range. Accepts lo = mid + 1 / hi = mid - 1 with ANY
        // midpoint variable name, the half-open hi = mid form, and slice
        // recursion (arr[mid+1:]).
        const bool bHasComparison = Normalized.Contains(TEXT("<"))
                                 || Normalized.Contains(TEXT(">"))
                                 || Compact.Contains(TEXT("compareto"));
        FRegexPattern BoundPattern(TEXT("(lo|low|left|start|hi|high|right|end|first|last)[a-z0-9_]*=[a-z0-9_\\(\\)\\[\\]\\.]*(mid|middle|center|m)[a-z0-9_]*([+-]1)?"));
        FRegexMatcher BoundMatcher(BoundPattern, Compact);
        const bool bHasBoundUpdate = Compact.Contains(TEXT("mid+1"))
                                   || Compact.Contains(TEXT("mid-1"))
                                   || BoundMatcher.FindNext();
        AddCheck(Result, TEXT("Compares mid against target and updates bounds"), bHasComparison && bHasBoundUpdate);

        // Anti-pattern guard: rejects an O(n) linear scan masquerading as bsearch.
        const bool bIsLinear = !bHasMid && !bHasBoundUpdate && Normalized.Contains(TEXT(" for "));
        AddCheck(Result, TEXT("Is not a linear scan (must halve the search space)"), !bIsLinear);
    }
    else
    {
        // Unknown challenge id — accept any non-empty solution as a best-effort
        // pass so the player can still complete custom missions in fallback mode.
        AddCheck(Result, TEXT("Solution is non-empty"), !UserCode.TrimStartAndEnd().IsEmpty());
    }

    Result.Score = FMath::RoundToInt(100.0f * Result.PassedChecks.Num() / FMath::Max(1, Result.PassedChecks.Num() + Result.FailedChecks.Num()));
    Result.bSuccess = Result.FailedChecks.Num() == 0;
    Result.Summary = Result.bSuccess
        ? TEXT("In-engine validation passed. (External compiler was not available; static-analysis tier was used.)")
        : TEXT("In-engine validation failed. Make sure your function shape matches the brief, then try again.");
    ApplyDeclarativeTestCaseCounts(Result, Challenge);
    return Result;
}

void UCodeRunnerLibrary::AddCheck(FCodeValidationResult& Result, const FString& Label, bool bPassed)
{
    if (bPassed)
    {
        Result.PassedChecks.Add(Label);
    }
    else
    {
        Result.FailedChecks.Add(Label);
    }
}

FString UCodeRunnerLibrary::GetSandboxDir()
{
    const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CodeSandbox"));
    IFileManager::Get().MakeDirectory(*Dir, true);
    return Dir;
}

bool UCodeRunnerLibrary::ExecProcess(const FString& Executable, const FString& Args, FString& OutStdOut, FString& OutStdErr, int32& OutReturnCode)
{
    OutStdOut.Empty();
    OutStdErr.Empty();
    OutReturnCode = -999;

    void* StdOutReadPipe = nullptr;
    void* StdOutWritePipe = nullptr;
    void* StdErrReadPipe = nullptr;
    void* StdErrWritePipe = nullptr;
    if (!FPlatformProcess::CreatePipe(StdOutReadPipe, StdOutWritePipe) ||
        !FPlatformProcess::CreatePipe(StdErrReadPipe, StdErrWritePipe))
    {
        FPlatformProcess::ClosePipe(StdOutReadPipe, StdOutWritePipe);
        FPlatformProcess::ClosePipe(StdErrReadPipe, StdErrWritePipe);
        OutStdErr = TEXT("Could not create process output pipes.");
        return false;
    }

    FProcHandle ProcHandle = FPlatformProcess::CreateProc(
        *Executable,
        *Args,
        true,
        true,
        true,
        nullptr,
        0,
        nullptr,
        StdOutWritePipe,
        nullptr,
        StdErrWritePipe);

    if (!ProcHandle.IsValid())
    {
        FPlatformProcess::ClosePipe(StdOutReadPipe, StdOutWritePipe);
        FPlatformProcess::ClosePipe(StdErrReadPipe, StdErrWritePipe);
        OutStdErr = FString::Printf(TEXT("Could not launch process: %s %s"), *Executable, *Args);
        return false;
    }

    const double StartTime = FPlatformTime::Seconds();
    bool bTimedOut = false;
    while (FPlatformProcess::IsProcRunning(ProcHandle))
    {
        OutStdOut += FPlatformProcess::ReadPipe(StdOutReadPipe);
        OutStdErr += FPlatformProcess::ReadPipe(StdErrReadPipe);
        if ((FPlatformTime::Seconds() - StartTime) > GCodeValidationProcessTimeoutSeconds)
        {
            bTimedOut = true;
            FPlatformProcess::TerminateProc(ProcHandle, true);
            break;
        }
        FPlatformProcess::Sleep(0.02f);
    }

    FPlatformProcess::WaitForProc(ProcHandle);
    for (;;)
    {
        const FString Chunk = FPlatformProcess::ReadPipe(StdOutReadPipe);
        if (Chunk.IsEmpty())
        {
            break;
        }
        OutStdOut += Chunk;
    }
    for (;;)
    {
        const FString Chunk = FPlatformProcess::ReadPipe(StdErrReadPipe);
        if (Chunk.IsEmpty())
        {
            break;
        }
        OutStdErr += Chunk;
    }

    if (bTimedOut)
    {
        OutReturnCode = -408;
        OutStdErr += FString::Printf(
            TEXT("\nCode validation timed out after %.0f seconds. Check for infinite loops or blocking input."),
            GCodeValidationProcessTimeoutSeconds);
    }
    else if (!FPlatformProcess::GetProcReturnCode(ProcHandle, &OutReturnCode))
    {
        OutReturnCode = -998;
        OutStdErr += TEXT("\nCode validation finished without an exit code.");
    }

    FPlatformProcess::CloseProc(ProcHandle);
    FPlatformProcess::ClosePipe(StdOutReadPipe, StdOutWritePipe);
    FPlatformProcess::ClosePipe(StdErrReadPipe, StdErrWritePipe);
    return true;
}

FCodeValidationResult UCodeRunnerLibrary::ValidateChallenge(const FChallengeSpec& Challenge, const FString& UserCode)
{
    if (!AreExternalValidatorsAllowed())
    {
        FCodeValidationResult Result = ValidateInEngine(Challenge, UserCode);
        Result.StdOut = ExternalValidationDisabledMessage() + TEXT("\n") + Result.StdOut;
        return Result;
    }

    const FString SandboxDir = GetSandboxDir();

    switch (Challenge.Language)
    {
    case ECodingLanguage::Java: return ValidateJava(Challenge, UserCode, SandboxDir);
    case ECodingLanguage::C: return ValidateC(Challenge, UserCode, SandboxDir);
    case ECodingLanguage::CPlus:
    case ECodingLanguage::Cpp: return ValidateCpp(Challenge, UserCode, SandboxDir);
    case ECodingLanguage::Python: return ValidatePython(Challenge, UserCode, SandboxDir);
    case ECodingLanguage::MATLAB: return ValidateMATLAB(Challenge, UserCode, SandboxDir);
    default:
        FCodeValidationResult Result;
        Result.Summary = TEXT("Unsupported language.");
        Result.bSuccess = false;
        return Result;
    }
}

FCodeValidationResult UCodeRunnerLibrary::ValidateJava(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir)
{
    if (!IsLanguageAvailable(ECodingLanguage::Java))
    {
        return ValidateInEngine(Challenge, UserCode);
    }

    FCodeValidationResult Result;
    const FString ClassName = TEXT("MissionSolution");
    const FString SourceFile = FPaths::Combine(SandboxDir, ClassName + TEXT(".java"));
    const FString HarnessFile = FPaths::Combine(SandboxDir, TEXT("MissionHarness.java"));

    FString Solution = UserCode;
    if (!Solution.Contains(TEXT("class MissionSolution")))
    {
        Solution = FString::Printf(TEXT("public class MissionSolution {\n%s\n}\n"), *UserCode);
    }

    FString Harness;
    int32 SumA = 0, SumB = 0, SumC = 0;
    HiddenSumCase(Challenge, SumA, SumB, SumC);
    const FString ReverseWord = HiddenReverseWord(Challenge);
    const FString ReverseExpected = ReverseStringLiteral(ReverseWord);
    const FString PalWord = HiddenPalindromeWord(Challenge);
    const FString NonPalWord = HiddenNonPalindromeWord(Challenge);
    const int32 FizzN = HiddenFizzBuzzN(Challenge);
    const FString FizzExpected = BuildFizzBuzzCsv(FizzN);
    const TArray<int32> EvenInput = HiddenEvenInput(Challenge);
    const TArray<int32> EvenExpected = FilterEvenValues(EvenInput);
    const TArray<int32> LinkedNext = HiddenLinkedNext(Challenge);
    const int32 LinkedStart = static_cast<int32>(ChallengeSeed(Challenge) % LinkedNext.Num());
    const int32 LinkedExpected = CountLinkedNodes(LinkedNext, LinkedStart);
    const TArray<int32> SearchValues = HiddenSearchValues(Challenge);
    const int32 SearchTarget = SearchValues[SearchValues.Num() / 2];
    const ECodeChallengeKind Kind = GetChallengeKind(Challenge);
    if (CanUseDeclarativeTests(Challenge, Kind))
    {
        Harness = BuildJavaDeclarativeHarness(Challenge, Kind);
    }
    else switch (Kind)
    {
    case ECodeChallengeKind::Sum:
        Harness = FString::Printf(TEXT("public class MissionHarness { public static void main(String[] args) { if (MissionSolution.totalPower(20,15,10) != 45) throw new RuntimeException(\"visible totalPower failed\"); if (MissionSolution.totalPower(%d,%d,%d) != %d) throw new RuntimeException(\"city hidden sum failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }"), SumA, SumB, SumC, SumA + SumB + SumC);
        break;
    case ECodeChallengeKind::Lock:
        Harness = TEXT("public class MissionHarness { public static void main(String[] args) { if (!MissionSolution.shouldUnlock(true,true)) throw new RuntimeException(\"visible true,true failed\"); if (MissionSolution.shouldUnlock(true,false)) throw new RuntimeException(\"hidden true,false failed\"); if (MissionSolution.shouldUnlock(false,true)) throw new RuntimeException(\"hidden false,true failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }");
        break;
    case ECodeChallengeKind::Reverse:
        Harness = FString::Printf(TEXT("public class MissionHarness { public static void main(String[] args) { if (!\"eucser\".equals(MissionSolution.reverseString(\"rescue\"))) throw new RuntimeException(\"visible reverse failed\"); if (!\"%s\".equals(MissionSolution.reverseString(\"%s\"))) throw new RuntimeException(\"city hidden reverse failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }"), *ReverseExpected, *ReverseWord);
        break;
    case ECodeChallengeKind::Palindrome:
        Harness = FString::Printf(TEXT("public class MissionHarness { public static void main(String[] args) { if (!MissionSolution.isPalindrome(\"racecar\")) throw new RuntimeException(\"visible palindrome failed\"); if (!MissionSolution.isPalindrome(\"%s\")) throw new RuntimeException(\"city hidden palindrome failed\"); if (MissionSolution.isPalindrome(\"%s\")) throw new RuntimeException(\"hidden non-palindrome failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }"), *PalWord, *NonPalWord);
        break;
    case ECodeChallengeKind::FizzBuzz:
        Harness = FString::Printf(TEXT("public class MissionHarness { public static void main(String[] args) { String expected = \"1,2,Fizz,4,Buzz,Fizz,7,8,Fizz,Buzz,11,Fizz,13,14,FizzBuzz\"; String actual = String.join(\",\", MissionSolution.fizzBuzz(15)); if (!expected.equals(actual)) throw new RuntimeException(\"fizzBuzz output failed\"); String hiddenExpected = \"%s\"; String hiddenActual = String.join(\",\", MissionSolution.fizzBuzz(%d)); if (!hiddenExpected.equals(hiddenActual)) throw new RuntimeException(\"city hidden fizzBuzz failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }"), *FizzExpected, FizzN);
        break;
    case ECodeChallengeKind::EvenFilter:
        Harness = FString::Printf(TEXT("import java.util.Arrays; public class MissionHarness { public static void main(String[] args) { if (!Arrays.equals(MissionSolution.evenNumbers(new int[]{1,2,3,4,5,6}), new int[]{2,4,6})) throw new RuntimeException(\"visible evenNumbers failed\"); if (!Arrays.equals(MissionSolution.evenNumbers(%s), %s)) throw new RuntimeException(\"city hidden evenNumbers failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }"), *JavaArrayLiteral(EvenInput), *JavaArrayLiteral(EvenExpected));
        break;
    case ECodeChallengeKind::LinkedListTraverse:
        Harness = FString::Printf(TEXT("public class MissionHarness { public static void main(String[] args) { if (MissionSolution.countNodes(new int[]{1,2,-1}, 0) != 3) throw new RuntimeException(\"visible linked chain failed\"); if (MissionSolution.countNodes(%s, %d) != %d) throw new RuntimeException(\"city hidden linked chain failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }"), *JavaArrayLiteral(LinkedNext), LinkedStart, LinkedExpected);
        break;
    case ECodeChallengeKind::BinarySearch:
        Harness = FString::Printf(TEXT("public class MissionHarness { public static void main(String[] args) { if (MissionSolution.binarySearch(new int[]{2,4,6,8,10}, 6) != 2) throw new RuntimeException(\"visible binary search failed\"); if (MissionSolution.binarySearch(%s, %d) != %d) throw new RuntimeException(\"city hidden binary search failed\"); if (MissionSolution.binarySearch(%s, 9999) != -1) throw new RuntimeException(\"not-found binary search failed\"); System.out.println(\"ALL_TESTS_PASSED\"); } }"), *JavaArrayLiteral(SearchValues), SearchTarget, SearchValues.Num() / 2, *JavaArrayLiteral(SearchValues));
        break;
    default:
        Harness = TEXT("public class MissionHarness { public static void main(String[] args) { MissionSolution.main(new String[]{}); System.out.println(\"ALL_TESTS_PASSED\"); } }");
        break;
    }

    FFileHelper::SaveStringToFile(Solution, *SourceFile);
    FFileHelper::SaveStringToFile(Harness, *HarnessFile);

    FString StdOut, StdErr;
    int32 Code = -1;
    ExecProcess(TEXT("/usr/bin/env"), FString::Printf(TEXT("javac %s %s"), *QuoteArg(SourceFile), *QuoteArg(HarnessFile)), StdOut, StdErr, Code);
    Result.StdOut += StdOut;
    Result.StdErr += StdErr;
    AddCheck(Result, TEXT("Java source compiles with javac"), Code == 0);

    if (Code == 0)
    {
        ExecProcess(TEXT("/usr/bin/env"), FString::Printf(TEXT("java -cp %s MissionHarness"), *QuoteArg(SandboxDir)), StdOut, StdErr, Code);
        Result.StdOut += StdOut;
        Result.StdErr += StdErr;
        AddCheck(Result, TEXT("Java harness executes without runtime failure"), Code == 0);
        AddCheck(Result, TEXT("Visible and hidden Java tests pass"), StdOut.Contains(TEXT("ALL_TESTS_PASSED")) && Code == 0);
    }
    else
    {
        AddCheck(Result, TEXT("Java harness executes without runtime failure"), false);
        AddCheck(Result, TEXT("Visible and hidden Java tests pass"), false);
    }

    Result.Score = FMath::RoundToInt(100.0f * Result.PassedChecks.Num() / FMath::Max(1, Result.PassedChecks.Num() + Result.FailedChecks.Num()));
    Result.bSuccess = Result.FailedChecks.Num() == 0;
    Result.Summary = Result.bSuccess ? TEXT("Java rescue terminal validated successfully.") : TEXT("Java validation failed. Review compiler output and failed checks.");
    ApplyDeclarativeTestCaseCounts(Result, Challenge);
    return Result;
}

FCodeValidationResult UCodeRunnerLibrary::ValidateC(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir)
{
    if (!IsLanguageAvailable(ECodingLanguage::C))
    {
        return ValidateInEngine(Challenge, UserCode);
    }

    FCodeValidationResult Result;
    const ECodeChallengeKind Kind = GetChallengeKind(Challenge);
    const FString SourceFile = FPaths::Combine(SandboxDir, TEXT("mission_solution.c"));
    const FString BinaryFile = FPaths::Combine(SandboxDir, TEXT("mission_solution"));
    int32 SumA = 0, SumB = 0, SumC = 0;
    HiddenSumCase(Challenge, SumA, SumB, SumC);
    const FString ReverseWord = HiddenReverseWord(Challenge);
    const FString ReverseExpected = ReverseStringLiteral(ReverseWord);
    const FString PalWord = HiddenPalindromeWord(Challenge);
    const FString NonPalWord = HiddenNonPalindromeWord(Challenge);
    const int32 FizzN = HiddenFizzBuzzN(Challenge);
    const FString FizzExpected = BuildFizzBuzzCsv(FizzN);
    const TArray<int32> EvenInput = HiddenEvenInput(Challenge);
    const TArray<int32> EvenExpected = FilterEvenValues(EvenInput);
    const TArray<int32> LinkedNext = HiddenLinkedNext(Challenge);
    const int32 LinkedStart = static_cast<int32>(ChallengeSeed(Challenge) % LinkedNext.Num());
    const int32 LinkedExpected = CountLinkedNodes(LinkedNext, LinkedStart);
    const TArray<int32> SearchValues = HiddenSearchValues(Challenge);
    const int32 SearchTarget = SearchValues[SearchValues.Num() / 2];
    const FString Sentinel = BuildValidationSentinel(Challenge, TEXT("C"));

    FString Source = TEXT("#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <stdbool.h>\n")
                     TEXT("#define main user_main\n") +
                     UserCode +
                     CValidationMacroUndefs();

    if (CanUseDeclarativeTests(Challenge, Kind))
    {
        Source += BuildCDeclarativeMain(Challenge, Kind, Sentinel);
    }
    else switch (Kind)
    {
    case ECodeChallengeKind::Lock:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    int ok = 1;\n")
                  TEXT("    if (!shouldUnlock(1, 1)) ok = 0;\n")
                  TEXT("    if ( shouldUnlock(1, 0)) ok = 0;\n")
                  TEXT("    if ( shouldUnlock(0, 1)) ok = 0;\n")
                  TEXT("    if ( shouldUnlock(0, 0)) ok = 0;\n")
                  TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), *Sentinel);
        break;
    case ECodeChallengeKind::Reverse:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    int ok = 1;\n")
                  TEXT("    char out[128];\n")
                  TEXT("    reverseString(\"rescue\", out);\n")
                  TEXT("    if (strcmp(out, \"eucser\") != 0) ok = 0;\n")
                  TEXT("    reverseString(\"%s\", out);\n")
                  TEXT("    if (strcmp(out, \"%s\") != 0) ok = 0;\n")
                  TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), *ReverseWord, *ReverseExpected, *Sentinel);
        break;
    case ECodeChallengeKind::Palindrome:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    int ok = 1;\n")
                  TEXT("    if (!isPalindrome(\"racecar\")) ok = 0;\n")
                  TEXT("    if (!isPalindrome(\"%s\")) ok = 0;\n")
                  TEXT("    if ( isPalindrome(\"%s\")) ok = 0;\n")
                  TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), *PalWord, *NonPalWord, *Sentinel);
        break;
    case ECodeChallengeKind::FizzBuzz:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    char out[256];\n")
                  TEXT("    fizzBuzz(15, out, (int)sizeof(out));\n")
                  TEXT("    if (strcmp(out, \"1,2,Fizz,4,Buzz,Fizz,7,8,Fizz,Buzz,11,Fizz,13,14,FizzBuzz\") != 0) {\n")
                  TEXT("        printf(\"FAIL\\n\");\n")
                  TEXT("        return 0;\n")
                  TEXT("    }\n")
                  TEXT("    fizzBuzz(%d, out, (int)sizeof(out));\n")
                  TEXT("    if (strcmp(out, \"%s\") != 0) {\n")
                  TEXT("        printf(\"FAIL\\n\");\n")
                  TEXT("        return 0;\n")
                  TEXT("    }\n")
                  TEXT("    printf(\"%s\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), FizzN, *FizzExpected, *Sentinel);
        break;
    case ECodeChallengeKind::EvenFilter:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    int input1[] = {1,2,3,4,5,6};\n")
                  TEXT("    int expected1[] = {2,4,6};\n")
                  TEXT("    int out1[6] = {0};\n")
                  TEXT("    int count1 = evenNumbers(input1, 6, out1);\n")
                  TEXT("    int input2[] = %s;\n")
                  TEXT("    int expected2[] = %s;\n")
                  TEXT("    int out2[%d] = {0};\n")
                  TEXT("    int count2 = evenNumbers(input2, %d, out2);\n")
                  TEXT("    int ok = (count1 == 3 && memcmp(out1, expected1, sizeof(expected1)) == 0 && count2 == %d && memcmp(out2, expected2, sizeof(expected2)) == 0);\n")
                  TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), *CArrayLiteral(EvenInput), *CArrayLiteral(EvenExpected), FMath::Max(1, EvenExpected.Num()), EvenInput.Num(), EvenExpected.Num(), *Sentinel);
        break;
    case ECodeChallengeKind::LinkedListTraverse:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    int visible[] = {1,2,-1};\n")
                  TEXT("    int hidden[] = %s;\n")
                  TEXT("    int ok = 1;\n")
                  TEXT("    if (countNodes(visible, 3, 0) != 3) ok = 0;\n")
                  TEXT("    if (countNodes(hidden, %d, %d) != %d) ok = 0;\n")
                  TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), *CArrayLiteral(LinkedNext), LinkedNext.Num(), LinkedStart, LinkedExpected, *Sentinel);
        break;
    case ECodeChallengeKind::BinarySearch:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    int visible[] = {2,4,6,8,10};\n")
                  TEXT("    int hidden[] = %s;\n")
                  TEXT("    int ok = 1;\n")
                  TEXT("    if (binarySearch(visible, 5, 6) != 2) ok = 0;\n")
                  TEXT("    if (binarySearch(hidden, %d, %d) != %d) ok = 0;\n")
                  TEXT("    if (binarySearch(hidden, %d, 9999) != -1) ok = 0;\n")
                  TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), *CArrayLiteral(SearchValues), SearchValues.Num(), SearchTarget, SearchValues.Num() / 2, SearchValues.Num(), *Sentinel);
        break;
    case ECodeChallengeKind::Sum:
        Source += FString::Printf(TEXT("\nint main(void) {\n")
                  TEXT("    int ok = 1;\n")
                  TEXT("    if (totalPower(20,15,10) != 45) ok = 0;\n")
                  TEXT("    if (totalPower(%d,%d,%d) != %d) ok = 0;\n")
                  TEXT("    printf(ok ? \"%s\\n\" : \"FAIL\\n\");\n")
                  TEXT("    return 0;\n")
                  TEXT("}\n"), SumA, SumB, SumC, SumA + SumB + SumC, *Sentinel);
        break;
    default:
        Source += FString::Printf(TEXT("\nint main(void) { printf(\"%s\\n\"); return 0; }\n"), *Sentinel);
        break;
    }

    FFileHelper::SaveStringToFile(Source, *SourceFile);

    FString StdOut, StdErr;
    int32 Code = -1;
    ExecProcess(TEXT("/usr/bin/env"), FString::Printf(TEXT("clang -Wall -Wextra -std=c17 %s -o %s"), *QuoteArg(SourceFile), *QuoteArg(BinaryFile)), StdOut, StdErr, Code);
    Result.StdOut += StdOut;
    Result.StdErr += StdErr;
    AddCheck(Result, TEXT("C source compiles with clang"), Code == 0);

    if (Code == 0)
    {
        ExecProcess(BinaryFile, TEXT(""), StdOut, StdErr, Code);
        Result.StdOut += StdOut;
        Result.StdErr += StdErr;
        AddCheck(Result, TEXT("C executable runs"), Code == 0);
        AddCheck(Result, TEXT("Visible and hidden C tests pass"), StdOut.Contains(Sentinel) && Code == 0);
    }
    else
    {
        AddCheck(Result, TEXT("C executable runs"), false);
        AddCheck(Result, TEXT("Visible and hidden C tests pass"), false);
    }

    Result.Score = FMath::RoundToInt(100.0f * Result.PassedChecks.Num() / FMath::Max(1, Result.PassedChecks.Num() + Result.FailedChecks.Num()));
    Result.bSuccess = Result.FailedChecks.Num() == 0;
    Result.Summary = Result.bSuccess ? TEXT("C rescue terminal validated successfully.") : TEXT("C validation failed. Review compiler output and failed checks.");
    ApplyDeclarativeTestCaseCounts(Result, Challenge);
    return Result;
}

FCodeValidationResult UCodeRunnerLibrary::ValidateCpp(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir)
{
    if (!IsLanguageAvailable(Challenge.Language))
    {
        return ValidateInEngine(Challenge, UserCode);
    }

    FCodeValidationResult Result;
    const ECodeChallengeKind Kind = GetChallengeKind(Challenge);
    const FString SourceFile = FPaths::Combine(SandboxDir, TEXT("mission_solution.cpp"));
    const FString BinaryFile = FPaths::Combine(SandboxDir, TEXT("mission_solution_cpp"));
    int32 SumA = 0, SumB = 0, SumC = 0;
    HiddenSumCase(Challenge, SumA, SumB, SumC);
    const FString ReverseWord = HiddenReverseWord(Challenge);
    const FString ReverseExpected = ReverseStringLiteral(ReverseWord);
    const FString PalWord = HiddenPalindromeWord(Challenge);
    const FString NonPalWord = HiddenNonPalindromeWord(Challenge);
    const int32 FizzN = HiddenFizzBuzzN(Challenge);
    const FString FizzExpected = BuildFizzBuzzCsv(FizzN);
    const TArray<int32> EvenInput = HiddenEvenInput(Challenge);
    const TArray<int32> EvenExpected = FilterEvenValues(EvenInput);
    const TArray<int32> LinkedNext = HiddenLinkedNext(Challenge);
    const int32 LinkedStart = static_cast<int32>(ChallengeSeed(Challenge) % LinkedNext.Num());
    const int32 LinkedExpected = CountLinkedNodes(LinkedNext, LinkedStart);
    const TArray<int32> SearchValues = HiddenSearchValues(Challenge);
    const int32 SearchTarget = SearchValues[SearchValues.Num() / 2];
    const FString Sentinel = BuildValidationSentinel(Challenge, Challenge.Language == ECodingLanguage::CPlus ? TEXT("CPLUS") : TEXT("CPP"));

    FString Source =
        TEXT("#include <algorithm>\n#include <iostream>\n#include <sstream>\n#include <string>\n#include <vector>\nusing namespace std;\n")
        TEXT("#define main user_main\n") +
        UserCode +
        CValidationMacroUndefs() +
        TEXT("static string cr_join(const vector<string>& values) { string out; for (size_t i = 0; i < values.size(); ++i) { if (i) out += \",\"; out += values[i]; } return out; }\n")
        TEXT("static bool cr_equal(const vector<int>& a, const vector<int>& b) { return a == b; }\n");

    if (CanUseDeclarativeTests(Challenge, Kind))
    {
        Source += BuildCppDeclarativeMain(Challenge, Kind, Sentinel);
    }
    else switch (Kind)
    {
    case ECodeChallengeKind::Lock:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && shouldUnlock(true, true); ok = ok && !shouldUnlock(true, false); ok = ok && !shouldUnlock(false, true); cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *Sentinel);
        break;
    case ECodeChallengeKind::Reverse:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && reverseString(\"rescue\") == \"eucser\"; ok = ok && reverseString(\"%s\") == \"%s\"; cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *ReverseWord, *ReverseExpected, *Sentinel);
        break;
    case ECodeChallengeKind::Palindrome:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && isPalindrome(\"racecar\"); ok = ok && isPalindrome(\"%s\"); ok = ok && !isPalindrome(\"%s\"); cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *PalWord, *NonPalWord, *Sentinel);
        break;
    case ECodeChallengeKind::FizzBuzz:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && cr_join(fizzBuzz(15)) == \"1,2,Fizz,4,Buzz,Fizz,7,8,Fizz,Buzz,11,Fizz,13,14,FizzBuzz\"; ok = ok && cr_join(fizzBuzz(%d)) == \"%s\"; cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), FizzN, *FizzExpected, *Sentinel);
        break;
    case ECodeChallengeKind::EvenFilter:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && cr_equal(evenNumbers(vector<int>{1,2,3,4,5,6}), vector<int>{2,4,6}); ok = ok && cr_equal(evenNumbers(vector<int>%s), vector<int>%s); cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *CArrayLiteral(EvenInput), *CArrayLiteral(EvenExpected), *Sentinel);
        break;
    case ECodeChallengeKind::LinkedListTraverse:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && countNodes(vector<int>{1,2,-1}, 0) == 3; ok = ok && countNodes(vector<int>%s, %d) == %d; cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *CArrayLiteral(LinkedNext), LinkedStart, LinkedExpected, *Sentinel);
        break;
    case ECodeChallengeKind::BinarySearch:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && binarySearch(vector<int>{2,4,6,8,10}, 6) == 2; ok = ok && binarySearch(vector<int>%s, %d) == %d; ok = ok && binarySearch(vector<int>%s, 9999) == -1; cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), *CArrayLiteral(SearchValues), SearchTarget, SearchValues.Num() / 2, *CArrayLiteral(SearchValues), *Sentinel);
        break;
    case ECodeChallengeKind::Sum:
        Source += FString::Printf(TEXT("int main() { bool ok = true; ok = ok && totalPower(20,15,10) == 45; ok = ok && totalPower(%d,%d,%d) == %d; cout << (ok ? \"%s\\n\" : \"FAIL\\n\"); return 0; }\n"), SumA, SumB, SumC, SumA + SumB + SumC, *Sentinel);
        break;
    default:
        Source += FString::Printf(TEXT("int main() { cout << \"%s\\n\"; return 0; }\n"), *Sentinel);
        break;
    }

    FFileHelper::SaveStringToFile(Source, *SourceFile);

    FString StdOut, StdErr;
    int32 Code = -1;
    ExecProcess(TEXT("/usr/bin/env"), FString::Printf(TEXT("clang++ -Wall -Wextra -std=c++17 %s -o %s"), *QuoteArg(SourceFile), *QuoteArg(BinaryFile)), StdOut, StdErr, Code);
    Result.StdOut += StdOut;
    Result.StdErr += StdErr;
    AddCheck(Result, TEXT("C++ source compiles with clang++"), Code == 0);

    if (Code == 0)
    {
        ExecProcess(BinaryFile, TEXT(""), StdOut, StdErr, Code);
        Result.StdOut += StdOut;
        Result.StdErr += StdErr;
        AddCheck(Result, TEXT("C++ executable runs"), Code == 0);
        AddCheck(Result, TEXT("Visible and hidden C++ tests pass"), StdOut.Contains(Sentinel) && Code == 0);
    }
    else
    {
        AddCheck(Result, TEXT("C++ executable runs"), false);
        AddCheck(Result, TEXT("Visible and hidden C++ tests pass"), false);
    }

    Result.Score = FMath::RoundToInt(100.0f * Result.PassedChecks.Num() / FMath::Max(1, Result.PassedChecks.Num() + Result.FailedChecks.Num()));
    Result.bSuccess = Result.FailedChecks.Num() == 0;
    Result.Summary = Result.bSuccess
        ? FString::Printf(TEXT("%s rescue terminal validated successfully."), Challenge.Language == ECodingLanguage::CPlus ? TEXT("C+") : TEXT("C++"))
        : FString::Printf(TEXT("%s validation failed. Review compiler output and failed checks."), Challenge.Language == ECodingLanguage::CPlus ? TEXT("C+") : TEXT("C++"));
    ApplyDeclarativeTestCaseCounts(Result, Challenge);
    return Result;
}

FCodeValidationResult UCodeRunnerLibrary::ValidatePython(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir)
{
    if (!IsLanguageAvailable(ECodingLanguage::Python))
    {
        return ValidateInEngine(Challenge, UserCode);
    }

    FCodeValidationResult Result;
    const FString SourceFile = FPaths::Combine(SandboxDir, TEXT("mission_solution.py"));
    const FString HarnessFile = FPaths::Combine(SandboxDir, TEXT("mission_harness.py"));
    int32 SumA = 0, SumB = 0, SumC = 0;
    HiddenSumCase(Challenge, SumA, SumB, SumC);
    const FString ReverseWord = HiddenReverseWord(Challenge);
    const FString ReverseExpected = ReverseStringLiteral(ReverseWord);
    const FString PalWord = HiddenPalindromeWord(Challenge);
    const FString NonPalWord = HiddenNonPalindromeWord(Challenge);
    const int32 FizzN = HiddenFizzBuzzN(Challenge);
    const FString FizzExpected = BuildFizzBuzzCsv(FizzN);
    const TArray<int32> EvenInput = HiddenEvenInput(Challenge);
    const TArray<int32> EvenExpected = FilterEvenValues(EvenInput);
    const TArray<int32> LinkedNext = HiddenLinkedNext(Challenge);
    const int32 LinkedStart = static_cast<int32>(ChallengeSeed(Challenge) % LinkedNext.Num());
    const int32 LinkedExpected = CountLinkedNodes(LinkedNext, LinkedStart);
    const TArray<int32> SearchValues = HiddenSearchValues(Challenge);
    const int32 SearchTarget = SearchValues[SearchValues.Num() / 2];
    const FString Sentinel = BuildValidationSentinel(Challenge, TEXT("PY"));
    const FString SentinelLiteral = PythonStringLiteral(Sentinel);

    FString Harness = FString::Printf(TEXT(
        "import importlib.util\n"
        "import pathlib\n"
        "_solution_path = pathlib.Path(%s)\n"
        "_spec = importlib.util.spec_from_file_location('mission_solution_user', _solution_path)\n"
        "_solution = importlib.util.module_from_spec(_spec)\n"
        "try:\n"
        "    _spec.loader.exec_module(_solution)\n"
        "except SystemExit as exc:\n"
        "    raise AssertionError('solution called sys.exit before validation completed') from exc\n"
        "_ns = vars(_solution)\n\n"),
        *PythonStringLiteral(SourceFile));
    const ECodeChallengeKind Kind = GetChallengeKind(Challenge);
    if (CanUseDeclarativeTests(Challenge, Kind))
    {
        Harness += BuildPythonDeclarativeTestBlock(Challenge, Kind, SentinelLiteral);
    }
    else switch (Kind)
    {
    case ECodeChallengeKind::Sum:
        Harness += FString::Printf(TEXT("_fn = _ns.get('total_power') or _ns.get('totalPower')\nassert _fn is not None\nassert _fn(20,15,10) == 45\nassert _fn(%d,%d,%d) == %d\nprint(%s)\n"), SumA, SumB, SumC, SumA + SumB + SumC, *SentinelLiteral);
        break;
    case ECodeChallengeKind::Lock:
        Harness += FString::Printf(TEXT("_fn = _ns.get('should_unlock') or _ns.get('shouldUnlock')\nassert _fn is not None\nassert _fn(True, True) is True\nassert _fn(True, False) is False\nassert _fn(False, True) is False\nprint(%s)\n"), *SentinelLiteral);
        break;
    case ECodeChallengeKind::Reverse:
        Harness += FString::Printf(TEXT("_fn = _ns.get('reverse_string') or _ns.get('reverseString') or _ns.get('reverse')\nassert _fn is not None\nassert _fn('rescue') == 'eucser'\nassert _fn('%s') == '%s'\nprint(%s)\n"), *ReverseWord, *ReverseExpected, *SentinelLiteral);
        break;
    case ECodeChallengeKind::Palindrome:
        Harness += FString::Printf(TEXT("_fn = _ns.get('is_palindrome') or _ns.get('isPalindrome') or _ns.get('palindrome')\nassert _fn is not None\nassert _fn('racecar') is True\nassert _fn('%s') is True\nassert _fn('%s') is False\nprint(%s)\n"), *PalWord, *NonPalWord, *SentinelLiteral);
        break;
    case ECodeChallengeKind::FizzBuzz:
        Harness += FString::Printf(TEXT("_fn = _ns.get('fizz_buzz') or _ns.get('fizzBuzz') or _ns.get('fizzbuzz')\nassert _fn is not None\nassert [str(x) for x in _fn(15)] == ['1','2','Fizz','4','Buzz','Fizz','7','8','Fizz','Buzz','11','Fizz','13','14','FizzBuzz']\nassert ','.join(str(x) for x in _fn(%d)) == '%s'\nprint(%s)\n"), FizzN, *FizzExpected, *SentinelLiteral);
        break;
    case ECodeChallengeKind::EvenFilter:
        Harness += FString::Printf(TEXT("_fn = _ns.get('even_numbers') or _ns.get('evenNumbers') or _ns.get('evens') or _ns.get('filter_evens')\nassert _fn is not None\nassert list(_fn([1,2,3,4,5,6])) == [2,4,6]\nassert list(_fn(%s)) == %s\nprint(%s)\n"), *PythonListLiteral(EvenInput), *PythonListLiteral(EvenExpected), *SentinelLiteral);
        break;
    case ECodeChallengeKind::LinkedListTraverse:
        Harness += FString::Printf(TEXT("_fn = _ns.get('count_nodes') or _ns.get('countNodes') or _ns.get('traverse_list') or _ns.get('traverseList')\nassert _fn is not None\nassert _fn([1,2,-1], 0) == 3\nassert _fn(%s, %d) == %d\nprint(%s)\n"), *PythonListLiteral(LinkedNext), LinkedStart, LinkedExpected, *SentinelLiteral);
        break;
    case ECodeChallengeKind::BinarySearch:
        Harness += FString::Printf(TEXT("_fn = _ns.get('binary_search') or _ns.get('binarySearch') or _ns.get('bsearch')\nassert _fn is not None\nassert _fn([2,4,6,8,10], 6) == 2\nassert _fn(%s, %d) == %d\nassert _fn(%s, 9999) == -1\nprint(%s)\n"), *PythonListLiteral(SearchValues), SearchTarget, SearchValues.Num() / 2, *PythonListLiteral(SearchValues), *SentinelLiteral);
        break;
    default:
        Harness += FString::Printf(TEXT("print(%s)\n"), *SentinelLiteral);
        break;
    }

    FFileHelper::SaveStringToFile(UserCode, *SourceFile);
    FFileHelper::SaveStringToFile(Harness, *HarnessFile);

    FString StdOut, StdErr;
    int32 Code = -1;
    ExecProcess(TEXT("/usr/bin/env"), FString::Printf(TEXT("python3 %s"), *QuoteArg(HarnessFile)), StdOut, StdErr, Code);
    Result.StdOut = StdOut;
    Result.StdErr = StdErr;
    AddCheck(Result, TEXT("Python script executes"), Code == 0);
    AddCheck(Result, TEXT("Visible and hidden Python tests pass"), StdOut.Contains(Sentinel) && Code == 0);

    Result.Score = FMath::RoundToInt(100.0f * Result.PassedChecks.Num() / FMath::Max(1, Result.PassedChecks.Num() + Result.FailedChecks.Num()));
    Result.bSuccess = Result.FailedChecks.Num() == 0;
    Result.Summary = Result.bSuccess ? TEXT("Python rescue terminal validated successfully.") : TEXT("Python validation failed. Review output and failed checks.");
    ApplyDeclarativeTestCaseCounts(Result, Challenge);
    return Result;
}

FString UCodeRunnerLibrary::FindMATLABExecutable()
{
    const FString EnvBin = FPlatformMisc::GetEnvironmentVariable(TEXT("MATLAB_BIN"));
    if (!EnvBin.IsEmpty() && FPaths::FileExists(EnvBin)) return EnvBin;

    const FString MatlabRoot = FPlatformMisc::GetEnvironmentVariable(TEXT("MATLABROOT"));
    if (!MatlabRoot.IsEmpty())
    {
        const FString Candidate = FPaths::Combine(MatlabRoot, TEXT("bin/matlab"));
        if (FPaths::FileExists(Candidate)) return Candidate;
    }

#if PLATFORM_MAC
    TArray<FString> Apps;
    IFileManager::Get().FindFiles(Apps, TEXT("/Applications/MATLAB_R*.app"), false, true);
    Apps.Sort([](const FString& A, const FString& B) { return A > B; });
    for (const FString& App : Apps)
    {
        const FString Candidate = FPaths::Combine(TEXT("/Applications"), App, TEXT("bin/matlab"));
        if (FPaths::FileExists(Candidate)) return Candidate;
    }
#endif

    return TEXT("matlab");
}

bool UCodeRunnerLibrary::LaunchMATLABDesktop()
{
    if (!AreExternalValidatorsAllowed())
    {
        return false;
    }

    const FString Matlab = FindMATLABExecutable();
    FProcHandle Handle = Matlab == TEXT("matlab")
        ? FPlatformProcess::CreateProc(TEXT("/usr/bin/env"), TEXT("matlab"), true, false, false, nullptr, 0, nullptr, nullptr)
        : FPlatformProcess::CreateProc(*Matlab, TEXT(""), true, false, false, nullptr, 0, nullptr, nullptr);
    return Handle.IsValid();
}

FCodeValidationResult UCodeRunnerLibrary::ValidateMATLAB(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir)
{
    if (!IsLanguageAvailable(ECodingLanguage::MATLAB))
    {
        return ValidateInEngine(Challenge, UserCode);
    }

    FCodeValidationResult Result;
    const ECodeChallengeKind Kind = GetChallengeKind(Challenge);
    int32 SumA = 0, SumB = 0, SumC = 0;
    HiddenSumCase(Challenge, SumA, SumB, SumC);
    const FString ReverseWord = HiddenReverseWord(Challenge);
    const FString ReverseExpected = ReverseStringLiteral(ReverseWord);
    const FString PalWord = HiddenPalindromeWord(Challenge);
    const FString NonPalWord = HiddenNonPalindromeWord(Challenge);
    const int32 FizzN = HiddenFizzBuzzN(Challenge);
    const FString FizzExpected = BuildFizzBuzzCsv(FizzN);
    const TArray<int32> EvenInput = HiddenEvenInput(Challenge);
    const TArray<int32> EvenExpected = FilterEvenValues(EvenInput);
    const TArray<int32> LinkedNext = HiddenLinkedNext(Challenge);
    const int32 LinkedStart = static_cast<int32>(ChallengeSeed(Challenge) % LinkedNext.Num());
    const int32 LinkedExpected = CountLinkedNodes(LinkedNext, LinkedStart);
    const TArray<int32> SearchValues = HiddenSearchValues(Challenge);
    const int32 SearchTarget = SearchValues[SearchValues.Num() / 2];
    FString FunctionFileName;
    switch (Kind)
    {
    case ECodeChallengeKind::Lock:       FunctionFileName = TEXT("should_unlock.m"); break;
    case ECodeChallengeKind::Reverse:    FunctionFileName = TEXT("reverse_string.m"); break;
    case ECodeChallengeKind::Palindrome: FunctionFileName = TEXT("is_palindrome.m"); break;
    case ECodeChallengeKind::FizzBuzz:   FunctionFileName = TEXT("fizz_buzz.m"); break;
    case ECodeChallengeKind::EvenFilter: FunctionFileName = TEXT("even_numbers.m"); break;
    case ECodeChallengeKind::LinkedListTraverse: FunctionFileName = TEXT("count_nodes.m"); break;
    case ECodeChallengeKind::BinarySearch: FunctionFileName = TEXT("binary_search.m"); break;
    default:                             FunctionFileName = TEXT("total_power.m"); break;
    }
    const FString SourceFile = FPaths::Combine(SandboxDir, FunctionFileName);
    const FString RunnerFile = FPaths::Combine(SandboxDir, TEXT("run_mission_validation.m"));

    FFileHelper::SaveStringToFile(UserCode, *SourceFile);

    FString Runner;
    if (CanUseDeclarativeTests(Challenge, Kind))
    {
        Runner = BuildMatlabDeclarativeRunner(Challenge, Kind, SandboxDir);
    }
    else switch (Kind)
    {
    case ECodeChallengeKind::Sum:
        Runner = FString::Printf(TEXT("addpath('%s'); assert(total_power(20,15,10)==45); assert(total_power(%d,%d,%d)==%d); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")), SumA, SumB, SumC, SumA + SumB + SumC);
        break;
    case ECodeChallengeKind::Lock:
        Runner = FString::Printf(TEXT("addpath('%s'); assert(should_unlock(true,true)==true); assert(should_unlock(true,false)==false); assert(should_unlock(false,true)==false); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")));
        break;
    case ECodeChallengeKind::Reverse:
        Runner = FString::Printf(TEXT("addpath('%s'); assert(strcmp(reverse_string('rescue'),'eucser')); assert(strcmp(reverse_string('%s'),'%s')); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")), *ReverseWord, *ReverseExpected);
        break;
    case ECodeChallengeKind::Palindrome:
        Runner = FString::Printf(TEXT("addpath('%s'); assert(is_palindrome('racecar')==true); assert(is_palindrome('%s')==true); assert(is_palindrome('%s')==false); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")), *PalWord, *NonPalWord);
        break;
    case ECodeChallengeKind::FizzBuzz:
        Runner = FString::Printf(TEXT("addpath('%s'); expected='1,2,Fizz,4,Buzz,Fizz,7,8,Fizz,Buzz,11,Fizz,13,14,FizzBuzz'; actual=strjoin(string(fizz_buzz(15)), ','); assert(strcmp(actual, expected)); hiddenExpected='%s'; hiddenActual=strjoin(string(fizz_buzz(%d)), ','); assert(strcmp(hiddenActual, hiddenExpected)); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")), *FizzExpected, FizzN);
        break;
    case ECodeChallengeKind::EvenFilter:
        Runner = FString::Printf(TEXT("addpath('%s'); assert(isequal(even_numbers([1 2 3 4 5 6]), [2 4 6])); assert(isequal(even_numbers(%s), %s)); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")), *MatlabVectorLiteral(EvenInput), *MatlabVectorLiteral(EvenExpected));
        break;
    case ECodeChallengeKind::LinkedListTraverse:
        Runner = FString::Printf(TEXT("addpath('%s'); assert(count_nodes([2 3 0], 1)==3); assert(count_nodes(%s, %d)==%d); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")), *MatlabLinkedVectorLiteral(LinkedNext), LinkedStart + 1, LinkedExpected);
        break;
    case ECodeChallengeKind::BinarySearch:
        Runner = FString::Printf(TEXT("addpath('%s'); assert(binary_search([2 4 6 8 10], 6)==3); assert(binary_search(%s, %d)==%d); assert(binary_search(%s, 9999)==0); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")), *MatlabVectorLiteral(SearchValues), SearchTarget, (SearchValues.Num() / 2) + 1, *MatlabVectorLiteral(SearchValues));
        break;
    default:
        Runner = FString::Printf(TEXT("addpath('%s'); disp('ALL_TESTS_PASSED');"), *SandboxDir.Replace(TEXT("\\"), TEXT("/")));
        break;
    }
    FFileHelper::SaveStringToFile(Runner, *RunnerFile);

    const FString Matlab = FindMATLABExecutable();
    FString StdOut, StdErr;
    int32 Code = -1;
    if (Matlab == TEXT("matlab"))
    {
        ExecProcess(TEXT("/usr/bin/env"), FString::Printf(TEXT("matlab -batch %s"), *QuoteArg(Runner)), StdOut, StdErr, Code);
    }
    else
    {
        ExecProcess(Matlab, FString::Printf(TEXT("-batch %s"), *QuoteArg(Runner)), StdOut, StdErr, Code);
    }
    if (Code == -408)
    {
        GHasMATLAB = false;
        FCodeValidationResult Fallback = ValidateInEngine(Challenge, UserCode);
        Fallback.StdOut = StdOut;
        Fallback.StdErr = StdErr;
        Fallback.Summary = Fallback.bSuccess
            ? TEXT("MATLAB batch mode timed out locally; in-engine MATLAB-compatible validator accepted the solution.")
            : TEXT("MATLAB batch mode timed out locally, and the in-engine fallback could not validate the solution.");
        return Fallback;
    }
    Result.StdOut = StdOut;
    Result.StdErr = StdErr;
    const bool bAllTestsPassed = StdOut.Contains(TEXT("ALL_TESTS_PASSED"));
    AddCheck(Result, TEXT("MATLAB launched in batch mode"), Code == 0 || bAllTestsPassed);
    AddCheck(Result, TEXT("Visible and hidden MATLAB tests pass"), bAllTestsPassed);

    Result.Score = FMath::RoundToInt(100.0f * Result.PassedChecks.Num() / FMath::Max(1, Result.PassedChecks.Num() + Result.FailedChecks.Num()));
    Result.bSuccess = Result.FailedChecks.Num() == 0;
    Result.Summary = Result.bSuccess ? TEXT("MATLAB rescue terminal validated successfully.") : TEXT("MATLAB validation failed. Make sure MATLAB_BIN or MATLABROOT points to your MATLAB installation if auto-detection fails.");
    ApplyDeclarativeTestCaseCounts(Result, Challenge);
    return Result;
}

// =====================================================================
// #46 syntax highlighting, #47 auto-indent, #49 custom challenge loader
// =====================================================================

namespace
{
const TSet<FString>& GetKeywordsFor(ECodingLanguage L)
{
    static const TSet<FString> Java = {
        TEXT("abstract"), TEXT("assert"), TEXT("boolean"), TEXT("break"), TEXT("byte"),
        TEXT("case"), TEXT("catch"), TEXT("char"), TEXT("class"), TEXT("const"),
        TEXT("continue"), TEXT("default"), TEXT("do"), TEXT("double"), TEXT("else"),
        TEXT("enum"), TEXT("extends"), TEXT("final"), TEXT("finally"), TEXT("float"),
        TEXT("for"), TEXT("if"), TEXT("implements"), TEXT("import"), TEXT("instanceof"),
        TEXT("int"), TEXT("interface"), TEXT("long"), TEXT("native"), TEXT("new"),
        TEXT("null"), TEXT("package"), TEXT("private"), TEXT("protected"), TEXT("public"),
        TEXT("return"), TEXT("short"), TEXT("static"), TEXT("super"), TEXT("switch"),
        TEXT("synchronized"), TEXT("this"), TEXT("throw"), TEXT("throws"), TEXT("transient"),
        TEXT("true"), TEXT("false"), TEXT("try"), TEXT("void"), TEXT("volatile"), TEXT("while")
    };
    static const TSet<FString> C = {
        TEXT("auto"), TEXT("break"), TEXT("case"), TEXT("char"), TEXT("const"),
        TEXT("continue"), TEXT("default"), TEXT("do"), TEXT("double"), TEXT("else"),
        TEXT("enum"), TEXT("extern"), TEXT("float"), TEXT("for"), TEXT("goto"),
        TEXT("if"), TEXT("inline"), TEXT("int"), TEXT("long"), TEXT("register"),
        TEXT("return"), TEXT("short"), TEXT("signed"), TEXT("sizeof"), TEXT("static"),
        TEXT("struct"), TEXT("switch"), TEXT("typedef"), TEXT("union"), TEXT("unsigned"),
        TEXT("void"), TEXT("volatile"), TEXT("while")
    };
    static const TSet<FString> Cpp = {
        TEXT("auto"), TEXT("bool"), TEXT("break"), TEXT("case"), TEXT("catch"),
        TEXT("char"), TEXT("class"), TEXT("const"), TEXT("constexpr"), TEXT("continue"),
        TEXT("default"), TEXT("delete"), TEXT("do"), TEXT("double"), TEXT("else"),
        TEXT("enum"), TEXT("explicit"), TEXT("false"), TEXT("float"), TEXT("for"),
        TEXT("if"), TEXT("include"), TEXT("inline"), TEXT("int"), TEXT("long"),
        TEXT("namespace"), TEXT("new"), TEXT("private"), TEXT("protected"), TEXT("public"),
        TEXT("return"), TEXT("short"), TEXT("sizeof"), TEXT("static"), TEXT("std"),
        TEXT("string"), TEXT("struct"), TEXT("switch"), TEXT("template"), TEXT("true"),
        TEXT("try"), TEXT("typedef"), TEXT("typename"), TEXT("using"), TEXT("vector"),
        TEXT("void"), TEXT("while")
    };
    static const TSet<FString> Python = {
        TEXT("False"), TEXT("None"), TEXT("True"), TEXT("and"), TEXT("as"), TEXT("assert"),
        TEXT("async"), TEXT("await"), TEXT("break"), TEXT("class"), TEXT("continue"),
        TEXT("def"), TEXT("del"), TEXT("elif"), TEXT("else"), TEXT("except"), TEXT("finally"),
        TEXT("for"), TEXT("from"), TEXT("global"), TEXT("if"), TEXT("import"), TEXT("in"),
        TEXT("is"), TEXT("lambda"), TEXT("nonlocal"), TEXT("not"), TEXT("or"), TEXT("pass"),
        TEXT("raise"), TEXT("return"), TEXT("try"), TEXT("while"), TEXT("with"), TEXT("yield")
    };
    static const TSet<FString> MATLAB = {
        TEXT("break"), TEXT("case"), TEXT("catch"), TEXT("classdef"), TEXT("continue"),
        TEXT("else"), TEXT("elseif"), TEXT("end"), TEXT("for"), TEXT("function"),
        TEXT("global"), TEXT("if"), TEXT("otherwise"), TEXT("parfor"), TEXT("persistent"),
        TEXT("return"), TEXT("spmd"), TEXT("switch"), TEXT("try"), TEXT("while"),
        TEXT("true"), TEXT("false")
    };
    switch (L)
    {
    case ECodingLanguage::Java:   return Java;
    case ECodingLanguage::C:      return C;
    case ECodingLanguage::Python: return Python;
    case ECodingLanguage::MATLAB: return MATLAB;
    case ECodingLanguage::CPlus:  return Cpp;
    case ECodingLanguage::Cpp:    return Cpp;
    }
    return Java;
}

bool IsIdChar(TCHAR Ch)
{
    return FChar::IsAlnum(Ch) || Ch == TEXT('_');
}
}

FString UCodeRunnerLibrary::HighlightCode(ECodingLanguage Language, const FString& UserCode)
{
    const TSet<FString>& Keywords = GetKeywordsFor(Language);
    FString Out;
    Out.Reserve(UserCode.Len() + 256);

    const int32 N = UserCode.Len();
    int32 i = 0;
    while (i < N)
    {
        const TCHAR C = UserCode[i];

        // --- comments
        const bool bDoubleSlashComment = (Language != ECodingLanguage::Python && Language != ECodingLanguage::MATLAB)
                                       && C == TEXT('/') && i + 1 < N && UserCode[i + 1] == TEXT('/');
        const bool bHashComment = (Language == ECodingLanguage::Python) && C == TEXT('#');
        const bool bPercentComment = (Language == ECodingLanguage::MATLAB) && C == TEXT('%');
        if (bDoubleSlashComment || bHashComment || bPercentComment)
        {
            Out += TEXT("<Cmt>");
            while (i < N && UserCode[i] != TEXT('\n'))
            {
                Out.AppendChar(UserCode[i]);
                ++i;
            }
            Out += TEXT("</>");
            continue;
        }

        // --- strings (single OR double-quoted; no escape handling)
        if (C == TEXT('"') || C == TEXT('\''))
        {
            const TCHAR Quote = C;
            Out += TEXT("<Str>");
            Out.AppendChar(Quote);
            ++i;
            while (i < N && UserCode[i] != Quote && UserCode[i] != TEXT('\n'))
            {
                Out.AppendChar(UserCode[i]);
                ++i;
            }
            if (i < N && UserCode[i] == Quote)
            {
                Out.AppendChar(Quote);
                ++i;
            }
            Out += TEXT("</>");
            continue;
        }

        // --- numbers (very loose)
        if (FChar::IsDigit(C))
        {
            Out += TEXT("<Num>");
            while (i < N && (FChar::IsDigit(UserCode[i]) || UserCode[i] == TEXT('.')))
            {
                Out.AppendChar(UserCode[i]);
                ++i;
            }
            Out += TEXT("</>");
            continue;
        }

        // --- identifiers / keywords
        if (FChar::IsAlpha(C) || C == TEXT('_'))
        {
            int32 j = i;
            while (j < N && IsIdChar(UserCode[j])) ++j;
            const FString Word = UserCode.Mid(i, j - i);
            if (Keywords.Contains(Word))
            {
                Out += TEXT("<Keyword>") + Word + TEXT("</>");
            }
            else
            {
                Out += Word;
            }
            i = j;
            continue;
        }

        Out.AppendChar(C);
        ++i;
    }
    return Out;
}

FString UCodeRunnerLibrary::ComputeAutoIndentForNewline(const FString& PrevLine)
{
    // Capture leading whitespace.
    FString Indent;
    for (int32 k = 0; k < PrevLine.Len(); ++k)
    {
        if (PrevLine[k] == TEXT(' ') || PrevLine[k] == TEXT('\t'))
        {
            Indent.AppendChar(PrevLine[k]);
        }
        else
        {
            break;
        }
    }

    // Trim trailing whitespace to find the last non-space char.
    FString Trimmed = PrevLine.TrimEnd();
    if (Trimmed.IsEmpty()) return Indent;
    const TCHAR Last = Trimmed[Trimmed.Len() - 1];
    if (Last == TEXT('{') || Last == TEXT(':'))
    {
        Indent += TEXT("    ");
    }
    return Indent;
}

FString UCodeRunnerLibrary::GetAutoCloseFor(const FString& JustTyped)
{
    if (JustTyped == TEXT("{")) return TEXT("}");
    if (JustTyped == TEXT("[")) return TEXT("]");
    if (JustTyped == TEXT("(")) return TEXT(")");
    return TEXT("");
}

TArray<FChallengeSpec> UCodeRunnerLibrary::LoadCustomChallenges()
{
    TArray<FChallengeSpec> Out;
    const FString Path = FPaths::ProjectContentDir() / TEXT("CodeRescueData/custom_challenges.json");
    FString Raw;
    if (!FFileHelper::LoadFileToString(Raw, *Path))
    {
        return Out;
    }
    TArray<TSharedPtr<FJsonValue>> Items;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
    if (!FJsonSerializer::Deserialize(Reader, Items))
    {
        UE_LOG(LogTemp, Warning, TEXT("[LoadCustomChallenges] Failed to parse %s"), *Path);
        return Out;
    }
    for (const TSharedPtr<FJsonValue>& Item : Items)
    {
        const TSharedPtr<FJsonObject>* Obj;
        if (!Item->TryGetObject(Obj)) continue;
        FChallengeSpec C;
        C.Id = (*Obj)->GetStringField(TEXT("id"));
        C.Title = (*Obj)->GetStringField(TEXT("title"));
        C.MissionBrief = (*Obj)->GetStringField(TEXT("brief"));
        const FString LangStr = (*Obj)->GetStringField(TEXT("language"));
        if      (LangStr == TEXT("Java"))   C.Language = ECodingLanguage::Java;
        else if (LangStr == TEXT("C"))      C.Language = ECodingLanguage::C;
        else if (LangStr == TEXT("Python")) C.Language = ECodingLanguage::Python;
        else if (LangStr == TEXT("MATLAB")) C.Language = ECodingLanguage::MATLAB;
        else if (LangStr == TEXT("C+"))     C.Language = ECodingLanguage::CPlus;
        else if (LangStr == TEXT("C++"))    C.Language = ECodingLanguage::Cpp;
        Out.Add(C);
    }
    UE_LOG(LogTemp, Log, TEXT("[LoadCustomChallenges] Loaded %d custom challenges from %s"), Out.Num(), *Path);
    return Out;
}
