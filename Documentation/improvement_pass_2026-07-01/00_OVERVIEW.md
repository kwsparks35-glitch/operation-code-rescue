# Improvement Pass 2026-07-01 — Overview

**Goal (from Kenny):** implement improvements to maximize gameplay *learning* while protecting the
user experience, using the five June-25 vision documents as guidelines.

**Approach:** author everything implementable from a non-UE environment as real, additive code + data,
going deepest on pedagogy (highest leverage + fully reachable), then wire focused concept nodes into the
runtime terminal and solved-route loop while preserving the legacy fallback path. Consistent with the
checks-and-balances established 2026-06-30.

## Contents of this pass

- `LEARNING_VERTICAL_SLICE.md` — the pedagogy implementation: curriculum content pack (36 challenges,
  10-tier concept graph), the additive `UCodeRescueLearningLibrary` (teach → apply → reinforce,
  adaptive scaffold, telemetry, data-driven selection), terminal runtime wiring for the tier-4 filter
  node, tier-2 boolean/conditionals node, and tier-5 strings/reverse node, plus the cook-safe intrinsic
  world-effect actor spawned by the solved route.
- `REVISED_RECOMMENDATIONS_RESPONSE.md` — the July 1 revised-recommendations response, including the
  validator-key terminal generalization and curriculum-declared external test harness bridge.

## Traceability to the vision docs

- **World Development Deep-Dive §7.1** ("coding diegetically load-bearing; the puzzles ARE the rescues")
  → `ACodeRescueSolveEffectActor` driven by solution `OutputMagnitude`; every challenge has a `world_effect`.
- **World Development Deep-Dive §6.1** (combat/coding "safe beats") → teach-then-apply keeps terminals as
  calm, skippable learning beats.
- **Top 50 item 45** (tighten teach-then-apply; post-solve explanations) → `BuildTeachPayload` / `BuildPostSolve`.
- **Top 50 items 46–47** (mastery clarity; difficulty/onboarding) → learning telemetry + tiered concept graph.
- **Top 50 item 5 / 9** (terminal restyle / objective clarity) → wiring notes for the terminal UI.
- **Game Physics Deep-Dive / Character Animation Deep-Dive placeholders** → nonblocking filter/boolean/reverse
  world-response props, mentor pose proxies, and survivor pose proxies are tagged for future authored
  physics/animation replacement without blocking traversal.

## Status

Code + data + docs authored and statically verified in-sandbox. **Not yet compiled or playtested** —
that is the documented Mac Definition-of-Done gate (`Documentation/CLAUDE_OVERSIGHT/GOVERNANCE_PROTOCOL.md`).
Art pillars (bespoke animation clips, meshes, sculpted world art) remain editor/artist work; this pass
delivers the systems, data, reviewable runtime placeholders, and tags they slot into.

The revised July 1 recommendations are addressed by adding curriculum `validator` metadata, routing
terminal selection through a single validator-key path, and generating external harness assertions from
the selected row's visible/hidden tests for the three runtime-wired validator families.
