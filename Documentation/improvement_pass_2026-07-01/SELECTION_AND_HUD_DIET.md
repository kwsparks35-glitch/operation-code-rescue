# Language Selection + HUD Diet — 2026-07-01 (live-verified)

Two player-facing corrections from Kenny's playtest, each fixed and then **verified by Claude playing
the packaged app via computer-use** (not reasoned from code).

## 1. Language selection (commit `16ef484`, after `620917a`/`1f2e979`)

**Symptom:** Tab auto-selected C++; menu buttons and keyboard focus did nothing.

**Root cause (layered, found by playtesting):**
- UMG button `OnClicked` and widget `NativeOnKeyDown` **never delivered input** in the packaged build,
  even after the RebuildWidget visibility fix — Slate focus + GameAndUI routing is unreliable here.
- `Tab`/`Enter` were reaching the pawn's `Interact()` and insta-deploying the spawn-adjacent stations
  (9000uu assist radius), so a stray keypress picked a language before the player saw the menu.

**Fix:** deploy is handled entirely in the pawn's **polled** input path (the same mechanism that makes
WASD/T reliable), independent of Slate/UMG:
- Arrows/Left-Right highlight; number keys `1-6` pick directly; **Enter / Space / E deploys** the
  highlighted language (resume its save if present, else fresh) and loads the city.
- The launch menu no longer grabs keyboard focus; input mode is GameOnly. The visible menu is the
  on-screen guide + highlight, not the input surface.
- During the language gate, all other polled actions and the station-interact are suppressed, so
  nothing fires behind the menu.

**Verified:** one press of Enter at CHOOSE CODING LANGUAGE deploys the highlighted track into New York.

## 2. HUD diet (Kenny: "decrease the font; keep only absolute necessities; rest behind a menu")

- Default `UITextScale` 1.0 → **0.90** (raisable in Settings).
- `bMinimalHUD` (default on) in `UCodeRescueHUDWidget`:
  - `StatusText` collapses from a 5-line campaign dump to **one compact vitals line**:
    `HP x/y  Armor a/b  Ammo m/n  Kits p/q  <Language>`.
  - Collapsed off-screen (still populated, surfaced via **J** journal / **P** pause): navigation strip,
    threat compass, sound-cue readout, field checklist, squad status, tactical readout, weapon strip,
    reload status, autosave line, objective toast, and the minimap.
  - Kept on screen (necessities): the vitals line, health bar, current-objective line, crosshair, and
    the interact prompt.
- Spawn no longer dumps the full control sheet; deploy shows **two** short lines (STEP 1/3 + key hints),
  and the selection-controls line while the language gate is open.
- Terminal brief strips internal dev-plan paragraphs (Progression/Character/Flow/Accessibility/QA/
  Architecture), leaving the fiction + lesson + worked example + tests for the student.

**Verified:** in-city HUD is a single top line in smaller type; T→E opens the terminal with the teach payload.

## Honest status / what's next (not done this pass)

Kenny also asked to "continue working on UI/UX for pedagogy+playability+aesthetics" and "continue
character/world/physics." Those are larger multi-pass efforts, deliberately **not** claimed here.
Concrete next steps, in priority order:
1. Route the runtime city generator at the imported `/Game/CodeRescueArt` kit + characters (they are
   built + packaged; the generator still spawns primitives). This is the single biggest aesthetic win.
2. Remaining invisible-UMG widget sweep (settings/death/victory/etc.) per `CLAUDE_TO_CODEX.md`.
3. Trim the residual world-text and the safehouse-proximity toast; terminal readability polish.
4. Physics: collision channels + ragdoll/impulse pass (Top-50 C21-C23).

## Verify
```
python3 Scripts/verify_ui_declutter_pass.py
python3 Scripts/claude_oversight_watchdog.py
```
