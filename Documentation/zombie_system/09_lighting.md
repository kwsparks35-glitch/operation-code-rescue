# Item 9 — Lighting + post-process pass

**Status:** DEFERRED — perceptual pass that needs a real display, not
pixel-screenshots over a video stream.

## Current state

Per memory notes (`Disable_Lumen_Mac_Lighting_Fix.command`) Lumen is
disabled for Mac performance. The project ships with a SkyLight,
DirectionalLight, and SkyAtmosphere — all spawned procedurally by
`ACodeRescueGameMode::SpawnWorld`. There's no `APostProcessVolume` per
zone, so the visual mood is the same wherever the player is.

## Recommended approach

### Option A — stylized baked lighting (recommended for Mac)

1. After item 8 lands real environment geometry, mark the zone's static
   meshes as `Mobility=Static`.
2. Add a `BuildLighting` step to the deploy pipeline. UE's Build →
   Build All Levels with Static Lighting bakes Lightmass.
3. Per zone, drop an `APostProcessVolume` covering the zone bounds with
   distinct grading:
   - **Anchorage** — cool blue, moderate contrast, slight desaturation
     (2700K → 5500K → 3500K curve), heavy fog.
   - **Seattle** — green-cyan tint, wet-look reflections, warm street
     lamps as point lights.
   - **Tokyo** — neon-magenta and cyan saturation, very low ambient,
     bloom dialed up. Most "video-game" look.

### Option B — re-enable Lumen with quality budget

If Mac perf turns out fine on the M-series target machines:

```ini
[/Script/Engine.RendererSettings]
r.DynamicGlobalIlluminationMethod=1
r.ReflectionMethod=1
r.Lumen.HardwareRayTracing=0
r.Lumen.ScreenProbeGather.RadianceCache=1
r.Lumen.ScreenProbeGather.ImportanceSampling=1
r.Lumen.DiffuseIndirect.MaxMeshSDFTraceDistance=400
```

Test on the actual M4 Pro target. If frame time stays under 16 ms in
PIE, ship it. If not, fall back to Option A.

### Cheap intermediate

Even without baking, adding three `APostProcessVolume`s with distinct
`Grading > Saturation` and `Bloom > Intensity` settings makes the zones
read differently for ~10 minutes of work. This is the lowest-effort
"feels different" win.

## Why deferred

Lighting decisions need a human eyeballing on the actual display. Pixel
screenshots compressed through the screen-automation pipeline don't
reliably reveal banding, hot spots, color casts, or whether the mood
hits.

## Effort budget

- Cheap intermediate: ~15 minutes.
- Stylized baked: ~3–5 hours per zone (after item 8's geometry).
- Lumen re-enable: ~30 minutes to set, then perf testing on target.
