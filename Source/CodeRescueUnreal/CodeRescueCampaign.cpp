#include "CodeRescueCampaign.h"
#include "CodeRescueGameInstance.h"

namespace
{
const TCHAR* LessonToken(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:       return TEXT("lock");
    case ECampaignLessonKind::Reverse:    return TEXT("reverse");
    case ECampaignLessonKind::Palindrome: return TEXT("palindrome");
    case ECampaignLessonKind::FizzBuzz:   return TEXT("fizzbuzz");
    case ECampaignLessonKind::EvenFilter: return TEXT("even_filter");
    case ECampaignLessonKind::LinkedListTraverse: return TEXT("linked_list_traverse");
    case ECampaignLessonKind::BinarySearch: return TEXT("binary_search");
    default:                              return TEXT("sum");
    }
}

FString LessonTitle(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:       return TEXT("Access Gate Boolean Logic");
    case ECampaignLessonKind::Reverse:    return TEXT("Signal Reversal Drill");
    case ECampaignLessonKind::Palindrome: return TEXT("Mirror-Code Integrity Check");
    case ECampaignLessonKind::FizzBuzz:   return TEXT("Beacon Cycle FizzBuzz");
    case ECampaignLessonKind::EvenFilter: return TEXT("Even-Unit Triage Filter");
    case ECampaignLessonKind::LinkedListTraverse: return TEXT("Evacuation Chain Traversal");
    case ECampaignLessonKind::BinarySearch: return TEXT("Sorted Cache Binary Search");
    default:                              return TEXT("Emergency Grid Sum");
    }
}

FString LessonConcept(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:       return TEXT("boolean safety gates");
    case ECampaignLessonKind::Reverse:    return TEXT("string traversal and reversal");
    case ECampaignLessonKind::Palindrome: return TEXT("mirror checks and edge cases");
    case ECampaignLessonKind::FizzBuzz:   return TEXT("loops, divisibility, and ordered branches");
    case ECampaignLessonKind::EvenFilter: return TEXT("array filtering with predicates");
    case ECampaignLessonKind::LinkedListTraverse: return TEXT("stateful traversal through linked rescue nodes");
    case ECampaignLessonKind::BinarySearch: return TEXT("halving a sorted search space with bounds");
    default:                              return TEXT("function inputs, arithmetic, and return values");
    }
}

FString LessonStrategy(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("build a tiny truth table before writing the condition");
    case ECampaignLessonKind::Reverse:
        return TEXT("walk the input from the far end back to the front");
    case ECampaignLessonKind::Palindrome:
        return TEXT("compare paired characters and test both pass and fail cases");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("check the combined 3-and-5 case before the individual cases");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("preserve input order while keeping only values that pass the even test");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("advance from one rescue node to the next until the chain ends");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("track low/high bounds, test the midpoint, and discard half the search space");
    default:
        return TEXT("return the computed total from the function instead of printing it");
    }
}

FString LanguageName(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java:   return TEXT("Java");
    case ECodingLanguage::C:      return TEXT("C");
    case ECodingLanguage::Python: return TEXT("Python");
    case ECodingLanguage::MATLAB: return TEXT("MATLAB");
    case ECodingLanguage::CPlus:  return TEXT("C+");
    case ECodingLanguage::Cpp:    return TEXT("C++");
    default:                      return TEXT("Unknown");
    }
}

FString LessonBrief(const FString& City, ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return FString::Printf(TEXT("%s is locked behind a rescue gate. Write shouldUnlock/should_unlock so the gate opens only when both safety conditions are true."), *City);
    case ECampaignLessonKind::Reverse:
        return FString::Printf(TEXT("%s is receiving reversed radio packets. Write reverseString/reverse_string to restore the message before evacuation can continue."), *City);
    case ECampaignLessonKind::Palindrome:
        return FString::Printf(TEXT("%s uses mirrored access codes. Write isPalindrome/is_palindrome to accept real mirror codes and reject impostors."), *City);
    case ECampaignLessonKind::FizzBuzz:
        return FString::Printf(TEXT("%s needs a timed beacon sweep. Write fizzBuzz/fizz_buzz so rescue drones receive the correct cycle labels."), *City);
    case ECampaignLessonKind::EvenFilter:
        return FString::Printf(TEXT("%s must route even-numbered emergency units first. Write evenNumbers/even_numbers to keep only even values in order."), *City);
    case ECampaignLessonKind::LinkedListTraverse:
        return FString::Printf(TEXT("%s has a broken evacuation chain. Write countNodes/count_nodes to follow next-index links from the start node until the chain ends."), *City);
    case ECampaignLessonKind::BinarySearch:
        return FString::Printf(TEXT("%s is searching a sorted supply cache. Write binarySearch/binary_search to find a target by halving the search space, returning the target index or the not-found value."), *City);
    default:
        return FString::Printf(TEXT("%s has unstable backup cells. Write totalPower/total_power to sum three battery readings and restore the city hub."), *City);
    }
}

FLinearColor AccentFor(int32 Rank, ECampaignLessonKind Kind)
{
    const FLinearColor LessonBase =
        Kind == ECampaignLessonKind::Lock ? FLinearColor(0.05f, 0.75f, 1.0f) :
        Kind == ECampaignLessonKind::Reverse ? FLinearColor(1.0f, 0.35f, 0.75f) :
        Kind == ECampaignLessonKind::Palindrome ? FLinearColor(0.75f, 0.55f, 1.0f) :
        Kind == ECampaignLessonKind::FizzBuzz ? FLinearColor(1.0f, 0.85f, 0.2f) :
        Kind == ECampaignLessonKind::EvenFilter ? FLinearColor(0.25f, 1.0f, 0.45f) :
        Kind == ECampaignLessonKind::LinkedListTraverse ? FLinearColor(0.95f, 0.55f, 0.25f) :
        Kind == ECampaignLessonKind::BinarySearch ? FLinearColor(0.25f, 0.95f, 0.85f) :
        FLinearColor(0.25f, 0.55f, 1.0f);

    const float Pulse = 0.85f + 0.03f * (Rank % 5);
    return LessonBase * Pulse;
}

ECodingLanguage RecommendedLanguageFor(int32 Rank)
{
    switch (Rank % 6)
    {
    case 1: return ECodingLanguage::Java;
    case 2: return ECodingLanguage::C;
    case 3: return ECodingLanguage::Python;
    case 4: return ECodingLanguage::MATLAB;
    case 5: return ECodingLanguage::CPlus;
    default: return ECodingLanguage::Cpp;
    }
}

FString RegionForState(const FString& State)
{
    static const TSet<FString> West = {
        TEXT("AK"), TEXT("AZ"), TEXT("CA"), TEXT("CO"), TEXT("HI"), TEXT("ID"), TEXT("MT"),
        TEXT("NM"), TEXT("NV"), TEXT("OR"), TEXT("UT"), TEXT("WA"), TEXT("WY")
    };
    static const TSet<FString> Midwest = {
        TEXT("IA"), TEXT("IL"), TEXT("IN"), TEXT("KS"), TEXT("MI"), TEXT("MN"), TEXT("MO"),
        TEXT("ND"), TEXT("NE"), TEXT("OH"), TEXT("SD"), TEXT("WI")
    };
    static const TSet<FString> Northeast = {
        TEXT("CT"), TEXT("MA"), TEXT("ME"), TEXT("NH"), TEXT("NJ"), TEXT("NY"),
        TEXT("PA"), TEXT("RI"), TEXT("VT")
    };
    static const TSet<FString> South = {
        TEXT("AL"), TEXT("AR"), TEXT("DC"), TEXT("DE"), TEXT("FL"), TEXT("GA"), TEXT("KY"),
        TEXT("LA"), TEXT("MD"), TEXT("MS"), TEXT("NC"), TEXT("OK"), TEXT("SC"), TEXT("TN"),
        TEXT("TX"), TEXT("VA"), TEXT("WV")
    };

    if (West.Contains(State)) return TEXT("Western Corridor");
    if (Midwest.Contains(State)) return TEXT("Midwest Grid");
    if (Northeast.Contains(State)) return TEXT("Northeast Relay");
    if (South.Contains(State)) return TEXT("Southern Evac Line");

    static const TSet<FString> EastAsia = {
        TEXT("CN"), TEXT("HK"), TEXT("JP"), TEXT("KR"), TEXT("TW")
    };
    static const TSet<FString> SouthAsia = {
        TEXT("BD"), TEXT("IN"), TEXT("LK"), TEXT("NP"), TEXT("PK")
    };
    static const TSet<FString> SoutheastAsia = {
        TEXT("ID"), TEXT("KH"), TEXT("LAO"), TEXT("MM"), TEXT("MY"), TEXT("PH"), TEXT("SG"), TEXT("TH"), TEXT("VN")
    };
    static const TSet<FString> Europe = {
        TEXT("AT"), TEXT("CZ"), TEXT("DE"), TEXT("ES"), TEXT("FR"), TEXT("GB"), TEXT("IT"), TEXT("NL"), TEXT("PL"), TEXT("PT"), TEXT("RU"), TEXT("UA")
    };
    static const TSet<FString> LatinAmerica = {
        TEXT("AR"), TEXT("BO"), TEXT("BR"), TEXT("CL"), TEXT("CO"), TEXT("CU"), TEXT("DO"), TEXT("EC"), TEXT("GT"),
        TEXT("MX"), TEXT("PA"), TEXT("PE"), TEXT("PY"), TEXT("SV"), TEXT("UY"), TEXT("VE")
    };
    static const TSet<FString> Africa = {
        TEXT("AO"), TEXT("CD"), TEXT("CI"), TEXT("DZ"), TEXT("EG"), TEXT("ET"), TEXT("GH"), TEXT("KE"), TEXT("MA"), TEXT("NG"),
        TEXT("TN"), TEXT("TZ"), TEXT("ZA")
    };
    static const TSet<FString> MiddleEast = {
        TEXT("AE"), TEXT("IQ"), TEXT("IR"), TEXT("SA"), TEXT("TR")
    };
    static const TSet<FString> Oceania = {
        TEXT("AU"), TEXT("NZ")
    };
    static const TSet<FString> Canada = {
        TEXT("CA-AB"), TEXT("CA-BC"), TEXT("CA-ON"), TEXT("CA-QC")
    };

    if (EastAsia.Contains(State)) return TEXT("East Asia Megacity Belt");
    if (SouthAsia.Contains(State)) return TEXT("South Asia Monsoon Arc");
    if (SoutheastAsia.Contains(State)) return TEXT("Southeast Asia Delta Route");
    if (Europe.Contains(State)) return TEXT("European Historic Core");
    if (LatinAmerica.Contains(State)) return TEXT("Latin America Metro Spine");
    if (Africa.Contains(State)) return TEXT("African Urban Relay");
    if (MiddleEast.Contains(State)) return TEXT("Middle East Solar Route");
    if (Oceania.Contains(State)) return TEXT("Oceania Harbor Ring");
    if (Canada.Contains(State)) return TEXT("Canadian Shield Corridor");
    return TEXT("Global Route");
}

