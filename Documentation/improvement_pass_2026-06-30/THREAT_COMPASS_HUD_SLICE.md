# Threat Compass HUD Slice

This slice continues the June 25 combat readability and accessibility backlog by making nearby hostile pressure visible in the active HUD. It complements the threat-audio caption work by giving players a persistent visual way to read the closest threat even when audio, subtitles, or minimap scanning are not enough.

## Implementation

- Added `ThreatCompassText` to `UCodeRescueHUDWidget`, positioned below the active objective readout.
- Added `FCodeRescueThreatHudInfo` and `GetNearestHudThreat()` to scan living bosses and zombies once per HUD refresh.
- Reports threat urgency, encounter-director role, variant label, player-relative direction, and distance.
- Prioritizes boss and elite pressure with separate urgency labels, while preserving the existing close-hostile reload/alert behavior.
- Replaced the old tactical readout's plain `Nearest hostile` distance with a fuller `Threat` line.
- Honors high-contrast HUD mode by using the saved accessibility flag when coloring the threat compass.
- Updated `Content/CodeRescueData/enemy_readability_manifest.tsv` with a `ThreatCompassHUD` row for future review.

## Player Impact

The HUD now reads like `THREAT COMPASS  ELITE | pressure spitter | 24m RIGHT` or `THREAT COMPASS  BOSS | boss infected | 43m AHEAD`. This helps players make better moment-to-moment decisions about whether to reload, use a medkit, back away from a boss, or turn toward an elite threat.

## Verification

Added `Scripts/verify_threat_compass_hud_slice_pass.py`, wired into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the HUD field, threat info structure, boss/zombie scanning, role and variant labels, urgency labels, high-contrast coloring, tactical readout replacement, manifest row, progress entry, documentation, and QA wiring.

## Remaining QA

Human playtest should confirm that the strip does not obscure objective text on unusual aspect ratios and that boss/elite priority feels helpful during dense hordes.
