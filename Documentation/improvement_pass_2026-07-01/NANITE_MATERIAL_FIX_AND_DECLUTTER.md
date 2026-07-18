# Nanite Material Root-Cause Fix + World-Text Declutter (2026-07-02, follow-up)

Follow-up pass after the tutorial/kit/lighting work. Headline: found and fixed the **true root
cause** of the checkerboard surfaces that three earlier attempts only partially addressed. Every
change was validated in the packaged Mac app; the decisive evidence came from the runtime log.

## THE big one — Nanite was forcing the checkerboard Default Material

**Symptom:** large flat surfaces (safehouse panels, and — critically — the authored city-kit
facades) rendered as a teal/magenta checkerboard instead of their assigned concrete/brick/steel.

**Why the earlier fixes didn't land it:** a null-material fallback (only touches empty slots) and
cooking `StarterContent/Textures` both assumed a missing-asset problem. It wasn't. The runtime log
named the real cause outright:

```
LogMaterial: Warning: Material /Game/StarterContent/Materials/M_Concrete_Panels missing
             bUsedWithNanite=True! Default Material will be used in game.
```

The imported meshes cook as **Nanite** geometry, but the StarterContent materials assigned to them
were never compiled with the `bUsedWithNanite` usage flag. In the editor UE silently recompiles the
material to add the flag; in a **cooked build it cannot recompile**, so the renderer substitutes the
checkerboard **Default Material**. This hit every StarterContent material the game uses — including
the ones I had just assigned to the city-kit facades, so the "real buildings" were actually
rendering as the default checker too.

**Fix — disable Nanite project-wide (`Config/DefaultEngine.ini`):**

```ini
[/Script/Engine.RendererSettings]
r.Nanite.ProjectEnabled=False
```

This is the correct call for the target: the project's own config comments already note that
"M1 Macs cannot rely on Nanite and SM6 requires newer macOS," and the runtime falls back to
non-Nanite geometry anyway. With Nanite off, every mesh renders through the standard material path.

**Verified in the packaged build:**
- Log now shows `LogConfig: Set CVar [[r.Nanite.ProjectEnabled:0]]`.
- **Zero** `bUsedWithNanite` warnings and **zero** `Default Material will be used` warnings
  (previously many).
- Playtest: the safehouse interior renders solid dark **concrete-panel walls with real surface
  detail** (horizontal panel lines), not checker. Surfaces across the world now show their true
  materials.

This single fix is the highest-impact aesthetic change of the whole art effort — it's what finally
makes the authored materials (concrete/brick/steel) actually appear in game.

## Supporting material fixes (kept — they harden other cases)
- **`SpawnStaticMeshProp` fallback + override** (`CodeRescueGameModeSpawning.cpp`): null material
  slots are filled with solid concrete; an optional `MaterialOverride` forces a solid material on
  known-bad props. Zero risk to valid materials.
- **Glass → steel**: the StarterContent glass panes (safehouse observation glass + two explicit
  `M_Glass` uses) are switched to `M_Metal_Steel`, since translucent glass read poorly.
- **`StarterContent/Textures` added to `DirectoriesToAlwaysCook`** (`DefaultGame.ini`): guarantees
  every StarterContent material's textures are in the pak (belt-and-suspenders now that materials
  actually render).

## World-text declutter (Kenny: "words competing for attention")
`IsEssentialGuideText` was tightened so only genuine control prompts and the core-loop labels
(`[E]`, `WASD`, `ENTER`, `BACKSPACE`, `OBJECTIVE`, `SELECT CODING LANGUAGE`, `RESUME SAVE`) stay as
on-screen text. Descriptive/location/lore/weather labels now become compact hover markers instead.
**Playtest-verified:** the safehouse walls that previously showed stacked "FLARES / SMOKE / X slot"
text are now clean.

## Lighting continued (entry-plaza over-exposure)
- Weather-accent emissive strips dialed down (`*2.2 → *1.15`, `*1.65 → *1.0`) so they stop blooming
  into a teal glow at the bright open plaza.
- Auto-exposure tightened again (bias `0.25 → 0.1`, max `1.3 → 1.15`).
- Result: enclosed areas read neutral/warm with proper shadows; the fully-open entry plaza is
  improved but still the brightest spot (auto-exposure adapts up in a large open area).

## Zombie ragdoll (physics) — code-verified
The death-ragdoll system (`CodeZombieActor::TryActivateDeathRagdoll`) is a complete implementation:
enabled by default (`bEnableDeathRagdoll = true`), guarded on `GetPhysicsAsset()`, capped at 10
active corpses, applies a death impulse, and is called on death. Confirmed the zombie skeletal
meshes **have** cooked physics assets (`PHYS_ZombieM04`, `PHYS_Zombie_F01`, `DogZombie_PhysicsAsset`,
etc.), so ragdoll will trigger for skeletal zombies. A live on-screen kill wasn't captured because
the early safehouse/spawn area is a safe zone and `T` auto-travel cycles within it; visual
confirmation needs a deeper combat playthrough.

## Honest remaining follow-ups
- **Distant translucent panels near the safehouse** still read as a teal grid. The log is clean (no
  material failures), so these render a *valid* material — most likely translucent glass with a
  teal tint, i.e. an art-direction choice rather than a bug. Pinning the exact actor is a 2-minute
  job **in the editor** (click it, read its material) and the right tool for it.
- **Entry-plaza brightness** (open-area auto-exposure).
- **Anchorage `GlassPanel_E`** (a different city) still uses the glass mesh's default material.
- **Live ragdoll capture** pending a combat playthrough.

## Build / package facts
- 11 packages this session; each `Package_Mac_App.command` ~1.5–2.5 min. The Nanite change forced a
  shader re-cook (~2 min). Runtime log:
  `~/Library/Containers/com.operationcoderescue.CodeRescueUnreal/Data/Library/Logs/CodeRescueUnreal/CodeRescueUnreal.log`
  — grepping it for `LogMaterial: Warning` is the fastest way to catch cooked-material problems.
