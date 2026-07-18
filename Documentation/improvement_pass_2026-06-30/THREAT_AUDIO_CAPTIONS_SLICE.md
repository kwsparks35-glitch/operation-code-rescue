# Threat Audio Captions Slice

This slice continues the June 25 accessibility/readability backlog by giving nearby hostile audio events a subtitle-backed fallback. It supports `TOP_50_RECOMMENDATIONS` accessibility/readability guidance, `CHARACTER_ANIMATION_DEEPDIVE` combat readability, and the release dossier's first-session clarity goals.

## Implementation

- Added `ACodeZombieActor::PushThreatCaption`, a proximity-limited and cooldown-limited subtitle helper for hostile events.
- Added direction labels from the player's facing: `ahead`, `right`, `left`, `behind`, and `here`.
- Added variant labels for dog, urban, business, bloated, nurse, base-mesh, spitter, charger, boomer, and fallback infected threats.
- Added encounter-director role prefixes for anchor, flanker, pressure, and sentinel threats.
- Routed captions through the existing `UCodeRescueSubtitlesWidget::Push` path, so they honor the saved subtitles toggle and subtitle scale.
- Added caption hooks to ambient growl, melee attack, barricade attack, death, spitter acid, charger dash, and boomer split-spawn events.
- Updated `Content/CodeRescueData/enemy_readability_manifest.tsv` with a `ThreatAudioCaptions` row for future review.

## Player Impact

Players with subtitles enabled now receive short tactical captions such as `[Threat ahead]: pressure spitter acid spit, 14m.` when nearby hostile audio or elite pressure events occur. The captions are intentionally brief and throttled per zombie so they support low-audio play without flooding mission dialogue.

## Verification

Added `Scripts/verify_threat_audio_captions_slice_pass.py`, wired into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the subtitle include, helper declaration, cooldown/proximity gating, direction/variant/role labels, event hook coverage, manifest row, progress entry, documentation, and QA wiring.

## Remaining QA

Human playtest should confirm the caption cadence during large hordes and tune cooldowns if mission subtitles are being delayed too aggressively.