FString DistrictStyleFor(const FString& Region, ECampaignLessonKind Kind)
{
    const FString LessonDistrict =
        Kind == ECampaignLessonKind::Lock ? TEXT("security checkpoint") :
        Kind == ECampaignLessonKind::Reverse ? TEXT("radio tower district") :
        Kind == ECampaignLessonKind::Palindrome ? TEXT("mirror-code archive") :
        Kind == ECampaignLessonKind::FizzBuzz ? TEXT("drone beacon yard") :
        Kind == ECampaignLessonKind::EvenFilter ? TEXT("triage routing depot") :
        Kind == ECampaignLessonKind::LinkedListTraverse ? TEXT("evacuation chain depot") :
        Kind == ECampaignLessonKind::BinarySearch ? TEXT("sorted supply-cache vault") :
        TEXT("power-grid substation");

    return FString::Printf(TEXT("%s %s"), *Region, *LessonDistrict);
}

FString LandmarkFor(const FString& City, ECampaignLessonKind Kind, int32 Rank)
{
    const TCHAR* Shapes[] = {
        TEXT("Signal Spire"),
        TEXT("Rescue Arch"),
        TEXT("Data Beacon"),
        TEXT("Command Obelisk"),
        TEXT("Relay Bridge"),
        TEXT("Power Crown")
    };

    const int32 ShapeIndex = FMath::Abs(Rank - 1) % UE_ARRAY_COUNT(Shapes);
    return FString::Printf(TEXT("%s %s"), *City, Shapes[ShapeIndex]);
}

FString ArtKitFor(const FString& City, const FString& State, const FString& Region)
{
    static const TSet<FString> Coastal = {
        TEXT("CA"), TEXT("CT"), TEXT("DE"), TEXT("FL"), TEXT("GA"), TEXT("HI"), TEXT("LA"),
        TEXT("MA"), TEXT("MD"), TEXT("ME"), TEXT("NC"), TEXT("NJ"), TEXT("NY"), TEXT("OR"),
        TEXT("RI"), TEXT("SC"), TEXT("VA"), TEXT("WA")
    };
    static const TSet<FString> Desert = {
        TEXT("AZ"), TEXT("NV"), TEXT("NM"), TEXT("TX")
    };
    static const TSet<FString> Mountain = {
        TEXT("AK"), TEXT("CO"), TEXT("ID"), TEXT("MT"), TEXT("UT"), TEXT("WY")
    };
    static const TSet<FString> GreatLakes = {
        TEXT("IL"), TEXT("IN"), TEXT("MI"), TEXT("MN"), TEXT("OH"), TEXT("WI")
    };
    static const TSet<FString> River = {
        TEXT("AR"), TEXT("IA"), TEXT("KY"), TEXT("LA"), TEXT("MO"), TEXT("MS"), TEXT("NE"), TEXT("TN")
    };
    static const TSet<FString> NeonMegacity = {
        TEXT("CN"), TEXT("HK"), TEXT("JP"), TEXT("KR"), TEXT("TW"), TEXT("SG")
    };
    static const TSet<FString> MonsoonMegacity = {
        TEXT("BD"), TEXT("IN"), TEXT("LK"), TEXT("NP"), TEXT("PK")
    };
    static const TSet<FString> MonsoonPort = {
        TEXT("ID"), TEXT("KH"), TEXT("LAO"), TEXT("MM"), TEXT("MY"), TEXT("PH"), TEXT("TH"), TEXT("VN")
    };
    static const TSet<FString> HistoricCore = {
        TEXT("AT"), TEXT("CZ"), TEXT("DE"), TEXT("ES"), TEXT("FR"), TEXT("GB"), TEXT("IT"), TEXT("NL"), TEXT("PL"), TEXT("PT"), TEXT("RU"), TEXT("TR"), TEXT("UA")
    };
    static const TSet<FString> LatinMetro = {
        TEXT("AR"), TEXT("BO"), TEXT("BR"), TEXT("CL"), TEXT("CO"), TEXT("CU"), TEXT("DO"), TEXT("EC"), TEXT("GT"),
        TEXT("MX"), TEXT("PA"), TEXT("PE"), TEXT("PY"), TEXT("SV"), TEXT("UY"), TEXT("VE")
    };
    static const TSet<FString> AfricanRelay = {
        TEXT("AO"), TEXT("CD"), TEXT("CI"), TEXT("DZ"), TEXT("EG"), TEXT("ET"), TEXT("GH"), TEXT("KE"), TEXT("MA"), TEXT("NG"),
        TEXT("TN"), TEXT("TZ"), TEXT("ZA")
    };
    static const TSet<FString> MiddleEastSolar = {
        TEXT("AE"), TEXT("IQ"), TEXT("IR"), TEXT("SA")
    };
    static const TSet<FString> OceaniaHarbor = {
        TEXT("AU"), TEXT("NZ")
    };
    static const TSet<FString> CanadianShield = {
        TEXT("CA-AB"), TEXT("CA-BC"), TEXT("CA-ON"), TEXT("CA-QC")
    };

    if (State == TEXT("DC")) return TEXT("Capital Command");
    if (NeonMegacity.Contains(State)) return TEXT("Neon Megacity");
    if (MonsoonMegacity.Contains(State)) return TEXT("Monsoon Megacity");
    if (MonsoonPort.Contains(State)) return TEXT("Monsoon Port");
    if (HistoricCore.Contains(State)) return TEXT("Historic Core");
    if (LatinMetro.Contains(State)) return TEXT("Latin Metro");
    if (AfricanRelay.Contains(State)) return TEXT("African Urban Relay");
    if (MiddleEastSolar.Contains(State)) return TEXT("Middle East Solar Hub");
    if (OceaniaHarbor.Contains(State)) return TEXT("Oceania Harbor");
    if (CanadianShield.Contains(State)) return TEXT("Great Lakes Industrial");
    if (City.Contains(TEXT("Las Vegas")) || City.Contains(TEXT("Phoenix")) || Desert.Contains(State)) return TEXT("Desert Solar Grid");
    if (Mountain.Contains(State)) return TEXT("Mountain Relay");
    if (GreatLakes.Contains(State)) return TEXT("Great Lakes Industrial");
    if (Coastal.Contains(State)) return TEXT("Coastal Port");
    if (River.Contains(State)) return TEXT("River Lockworks");
    if (Region.Contains(TEXT("Midwest"))) return TEXT("Rail Yard");
    return TEXT("Metro Core");
}

FString HintFor(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("Hint: list every boolean input pair before deciding whether && or || matches the safety rule.");
    case ECampaignLessonKind::Reverse:
        return TEXT("Hint: either use the language's reverse tool or copy characters from the end toward the front.");
    case ECampaignLessonKind::Palindrome:
        return TEXT("Hint: compare the first and last characters, then move inward until the scan crosses.");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("Hint: test divisible by 15 first, because those numbers are also divisible by 3 and 5.");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("Hint: keep a separate output index or append only when value % 2 equals 0.");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("Hint: start at the given node, count it, then replace current with the next index until the chain says stop.");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("Hint: keep low and high bounds; if the midpoint is too small, move low up, otherwise move high down.");
    default:
        return TEXT("Hint: the harness calls your function directly, so return the value instead of only printing it.");
    }
}

FString VisibleTestFor(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("Visible test: shouldUnlock(true, true) must return true.");
    case ECampaignLessonKind::Reverse:
        return TEXT("Visible test: reverseString(\"rescue\") must return \"eucser\".");
    case ECampaignLessonKind::Palindrome:
        return TEXT("Visible test: isPalindrome(\"racecar\") must return true.");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("Visible test: fizzBuzz(15) must produce the classic 1..15 FizzBuzz sequence.");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("Visible test: evenNumbers([1,2,3,4,5,6]) must return [2,4,6].");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("Visible test: countNodes over the chain 0 -> 1 -> 2 -> stop must return 3.");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("Visible test: binarySearch([2,4,6,8,10], 6) must find the target without scanning linearly.");
    default:
        return TEXT("Visible test: totalPower(20, 15, 10) must return 45.");
    }
}

FString HiddenTestFor(ECampaignLessonKind Kind, int32 Rank)
{
    const int32 Variant = FMath::Abs(Rank) % 5;
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("Hidden tests: unsafe pairs true/false, false/true, and false/false must all stay locked.");
    case ECampaignLessonKind::Reverse:
        return FString::Printf(TEXT("Hidden tests: mixed-case signal %d and a second city packet must reverse exactly."), Variant + 1);
    case ECampaignLessonKind::Palindrome:
        return TEXT("Hidden tests: a real mirror word must pass and a rescue-code impostor must fail.");
    case ECampaignLessonKind::FizzBuzz:
        return FString::Printf(TEXT("Hidden tests: extended beacon sweep length %d checks later multiples of 3, 5, and 15."), 16 + Variant);
    case ECampaignLessonKind::EvenFilter:
        return FString::Printf(TEXT("Hidden tests: city batch %d includes odd-only edges and preserves even-number order."), Variant + 1);
    case ECampaignLessonKind::LinkedListTraverse:
        return FString::Printf(TEXT("Hidden tests: evacuation chain batch %d changes the start node and stops only at the sentinel."), Variant + 1);
    case ECampaignLessonKind::BinarySearch:
        return FString::Printf(TEXT("Hidden tests: sorted-cache batch %d includes first, middle, last, and not-found targets."), Variant + 1);
    default:
        return FString::Printf(TEXT("Hidden tests: smaller city-grid inputs include zeros and variant batch %d."), Variant + 1);
    }
}

FString WhyThisMattersFor(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("boolean logic is how software turns policy into a safe yes-or-no decision");
    case ECampaignLessonKind::Reverse:
        return TEXT("reverse traversal teaches indexes, bounds, and careful movement through strings");
    case ECampaignLessonKind::Palindrome:
        return TEXT("two-sided comparison builds the habit of proving both acceptance and rejection cases");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("ordered branches are the core of rule systems, validators, and game-state scripts");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("filtering is the foundation of data cleanup, search results, and mission triage");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("node traversal teaches state, sentinel values, and moving through connected structures");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("halving a search space turns brute force into a fast algorithmic decision");
    default:
        return TEXT("returning a computed value is the first reliable contract between code and the game world");
    }
}

FString PredictPromptFor(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("predict which of the four true/false input pairs should open the route");
    case ECampaignLessonKind::Reverse:
        return TEXT("predict the first and last output characters before writing the loop");
    case ECampaignLessonKind::Palindrome:
        return TEXT("predict one passing mirror code and one failing impostor code");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("predict outputs for 3, 5, and 15 before coding the branch order");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("predict which values survive a short mixed odd/even list");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("predict the node visit order from start to sentinel");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("predict low, mid, and high after the first comparison");
    default:
        return TEXT("predict the returned total for three small inputs before editing");
    }
}

FString WorkedExampleFor(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("worked example: true && false stays locked; true && true opens");
    case ECampaignLessonKind::Reverse:
        return TEXT("worked example: rescue becomes eucser by reading from the end");
    case ECampaignLessonKind::Palindrome:
        return TEXT("worked example: level passes because l/e mirror e/l; signal fails");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("worked example: 15 is FizzBuzz because the combined rule wins first");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("worked example: [1,2,3,4] keeps [2,4] without reordering");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("worked example: next=[1,2,-1], start=0 visits three nodes");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("worked example: target 6 in [2,4,6,8] checks the middle before shrinking");
    default:
        return TEXT("worked example: 20, 15, and 10 return 45");
    }
}

