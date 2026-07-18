# Item 9 — Per-zone post-process volumes

## What changed
`ACodeRescueGameMode::SpawnPerZonePostProcessVolume(CityIndex, Origin, Accent)`
spawns one `APostProcessVolume` per city, sized to cover the full 50× city
(scale `(95, 82, 30)` units pre-CityExtent). City color grading cycles
through three presets indexed by `CityIndex % 3`:

| Preset | Cities | Saturation | Contrast | Gamma | Vignette |
|---|---|---|---|---|---|
| Anchorage cool/blue | 0, 3, 6, … | (0.85, 0.85, 1.05) | 1.05 | (0.95, 0.95, 1.02) | 0.35 |
| Seattle overcast | 1, 4, 7, … | (0.7, 0.7, 0.7) | 0.95 | 1.0 | 0.45 |
| Tokyo neon | 2, 5, 8, … | (1.2, 1.05, 1.25) | 1.10 | (1.05, 0.98, 1.05) | 0.25 |

## Files touched
- `Source/CodeRescueUnreal/CodeRescueGameMode.h/.cpp`

## Design decisions
- Bounded volume (not `bUnbound=true`) so two adjacent cities don't fight for
  active grading. The 95×82×30 box is generous enough that the player rarely
  walks out of one before entering the next.
- Color presets cycle every 3 cities so the 342-city campaign feels visually
  varied without bespoke per-city tuning.

## Known limitations
- No bloom / chromatic-aberration overrides yet. The presets are
  contrast/saturation-only.
- Lumen and Nanite are not enabled, so per-zone GI tuning is moot for now.

## Follow-up work
- Add a 4th "post-apocalyptic" preset (heavy desaturation + green tint) for
  high-difficulty tier cities.
- Bloom + auto-exposure adjustments per preset.
- Consider re-enabling Lumen on the hero cities only, gated by GameMode.
