# Curriculum Authoring Guide

The file `Content/CodeRescueData/curriculum_database.json` is the source database for coding strategy content.

Each entry contains:

- `id`: unique mission/concept id
- `title`: player-facing title
- `language`: Java, C, Python, MATLAB, or All
- `concept`: concept group
- `difficulty`: 1-5
- `strategies`: accurate coding strategy tips
- `common_mistakes`: failure patterns the game should teach against

## Design principle

Every coding terminal should include:

1. A tactical rescue reason for the code.
2. A precise concept target.
3. A visible test.
4. Hidden tests that verify the general rule.
5. Strategy hints that teach transferable thinking.

## Example

```json
{
  "id": "python-functions-and-tests",
  "title": "Python: pure functions and readable tests",
  "language": "Python",
  "concept": "functions and unit thinking",
  "difficulty": 1,
  "strategies": [
    "Keep mission functions pure when possible.",
    "Use descriptive parameter names.",
    "Test visible and hidden-style cases."
  ],
  "common_mistakes": [
    "Printing when the mission requires return.",
    "Indenting inconsistently."
  ]
}
```