FString VisualDebuggerFor(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("Visual debugger: truth-table lamps show boolean inputs and the final gate state so players can trace every safe and unsafe pair before coding.");
    case ECampaignLessonKind::Reverse:
        return TEXT("Visual debugger: packet tiles light from right to left as each character moves so off-by-one errors become visible before validation.");
    case ECampaignLessonKind::Palindrome:
        return TEXT("Visual debugger: paired mirror posts highlight left and right indexes together so players can see why a mismatch rejects the code.");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("Visual debugger: numbered beacon pylons color 3, 5, and 15 decisions distinctly so branch order is readable in the world.");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("Visual debugger: triage lanes split kept even values from rejected odd values so predicate filtering has a physical route.");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("Visual debugger: linked rescue posts label current, next, count, and sentinel so traversal state is visible at each step.");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("Visual debugger: shrinking light bands mark low, mid, high, and discarded ranges so each comparison visibly halves the space.");
    default:
        return TEXT("Visual debugger: three power cells light as inputs combine into one return value so the function contract is visible.");
    }
}

FString MistakeGlossaryFor(ECampaignLessonKind Kind)
{
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return TEXT("Mistake glossary: OR opens unsafe pairs; missing return leaves the harness with no decision.");
    case ECampaignLessonKind::Reverse:
        return TEXT("Mistake glossary: off-by-one indexes drop the first or last character.");
    case ECampaignLessonKind::Palindrome:
        return TEXT("Mistake glossary: returning true too early accepts impostor codes.");
    case ECampaignLessonKind::FizzBuzz:
        return TEXT("Mistake glossary: checking 3 before 15 makes FizzBuzz numbers become only Fizz.");
    case ECampaignLessonKind::EvenFilter:
        return TEXT("Mistake glossary: appending every value or changing order breaks triage routing.");
    case ECampaignLessonKind::LinkedListTraverse:
        return TEXT("Mistake glossary: forgetting to advance current creates an infinite evacuation loop.");
    case ECampaignLessonKind::BinarySearch:
        return TEXT("Mistake glossary: stale bounds repeat the same mid and never prove not-found.");
    default:
        return TEXT("Mistake glossary: printing instead of returning hides the value from the validator.");
    }
}

FString LanguageTrackFor(ECodingLanguage Language, ECampaignLessonKind Kind, int32 Rank)
{
    const FString Concept = LessonConcept(Kind);
    const FString TrackNote =
        Language == ECodingLanguage::Java ? TEXT("Java track emphasizes exact public static signatures, typed arrays, and readable branch blocks") :
        Language == ECodingLanguage::C ? TEXT("C track emphasizes buffer safety, pointer discipline, null terminators, and explicit counts") :
        Language == ECodingLanguage::Python ? TEXT("Python track emphasizes small readable helpers, list operations, and clear loop intent") :
        Language == ECodingLanguage::MATLAB ? TEXT("MATLAB track emphasizes function-file naming, one-based indexing, arrays, and vector-friendly reasoning") :
        Language == ECodingLanguage::CPlus ? TEXT("C+ track emphasizes C-style fundamentals with gentle C++ standard-library introductions") :
        TEXT("C++ track emphasizes typed functions, vectors, strings, and standard-library algorithms");

    return FString::Printf(
        TEXT("%s. Current concept: %s. Rank %03d launch track note: this run keeps the campaign terminal on the language selected before deployment."),
        *TrackNote,
        *Concept,
        Rank);
}

FString LearningSupportFor(ECampaignLessonKind Kind, int32 Rank)
{
    return FString::Printf(
        TEXT("Why this matters: %s. Predict before coding: %s. Optional worked example: %s. Code-trace mini-game: trace one visible case before editing. %s Hidden-test replay: after validation, compare the visible case with the seeded hidden batch %d."),
        *WhyThisMattersFor(Kind),
        *PredictPromptFor(Kind),
        *WorkedExampleFor(Kind),
        *MistakeGlossaryFor(Kind),
        (FMath::Abs(Rank) % 5) + 1);
}

FString ProgressionPlanFor(ECampaignLessonKind Kind, int32 Rank)
{
    (void)Kind;

    const FString StageName =
        Rank <= 24 ? TEXT("Stage 1 - Foundations") :
        Rank <= 84 ? TEXT("Stage 2 - Control Flow") :
        Rank <= 174 ? TEXT("Stage 3 - Collections and Strings") :
        Rank <= 294 ? TEXT("Stage 4 - Data Structures") :
        TEXT("Stage 5 - Algorithmic Search");
    const FString Review =
        Rank <= 24 ? TEXT("first-pass foundations") :
        Rank <= 84 ? TEXT("spaced review of arithmetic, booleans, and reversal") :
        Rank <= 174 ? TEXT("spaced review of branches, strings, and loop outputs") :
        Rank <= 294 ? TEXT("spaced review of filters plus linked-list and search previews") :
        TEXT("capstone review combining structures, validation, and search");

    const FString Expansion =
        Rank <= 84 ? TEXT("nested-loop and radio parsing preview") :
        Rank <= 174 ? TEXT("dictionary/map lookup, stack, and queue rescue metaphors") :
        Rank <= 294 ? TEXT("recursion preview, sorting intuition, and input validation") :
        TEXT("multi-function capstone combining two or three concepts");

    return FString::Printf(
        TEXT("Progression plan: curriculum map places this mission in %s; spaced review path: %s; expansion bridge: %s; side objectives reward comments, edge cases, and clean returns."),
        *StageName,
        *Review,
        *Expansion);
}

FString CharacterStoryPlanFor(ECampaignLessonKind Kind, const FString& City)
{
    const FString Archetype =
        Kind == ECampaignLessonKind::Lock ? TEXT("systems mechanic") :
        Kind == ECampaignLessonKind::Reverse ? TEXT("radio-code cleanup specialist") :
        Kind == ECampaignLessonKind::Palindrome ? TEXT("archive integrity analyst") :
        Kind == ECampaignLessonKind::FizzBuzz ? TEXT("drone timing coordinator") :
        Kind == ECampaignLessonKind::EvenFilter ? TEXT("data medic") :
        Kind == ECampaignLessonKind::LinkedListTraverse ? TEXT("network engineer") :
        Kind == ECampaignLessonKind::BinarySearch ? TEXT("supply-cache analyst") :
        TEXT("power-grid apprentice");

    return FString::Printf(
        TEXT("Character plan: %s mentor NPC, named survivor team, role icon, post-rescue dialogue, recurring radio ally, readable nameplate, and debrief vignette for %s."),
        *Archetype,
        *City);
}

FString GameplayFlowPlanFor(int32 Rank)
{
    return FString::Printf(
        TEXT("Flow plan: first-session route preview, practice-only terminal option, replay-from-journal hook, beginner/normal/challenge tuning band %d, terminal-pressure dampener, return markers, fail-safe objective board, reward choice kiosk, stage recap, profile stats, review recommendation, and save-slot preview."),
        FMath::Clamp(1 + ((Rank - 1) / 70), 1, 5));
}

FString AccessibilityPolishPlanFor(const FString& Region)
{
    return FString::Printf(
        TEXT("Accessibility and polish plan: scalable terminal/HUD/subtitle text, colorblind-safe markers, remappable keyboard/controller prompts, screen-reader mission summary, wrapped severity-labeled errors, audio stingers, %s ambient loop, low-combat learning note, photo/inspect cue, main-menu curriculum access, loading tip, and subtitle-safe radio layout."),
        *Region);
}

FString QAVerificationPlanFor(int32 Rank)
{
    return FString::Printf(
        TEXT("QA plan: runtime spawn audit sample %03d, tutorial-terminal-rescue-extraction-fast-travel smoke path, performance budgets for enemies/proxies/props/lights/text, packaged-build audit, and weekly release checklist sign-off."),
        Rank);
}

FString RadioVoiceFor(const FString& Region)
{
    if (Region.Contains(TEXT("Western"))) return TEXT("Samantha");
    if (Region.Contains(TEXT("Midwest"))) return TEXT("Alex");
    if (Region.Contains(TEXT("Northeast"))) return TEXT("Victoria");
    if (Region.Contains(TEXT("Southern"))) return TEXT("Daniel");
    if (Region.Contains(TEXT("East Asia"))) return TEXT("Kyoko");
    if (Region.Contains(TEXT("South Asia"))) return TEXT("Rishi");
    if (Region.Contains(TEXT("Europe"))) return TEXT("Daniel");
    if (Region.Contains(TEXT("Latin America"))) return TEXT("Diego");
    if (Region.Contains(TEXT("Africa"))) return TEXT("Tessa");
    if (Region.Contains(TEXT("Middle East"))) return TEXT("Maged");
    if (Region.Contains(TEXT("Oceania"))) return TEXT("Karen");
    return TEXT("Samantha");
}

FLinearColor SecondaryAccentFor(const FString& Region, ECampaignLessonKind Kind)
{
    const FLinearColor RegionTint =
        Region.Contains(TEXT("Western")) ? FLinearColor(1.0f, 0.42f, 0.22f) :
        Region.Contains(TEXT("Midwest")) ? FLinearColor(0.35f, 0.95f, 0.75f) :
        Region.Contains(TEXT("Northeast")) ? FLinearColor(0.70f, 0.90f, 1.0f) :
        Region.Contains(TEXT("Southern")) ? FLinearColor(1.0f, 0.72f, 0.30f) :
        Region.Contains(TEXT("East Asia")) ? FLinearColor(0.20f, 0.95f, 1.0f) :
        Region.Contains(TEXT("South Asia")) ? FLinearColor(0.95f, 0.42f, 1.0f) :
        Region.Contains(TEXT("Southeast Asia")) ? FLinearColor(0.25f, 1.0f, 0.72f) :
        Region.Contains(TEXT("European")) ? FLinearColor(0.85f, 0.78f, 1.0f) :
        Region.Contains(TEXT("Latin America")) ? FLinearColor(1.0f, 0.36f, 0.24f) :
        Region.Contains(TEXT("African")) ? FLinearColor(0.95f, 0.76f, 0.30f) :
        Region.Contains(TEXT("Middle East")) ? FLinearColor(1.0f, 0.82f, 0.42f) :
        Region.Contains(TEXT("Oceania")) ? FLinearColor(0.28f, 0.72f, 1.0f) :
        Region.Contains(TEXT("Canadian")) ? FLinearColor(0.78f, 0.92f, 1.0f) :
        FLinearColor(0.8f, 0.8f, 0.9f);

    const FLinearColor LessonTint =
        Kind == ECampaignLessonKind::FizzBuzz ? FLinearColor(1.0f, 0.9f, 0.25f) :
        Kind == ECampaignLessonKind::EvenFilter ? FLinearColor(0.25f, 1.0f, 0.55f) :
        FLinearColor(0.65f, 0.85f, 1.0f);

    return (RegionTint * 0.65f) + (LessonTint * 0.35f);
}

int32 DifficultyTierForRank(int32 Rank)
{
    return FMath::Clamp(1 + ((Rank - 1) / 70), 1, 5);
}

float EncounterIntensityForRank(int32 Rank, ECampaignLessonKind Kind)
{
    const float LessonBump =
        Kind == ECampaignLessonKind::BinarySearch ? 0.10f :
        Kind == ECampaignLessonKind::LinkedListTraverse ? 0.09f :
        Kind == ECampaignLessonKind::FizzBuzz ? 0.08f :
        Kind == ECampaignLessonKind::EvenFilter ? 0.06f :
        Kind == ECampaignLessonKind::Palindrome ? 0.04f :
        0.0f;
    return 0.85f + DifficultyTierForRank(Rank) * 0.13f + LessonBump + (Rank % 5) * 0.015f;
}

