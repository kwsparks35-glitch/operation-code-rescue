# Pass 7 — The 12%-speed trap, the sandbox screenshot mystery, trackpad fire
**Date:** 2026-07-17 · **Requested by:** Kenny · **Status:** shipped

Kenny's report: (1) game STILL extremely slow — "nothing is functioning at
appropriate speed"; (2) Cmd+Shift+4 freezes his mouse on the macOS
crosshair, and F12 claims success but no file ever appears; (3) he can aim
but cannot FIRE while aiming on his MacBook trackpad.

## 1. "EXTREMELY slow" — it was never the frame rate

Instrumented reproduction on his machine (M4 Pro, 3024×1964): the packaged
app on his save runs **117–120 FPS fullscreen native with full audio** —
measured by `[ResumeHealth]` pulses in every configuration. The game is
objectively fast. The real cause:

> **F10 "photo mode" set `SetGlobalTimeDilation(0.12)`** — 12% world speed —
> and hid the ENTIRE HUD, with no persistent on-screen indicator. F10 sits
> directly beside the F12 screenshot key I told Kenny to use. One accidental
> press put his whole session into slow motion with nothing to explain why —
> "nothing is functioning at appropriate speed" is a literal description of
> 12% time dilation.

Fixes: the F10 binding is REMOVED (TogglePhotoMode retained for a future
menu entry); BeginPlay force-restores `GlobalTimeDilation(1.0)` so no stale
slow-motion state can survive a spawn. Verifier
`verify_fading_review_trail_photomode_2026_07_04c.py` migrated: it now pins
that NO hotkey toggles photo mode.

## 2. Screenshots — the app is SANDBOXED

The packaged Mac app runs in an App Sandbox container
(`~/Library/Containers/com.operationcoderescue.CodeRescueUnreal/`). Every
"vanished" capture was really written — INSIDE the container — while writes
to the Desktop folder are denied by macOS (`MoveFile ... Error Code 1`).
Kenny's four "lost" F12 screenshots were found intact in the container.

New pipeline (`TakeGameplayScreenshot`, F12 or Cmd+Shift+4):
1. capture through the engine high-res screenshot system (packaged-proven);
2. attempt the direct move into `Screenshots_for_Correction/` (works in
   editor/unsandboxed runs), with no blocking retries;
3. on sandbox denial, rename the capture in place to `InGame_<ts>.png` —
   which appears in **`Screenshots_for_Correction/InGame_Captures/`**, a
   symlink (created from outside the sandbox) pointing at the container's
   screenshot folder;
4. the on-screen confirmation prints ONLY after the file verifiably exists,
   with the exact location — no more success messages for missing files.

About Cmd+Shift-4's frozen crosshair: that overlay belongs to macOS itself
(the OS shortcut cannot be suppressed by a game). The in-game capture fires
instantly on the same chord, so the file is already saved — press Esc to
dismiss the macOS overlay, or simply use F12.

## 3. Fire while aiming — trackpad physics

A MacBook trackpad reports ONE button at a time: with two fingers held down
(right-click = aim), a further click cannot deliver a left-click — firing
while holding aim is PHYSICALLY impossible on the pad. The aim control is
now a **toggle/hold hybrid** (`OnAimPressed`/`OnAimReleased`):

- **quick right-click (<0.35 s)** — sights LATCH up ("Sights locked — click
  to fire, right-click again to lower"); a normal click fires down the
  scope; right-click again lowers. Trackpad-complete.
- **long hold** — classic hold-to-aim; releasing lowers. Mouse-friendly.
- Wall-clock timed (immune to any time-dilation effects); modal UIs still
  auto-lower; the any-camera raise/restore behavior is unchanged.

## Verification (single run of everything together)

Scripted probe (`-CodeRescueMovementProbe`) on his real save, PACKAGED app,
fullscreen native: `after_quick_release ads=1 latched=1` →
`fired_while_latched ads=1` → `after_second_click ads=0 latched=0`;
screenshot `delivered=1` with the file confirmed on disk through his
folder's `InGame_Captures` link; `[ResumeHealth]` 120 FPS, `vel=900
maxwalk=900`. Watchdog **VERDICT: PASS** (photo-mode pin migrated), editor
integrated audit **21/21**, packaged integrated audit **21/21** on the final
binary.

## Notes
- The packaged app being sandboxed explains EVERY historical absolute-path
  write failure (screenshots, logs) — file exchange with the app must go
  through its container; symlinks INTO the container from user folders work.
- Kenny's four recovered captures remain in `InGame_Captures/`
  (HighresScreenshot00000–00003.png).
