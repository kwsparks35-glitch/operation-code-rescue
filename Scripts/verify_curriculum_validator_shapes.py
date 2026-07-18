"""
Verify that every shipped curriculum challenge shape is completable through the
runtime validator path.

Run from the project root:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/verify_curriculum_validator_shapes.py" \
        -unattended -NoSound -NullRHI
"""

import unreal


LESSON_IDS = {
    "sum": "audit_city_sum",
    "lock": "audit_city_lock",
    "reverse": "audit_city_reverse",
    "palindrome": "audit_city_palindrome",
    "fizzbuzz": "audit_city_fizzbuzz",
    "even_filter": "audit_city_even_filter",
    "linked_list": "audit_city_linked_list_traverse",
    "binary_search": "audit_city_binary_search",
}

LANGUAGE_ENUMS = {
    "Java": "JAVA",
    "C": "C",
    "Python": "PYTHON",
    "MATLAB": "MATLAB",
    "C+": ("C_PLUS", "CPLUS", "C_PLUS_PLUS_PLACEHOLDER"),
    "C++": ("CPP", "C_PLUS_PLUS", "CPLUSPLUS"),
}

SOLUTIONS = {
    "Java": {
        "sum": "public static int totalPower(int a, int b, int c) {\n    return a + b + c;\n}\n",
        "lock": "public static boolean shouldUnlock(boolean hasKey, boolean powerOn) {\n    return hasKey && powerOn;\n}\n",
        "reverse": "public static String reverseString(String s) {\n    return new StringBuilder(s).reverse().toString();\n}\n",
        "palindrome": "public static boolean isPalindrome(String s) {\n    return s.equals(new StringBuilder(s).reverse().toString());\n}\n",
        "fizzbuzz": "public static String[] fizzBuzz(int n) {\n    String[] result = new String[n];\n    for (int i = 1; i <= n; i++) {\n        if (i % 15 == 0) result[i - 1] = \"FizzBuzz\";\n        else if (i % 3 == 0) result[i - 1] = \"Fizz\";\n        else if (i % 5 == 0) result[i - 1] = \"Buzz\";\n        else result[i - 1] = Integer.toString(i);\n    }\n    return result;\n}\n",
        "even_filter": "public static int[] evenNumbers(int[] values) {\n    int count = 0;\n    for (int value : values) if (value % 2 == 0) count++;\n    int[] result = new int[count];\n    int index = 0;\n    for (int value : values) if (value % 2 == 0) result[index++] = value;\n    return result;\n}\n",
        "linked_list": "public static int countNodes(int[] next, int start) {\n    int total = 0;\n    int current = start;\n    while (current != -1) {\n        total++;\n        current = next[current];\n    }\n    return total;\n}\n",
        "binary_search": "public static int binarySearch(int[] values, int target) {\n    int low = 0;\n    int high = values.length - 1;\n    while (low <= high) {\n        int mid = (low + high) / 2;\n        if (values[mid] == target) return mid;\n        if (values[mid] < target) low = mid + 1;\n        else high = mid - 1;\n    }\n    return -1;\n}\n",
    },
    "C": {
        "sum": "int totalPower(int a, int b, int c) {\n    return a + b + c;\n}\n",
        "lock": "int shouldUnlock(int hasKey, int powerOn) {\n    return hasKey && powerOn;\n}\n",
        "reverse": "void reverseString(const char* input, char* output) {\n    int length = (int)strlen(input);\n    for (int i = 0; i < length; i++) output[i] = input[length - 1 - i];\n    output[length] = '\\0';\n}\n",
        "palindrome": "int isPalindrome(const char* s) {\n    int left = 0;\n    int right = (int)strlen(s) - 1;\n    while (left < right) {\n        if (s[left++] != s[right--]) return 0;\n    }\n    return 1;\n}\n",
        "fizzbuzz": "void fizzBuzz(int n, char* output, int outputSize) {\n    output[0] = '\\0';\n    for (int i = 1; i <= n; i++) {\n        char piece[16];\n        if (i % 15 == 0) snprintf(piece, sizeof(piece), \"FizzBuzz\");\n        else if (i % 3 == 0) snprintf(piece, sizeof(piece), \"Fizz\");\n        else if (i % 5 == 0) snprintf(piece, sizeof(piece), \"Buzz\");\n        else snprintf(piece, sizeof(piece), \"%d\", i);\n        if (i > 1) strncat(output, \",\", outputSize - strlen(output) - 1);\n        strncat(output, piece, outputSize - strlen(output) - 1);\n    }\n}\n",
        "even_filter": "int evenNumbers(const int* input, int count, int* output) {\n    int outCount = 0;\n    for (int i = 0; i < count; i++) {\n        if (input[i] % 2 == 0) output[outCount++] = input[i];\n    }\n    return outCount;\n}\n",
        "linked_list": "int countNodes(const int* next, int count, int start) {\n    int total = 0;\n    int current = start;\n    while (current != -1 && current >= 0 && current < count) {\n        total++;\n        current = next[current];\n    }\n    return total;\n}\n",
        "binary_search": "int binarySearch(const int* values, int count, int target) {\n    int low = 0;\n    int high = count - 1;\n    while (low <= high) {\n        int mid = (low + high) / 2;\n        if (values[mid] == target) return mid;\n        if (values[mid] < target) low = mid + 1;\n        else high = mid - 1;\n    }\n    return -1;\n}\n",
    },
    "Python": {
        "sum": "def total_power(a, b, c):\n    return a + b + c\n",
        "lock": "def should_unlock(has_key, power_on):\n    return has_key and power_on\n",
        "reverse": "def reverse_string(s):\n    return s[::-1]\n",
        "palindrome": "def is_palindrome(s):\n    return s == s[::-1]\n",
        "fizzbuzz": "def fizz_buzz(n):\n    result = []\n    for i in range(1, n + 1):\n        if i % 15 == 0:\n            result.append(\"FizzBuzz\")\n        elif i % 3 == 0:\n            result.append(\"Fizz\")\n        elif i % 5 == 0:\n            result.append(\"Buzz\")\n        else:\n            result.append(str(i))\n    return result\n",
        "even_filter": "def even_numbers(values):\n    return [value for value in values if value % 2 == 0]\n",
        "linked_list": "def count_nodes(next_indices, start):\n    total = 0\n    current = start\n    while current != -1:\n        total += 1\n        current = next_indices[current]\n    return total\n",
        "binary_search": "def binary_search(values, target):\n    low = 0\n    high = len(values) - 1\n    while low <= high:\n        mid = (low + high) // 2\n        if values[mid] == target:\n            return mid\n        if values[mid] < target:\n            low = mid + 1\n        else:\n            high = mid - 1\n    return -1\n",
    },
    "MATLAB": {
        "sum": "function result = total_power(a, b, c)\n    result = a + b + c;\nend\n",
        "lock": "function result = should_unlock(has_key, power_on)\n    result = has_key && power_on;\nend\n",
        "reverse": "function result = reverse_string(s)\n    result = fliplr(s);\nend\n",
        "palindrome": "function result = is_palindrome(s)\n    result = strcmp(s, fliplr(s));\nend\n",
        "fizzbuzz": "function result = fizz_buzz(n)\n    result = strings(1, n);\n    for i = 1:n\n        if mod(i, 15) == 0\n            result(i) = \"FizzBuzz\";\n        elseif mod(i, 3) == 0\n            result(i) = \"Fizz\";\n        elseif mod(i, 5) == 0\n            result(i) = \"Buzz\";\n        else\n            result(i) = string(i);\n        end\n    end\nend\n",
        "even_filter": "function result = even_numbers(values)\n    result = values(mod(values, 2) == 0);\nend\n",
        "linked_list": "function result = count_nodes(next_indices, start)\n    result = 0;\n    current = start;\n    while current ~= 0\n        result = result + 1;\n        current = next_indices(current);\n    end\nend\n",
        "binary_search": "function result = binary_search(values, target)\n    low = 1;\n    high = numel(values);\n    while low <= high\n        mid = floor((low + high) / 2);\n        if values(mid) == target\n            result = mid;\n            return;\n        elseif values(mid) < target\n            low = mid + 1;\n        else\n            high = mid - 1;\n        end\n    end\n    result = 0;\nend\n",
    },
}