FString CurriculumStageForRank(int32 Rank)
{
    if (Rank <= 24)  return TEXT("Stage 1 - Foundations");
    if (Rank <= 84)  return TEXT("Stage 2 - Control Flow");
    if (Rank <= 174) return TEXT("Stage 3 - Collections and Strings");
    if (Rank <= 294) return TEXT("Stage 4 - Data Structures");
    return TEXT("Stage 5 - Algorithmic Search");
}

FString StageGoalForRank(int32 Rank)
{
    if (Rank <= 24)
    {
        return TEXT("Build reliable function signatures, returns, arithmetic, booleans, and short string transforms.");
    }
    if (Rank <= 84)
    {
        return TEXT("Practice branching order, loop rhythm, and visible-versus-hidden test thinking.");
    }
    if (Rank <= 174)
    {
        return TEXT("Move from single values into collections, edge cases, and ordered outputs.");
    }
    if (Rank <= 294)
    {
        return TEXT("Track state across sequences and reason about node-to-node traversal.");
    }
    return TEXT("Use algorithmic invariants to reduce work, especially bounds, sentinels, and sorted data.");
}

FString ArchitectureSignatureFor(const FString& ArtKit, const FString& Landmark, ECampaignLessonKind Kind, int32 Rank)
{
    const FString LessonForm =
        Kind == ECampaignLessonKind::Lock ? TEXT("dual-gate checkpoint") :
        Kind == ECampaignLessonKind::Reverse ? TEXT("backward radio mast") :
        Kind == ECampaignLessonKind::Palindrome ? TEXT("mirrored plaza") :
        Kind == ECampaignLessonKind::FizzBuzz ? TEXT("timed beacon pylons") :
        Kind == ECampaignLessonKind::EvenFilter ? TEXT("triage conveyor lanes") :
        Kind == ECampaignLessonKind::LinkedListTraverse ? TEXT("linked evacuation posts") :
        Kind == ECampaignLessonKind::BinarySearch ? TEXT("halved search vault") :
        TEXT("three-cell power dais");

    return FString::Printf(TEXT("%s with %s around %s, signature %03d"),
        *ArtKit,
        *LessonForm,
        *Landmark,
        Rank);
}

FString NovelGameplayDetailFor(ECampaignLessonKind Kind, int32 Rank)
{
    const int32 Variant = ((Rank - 1) % 4) + 1;
    switch (Kind)
    {
    case ECampaignLessonKind::Lock:
        return FString::Printf(TEXT("Truth-table gate %d: compare four safety states before opening the survivor route."), Variant);
    case ECampaignLessonKind::Reverse:
        return FString::Printf(TEXT("Radio rewind mast %d: visual packets run backward from tower to terminal."), Variant);
    case ECampaignLessonKind::Palindrome:
        return FString::Printf(TEXT("Mirror-code walk %d: paired light posts teach left/right character comparisons."), Variant);
    case ECampaignLessonKind::FizzBuzz:
        return FString::Printf(TEXT("Beacon cycle yard %d: numbered pylons emphasize checking 15 before 3 or 5."), Variant);
    case ECampaignLessonKind::EvenFilter:
        return FString::Printf(TEXT("Triage sorter %d: even-number lanes lead to rescue while odd lanes become decoys."), Variant);
    case ECampaignLessonKind::LinkedListTraverse:
        return FString::Printf(TEXT("Evacuation chain %d: node markers show current, next, and stop sentinel movement."), Variant);
    case ECampaignLessonKind::BinarySearch:
        return FString::Printf(TEXT("Sorted-cache vault %d: shrinking light bands show low, mid, and high bounds."), Variant);
    default:
        return FString::Printf(TEXT("Power-cell puzzle %d: three visible cells feed one returned total."), Variant);
    }
}

FString MakeSlug(const FString& City, const FString& State)
{
    FString Source = City + TEXT("_") + State;
    Source.ToLowerInline();

    FString Slug;
    bool bLastWasSeparator = false;
    for (TCHAR Ch : Source)
    {
        if (FChar::IsAlnum(Ch))
        {
            Slug.AppendChar(Ch);
            bLastWasSeparator = false;
        }
        else if (!bLastWasSeparator)
        {
            Slug.AppendChar(TCHAR('_'));
            bLastWasSeparator = true;
        }
    }

    while (Slug.Len() > 0 && Slug[0] == TCHAR('_'))
    {
        Slug.RemoveAt(0, 1, EAllowShrinking::No);
    }
    while (Slug.Len() > 0 && Slug[Slug.Len() - 1] == TCHAR('_'))
    {
        Slug.RemoveAt(Slug.Len() - 1, 1, EAllowShrinking::No);
    }

    return Slug;
}

ECampaignLessonKind LessonForRank(int32 Rank)
{
    static const ECampaignLessonKind Foundations[] = {
        ECampaignLessonKind::Sum,
        ECampaignLessonKind::Lock,
        ECampaignLessonKind::Sum,
        ECampaignLessonKind::Reverse
    };
    static const ECampaignLessonKind ControlFlow[] = {
        ECampaignLessonKind::Sum,
        ECampaignLessonKind::Lock,
        ECampaignLessonKind::Reverse,
        ECampaignLessonKind::Palindrome,
        ECampaignLessonKind::FizzBuzz
    };
    static const ECampaignLessonKind Collections[] = {
        ECampaignLessonKind::Reverse,
        ECampaignLessonKind::Palindrome,
        ECampaignLessonKind::FizzBuzz,
        ECampaignLessonKind::EvenFilter,
        ECampaignLessonKind::EvenFilter
    };
    static const ECampaignLessonKind DataStructures[] = {
        ECampaignLessonKind::EvenFilter,
        ECampaignLessonKind::LinkedListTraverse,
        ECampaignLessonKind::Palindrome,
        ECampaignLessonKind::BinarySearch,
        ECampaignLessonKind::FizzBuzz
    };
    static const ECampaignLessonKind Algorithms[] = {
        ECampaignLessonKind::LinkedListTraverse,
        ECampaignLessonKind::BinarySearch,
        ECampaignLessonKind::EvenFilter,
        ECampaignLessonKind::FizzBuzz,
        ECampaignLessonKind::Palindrome,
        ECampaignLessonKind::BinarySearch
    };

    const int32 SafeRank = FMath::Max(1, Rank);
    if (SafeRank <= 24)
    {
        return Foundations[(SafeRank - 1) % UE_ARRAY_COUNT(Foundations)];
    }
    if (SafeRank <= 84)
    {
        return ControlFlow[(SafeRank - 25) % UE_ARRAY_COUNT(ControlFlow)];
    }
    if (SafeRank <= 174)
    {
        return Collections[(SafeRank - 85) % UE_ARRAY_COUNT(Collections)];
    }
    if (SafeRank <= 294)
    {
        return DataStructures[(SafeRank - 175) % UE_ARRAY_COUNT(DataStructures)];
    }
    return Algorithms[(SafeRank - 295) % UE_ARRAY_COUNT(Algorithms)];
}

FCodeRescueCityMission MakeMission(int32 Rank, const TCHAR* City, const TCHAR* State)
{
    const ECampaignLessonKind Kind = LessonForRank(Rank);
    const ECodingLanguage RecommendedLanguage = RecommendedLanguageFor(Rank);

    FCodeRescueCityMission Mission;
    Mission.Rank = Rank;
    Mission.CityName = City;
    Mission.StateName = State;
    Mission.Slug = MakeSlug(City, State);
    Mission.TerminalId = FString::Printf(TEXT("%s_%s"), *Mission.Slug, LessonToken(Kind));
    Mission.TerminalTitle = FString::Printf(TEXT("%s: %s"), City, *LessonTitle(Kind));
    Mission.RegionName = RegionForState(State);
    Mission.DistrictStyle = DistrictStyleFor(Mission.RegionName, Kind);
    Mission.LandmarkName = LandmarkFor(City, Kind, Rank);
    Mission.ArtKitName = ArtKitFor(City, State, Mission.RegionName);
    Mission.CurriculumStageName = CurriculumStageForRank(Rank);
    Mission.ArchitectureSignature = ArchitectureSignatureFor(Mission.ArtKitName, Mission.LandmarkName, Kind, Rank);
    Mission.NovelGameplayDetail = NovelGameplayDetailFor(Kind, Rank);
    Mission.LanguageTrackText = LanguageTrackFor(RecommendedLanguage, Kind, Rank);
    Mission.LearningSupportText = LearningSupportFor(Kind, Rank);
    Mission.VisualDebuggerPlan = VisualDebuggerFor(Kind);
    Mission.ProgressionPlan = ProgressionPlanFor(Kind, Rank);
    Mission.CharacterStoryPlan = CharacterStoryPlanFor(Kind, City);
    Mission.GameplayFlowPlan = GameplayFlowPlanFor(Rank);
    Mission.AccessibilityPolishPlan = AccessibilityPolishPlanFor(Mission.RegionName);
    Mission.QAVerificationPlan = QAVerificationPlanFor(Rank);
    Mission.HintText = HintFor(Kind);
    Mission.VisibleTestBrief = VisibleTestFor(Kind);
    Mission.HiddenTestBrief = HiddenTestFor(Kind, Rank);
    Mission.CurriculumFocus = FString::Printf(
        TEXT("%s. Curriculum focus: %s. Stage goal: %s Graduation strategy: %s. Recommended first pass: %s. %s"),
        *Mission.CurriculumStageName,
        *LessonConcept(Kind),
        *StageGoalForRank(Rank),
        *LessonStrategy(Kind),
        *LanguageName(RecommendedLanguage),
        *Mission.HintText);
    Mission.RadioBriefing = FString::Printf(
        TEXT("Radio briefing: %s, %s is stop %03d on the national route. Secure the %s in the %s, finish the %s lesson, then extract the survivor team. New city feature: %s"),
        City,
        State,
        Rank,
        *Mission.LandmarkName,
        *Mission.ArtKitName,
        *LessonConcept(Kind),
        *Mission.NovelGameplayDetail);
    Mission.RadioVoiceName = RadioVoiceFor(Mission.RegionName);
    Mission.MissionBrief = LessonBrief(City, Kind)
        + TEXT("\n\n") + Mission.CurriculumFocus
        + TEXT("\nArchitecture: ") + Mission.ArchitectureSignature
        + TEXT("\nPlay detail: ") + Mission.NovelGameplayDetail
        + TEXT("\nLanguage track: ") + Mission.LanguageTrackText
        + TEXT("\nLearning support: ") + Mission.LearningSupportText
        + TEXT("\n") + Mission.VisualDebuggerPlan
        + TEXT("\n") + Mission.ProgressionPlan
        + TEXT("\n") + Mission.CharacterStoryPlan
        + TEXT("\n") + Mission.GameplayFlowPlan
        + TEXT("\n") + Mission.AccessibilityPolishPlan
        + TEXT("\n") + Mission.QAVerificationPlan
        + TEXT("\n") + Mission.VisibleTestBrief
        + TEXT("\n") + Mission.HiddenTestBrief;
    Mission.SurvivorName = FString::Printf(TEXT("%s Survivor Team"), City);
    Mission.LessonKind = Kind;
    Mission.RecommendedLanguage = RecommendedLanguage;
    Mission.AccentColor = AccentFor(Rank, Kind);
    Mission.SecondaryAccentColor = SecondaryAccentFor(Mission.RegionName, Kind);
    Mission.DifficultyTier = DifficultyTierForRank(Rank);
    Mission.EncounterIntensity = EncounterIntensityForRank(Rank, Kind);
    Mission.SkylineSeed = GetTypeHash(Mission.Slug) ^ (Rank * 7919);
    return Mission;
}
}

