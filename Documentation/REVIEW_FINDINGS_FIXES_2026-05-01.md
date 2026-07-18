# Review Findings Fixes - 2026-05-01

This note documents the fixes made after the May 1 review pass. The goal of
this pass was to close the five review findings without changing the larger
game direction.

## Summary

Fixed areas:

- Interaction reliability around terminals, language stations, and survivors.
- Compiler-backed validation for the new bonus challenge types.
- C challenge validation so hidden tests cannot be skipped by starter `main`.
- Duplicate solved-terminal accounting.
- Survivor-gating feedback.

Verification performed:

- Non-clean UnrealBuildTool build, target `CodeRescueUnrealEditor Mac Development`.
- Result: succeeded.

## Finding 1: Helper Meshes Could Block Interaction

Files changed:

- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

What changed:

- Added an `IsInteractableActor` helper in `ACodeRescueCharacter`.
- `Interact()` now falls back to `FindNearestInteractable(5000.0f)` when the
  forward trace hits a non-interactable actor.
- `SpawnBlock()` now accepts `bEnableCollision`.
- Visual helper blocks for terminals, language stations, survivors, survivor
  halos, terminal halos, language-station halos, and zombie proxy markers are
  spawned with collision disabled.

Why it matters:

The visible marker cubes are not the gameplay actors. Before this fix, looking
directly at a marker could prevent the player from interacting with the real
terminal, station, or survivor behind/near it. The interaction path now handles
that case from both sides: marker blocks do not collide, and non-interactable
trace hits still fall back to the nearest usable gameplay actor.

## Finding 2: New Challenge Types Needed Compiler Validators

Files changed:

- `Source/CodeRescueUnreal/CodeRunnerLibrary.cpp`
- `Source/CodeRescueUnreal/CodeRunnerLibrary.h`
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp`

What changed:

- Added a shared `ECodeChallengeKind` classifier for:
  - Sum
  - Lock
  - Reverse
  - Palindrome
  - FizzBuzz
  - EvenFilter
- Added compiler-backed harnesses for Java, C, Python, and MATLAB for the new
  reverse, palindrome, fizzBuzz, and even-number filter challenge types.
- Updated terminal starter code for Java, C, Python, and MATLAB so the starter
  matches the expected function signatures for each challenge type.

Language-specific expectations:

- Java:
  - `totalPower(int a, int b, int c)`
  - `shouldUnlock(boolean hasKey, boolean powerOn)`
  - `reverseString(String s)`
  - `isPalindrome(String s)`
  - `fizzBuzz(int n)` returning `String[]`
  - `evenNumbers(int[] values)` returning `int[]`
- C:
  - `totalPower(int a, int b, int c)`
  - `shouldUnlock(int hasKey, int powerOn)`
  - `reverseString(const char* input, char* output)`
  - `isPalindrome(const char* s)`
  - `fizzBuzz(int n, char* output, int outputSize)`
  - `evenNumbers(const int* input, int count, int* output)` returning output count
- Python:
  - `total_power(a, b, c)`
  - `should_unlock(has_key, power_on)`
  - `reverse_string(s)` or compatible aliases
  - `is_palindrome(s)` or compatible aliases
  - `fizz_buzz(n)` or compatible aliases, returning an iterable
  - `even_numbers(values)` or compatible aliases, returning an iterable
- MATLAB:
  - `total_power(a, b, c)`
  - `should_unlock(has_key, power_on)`
  - `reverse_string(s)`
  - `is_palindrome(s)`
  - `fizz_buzz(n)`
  - `even_numbers(values)`

Why it matters:

The bonus terminals now teach the actual concepts they advertise even when the
machine has the external language toolchain installed. The in-engine fallback
and compiler-backed path are now aligned in intent.

## Finding 3: C Harness Could Be Skipped

Files changed:

- `Source/CodeRescueUnreal/CodeRunnerLibrary.cpp`
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp`

What changed:

- C starter code no longer includes `int main`.
- The C validator always injects its own harness.
- User-provided `main` is renamed with `#define main user_main` before the
  hidden harness is appended.
- C validation now checks for the `ALL_TESTS_PASSED` token produced only by the
  harness.

Why it matters:

The validator now tests the submitted function directly. A player cannot pass
by printing the expected output from their own `main`, and a correct function
starter is no longer failed just because it did not print the harness token.

## Finding 4: Solved Terminals Counted Repeatedly

Files changed:

- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp`

What changed:

- `RecordTerminalSolved()` now returns early if the mission ID is empty or
  already present in `SolvedTerminalIds`.
- `TerminalsSolved` is derived from `SolvedTerminalIds.Num()` after a new solve.
- The terminal validate button is disabled after a successful solve.
- The code box is set read-only after success.
- Already solved terminals display an "already solved" message instead of
  revalidating.

Why it matters:

Progress, score, and win condition counts cannot be inflated by clicking
validate repeatedly on the same terminal.

## Finding 5: Gated Survivor Reported False Success

Files changed:

- `Source/CodeRescueUnreal/SurvivorActor.h`
- `Source/CodeRescueUnreal/SurvivorActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`

What changed:

- `ASurvivorActor::Rescue()` now returns `true` only when the rescue actually
  happens.
- It returns `false` when the survivor is already rescued or blocked by a
  required terminal.
- `ACodeRescueCharacter::Interact()` only prints the success message when
  `Rescue()` returns `true`.

Why it matters:

The player no longer receives contradictory feedback when a survivor refuses
to move until its required terminal is solved.

## Remaining Follow-Up

Recommended next checks:

- Playtest each terminal in each language, especially C bonus missions, because
  the C signatures are necessarily more explicit than the Python/Java/MATLAB
  versions.
- Consider adding automated validator smoke tests outside Unreal for the code
  snippets generated by `MakeStarterForLanguage`.
- Decide whether bonus terminals should count toward victory or remain optional
  score opportunities. The current win condition still checks for at least
  three solved terminals and four rescued survivors.
- Move the working project into source control and exclude generated folders
  such as `Binaries`, `Intermediate`, `Saved`, and `DerivedDataCache`.
