# UX pass — Enter-to-select + world-text declutter (2026-07-01)

Two player-facing fixes requested from a screenshot where overlapping world text made the level
unreadable. Both are implemented additively; a Mac compile + playtest is the Definition-of-Done gate.

## 1. Language screen: press Enter to select your language

`UCodeRescueMainMenuWidget` now handles keyboard input:
- **Enter** (or gamepad A) confirms the currently highlighted language and starts that focused run
  (`StartLanguageRun`).
- **Arrow keys / d-pad** move the highlight (`CycleSelectedLanguage` → `SetSelectedLanguage`, which
  refreshes the on-screen selection).
- **Number keys 1–6** pick a language directly (Java, C, C+, C++, Python, MATLAB).
- `NativeSupportsKeyboardFocus()` returns true and the widget already grabbed focus, so key events
  reach it. Mouse clicking still works exactly as before.

Files: `CodeRescueMainMenuWidget.{h,cpp}`.

## 2. World-text declutter: hovering id markers + a separate scrollable reader

**The problem:** the level rendered dozens of full paragraphs as world text (via `SpawnGuideText`,
called 228×), which competed for the player's attention.

**The fix — one chokepoint, whole level declutters.** `ACodeRescueGameMode::SpawnGuideText` now, for
any *substantive* message (multi-line or > 24 chars), spawns a compact **`ACodeRescueMessageMarkerActor`**
instead of the paragraph:
- The marker shows a short **unique id** (e.g. `[007] OBJECTIVE STORY`) hovering over the message's
  location, billboards toward the player, and reveals a `READ [E]` prompt when the player is near.
- Reading it (look at / stand near + the existing interact key **E / Enter / Tab / G**) opens
  **`UCodeRescueMessageReaderWidget`** — a separate, calm, **scrollable** screen showing the full title
  and paragraph. Close with E / Enter / Esc.
- Interaction reuses the project's existing pattern: the marker is tagged `MessageMarker` and exposes
  `OpenMessageReader()`, which `ACodeRescueCharacter::Interact()` calls by name — the same decoupled
  approach as the Helipad, so no new input bindings and no header dependency were added to the character.
- Short one-line signs (≤ 24 chars) keep their existing compact display; only the attention-competing
  paragraphs become markers. Launch with `-NoHoverMarkers` to restore the legacy full-text path.

Files: new `CodeRescueMessageMarkerActor.{h,cpp}`, new `CodeRescueMessageReaderWidget.{h,cpp}`;
edits to `CodeRescueGameMode.cpp` (SpawnGuideText) and `CodeRescueCharacter.cpp` (3 small, Helipad-pattern
edits: recognize + assist-find + dispatch the marker).

## Honest status / Definition of Done

- **Done here:** both features authored; structurally checked (balanced, UE-idiomatic); the marker/reader
  are cook-safe (engine primitives + standard UMG). Verifier `Scripts/verify_ui_declutter_pass.py` passes;
  oversight watchdog reports 0 regressions. New `.cpp` files auto-compile (no Build.cs change needed).
- **Your Mac gate (I can't do it from here):** compile the module and playtest one city — confirm the
  wall of text is now markers, that reading opens the scroll panel, and that Enter launches the chosen
  language. Then commit.
- **Tunable:** `-NoHoverMarkers` restores legacy text; marker threshold is the 24-char / multi-line rule
  in `SpawnGuideText` (easy to change if you want even short labels markerized).

## Verify
```
python3 Scripts/verify_ui_declutter_pass.py
python3 Scripts/claude_oversight_watchdog.py
```