const TArray<FCodeRescueCityMission>& FCodeRescueCampaign::GetMissions()
{
    struct FCityRow
    {
        int32 Rank;
        const TCHAR* City;
        const TCHAR* State;
    };

    static const FCityRow CityRows[] = {
        {   1, TEXT("New York"),                    TEXT("NY") },
        {   2, TEXT("Los Angeles"),                 TEXT("CA") },
        {   3, TEXT("Chicago"),                     TEXT("IL") },
        {   4, TEXT("Houston"),                     TEXT("TX") },
        {   5, TEXT("Phoenix"),                     TEXT("AZ") },
        {   6, TEXT("Philadelphia"),                TEXT("PA") },
        {   7, TEXT("San Antonio"),                 TEXT("TX") },
        {   8, TEXT("San Diego"),                   TEXT("CA") },
        {   9, TEXT("Dallas"),                      TEXT("TX") },
        {  10, TEXT("Jacksonville"),                TEXT("FL") },
        {  11, TEXT("Fort Worth"),                  TEXT("TX") },
        {  12, TEXT("San Jose"),                    TEXT("CA") },
        {  13, TEXT("Austin"),                      TEXT("TX") },
        {  14, TEXT("Charlotte"),                   TEXT("NC") },
        {  15, TEXT("Columbus"),                    TEXT("OH") },
        {  16, TEXT("Indianapolis"),                TEXT("IN") },
        {  17, TEXT("San Francisco"),               TEXT("CA") },
        {  18, TEXT("Seattle"),                     TEXT("WA") },
        {  19, TEXT("Denver"),                      TEXT("CO") },
        {  20, TEXT("Oklahoma City"),               TEXT("OK") },
        {  21, TEXT("Nashville-Davidson"),          TEXT("TN") },
        {  22, TEXT("Washington"),                  TEXT("DC") },
        {  23, TEXT("El Paso"),                     TEXT("TX") },
        {  24, TEXT("Las Vegas"),                   TEXT("NV") },
        {  25, TEXT("Boston"),                      TEXT("MA") },
        {  26, TEXT("Detroit"),                     TEXT("MI") },
        {  27, TEXT("Louisville/Jefferson County"), TEXT("KY") },
        {  28, TEXT("Portland"),                    TEXT("OR") },
        {  29, TEXT("Memphis"),                     TEXT("TN") },
        {  30, TEXT("Baltimore"),                   TEXT("MD") },
        {  31, TEXT("Milwaukee"),                   TEXT("WI") },
        {  32, TEXT("Albuquerque"),                 TEXT("NM") },
        {  33, TEXT("Tucson"),                      TEXT("AZ") },
        {  34, TEXT("Fresno"),                      TEXT("CA") },
        {  35, TEXT("Sacramento"),                  TEXT("CA") },
        {  36, TEXT("Atlanta"),                     TEXT("GA") },
        {  37, TEXT("Mesa"),                        TEXT("AZ") },
        {  38, TEXT("Kansas City"),                 TEXT("MO") },
        {  39, TEXT("Raleigh"),                     TEXT("NC") },
        {  40, TEXT("Colorado Springs"),            TEXT("CO") },
        {  41, TEXT("Omaha"),                       TEXT("NE") },
        {  42, TEXT("Miami"),                       TEXT("FL") },
        {  43, TEXT("Virginia Beach"),              TEXT("VA") },
        {  44, TEXT("Long Beach"),                  TEXT("CA") },
        {  45, TEXT("Oakland"),                     TEXT("CA") },
        {  46, TEXT("Minneapolis"),                 TEXT("MN") },
        {  47, TEXT("Bakersfield"),                 TEXT("CA") },
        {  48, TEXT("Tulsa"),                       TEXT("OK") },
        {  49, TEXT("Tampa"),                       TEXT("FL") },
        {  50, TEXT("Arlington"),                   TEXT("TX") },
        {  51, TEXT("Aurora"),                      TEXT("CO") },
        {  52, TEXT("Wichita"),                     TEXT("KS") },
        {  53, TEXT("Cleveland"),                   TEXT("OH") },
        {  54, TEXT("New Orleans"),                 TEXT("LA") },
        {  55, TEXT("Henderson"),                   TEXT("NV") },
        {  56, TEXT("Urban Honolulu"),              TEXT("HI") },
        {  57, TEXT("Anaheim"),                     TEXT("CA") },
        {  58, TEXT("Orlando"),                     TEXT("FL") },
        {  59, TEXT("Lexington-Fayette"),           TEXT("KY") },
        {  60, TEXT("Stockton"),                    TEXT("CA") },
        {  61, TEXT("Riverside"),                   TEXT("CA") },
        {  62, TEXT("Irvine"),                      TEXT("CA") },
        {  63, TEXT("Corpus Christi"),              TEXT("TX") },
        {  64, TEXT("Newark"),                      TEXT("NJ") },
        {  65, TEXT("Santa Ana"),                   TEXT("CA") },
        {  66, TEXT("Cincinnati"),                  TEXT("OH") },
        {  67, TEXT("Pittsburgh"),                  TEXT("PA") },
        {  68, TEXT("St. Paul"),                    TEXT("MN") },
        {  69, TEXT("Greensboro"),                  TEXT("NC") },
        {  70, TEXT("Jersey City"),                 TEXT("NJ") },
        {  71, TEXT("Durham"),                      TEXT("NC") },
        {  72, TEXT("Lincoln"),                     TEXT("NE") },
        {  73, TEXT("North Las Vegas"),             TEXT("NV") },
        {  74, TEXT("Plano"),                       TEXT("TX") },
        {  75, TEXT("Anchorage"),                   TEXT("AK") },
        {  76, TEXT("Gilbert"),                     TEXT("AZ") },
        {  77, TEXT("Madison"),                     TEXT("WI") },
        {  78, TEXT("Reno"),                        TEXT("NV") },
        {  79, TEXT("Chandler"),                    TEXT("AZ") },
        {  80, TEXT("St. Louis"),                   TEXT("MO") },
        {  81, TEXT("Chula Vista"),                 TEXT("CA") },
        {  82, TEXT("Buffalo"),                     TEXT("NY") },
        {  83, TEXT("Fort Wayne"),                  TEXT("IN") },
        {  84, TEXT("Lubbock"),                     TEXT("TX") },
        {  85, TEXT("St. Petersburg"),              TEXT("FL") },
        {  86, TEXT("Toledo"),                      TEXT("OH") },
        {  87, TEXT("Laredo"),                      TEXT("TX") },
        {  88, TEXT("Port St. Lucie"),              TEXT("FL") },
        {  89, TEXT("Glendale"),                    TEXT("AZ") },
        {  90, TEXT("Irving"),                      TEXT("TX") },
        {  91, TEXT("Winston-Salem"),               TEXT("NC") },
        {  92, TEXT("Chesapeake"),                  TEXT("VA") },
        {  93, TEXT("Garland"),                     TEXT("TX") },
        {  94, TEXT("Scottsdale"),                  TEXT("AZ") },
        {  95, TEXT("Boise City"),                  TEXT("ID") },
        {  96, TEXT("Hialeah"),                     TEXT("FL") },
        {  97, TEXT("Frisco"),                      TEXT("TX") },
        {  98, TEXT("Richmond"),                    TEXT("VA") },
        {  99, TEXT("Cape Coral"),                  TEXT("FL") },
        { 100, TEXT("Norfolk"),                     TEXT("VA") },
        { 101, TEXT("Spokane"),                     TEXT("WA") },
        { 102, TEXT("Huntsville"),                  TEXT("AL") },
        { 103, TEXT("Santa Clarita"),               TEXT("CA") },
        { 104, TEXT("Tacoma"),                      TEXT("WA") },
        { 105, TEXT("Fremont"),                     TEXT("CA") },
        { 106, TEXT("McKinney"),                    TEXT("TX") },
        { 107, TEXT("San Bernardino"),              TEXT("CA") },
        { 108, TEXT("Baton Rouge"),                 TEXT("LA") },
        { 109, TEXT("Modesto"),                     TEXT("CA") },
        { 110, TEXT("Fontana"),                     TEXT("CA") },
        { 111, TEXT("Salt Lake City"),              TEXT("UT") },
        { 112, TEXT("Moreno Valley"),               TEXT("CA") },
        { 113, TEXT("Des Moines"),                  TEXT("IA") },
        { 114, TEXT("Worcester"),                   TEXT("MA") },
        { 115, TEXT("Yonkers"),                     TEXT("NY") },
        { 116, TEXT("Fayetteville"),                TEXT("NC") },
        { 117, TEXT("Sioux Falls"),                 TEXT("SD") },
        { 118, TEXT("Grand Prairie"),               TEXT("TX") },
        { 119, TEXT("Rochester"),                   TEXT("NY") },
        { 120, TEXT("Tallahassee"),                 TEXT("FL") },
        { 121, TEXT("Little Rock"),                 TEXT("AR") },
        { 122, TEXT("Amarillo"),                    TEXT("TX") },
        { 123, TEXT("Overland Park"),               TEXT("KS") },
        { 124, TEXT("Columbus"),                    TEXT("GA") },
        { 125, TEXT("Augusta-Richmond County"),     TEXT("GA") },
        { 126, TEXT("Mobile"),                      TEXT("AL") },
        { 127, TEXT("Oxnard"),                      TEXT("CA") },
        { 128, TEXT("Grand Rapids"),                TEXT("MI") },
        { 129, TEXT("Peoria"),                      TEXT("AZ") },
        { 130, TEXT("Vancouver"),                   TEXT("WA") },
        { 131, TEXT("Knoxville"),                   TEXT("TN") },
        { 132, TEXT("Birmingham"),                  TEXT("AL") },
        { 133, TEXT("Montgomery"),                  TEXT("AL") },
        { 134, TEXT("Providence"),                  TEXT("RI") },
        { 135, TEXT("Huntington Beach"),            TEXT("CA") },
        { 136, TEXT("Brownsville"),                 TEXT("TX") },
        { 137, TEXT("Chattanooga"),                 TEXT("TN") },
        { 138, TEXT("Fort Lauderdale"),             TEXT("FL") },
        { 139, TEXT("Tempe"),                       TEXT("AZ") },
        { 140, TEXT("Akron"),                       TEXT("OH") },
        { 141, TEXT("Glendale"),                    TEXT("CA") },
        { 142, TEXT("Clarksville"),                 TEXT("TN") },
        { 143, TEXT("Ontario"),                     TEXT("CA") },
        { 144, TEXT("Newport News"),                TEXT("VA") },
        { 145, TEXT("Elk Grove"),                   TEXT("CA") },
        { 146, TEXT("Cary"),                        TEXT("NC") },
        { 147, TEXT("Aurora"),                      TEXT("IL") },
        { 148, TEXT("Salem"),                       TEXT("OR") },
        { 149, TEXT("Pembroke Pines"),              TEXT("FL") },
        { 150, TEXT("Eugene"),                      TEXT("OR") },
        { 151, TEXT("Santa Rosa"),                  TEXT("CA") },
        { 152, TEXT("Rancho Cucamonga"),            TEXT("CA") },
        { 153, TEXT("Shreveport"),                  TEXT("LA") },
        { 154, TEXT("Garden Grove"),                TEXT("CA") },
        { 155, TEXT("Oceanside"),                   TEXT("CA") },
        { 156, TEXT("Fort Collins"),                TEXT("CO") },
        { 157, TEXT("Springfield"),                 TEXT("MO") },
        { 158, TEXT("Murfreesboro"),                TEXT("TN") },
        { 159, TEXT("Surprise"),                    TEXT("AZ") },
        { 160, TEXT("Lancaster"),                   TEXT("CA") },
        { 161, TEXT("Denton"),                      TEXT("TX") },
        { 162, TEXT("Roseville"),                   TEXT("CA") },
        { 163, TEXT("Palmdale"),                    TEXT("CA") },
        { 164, TEXT("Corona"),                      TEXT("CA") },
        { 165, TEXT("Salinas"),                     TEXT("CA") },
        { 166, TEXT("Killeen"),                     TEXT("TX") },
        { 167, TEXT("Paterson"),                    TEXT("NJ") },
        { 168, TEXT("Alexandria"),                  TEXT("VA") },
        { 169, TEXT("Hollywood"),                   TEXT("FL") },
        { 170, TEXT("Hayward"),                     TEXT("CA") },
        { 171, TEXT("Charleston"),                  TEXT("SC") },
        { 172, TEXT("Macon-Bibb County"),           TEXT("GA") },
        { 173, TEXT("Lakewood"),                    TEXT("CO") },
        { 174, TEXT("Sunnyvale"),                   TEXT("CA") },
        { 175, TEXT("Kansas City"),                 TEXT("KS") },
        { 176, TEXT("Springfield"),                 TEXT("MA") },
        { 177, TEXT("Bellevue"),                    TEXT("WA") },
        { 178, TEXT("Naperville"),                  TEXT("IL") },
        { 179, TEXT("Joliet"),                      TEXT("IL") },
        { 180, TEXT("Bridgeport"),                  TEXT("CT") },
        { 181, TEXT("Mesquite"),                    TEXT("TX") },
        { 182, TEXT("Pasadena"),                    TEXT("TX") },
        { 183, TEXT("Olathe"),                      TEXT("KS") },
        { 184, TEXT("Escondido"),                   TEXT("CA") },
        { 185, TEXT("Savannah"),                    TEXT("GA") },
        { 186, TEXT("McAllen"),                     TEXT("TX") },
        { 187, TEXT("Gainesville"),                 TEXT("FL") },
        { 188, TEXT("Pomona"),                      TEXT("CA") },
        { 189, TEXT("Rockford"),                    TEXT("IL") },
        { 190, TEXT("Thornton"),                    TEXT("CO") },
        { 191, TEXT("Waco"),                        TEXT("TX") },
        { 192, TEXT("Visalia"),                     TEXT("CA") },
        { 193, TEXT("Syracuse"),                    TEXT("NY") },
        { 194, TEXT("Columbia"),                    TEXT("SC") },
        { 195, TEXT("Midland"),                     TEXT("TX") },
        { 196, TEXT("Miramar"),                     TEXT("FL") },
        { 197, TEXT("Palm Bay"),                    TEXT("FL") },
        { 198, TEXT("Jackson"),                     TEXT("MS") },
        { 199, TEXT("Coral Springs"),               TEXT("FL") },
        { 200, TEXT("Victorville"),                 TEXT("CA") },
        { 201, TEXT("Elizabeth"),                   TEXT("NJ") },
        { 202, TEXT("Fullerton"),                   TEXT("CA") },
        { 203, TEXT("Meridian"),                    TEXT("ID") },
        { 204, TEXT("Torrance"),                    TEXT("CA") },
        { 205, TEXT("Stamford"),                    TEXT("CT") },
        { 206, TEXT("West Valley City"),            TEXT("UT") },
        { 207, TEXT("Orange"),                      TEXT("CA") },
        { 208, TEXT("Cedar Rapids"),                TEXT("IA") },
        { 209, TEXT("Warren"),                      TEXT("MI") },
        { 210, TEXT("Hampton"),                     TEXT("VA") },
        { 211, TEXT("New Haven"),                   TEXT("CT") },
        { 212, TEXT("Pasadena"),                    TEXT("CA") },
        { 213, TEXT("Kent"),                        TEXT("WA") },
        { 214, TEXT("Dayton"),                      TEXT("OH") },
        { 215, TEXT("Fargo"),                       TEXT("ND") },
        { 216, TEXT("Lewisville"),                  TEXT("TX") },
        { 217, TEXT("Carrollton"),                  TEXT("TX") },
        { 218, TEXT("Round Rock"),                  TEXT("TX") },
        { 219, TEXT("Sterling Heights"),            TEXT("MI") },
        { 220, TEXT("Santa Clara"),                 TEXT("CA") },
        { 221, TEXT("Norman"),                      TEXT("OK") },
        { 222, TEXT("Columbia"),                    TEXT("MO") },
        { 223, TEXT("Abilene"),                     TEXT("TX") },
        { 224, TEXT("Pearland"),                    TEXT("TX") },
        { 225, TEXT("Athens-Clarke County"),        TEXT("GA") },
        { 226, TEXT("College Station"),             TEXT("TX") },
        { 227, TEXT("Clovis"),                      TEXT("CA") },
        { 228, TEXT("West Palm Beach"),             TEXT("FL") },
        { 229, TEXT("Allentown"),                   TEXT("PA") },
        { 230, TEXT("North Charleston"),            TEXT("SC") },
        { 231, TEXT("Simi Valley"),                 TEXT("CA") },
        { 232, TEXT("Topeka"),                      TEXT("KS") },
        { 233, TEXT("Wilmington"),                  TEXT("NC") },
        { 234, TEXT("Lakeland"),                    TEXT("FL") },
        { 235, TEXT("Thousand Oaks"),               TEXT("CA") },
        { 236, TEXT("Concord"),                     TEXT("CA") },
        { 237, TEXT("Rochester"),                   TEXT("MN") },
        { 238, TEXT("Vallejo"),                     TEXT("CA") },
        { 239, TEXT("Ann Arbor"),                   TEXT("MI") },
        { 240, TEXT("Broken Arrow"),                TEXT("OK") },
        { 241, TEXT("Fairfield"),                   TEXT("CA") },
        { 242, TEXT("Lafayette"),                   TEXT("LA") },
        { 243, TEXT("Hartford"),                    TEXT("CT") },
        { 244, TEXT("Arvada"),                      TEXT("CO") },
        { 245, TEXT("Berkeley"),                    TEXT("CA") },
        { 246, TEXT("Independence"),                TEXT("MO") },
        { 247, TEXT("Billings"),                    TEXT("MT") },
        { 248, TEXT("Cambridge"),                   TEXT("MA") },
        { 249, TEXT("Lowell"),                      TEXT("MA") },
        { 250, TEXT("Odessa"),                      TEXT("TX") },
        { 251, TEXT("High Point"),                  TEXT("NC") },
        { 252, TEXT("League City"),                 TEXT("TX") },
        { 253, TEXT("Antioch"),                     TEXT("CA") },
        { 254, TEXT("Richardson"),                  TEXT("TX") },
        { 255, TEXT("Goodyear"),                    TEXT("AZ") },
        { 256, TEXT("Pompano Beach"),               TEXT("FL") },
        { 257, TEXT("Nampa"),                       TEXT("ID") },
        { 258, TEXT("Menifee"),                     TEXT("CA") },
        { 259, TEXT("Las Cruces"),                  TEXT("NM") },
        { 260, TEXT("Clearwater"),                  TEXT("FL") },
        { 261, TEXT("West Jordan"),                 TEXT("UT") },
        { 262, TEXT("New Braunfels"),               TEXT("TX") },
        { 263, TEXT("Manchester"),                  TEXT("NH") },
        { 264, TEXT("Miami Gardens"),               TEXT("FL") },
        { 265, TEXT("Waterbury"),                   TEXT("CT") },
        { 266, TEXT("Provo"),                       TEXT("UT") },
        { 267, TEXT("Evansville"),                  TEXT("IN") },
        { 268, TEXT("Richmond"),                    TEXT("CA") },
        { 269, TEXT("Westminster"),                 TEXT("CO") },
        { 270, TEXT("Elgin"),                       TEXT("IL") },
        { 271, TEXT("Conroe"),                      TEXT("TX") },
        { 272, TEXT("Greeley"),                     TEXT("CO") },
        { 273, TEXT("Lansing"),                     TEXT("MI") },
        { 274, TEXT("Buckeye"),                     TEXT("AZ") },
        { 275, TEXT("Tuscaloosa"),                  TEXT("AL") },
        { 276, TEXT("Allen"),                       TEXT("TX") },
        { 277, TEXT("Carlsbad"),                    TEXT("CA") },
        { 278, TEXT("Everett"),                     TEXT("WA") },
        { 279, TEXT("Springfield"),                 TEXT("IL") },
        { 280, TEXT("Beaumont"),                    TEXT("TX") },
        { 281, TEXT("Murrieta"),                    TEXT("CA") },
        { 282, TEXT("Rio Rancho"),                  TEXT("NM") },
        { 283, TEXT("Temecula"),                    TEXT("CA") },
        { 284, TEXT("Concord"),                     TEXT("NC") },
        { 285, TEXT("Tyler"),                       TEXT("TX") },
        { 286, TEXT("Davie"),                       TEXT("FL") },
        { 287, TEXT("South Fulton"),                TEXT("GA") },
        { 288, TEXT("Peoria"),                      TEXT("IL") },
        { 289, TEXT("Sparks"),                      TEXT("NV") },
        { 290, TEXT("Gresham"),                     TEXT("OR") },
        { 291, TEXT("Santa Maria"),                 TEXT("CA") },
        { 292, TEXT("Pueblo"),                      TEXT("CO") },
        { 293, TEXT("Hillsboro"),                   TEXT("OR") },
        { 294, TEXT("Sugar Land"),                  TEXT("TX") },
        { 295, TEXT("San Buenaventura (Ventura)"),  TEXT("CA") },
        { 296, TEXT("Downey"),                      TEXT("CA") },
        { 297, TEXT("Costa Mesa"),                  TEXT("CA") },
        { 298, TEXT("Centennial"),                  TEXT("CO") },
        { 299, TEXT("Edinburg"),                    TEXT("TX") },
        { 300, TEXT("Spokane Valley"),              TEXT("WA") },
        { 301, TEXT("Jurupa Valley"),               TEXT("CA") },
        { 302, TEXT("Bend"),                        TEXT("OR") },
        { 303, TEXT("West Covina"),                 TEXT("CA") },
        { 304, TEXT("Boulder"),                     TEXT("CO") },
        { 305, TEXT("Palm Coast"),                  TEXT("FL") },
        { 306, TEXT("Lee's Summit"),                TEXT("MO") },
        { 307, TEXT("Dearborn"),                    TEXT("MI") },
        { 308, TEXT("Green Bay"),                   TEXT("WI") },
        { 309, TEXT("St. George"),                  TEXT("UT") },
        { 310, TEXT("Brockton"),                    TEXT("MA") },
        { 311, TEXT("Renton"),                      TEXT("WA") },
        { 312, TEXT("Sandy Springs"),               TEXT("GA") },
        { 313, TEXT("Rialto"),                      TEXT("CA") },
        { 314, TEXT("El Monte"),                    TEXT("CA") },
        { 315, TEXT("Vacaville"),                   TEXT("CA") },
        { 316, TEXT("Fishers"),                     TEXT("IN") },
        { 317, TEXT("South Bend"),                  TEXT("IN") },
        { 318, TEXT("Carmel"),                      TEXT("IN") },
        { 319, TEXT("Yuma"),                        TEXT("AZ") },
        { 320, TEXT("Burbank"),                     TEXT("CA") },
        { 321, TEXT("Lynn"),                        TEXT("MA") },
        { 322, TEXT("Quincy"),                      TEXT("MA") },
        { 323, TEXT("El Cajon"),                    TEXT("CA") },
        { 324, TEXT("Fayetteville"),                TEXT("AR") },
        { 325, TEXT("Suffolk"),                     TEXT("VA") },
        { 326, TEXT("San Mateo"),                   TEXT("CA") },
        { 327, TEXT("Chico"),                       TEXT("CA") },
        { 328, TEXT("Inglewood"),                   TEXT("CA") },
        { 329, TEXT("Wichita Falls"),               TEXT("TX") },
        { 330, TEXT("Boca Raton"),                  TEXT("FL") },
        { 331, TEXT("Hesperia"),                    TEXT("CA") },
        { 332, TEXT("Daly City"),                   TEXT("CA") },
        { 333, TEXT("Georgetown"),                  TEXT("TX") },
        { 334, TEXT("New Bedford"),                 TEXT("MA") },
        { 335, TEXT("Albany"),                      TEXT("NY") },
        { 336, TEXT("Davenport"),                   TEXT("IA") },
        { 337, TEXT("Plantation"),                  TEXT("FL") },
        { 338, TEXT("Deltona"),                     TEXT("FL") },
        { 339, TEXT("Federal Way"),                 TEXT("WA") },
        { 340, TEXT("San Angelo"),                  TEXT("TX") },
        { 341, TEXT("Tracy"),                       TEXT("CA") },
        { 342, TEXT("Sunrise"),                     TEXT("FL") },

        { 343, TEXT("Tokyo"),                       TEXT("JP") },
        { 344, TEXT("Delhi"),                       TEXT("IN") },
        { 345, TEXT("Shanghai"),                    TEXT("CN") },
        { 346, TEXT("Sao Paulo"),                   TEXT("BR") },
        { 347, TEXT("Mexico City"),                 TEXT("MX") },
        { 348, TEXT("Cairo"),                       TEXT("EG") },
        { 349, TEXT("Mumbai"),                      TEXT("IN") },
        { 350, TEXT("Beijing"),                     TEXT("CN") },
        { 351, TEXT("Dhaka"),                       TEXT("BD") },
        { 352, TEXT("Osaka"),                       TEXT("JP") },
        { 353, TEXT("Karachi"),                     TEXT("PK") },
        { 354, TEXT("Buenos Aires"),                TEXT("AR") },
        { 355, TEXT("Istanbul"),                    TEXT("TR") },
        { 356, TEXT("Kolkata"),                     TEXT("IN") },
        { 357, TEXT("Manila"),                      TEXT("PH") },
        { 358, TEXT("Lagos"),                       TEXT("NG") },
        { 359, TEXT("Rio de Janeiro"),              TEXT("BR") },
        { 360, TEXT("Tianjin"),                     TEXT("CN") },
        { 361, TEXT("Kinshasa"),                    TEXT("CD") },
        { 362, TEXT("Guangzhou"),                   TEXT("CN") },
        { 363, TEXT("Shenzhen"),                    TEXT("CN") },
        { 364, TEXT("Lahore"),                      TEXT("PK") },
        { 365, TEXT("Bangalore"),                   TEXT("IN") },
        { 366, TEXT("Paris"),                       TEXT("FR") },
        { 367, TEXT("Bogota"),                      TEXT("CO") },
        { 368, TEXT("Jakarta"),                     TEXT("ID") },
        { 369, TEXT("Chennai"),                     TEXT("IN") },
        { 370, TEXT("Lima"),                        TEXT("PE") },
        { 371, TEXT("Bangkok"),                     TEXT("TH") },
        { 372, TEXT("Seoul"),                       TEXT("KR") },
        { 373, TEXT("Nagoya"),                      TEXT("JP") },
        { 374, TEXT("Hyderabad"),                   TEXT("IN") },
        { 375, TEXT("London"),                      TEXT("GB") },
        { 376, TEXT("Tehran"),                      TEXT("IR") },
        { 377, TEXT("Chengdu"),                     TEXT("CN") },
        { 378, TEXT("Nanjing"),                     TEXT("CN") },
        { 379, TEXT("Wuhan"),                       TEXT("CN") },
        { 380, TEXT("Ho Chi Minh City"),            TEXT("VN") },
        { 381, TEXT("Luanda"),                      TEXT("AO") },
        { 382, TEXT("Ahmedabad"),                   TEXT("IN") },
        { 383, TEXT("Kuala Lumpur"),                TEXT("MY") },
        { 384, TEXT("Xi'an"),                       TEXT("CN") },
        { 385, TEXT("Hong Kong"),                   TEXT("HK") },
        { 386, TEXT("Hangzhou"),                    TEXT("CN") },
        { 387, TEXT("Foshan"),                      TEXT("CN") },
        { 388, TEXT("Shenyang"),                    TEXT("CN") },
        { 389, TEXT("Riyadh"),                      TEXT("SA") },
        { 390, TEXT("Baghdad"),                     TEXT("IQ") },
        { 391, TEXT("Santiago"),                    TEXT("CL") },
        { 392, TEXT("Surat"),                       TEXT("IN") },
        { 393, TEXT("Madrid"),                      TEXT("ES") },
        { 394, TEXT("Toronto"),                     TEXT("CA-ON") },
        { 395, TEXT("Belo Horizonte"),              TEXT("BR") },
        { 396, TEXT("Singapore"),                   TEXT("SG") },
        { 397, TEXT("Barcelona"),                   TEXT("ES") },
        { 398, TEXT("Alexandria"),                  TEXT("EG") },
        { 399, TEXT("Casablanca"),                  TEXT("MA") },
        { 400, TEXT("Melbourne"),                   TEXT("AU") },
        { 401, TEXT("Sydney"),                      TEXT("AU") },
        { 402, TEXT("Berlin"),                      TEXT("DE") },
        { 403, TEXT("Rome"),                        TEXT("IT") },
        { 404, TEXT("Dubai"),                       TEXT("AE") },
        { 405, TEXT("Abu Dhabi"),                   TEXT("AE") },
        { 406, TEXT("Nairobi"),                     TEXT("KE") },
        { 407, TEXT("Johannesburg"),                TEXT("ZA") },
        { 408, TEXT("Cape Town"),                   TEXT("ZA") },
        { 409, TEXT("Addis Ababa"),                 TEXT("ET") },
        { 410, TEXT("Dar es Salaam"),               TEXT("TZ") },
        { 411, TEXT("Accra"),                       TEXT("GH") },
        { 412, TEXT("Abidjan"),                     TEXT("CI") },
        { 413, TEXT("Algiers"),                     TEXT("DZ") },
        { 414, TEXT("Tunis"),                       TEXT("TN") },
        { 415, TEXT("Rabat"),                       TEXT("MA") },
        { 416, TEXT("Montreal"),                    TEXT("CA-QC") },
        { 417, TEXT("Vancouver"),                   TEXT("CA-BC") },
        { 418, TEXT("Calgary"),                     TEXT("CA-AB") },
        { 419, TEXT("Ottawa"),                      TEXT("CA-ON") },
        { 420, TEXT("Havana"),                      TEXT("CU") },
        { 421, TEXT("Santo Domingo"),               TEXT("DO") },
        { 422, TEXT("Guatemala City"),              TEXT("GT") },
        { 423, TEXT("San Salvador"),                TEXT("SV") },
        { 424, TEXT("Panama City"),                 TEXT("PA") },
        { 425, TEXT("Quito"),                       TEXT("EC") },
        { 426, TEXT("Caracas"),                     TEXT("VE") },
        { 427, TEXT("Medellin"),                    TEXT("CO") },
        { 428, TEXT("Montevideo"),                  TEXT("UY") },
        { 429, TEXT("La Paz"),                      TEXT("BO") },
        { 430, TEXT("Asuncion"),                    TEXT("PY") },
        { 431, TEXT("Brasilia"),                    TEXT("BR") },
        { 432, TEXT("Recife"),                      TEXT("BR") },
        { 433, TEXT("Salvador"),                    TEXT("BR") },
        { 434, TEXT("Fortaleza"),                   TEXT("BR") },
        { 435, TEXT("Manaus"),                      TEXT("BR") },
        { 436, TEXT("Curitiba"),                    TEXT("BR") },
        { 437, TEXT("Porto Alegre"),                TEXT("BR") },
        { 438, TEXT("Yokohama"),                    TEXT("JP") },
        { 439, TEXT("Fukuoka"),                     TEXT("JP") },
        { 440, TEXT("Sapporo"),                     TEXT("JP") },
        { 441, TEXT("Taipei"),                      TEXT("TW") },
        { 442, TEXT("Kaohsiung"),                   TEXT("TW") },
        { 443, TEXT("Quezon City"),                 TEXT("PH") },
        { 444, TEXT("Davao City"),                  TEXT("PH") },
        { 445, TEXT("Hanoi"),                       TEXT("VN") },
        { 446, TEXT("Phnom Penh"),                  TEXT("KH") },
        { 447, TEXT("Yangon"),                      TEXT("MM") },
        { 448, TEXT("Mandalay"),                    TEXT("MM") },
        { 449, TEXT("Vientiane"),                   TEXT("LAO") },
        { 450, TEXT("Bandung"),                     TEXT("ID") },
        { 451, TEXT("Surabaya"),                    TEXT("ID") },
        { 452, TEXT("Medan"),                       TEXT("ID") },
        { 453, TEXT("Denpasar"),                    TEXT("ID") },
        { 454, TEXT("Auckland"),                    TEXT("NZ") },
        { 455, TEXT("Warsaw"),                      TEXT("PL") },
        { 456, TEXT("Amsterdam"),                   TEXT("NL") },
        { 457, TEXT("Lisbon"),                      TEXT("PT") },
        { 458, TEXT("Rotterdam"),                   TEXT("NL") },
        { 459, TEXT("Manchester"),                  TEXT("GB") },
        { 460, TEXT("Birmingham"),                  TEXT("GB") },
        { 461, TEXT("Saint Petersburg"),            TEXT("RU") },
        { 462, TEXT("Moscow"),                      TEXT("RU") },
        { 463, TEXT("Kyiv"),                        TEXT("UA") },
        { 464, TEXT("Prague"),                      TEXT("CZ") },
        { 465, TEXT("Vienna"),                      TEXT("AT") }
    };

    static const TArray<FCodeRescueCityMission> Missions = [&]()
    {
        TArray<FCodeRescueCityMission> Built;
        Built.Reserve(UE_ARRAY_COUNT(CityRows));
        for (const FCityRow& Row : CityRows)
        {
            Built.Add(MakeMission(Row.Rank, Row.City, Row.State));
        }
        return Built;
    }();

    return Missions;
}

