# Item 15 — More challenge types

**Status:** DONE — four new shapes added to
`UCodeRunnerLibrary::ValidateInEngine` plus four bonus terminals in the
world that exercise them.

## New challenge shapes

The validator dispatches on substrings in `Challenge.Id`. Existing:

| Substring | Shape |
|---|---|
| `sum` or `generator` | three-way addition |
| `lock` | boolean AND |

New:

| Substring | Shape | Function name expected (any of) |
|---|---|---|
| `reverse` | string/array reversal | `reverseString`, `reverse_string`, `reverse` |
| `palindrome` | original-vs-reversed equality | `isPalindrome`, `is_palindrome`, `palindrome` |
| `fizzbuzz` | modulo + Fizz/Buzz literals | `fizzBuzz`, `fizz_buzz`, `fizzbuzz` |
| `filter` or `even` | modulo + iteration | `filter`, `evens`, `evenNumbers`, `even_numbers` |

Each new shape applies 3 checks:

### Reverse

1. Defines an expected function name.
2. Uses a built-in (`[::-1]`, `reverse(`, `.reverse`, MATLAB `flip(`/`flipud(`)
   OR a backwards-decrement loop (regex matches `i--` or `i -= 1`).
3. Returns / outputs the value (`return`, `disp(`, `result =`).

### Palindrome

1. Defines `isPalindrome` / `is_palindrome` / `palindrome`.
2. Either uses an equality check (`==`, `.equals(`, MATLAB `isequal(`)
   OR a two-pointer pattern (regex matches `i++` ... `j--` or `i+=1`...`j-=1`).
3. Returns / outputs.

### FizzBuzz

1. Defines a `fizzBuzz` / `fizz_buzz` / `fizzbuzz`.
2. Uses modulo (`%`, MATLAB `mod(`, Java `Math.floorMod`).
3. Mentions both `Fizz` and `Buzz` literals.

   Anti-pattern guard: a hard-coded `Fizz`/`Buzz` without modulo fails
   the "doesn't hardcode the output" check.

### Filter

1. Defines `filter`/`evens`/`evenNumbers`/`even_numbers`.
2. Uses modulo to test evenness/predicate.
3. Iterates over the input collection (any of: `for`, `while`,
   `.filter(`, `.stream(`, list comprehension).

## New terminals in the world

Four new `SpawnTerminal` calls in `ACodeRescueGameMode::SpawnWorld`,
all bonus (don't gate the win condition):

| Zone | ID | Title | Brief |
|---|---|---|---|
| Anchorage | `hospital_string_reverse` | Hospital Sign Reverser | Decode the inverted exit sign |
| Seattle | `dock_palindrome_check` | Dock Manifest Palindrome | Validate cargo code |
| Tokyo | `metro_fizzbuzz_signal` | Metro Diagnostic FizzBuzz | Sweep the signal grid |
| Anchorage | `triage_even_filter` | Triage Even-Bay Filter | Sort patient bays |

Win threshold remains 3 terminals — bonus terminals just add Coding Score
and language practice.

## Files touched

- `Source/CodeRescueUnreal/CodeRunnerLibrary.cpp` — `ValidateInEngine`
  body extended with four new branches.
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` — four new
  `SpawnTerminal` calls.

## How to author starter code per language

Each terminal's `StarterCode` defaults to empty. To wire boilerplate per
language, extend `ACodingTerminalActor::Interact` to populate
`Challenge.StarterCode` based on `GI->SelectedLanguage`. Example for
`hospital_string_reverse`:

```python
# Python
def reverseString(s):
    # TODO: return s with its characters in reverse order
    pass
```

```java
// Java
public class Solution {
    public String reverseString(String s) {
        // TODO
        return s;
    }
}
```

That's a future polish item; the validator already accepts whatever
shape the player writes as long as it matches the heuristic.

## Verifying

1. PIE.
2. Walk to one of the new bonus terminals (Anchorage has a Hospital Sign
   Reverser at coords listed in `SpawnWorld`).
3. Press E. Type a solution that matches the heuristic (e.g. for reverse,
   any function named `reverse` that uses `[::-1]` in Python).
4. Validate. Should report success with all three checks passed.