CPP_SOLUTIONS = {
    "sum": "int totalPower(int a, int b, int c) {\n    return a + b + c;\n}\n",
    "lock": "bool shouldUnlock(bool hasKey, bool powerOn) {\n    return hasKey && powerOn;\n}\n",
    "reverse": "#include <algorithm>\nstd::string reverseString(std::string s) {\n    std::reverse(s.begin(), s.end());\n    return s;\n}\n",
    "palindrome": "#include <algorithm>\nbool isPalindrome(const std::string& s) {\n    std::string reversed = s;\n    std::reverse(reversed.begin(), reversed.end());\n    return s == reversed;\n}\n",
    "fizzbuzz": "#include <string>\n#include <vector>\nstd::vector<std::string> fizzBuzz(int n) {\n    std::vector<std::string> result;\n    for (int i = 1; i <= n; ++i) {\n        if (i % 15 == 0) result.push_back(\"FizzBuzz\");\n        else if (i % 3 == 0) result.push_back(\"Fizz\");\n        else if (i % 5 == 0) result.push_back(\"Buzz\");\n        else result.push_back(std::to_string(i));\n    }\n    return result;\n}\n",
    "even_filter": "#include <vector>\nstd::vector<int> evenNumbers(const std::vector<int>& values) {\n    std::vector<int> result;\n    for (int value : values) {\n        if (value % 2 == 0) result.push_back(value);\n    }\n    return result;\n}\n",
    "linked_list": "#include <vector>\nint countNodes(const std::vector<int>& next, int start) {\n    int total = 0;\n    int current = start;\n    while (current != -1 && current >= 0 && current < static_cast<int>(next.size())) {\n        ++total;\n        current = next[current];\n    }\n    return total;\n}\n",
    "binary_search": "#include <vector>\nint binarySearch(const std::vector<int>& values, int target) {\n    int low = 0;\n    int high = static_cast<int>(values.size()) - 1;\n    while (low <= high) {\n        int mid = (low + high) / 2;\n        if (values[mid] == target) return mid;\n        if (values[mid] < target) low = mid + 1;\n        else high = mid - 1;\n    }\n    return -1;\n}\n",
}