int32 FCodeRescueCampaign::GetMissionCount()
{
    return GetMissions().Num();
}

const FCodeRescueCityMission* FCodeRescueCampaign::GetMission(int32 Index)
{
    const TArray<FCodeRescueCityMission>& Missions = GetMissions();
    return Missions.IsValidIndex(Index) ? &Missions[Index] : nullptr;
}

float FCodeRescueCampaign::GetCitySpanScale()
{
    // The city was previously expanded 50x, which scattered every terminal,
    // survivor, and NPC kilometres apart — the player spawned ~320 m from the
    // first objective and the world read as an empty endless field. A modest
    // 2x keeps each city compact and walkable: the whole objective route,
    // its characters, and set-pieces sit within a ~130 m area the player can
    // see and reach. Everything (offsets, extents, origins, player start,
    // nav bounds) scales through this one function, so this stays consistent.
    return 2.0f;
}

FVector FCodeRescueCampaign::ScaleCityOffset(const FVector& Offset)
{
    const float SpanScale = GetCitySpanScale();
    return FVector(Offset.X * SpanScale, Offset.Y * SpanScale, Offset.Z);
}

FVector FCodeRescueCampaign::ScaleCityExtent(const FVector& Extent)
{
    const float SpanScale = GetCitySpanScale();
    return FVector(Extent.X * SpanScale, Extent.Y * SpanScale, Extent.Z);
}

