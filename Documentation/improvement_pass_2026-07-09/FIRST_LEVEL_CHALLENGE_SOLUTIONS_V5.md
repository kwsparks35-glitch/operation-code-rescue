# First-Level Challenge Solutions and Validation Record

Date: 2026-07-09

> Superseded for the expanded ten-station level by
> `Documentation/improvement_pass_2026-07-10/FIRST_LEVEL_TEN_CHALLENGE_REFERENCE.md`.
> This V5 record remains the historical reference for the original single
> terminal.

Challenge ID: `new_york_ny_sum`

The first New York terminal asks the player to return the total power of three
integer inputs. The exact accepted implementation for each selectable language
is recorded below.

## Java

```java
public static int totalPower(int a, int b, int c) {
    return a + b + c;
}
```

## C

```c
int totalPower(int a, int b, int c) {
    return a + b + c;
}
```

## C+

`C+` is the game's C-compatible learning track and accepts this implementation:

```c
int totalPower(int a, int b, int c) {
    return a + b + c;
}
```

## C++

```cpp
int totalPower(int a, int b, int c) {
    return a + b + c;
}
```

## Python

```python
def total_power(a, b, c):
    return a + b + c
```

## MATLAB

```matlab
function result = total_power(a, b, c)
    result = a + b + c;
end
```

## Verification

`TestLogs/FirstLevelChallengeAuditV5.log` exercises these exact submissions
through `UCodeRunnerLibrary::ValidateChallenge`. Java, C, C+, C++, and Python
passed their available external execution paths. The local MATLAB batch process
timed out, after which the same submission passed the built-in MATLAB-compatible
validator; this fallback is the packaged application's supported path when an
external toolchain is unavailable.

`TestLogs/CurriculumValidatorShapesV5.log` additionally passes 48/48 validator
combinations: six languages across eight challenge shapes. The packaged
integrated run reports `languages=6/6`, score 100 for each language, all visible
checks passed, and hidden checks passed. Language selection remains isolated:
progress made in one language is saved to that language's own campaign history
and does not change the active implementation language during a run.
