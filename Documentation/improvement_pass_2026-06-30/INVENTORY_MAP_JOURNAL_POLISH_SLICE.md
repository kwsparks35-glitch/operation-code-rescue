# Inventory Map And Journal Polish Slice

This pass implements the P1 UI request for inventory, map, and journal polish. The prior game already had a HUD weapon strip, a minimap, an objective journal, and survivor intel text, but the information was scattered across several surfaces. This slice turns the `J` objective journal into a single field-ops readout that clearly connects the selected language run, save continuity, active route, inventory, survivor intel, and city progress.

## Player-Facing Changes

- Added `LANGUAGE SAVE`, showing the active coding language, the exact language-only save slot, whether a save exists, and the reminder that the start screen remains available for resume selection.
- Added `ROUTE MAP`, showing the current city, region, landmark, terminal state, survivor state, case-file summary, and next marker.
- Added `FIELD INVENTORY`, showing active weapon, ammo, role, weapon count, armor, medkits, scanner, flashlight, bypass kits, flares, smokes, stims, ammo pouch bonus, scrap, and research.
- Kept the existing `SURVIVOR INTEL DOSSIER`, now positioned after the save, route, and inventory summaries so the player can read the objective chain from broad state to specific next step.
- Expanded the journal overlay height so these readouts have room without crushing the mission progress rows.

## Survivor Intel

The survivor intel block remains the final readout in the overlay. It shows route status, contact, location, lesson payoff, validation attempts, language progress, and the next step after the player has inspected their language save, route map, and field inventory.

## Implementation

The runtime work lives in `UCodeRescueObjectiveJournalWidget`.

- `BuildLanguageSaveContinuityLine` reads `SelectedLanguage`, `SaveSlotName`, and `DoesLanguageSaveExist`.
- `BuildRouteMapReadout` reads `SolvedTerminalIds`, `RescuedSurvivorNames`, mission metadata, and `GetCaseFileCollectionSummary`.
- `BuildInventoryReadout` reads the live `ACodeRescueCharacter` weapon/loadout/resource state and `ResearchPoints`.
- `RefreshJournal` updates all sections from live save/player state and mirrors high-contrast/reduced-motion/text-scale settings through the shared UI theme.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/inventory_map_journal_manifest.tsv`.
- Updated the creative-development inclusion plan so the P1 UI row now routes through `verify_inventory_map_journal_polish_slice_pass.py`, the minimap verifier, the objective journal verifier, packaged smoke, and manual UI review.
- Added visual regression, human QA, accessibility, and first-ten-minutes checklist coverage.
- Wired the new verifier into full QA and local CI.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_inventory_map_journal_polish_slice_pass.py`
- `python3 Scripts/verify_inventory_map_journal_polish_slice_pass.py`
- existing minimap and objective-journal static verifiers
- module recompile
- Mac packaging
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Human QA Notes

Open the journal with `J` during at least three states: a fresh city before terminal solve, after terminal solve before survivor rescue, and after survivor rescue. The overlay should make language save, route map, field inventory, survivor intel, and city progress readable without relying on color alone.

## Remaining Art Hooks

This is a working C++ UI fallback, not final icon art. Future UI imports can replace the text-heavy sections with icon rows, swatches, and authored panels while keeping the same named runtime surfaces and save-state contracts.
