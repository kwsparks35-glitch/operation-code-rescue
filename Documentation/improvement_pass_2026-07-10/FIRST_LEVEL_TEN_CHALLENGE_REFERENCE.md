# First-Level Ten-Challenge Reference

Date: 2026-07-10

## Station Order

The New York level now requires all ten stations before the survivor can be
rescued. Every station uses only the language selected on the start screen.

| Stage | Challenge ID | Exercise |
| --- | --- | --- |
| 1 | `new_york_ny_sum` | Sum three power readings |
| 2 | `new_york_ny_stage02_lock` | Boolean safety gate |
| 3 | `new_york_ny_stage03_reverse` | Reverse a radio string |
| 4 | `new_york_ny_stage04_palindrome` | Validate a mirror code |
| 5 | `new_york_ny_stage05_fizzbuzz` | Build the beacon cycle |
| 6 | `new_york_ny_stage06_even_filter` | Keep even-numbered units |
| 7 | `new_york_ny_stage07_linkedlist` | Traverse an evacuation chain |
| 8 | `new_york_ny_stage08_binary_search` | Search a sorted cache |
| 9 | `new_york_ny_stage09_sum_relay` | Repeat the sum under relay conditions |
| 10 | `new_york_ny_stage10_lock_final` | Final Boolean extraction lock |

Stages 1 and 9 use the same sum signature. Stages 2 and 10 use the same lock
signature. The functions below are the exact canonical submissions used by the
runtime 60/60 acceptance audit.

## Java

### Sum: stages 1 and 9

```java
public static int totalPower(int a, int b, int c) {
    return a + b + c;
}
```

### Lock: stages 2 and 10

```java
public static boolean shouldUnlock(boolean hasKey, boolean powerOn) {
    return hasKey && powerOn;
}
```

### Reverse: stage 3

```java
public static String reverseString(String s) {
    return new StringBuilder(s).reverse().toString();
}
```

### Palindrome: stage 4

```java
public static boolean isPalindrome(String s) {
    return s.equals(new StringBuilder(s).reverse().toString());
}
```

### FizzBuzz: stage 5

```java
public static String[] fizzBuzz(int n) {
    String[] result = new String[n];
    for (int i = 1; i <= n; i++) {
        if (i % 15 == 0) result[i - 1] = "FizzBuzz";
        else if (i % 3 == 0) result[i - 1] = "Fizz";
        else if (i % 5 == 0) result[i - 1] = "Buzz";
        else result[i - 1] = Integer.toString(i);
    }
    return result;
}
```

### Even filter: stage 6

```java
public static int[] evenNumbers(int[] values) {
    int count = 0;
    for (int value : values) {
        if (value % 2 == 0) count++;
    }
    int[] result = new int[count];
    int index = 0;
    for (int value : values) {
        if (value % 2 == 0) result[index++] = value;
    }
    return result;
}
```

### Linked-list traversal: stage 7

```java
public static int countNodes(int[] next, int start) {
    int count = 0;
    int current = start;
    while (current != -1) {
        count++;
        current = next[current];
    }
    return count;
}
```

### Binary search: stage 8

```java
public static int binarySearch(int[] values, int target) {
    int low = 0;
    int high = values.length - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (values[mid] == target) return mid;
        if (values[mid] < target) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}
```

## C

The terminal harness supplies the standard string and formatting headers used
by these function-only submissions.

### Sum: stages 1 and 9

```c
int totalPower(int a, int b, int c) {
    return a + b + c;
}
```

### Lock: stages 2 and 10

```c
int shouldUnlock(int hasKey, int powerOn) {
    return hasKey && powerOn;
}
```

### Reverse: stage 3

```c
void reverseString(const char* input, char* output) {
    int length = (int)strlen(input);
    for (int i = 0; i < length; i++) {
        output[i] = input[length - 1 - i];
    }
    output[length] = '\0';
}
```

### Palindrome: stage 4

```c
int isPalindrome(const char* s) {
    int left = 0;
    int right = (int)strlen(s) - 1;
    while (left < right) {
        if (s[left++] != s[right--]) return 0;
    }
    return 1;
}
```

### FizzBuzz: stage 5

```c
void fizzBuzz(int n, char* output, int outputSize) {
    output[0] = '\0';
    for (int i = 1; i <= n; i++) {
        char piece[16];
        if (i % 15 == 0) snprintf(piece, sizeof(piece), "FizzBuzz");
        else if (i % 3 == 0) snprintf(piece, sizeof(piece), "Fizz");
        else if (i % 5 == 0) snprintf(piece, sizeof(piece), "Buzz");
        else snprintf(piece, sizeof(piece), "%d", i);
        if (i > 1) strncat(output, ",", outputSize - strlen(output) - 1);
        strncat(output, piece, outputSize - strlen(output) - 1);
    }
}
```

### Even filter: stage 6

```c
int evenNumbers(const int* input, int count, int* output) {
    int outCount = 0;
    for (int i = 0; i < count; i++) {
        if (input[i] % 2 == 0) {
            output[outCount++] = input[i];
        }
    }
    return outCount;
}
```

### Linked-list traversal: stage 7

```c
int countNodes(const int* next, int count, int start) {
    int total = 0;
    int current = start;
    while (current != -1 && current >= 0 && current < count) {
        total++;
        current = next[current];
    }
    return total;
}
```

### Binary search: stage 8

```c
int binarySearch(const int* values, int count, int target) {
    int low = 0;
    int high = count - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (values[mid] == target) return mid;
        if (values[mid] < target) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}
```