FVector FCodeRescueCampaign::GetCityOrigin(int32 Index)
{
    const int32 Columns = 19;
    const float XSpacing = 11500.0f * GetCitySpanScale();
    const float YSpacing = 10000.0f * GetCitySpanScale();
    const int32 Row = FMath::Max(0, Index) / Columns;
    const int32 Col = FMath::Max(0, Index) % Columns;
    return FVector(Col * XSpacing, Row * YSpacing, 0.0f);
}

FVector FCodeRescueCampaign::GetPlayerStartLocation(int32 Index)
{
    // Start on the universal entry pad, inside the actual playable level and
    // outside the dense objective props. There is no enclosing exterior wall
    // or gate to fight through before participating in the mission.
    // 2026-07-05 first-level polish: nudged +650/+420 out of the entry-pad alcove —
    // the third-person spring arm was collapsing against pad furniture at spawn and
    // after every arena recovery (camera ended up inside the player's back).
    return GetCityOrigin(Index) + ScaleCityOffset(FVector(-3170.0f, -2760.0f, 112.0f));
}

bool FCodeRescueCampaign::IsLocationInsideCityArenaXY(
    int32 Index,
    const FVector& Location,
    bool bUseOuterBounds)
{
    const FVector Relative = Location - GetCityOrigin(Index);
    const float SpanScale = GetCitySpanScale();
    const float HalfX = (bUseOuterBounds ? ArenaOuterHalfXLocal : ArenaInnerHalfXLocal) * SpanScale;
    const float HalfY = (bUseOuterBounds ? ArenaOuterHalfYLocal : ArenaInnerHalfYLocal) * SpanScale;
    return FMath::Abs(Relative.X) <= HalfX && FMath::Abs(Relative.Y) <= HalfY;
}

