# Item 19 — Save-slot UI + autosave indicator

## What changed
1. **Three named save slots** via new `UCodeRescueSaveSlotsWidget`. Each
   row: slot label, "(saved)/(empty)" status, and three buttons:
   `[Save]`, `[Load]`, `[Delete]`. Clicking Save writes
   `OperationCodeRescue_Slot{N}` (N = 0..2). Load swaps the GameInstance's
   `SaveSlotName` and runs `LoadPersistentRun`. Delete removes the file.
2. **Autosave indicator hook**: `UCodeRescueGameInstance::SavePersistentRun`
   stamps `LastSaveWallSeconds = GetWorld()->GetTimeSeconds()` on every
   successful save. The HUD widget can read this and flash a small
   "Saving…" pip when `(Now - LastSaveWallSeconds) < 1.0`.

## Files touched
- `Source/CodeRescueUnreal/CodeRescueSaveSlotsWidget.h/.cpp` (new)
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h/.cpp` —
  `LastSaveWallSeconds` field + stamp on save.

## Wiring required (one-line each)
- Pause widget → "Save Slots" button:
  ```cpp
  if (UCodeRescueSaveSlotsWidget* W = CreateWidget<UCodeRescueSaveSlotsWidget>(PC, UCodeRescueSaveSlotsWidget::StaticClass()))
  {
      W->AddToViewport(150);
  }
  ```
- HUD widget → autosave pip (in `RefreshHUD`, after the existing crosshair logic):
  ```cpp
  if (GI && (World->TimeSeconds - GI->LastSaveWallSeconds) < 1.0f)
  {
      // show small "Saving..." text top-right
  }
  ```

## Design decisions
- Slot files use a `_Slot{N}` suffix on the existing `OperationCodeRescue_`
  prefix — the prior single-slot save (`OperationCodeRescue_Profile0`) is
  untouched, so old saves keep loading.
- Click handlers use lambdas that capture `SlotIdx` so we don't need a
  separate UFUNCTION per slot.
- Autosave pip is opt-in HUD code — no per-frame work added unless the HUD
  reads the field.

## Known limitations
- No timestamp display per slot (we don't read filesystem mtime). Slots
  just show "(saved)" or "(empty)".
- "Quick save" / "Quick load" hotkeys not wired (would be a 4-line addition
  in `PollDirectKeys`).

## Follow-up work
- File mtime → "Saved 12 minutes ago" per slot.
- F5 / F9 quick save / quick load hotkeys.
- Confirmation dialog on overwrite.