SOLUTIONS["C+"] = CPP_SOLUTIONS
SOLUTIONS["C++"] = CPP_SOLUTIONS


def fail(message):
    unreal.log_error(f"[cr-validator-shapes] {message}")
    raise RuntimeError(message)


def enum_value(enum_class, name):
    if isinstance(name, (tuple, list)):
        failures = []
        for candidate in name:
            try:
                return enum_value(enum_class, candidate)
            except Exception as exc:
                failures.append(str(exc))
        fail(f"could not resolve enum value from candidates {name}: {'; '.join(failures)}")
    try:
        return getattr(enum_class, name)
    except Exception:
        pass
    try:
        return enum_class.cast(name)
    except Exception:
        fail(f"could not resolve enum value {name}")


def coding_language_enum():
    for enum_name in ("ECodingLanguage", "CodingLanguage"):
        if hasattr(unreal, enum_name):
            return getattr(unreal, enum_name)
    candidates = [name for name in dir(unreal) if "CodingLanguage" in name or name.endswith("Language")]
    fail("could not resolve coding-language enum; candidates=" + ", ".join(candidates[:20]))


def get_prop(obj, *names):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            pass
        if hasattr(obj, name):
            return getattr(obj, name)
    fail(f"could not read property {names[0]}")


def set_prop(obj, name, value):
    try:
        obj.set_editor_property(name, value)
        return
    except Exception:
        pass
    setattr(obj, name, value)


def make_challenge(language, lesson):
    spec = unreal.ChallengeSpec()
    set_prop(spec, "id", LESSON_IDS[lesson])
    set_prop(spec, "title", f"Audit {lesson} {language}")
    set_prop(spec, "mission_brief", f"Automated audit for {lesson} in {language}.")
    set_prop(spec, "language", enum_value(coding_language_enum(), LANGUAGE_ENUMS[language]))
    set_prop(spec, "starter_code", SOLUTIONS[language][lesson])
    return spec


def main():
    unreal.log("[cr-validator-shapes] === curriculum validator verification START ===")
    total_cases = sum(len(lessons) for lessons in SOLUTIONS.values())
    case_index = 0
    for language, lessons in SOLUTIONS.items():
        for lesson, code in lessons.items():
            case_index += 1
            unreal.log(f"[cr-validator-shapes] running {case_index}/{total_cases}: {language} {lesson}")
            challenge = make_challenge(language, lesson)
            result = unreal.CodeRunnerLibrary.validate_challenge(challenge, code)
            success = bool(get_prop(result, "b_success", "bSuccess"))
            score = int(get_prop(result, "score"))
            summary = str(get_prop(result, "summary"))
            if not success:
                stdout = str(get_prop(result, "std_out", "stdOut"))
                stderr = str(get_prop(result, "std_err", "stdErr"))
                fail(f"{language} {lesson} failed score={score} summary={summary} stdout={stdout} stderr={stderr}")
            unreal.log(f"[cr-validator-shapes] OK {language} {lesson} score={score}")

    unreal.log("[cr-validator-shapes] === curriculum validator verification PASSED ===")


main()
