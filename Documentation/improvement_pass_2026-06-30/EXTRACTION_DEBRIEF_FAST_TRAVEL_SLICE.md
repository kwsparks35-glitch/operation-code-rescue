# Extraction Debrief Fast-Travel Slice

Date: 2026-06-30

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: called for rescue set-pieces to be spatially clear and emotionally legible, especially around helipad extraction.
- `TOP_50_RECOMMENDATIONS.pdf`: emphasized stronger post-objective feedback and clearer next-step flow after major player actions.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasized demo-ready loops, save/load clarity, and reviewable implementation coverage.

## Implementation

Extended `UCityFastTravelWidget` so the helipad menu can act as an extraction debrief when opened from an extraction-ready helipad. `AHelipadActor::OpenFastTravelMenu()` now passes the helipad's city, survivor, ready state, and accent color into the widget before it is added to the viewport.

When the source helipad is extraction-ready, the widget now shows:

- an extraction debrief title
- rescued survivor and city context
- the active coding language save confirmation
- a `Continue operation` button targeting the next incomplete city
- the existing cleared-city fast travel list

The continue action uses `FCodeRescueCampaign::GetFirstIncompleteCityIndex()` so it follows the same completion rules as the rest of the campaign: a city counts complete only when its terminal is solved and its survivor has been rescued.

## Save Behavior

Fast travel now saves immediately after teleporting the player. This keeps the active language save aligned with the player's post-extraction location, so closing the game after redeploying resumes from the correct language-specific progression point.

## Player Impact

The helipad now closes the rescue loop in three steps:

- the world marks the helipad extraction-ready after survivor rescue
- interacting with the helipad shows an extraction debrief instead of only a generic destination list
- the player can continue to the next city or redeploy to a solved city without losing the current language save state

## Verification

Added `Scripts/verify_extraction_debrief_fast_travel_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks:

- helipad-to-widget extraction context wiring
- debrief UI construction
- next city selection through campaign completion helpers
- active language save confirmation text
- post-teleport save behavior
- documentation and QA wiring