TArray<FString> FCodeRescueCampaign::GetCityChallengeIds(int32 Index)
{
    TArray<FString> Result;
    const FCodeRescueCityMission* Mission = GetMission(Index);
    if (!Mission || Mission->TerminalId.IsEmpty())
    {
        return Result;
    }

    Result.Reserve(RequiredChallengesPerCity);
    Result.Add(Mission->TerminalId);
    static const TCHAR* StageSuffixes[] = {
        TEXT("stage02_lock"),
        TEXT("stage03_reverse"),
        TEXT("stage04_palindrome"),
        TEXT("stage05_fizzbuzz"),
        TEXT("stage06_even_filter"),
        TEXT("stage07_linkedlist"),
        TEXT("stage08_binary_search"),
        TEXT("stage09_sum_relay"),
        TEXT("stage10_lock_final"),
    };
    static_assert(UE_ARRAY_COUNT(StageSuffixes) + 1 == RequiredChallengesPerCity,
        "The city challenge id set must contain exactly ten stages.");
    for (const TCHAR* Suffix : StageSuffixes)
    {
        // Keep the legacy terminal ID only for stage one. Later stages use the
        // neutral city slug so validator keywords in one stage cannot be
        // shadowed by the original lesson keyword embedded in TerminalId.
        Result.Add(FString::Printf(TEXT("%s_%s"), *Mission->Slug, Suffix));
    }
    return Result;
}

int32 FCodeRescueCampaign::GetCityChallengeProgress(const UCodeRescueGameInstance* GI, int32 Index)
{
    if (!GI)
    {
        return 0;
    }
    int32 Completed = 0;
    for (const FString& ChallengeId : GetCityChallengeIds(Index))
    {
        Completed += GI->SolvedTerminalIds.Contains(ChallengeId) ? 1 : 0;
    }
    return Completed;
}

bool FCodeRescueCampaign::HasCompletedCityChallengeSet(const UCodeRescueGameInstance* GI, int32 Index)
{
    return GetCityChallengeProgress(GI, Index) >= RequiredChallengesPerCity;
}

FString FCodeRescueCampaign::GetFirstUnsolvedCityChallengeId(const UCodeRescueGameInstance* GI, int32 Index)
{
    for (const FString& ChallengeId : GetCityChallengeIds(Index))
    {
        if (!GI || !GI->SolvedTerminalIds.Contains(ChallengeId))
        {
            return ChallengeId;
        }
    }
    return FString();
}

FString FCodeRescueCampaign::GetMissionLabel(int32 Index)
{
    if (const FCodeRescueCityMission* Mission = GetMission(Index))
    {
        return FString::Printf(TEXT("%02d. %s, %s"), Mission->Rank, *Mission->CityName, *Mission->StateName);
    }
    return TEXT("Unknown City");
}

FCodeRescueSurvivorArchetypeProfile FCodeRescueCampaign::GetSurvivorArchetypeProfile(const FCodeRescueCityMission& Mission)
{
    FCodeRescueSurvivorArchetypeProfile Profile;
    switch (Mission.LessonKind)
    {
    case ECampaignLessonKind::Lock:
        Profile.Title = TEXT("Systems Mechanic");
        Profile.IconLabel = TEXT("GATE");
        Profile.FieldNeed = TEXT("manual override on the route-lock panel");
        Profile.RescueSkill = TEXT("can repair jammed doors and safety interlocks");
        Profile.DossierHook = TEXT("Keeps a pocket truth table for every failed gate.");
        Profile.AccentColor = FLinearColor(0.08f, 0.78f, 1.0f, 1.0f);
        break;
    case ECampaignLessonKind::Reverse:
        Profile.Title = TEXT("Radio-Code Cleanup Specialist");
        Profile.IconLabel = TEXT("RADIO");
        Profile.FieldNeed = TEXT("clean radio packets before the extraction call");
        Profile.RescueSkill = TEXT("can restore scrambled dispatch chatter");
        Profile.DossierHook = TEXT("Marks every backwards packet until the route reads forward again.");
        Profile.AccentColor = FLinearColor(1.0f, 0.36f, 0.76f, 1.0f);
        break;
    case ECampaignLessonKind::Palindrome:
        Profile.Title = TEXT("Archive Integrity Analyst");
        Profile.IconLabel = TEXT("MIRROR");
        Profile.FieldNeed = TEXT("verified access codes from the civic archive");
        Profile.RescueSkill = TEXT("can spot fake mirror-codes and clean survivor records");
        Profile.DossierHook = TEXT("Carries a torn access ledger with matching marks on both edges.");
        Profile.AccentColor = FLinearColor(0.78f, 0.58f, 1.0f, 1.0f);
        break;
    case ECampaignLessonKind::FizzBuzz:
        Profile.Title = TEXT("Drone Timing Coordinator");
        Profile.IconLabel = TEXT("DRONE");
        Profile.FieldNeed = TEXT("a corrected beacon rhythm for timed drone sweeps");
        Profile.RescueSkill = TEXT("can keep rescue drones on the right cycle");
        Profile.DossierHook = TEXT("Counts beacon pulses under pressure and never skips the combined cycle.");
        Profile.AccentColor = FLinearColor(1.0f, 0.82f, 0.18f, 1.0f);
        break;
    case ECampaignLessonKind::EvenFilter:
        Profile.Title = TEXT("Data Medic");
        Profile.IconLabel = TEXT("TRIAGE");
        Profile.FieldNeed = TEXT("a filtered triage list before anyone moves");
        Profile.RescueSkill = TEXT("can prioritize safe lanes and emergency unit order");
        Profile.DossierHook = TEXT("Keeps the patient queue calm by separating signal from noise.");
        Profile.AccentColor = FLinearColor(0.26f, 1.0f, 0.54f, 1.0f);
        break;
    case ECampaignLessonKind::LinkedListTraverse:
        Profile.Title = TEXT("Network Engineer");
        Profile.IconLabel = TEXT("CHAIN");
        Profile.FieldNeed = TEXT("the next evacuation node confirmed from the broken chain");
        Profile.RescueSkill = TEXT("can map handoff nodes and missing route links");
        Profile.DossierHook = TEXT("Draws the route as a chain of names so nobody gets skipped.");
        Profile.AccentColor = FLinearColor(0.98f, 0.56f, 0.26f, 1.0f);
        break;
    case ECampaignLessonKind::BinarySearch:
        Profile.Title = TEXT("Supply-Cache Analyst");
        Profile.IconLabel = TEXT("CACHE");
        Profile.FieldNeed = TEXT("the correct cache bay found before the supplies spoil");
        Profile.RescueSkill = TEXT("can narrow sorted supply caches under pressure");
        Profile.DossierHook = TEXT("Splits every search zone in half until only the answer remains.");
        Profile.AccentColor = FLinearColor(0.25f, 0.95f, 0.88f, 1.0f);
        break;
    case ECampaignLessonKind::Sum:
    default:
        Profile.Title = TEXT("Power-Grid Apprentice");
        Profile.IconLabel = TEXT("POWER");
        Profile.FieldNeed = TEXT("three backup-cell readings combined into one route total");
        Profile.RescueSkill = TEXT("can rebalance battery cells and field generators");
        Profile.DossierHook = TEXT("Writes every load reading in pencil before trusting the grid.");
        Profile.AccentColor = FLinearColor(0.25f, 0.58f, 1.0f, 1.0f);
        break;
    }

    if (!Mission.CityName.IsEmpty())
    {
        Profile.DossierHook = FString::Printf(TEXT("%s %s lead: %s"),
            *Mission.CityName,
            *Profile.IconLabel,
            *Profile.DossierHook);
    }
    Profile.AccentColor = Profile.AccentColor * 0.76f + Mission.SecondaryAccentColor * 0.24f;
    Profile.AccentColor.A = 1.0f;
    return Profile;
}

bool FCodeRescueCampaign::IsCityCompleted(const UCodeRescueGameInstance* GI, int32 Index)
{
    const FCodeRescueCityMission* Mission = GetMission(Index);
    if (!GI || !Mission)
    {
        return false;
    }
    return HasCompletedCityChallengeSet(GI, Index) &&
           GI->RescuedSurvivorNames.Contains(Mission->SurvivorName);
}

bool FCodeRescueCampaign::IsCityUnlocked(const UCodeRescueGameInstance* GI, int32 Index)
{
    if (Index <= 0)
    {
        return true;
    }
    if (!GI)
    {
        return false;
    }
    for (int32 i = 0; i < Index; ++i)
    {
        if (!IsCityCompleted(GI, i))
        {
            return false;
        }
    }
    return true;
}

int32 FCodeRescueCampaign::GetFirstIncompleteCityIndex(const UCodeRescueGameInstance* GI)
{
    const int32 Count = GetMissionCount();
    for (int32 i = 0; i < Count; ++i)
    {
        if (!IsCityCompleted(GI, i))
        {
            return i;
        }
    }
    return Count;
}
