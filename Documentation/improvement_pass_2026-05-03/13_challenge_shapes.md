# Item 13 — Linked-list & binary-search challenge shapes

## What changed
Added two new challenge kinds to `UCodeRunnerLibrary::ValidateInEngine`:
- **Linked-list traversal** — matches challenge IDs containing `linkedlist`,
  `linked_list`, or `traverse`. Validates: function name (`traverse`, `walk`,
  `countNodes`, etc.), `.next` / `->next` / `.Next` advance pattern, while/for
  loop, and an anti-pattern guard against constant returns.
- **Binary search** — matches IDs containing `binary_search`, `binarysearch`,
  or `bsearch`. Validates: function name, lo/hi bounds (or low/high,
  left/right, start/end), midpoint as `(lo+hi)/2` or `>>1` or `//2`, plus a
  comparison + bound update pattern, with a guard rejecting linear scans.

## Files touched
- `Source/CodeRescueUnreal/CodeRunnerLibrary.cpp` — enum cases, kind classifier,
  validation logic.

## Design decisions
- Pattern-matching is intentionally permissive across Java / C / Python /
  MATLAB. We accept built-in names (`reverse()`, `flip()`) as well as
  hand-written loops.
- Both shapes include an "anti-pattern" check so a student can't pass with
  a stub like `return 0` or `for x in arr: ...`.

## Known limitations
- Bonus terminals using these IDs aren't auto-spawned. To wire one up, drop a
  new `FCodeRescueCityMission` row whose `TerminalId` contains one of the
  matching substrings, or hand-edit the campaign data file.
- The midpoint regex requires the two operand identifiers to be on the same
  line; multi-line midpoint expressions will fall through to the lookup-only
  fallback (`midpoint`, `floor(`).

## Follow-up work
- Add 2 demo terminals to `FCodeRescueCampaign` so the new shapes appear in
  the curriculum without needing a designer to wire them.
- Add language-specific starter snippets in `MakeStarterForLanguage` so the
  player has a scaffold to fill in.