## C+ and C++

The C+ learning track and C++ track use the same canonical function contracts.
The terminal harness supplies the required standard-library headers.

### Sum: stages 1 and 9

```cpp
int totalPower(int a, int b, int c) {
    return a + b + c;
}
```

### Lock: stages 2 and 10

```cpp
bool shouldUnlock(bool hasKey, bool powerOn) {
    return hasKey && powerOn;
}
```

### Reverse: stage 3

```cpp
std::string reverseString(std::string s) {
    std::reverse(s.begin(), s.end());
    return s;
}
```

### Palindrome: stage 4

```cpp
bool isPalindrome(const std::string& s) {
    std::string reversed = s;
    std::reverse(reversed.begin(), reversed.end());
    return s == reversed;
}
```

### FizzBuzz: stage 5

```cpp
std::vector<std::string> fizzBuzz(int n) {
    std::vector<std::string> result;
    for (int i = 1; i <= n; ++i) {
        if (i % 15 == 0) result.push_back("FizzBuzz");
        else if (i % 3 == 0) result.push_back("Fizz");
        else if (i % 5 == 0) result.push_back("Buzz");
        else result.push_back(std::to_string(i));
    }
    return result;
}
```

### Even filter: stage 6

```cpp
std::vector<int> evenNumbers(const std::vector<int>& values) {
    std::vector<int> result;
    for (int value : values) {
        if (value % 2 == 0) result.push_back(value);
    }
    return result;
}
```

### Linked-list traversal: stage 7

```cpp
int countNodes(const std::vector<int>& next, int start) {
    int count = 0;
    int current = start;
    while (current != -1 && current >= 0 && current < static_cast<int>(next.size())) {
        ++count;
        current = next[current];
    }
    return count;
}
```

### Binary search: stage 8

```cpp
int binarySearch(const std::vector<int>& values, int target) {
    int low = 0;
    int high = static_cast<int>(values.size()) - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (values[mid] == target) return mid;
        if (values[mid] < target) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}
```

## Python

### Sum: stages 1 and 9

```python
def total_power(a, b, c):
    return a + b + c
```

### Lock: stages 2 and 10

```python
def should_unlock(has_key, power_on):
    return has_key and power_on
```

### Reverse: stage 3

```python
def reverse_string(s):
    return s[::-1]
```

### Palindrome: stage 4

```python
def is_palindrome(s):
    return s == s[::-1]
```

### FizzBuzz: stage 5

```python
def fizz_buzz(n):
    result = []
    for i in range(1, n + 1):
        if i % 15 == 0:
            result.append("FizzBuzz")
        elif i % 3 == 0:
            result.append("Fizz")
        elif i % 5 == 0:
            result.append("Buzz")
        else:
            result.append(str(i))
    return result
```

### Even filter: stage 6

```python
def even_numbers(values):
    return [value for value in values if value % 2 == 0]
```

### Linked-list traversal: stage 7

```python
def count_nodes(next_indices, start):
    total = 0
    current = start
    while current != -1:
        total += 1
        current = next_indices[current]
    return total
```

### Binary search: stage 8

```python
def binary_search(values, target):
    low = 0
    high = len(values) - 1
    while low <= high:
        mid = (low + high) // 2
        if values[mid] == target:
            return mid
        if values[mid] < target:
            low = mid + 1
        else:
            high = mid - 1
    return -1
```

## MATLAB

MATLAB uses one-based array indices. The linked-list challenge uses `0` as its
end sentinel, and binary search returns `0` when the target is not present.

### Sum: stages 1 and 9

```matlab
function result = total_power(a, b, c)
    result = a + b + c;
end
```

### Lock: stages 2 and 10

```matlab
function result = should_unlock(has_key, power_on)
    result = has_key && power_on;
end
```

### Reverse: stage 3

```matlab
function result = reverse_string(s)
    result = fliplr(s);
end
```

### Palindrome: stage 4

```matlab
function result = is_palindrome(s)
    result = strcmp(s, fliplr(s));
end
```

### FizzBuzz: stage 5

```matlab
function result = fizz_buzz(n)
    result = strings(1, n);
    for i = 1:n
        if mod(i, 15) == 0
            result(i) = "FizzBuzz";
        elseif mod(i, 3) == 0
            result(i) = "Fizz";
        elseif mod(i, 5) == 0
            result(i) = "Buzz";
        else
            result(i) = string(i);
        end
    end
end
```

### Even filter: stage 6

```matlab
function result = even_numbers(values)
    result = values(mod(values, 2) == 0);
end
```

### Linked-list traversal: stage 7

```matlab
function result = count_nodes(next_indices, start)
    result = 0;
    current = start;
    while current ~= 0
        result = result + 1;
        current = next_indices(current);
    end
end
```

### Binary search: stage 8

```matlab
function result = binary_search(values, target)
    low = 1;
    high = numel(values);
    while low <= high
        mid = floor((low + high) / 2);
        if values(mid) == target
            result = mid;
            return;
        elseif values(mid) < target
            low = mid + 1;
        else
            high = mid - 1;
        end
    end
    result = 0;
end
```

## Verification Contract

`UCodeTerminalWidget::GetCanonicalReferenceSolution` forwards to the same
language/station generator used to populate the live terminal. The first-level
challenge audit submits these exact implementations across ten stages and six
languages, for 60/60 accepted combinations. Java, C, C+, C++, and Python used
their available local toolchains; MATLAB used the packaged in-engine validator
after its external batch launch exceeded the cold-start timeout.

