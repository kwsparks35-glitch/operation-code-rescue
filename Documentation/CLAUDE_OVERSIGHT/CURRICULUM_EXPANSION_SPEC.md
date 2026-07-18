# Curriculum Expansion Spec — the highest-leverage learning work

**Companion to:** `Operation_Code_Rescue_Maximizing_Learning_2026-07-01.pdf` (project root).
**Purpose:** turn the 8 hard-coded concepts into a real, data-driven curriculum that feeds the
mastery/spaced-review engine that already exists. This is Top-50 item 45 done properly, promoted to P0.

## The problem in one line

`ECampaignLessonKind` and the terminal validator enum both hold exactly 8 concepts — Sum, Lock,
Reverse, Palindrome, FizzBuzz, EvenFilter, LinkedListTraverse, BinarySearch — reskinned across 465
cities × 6 languages, graded by keyword pattern-matching (`CodeRescue.AllowExternalCodeValidation=0`).

## Target concept graph (each node = many challenges, per language)

| Tier | Concept nodes | Maps from today's 8 |
|---|---|---|
| 1 | variables, types, I/O, expressions | (new) |
| 2 | conditionals & boolean logic | Lock |
| 3 | loops & accumulation | Sum, FizzBuzz |
| 4 | arrays / lists & iteration | EvenFilter |
| 5 | strings & indexing | Reverse, Palindrome |
| 6 | functions, params, return, scope | (new; today only implicit) |
| 7 | recursion | (new) |
| 8 | dictionaries / maps & structs | (new) |
| 9 | searching & sorting | BinarySearch, LinkedListTraverse |
| 10 | complexity & refactoring (capstones) | (new) |

Ship tiers gradually behind the same terminal; map city families to tiers so difficulty climbs with
the journey. The existing `DifficultyTierForRank` / spaced-review framing already expects this shape.

## Data-driven challenge schema (replaces hard-coded archetypes)

```json
{
  "id": "loops-evac-even-order",
  "concept": "arrays/filtering",
  "prerequisites": ["conditionals", "loops"],
  "tier": 4,
  "languages": ["Java", "Python", "C", "C++", "MATLAB"],
  "micro_lesson": "Keep items that pass a test, preserving order.",
  "worked_example": "[1,2,3,4] -> [2,4]  (order kept)",
  "prompt": "Route even-numbered evac units first, in order.",
  "starter": { "python": "def evens(xs):\n    # your code\n    return []" },
  "visible_tests": [ {"in": "[1,2,3,4]", "out": "[2,4]"} ],
  "hidden_tests":  [ {"in": "[]", "out": "[]"}, {"in": "[7,7,8]", "out": "[8]"} ],
  "misconceptions": ["reorders output", "keeps odds", "mutates input"],
  "world_effect": "survivors board the evac in the returned order",
  "post_solve": "Filtering preserves order; that is why triage lists stay stable."
}
```

`world_effect` is the point: the learner's **output** parameterizes the rescue (intrinsic
integration, per World Deep-Dive §7.1) — not just the fact that they solved it.

## Validation (item M2)

- Enable the existing external toolchain path in a sandboxed, timed, output-only mode for the safe
  local-teaching build; fall back to a per-concept unit-test harness (visible + hidden cases) when no
  compiler is present. Replace keyword checks with real behavior checks.
- Run async with a timeout; cache toolchain probes; never freeze the game on validation.

## Definition of Done for this work

A single concept node is "done" when: it has ≥5 challenges per language, real visible+hidden tests,
a micro-lesson + worked example + post-solve note, a `world_effect`, and it flows through the existing
mastery/spaced-review UI. Prove one node end-to-end in the dossier's "one polished city" vertical slice
before scaling. Guardrails (never wall-block, skippable lessons, difficulty-scaled rigor, terminals as
calm safe-beats) are in the report §4.
