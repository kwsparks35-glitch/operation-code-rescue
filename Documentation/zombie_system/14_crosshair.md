# Item 14 — Crosshair + interaction prompt

**Status:** DONE — folded into the HUD pass (item 13).

See [13_hud.md](13_hud.md) for the full implementation. This item is
listed separately in the roadmap because crosshair feedback is the #1
thing players notice missing in any FPS, but in code the work is one
contiguous block in `UCodeRescueHUDWidget::RefreshHUD`.

## Quick reference

| What you're aiming at | Crosshair color | Prompt |
|---|---|---|
| Zombie | red | (none) |
| Coding terminal | yellow | `[E] open coding terminal` |
| Survivor | cyan | `[E] rescue survivor` |
| Pickup (ammo/medkit) | light green | `[E] grab supplies` |
| Language station | purple | `[E] swap programming language` |
| Empty space | default green | (blank) |

Trace range: 600 units. Tweak by editing the `End = Start + Dir * 600.0f`
literal in `CodeRescueHUDWidget.cpp` if you want a longer or shorter
"pickup focus" range.
