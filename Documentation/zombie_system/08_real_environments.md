# Item 8 — Real environment art per zone

**Status:** DEFERRED — needs a kit-bashed Megascans/PCG content pass that
can't be done from screen-driven automation.

## The current state

`ACodeRescueGameMode::SpawnZone` procedurally drops cube buildings around
each zone center (Anchorage Medical, Seattle Harbor, Tokyo Metro). The
geometry reads as a city grid only by silhouette — there are no doors,
windows, signs, props, or material variation. Functional, but doesn't
sell the post-apocalyptic theme.

The project's `Content/CodeRescueAssets/Environments/{Anchorage,Seattle,Tokyo}/`
folders exist as placeholders with `README_IMPORT_HERE.txt` notes. They
are the targets for the work below.

## Recommended approach

1. **Pick one zone first** — start with **Anchorage Medical District**
   because it's the first zone the player encounters and has the
   tightest theme (clinical, signage-heavy).

2. **Source assets.** Two viable sources, in order of preference:
   - **Quixel Megascans** — free for UE projects. Look for the Hospital,
     Modular Industrial, and Urban Decay packs. Drag a few hero
     buildings + 5–10 prop sets into
     `/Game/CodeRescueAssets/Environments/Anchorage/`.
   - **Fab marketplace** — the City Sample / Modular City packs are
     proven for this aesthetic. The launcher's "Add to Project" flow
     mirrors what we did for the zombie packs (see
     `zombie_packs_inventory.md`).

3. **Replace the procedural cubes per zone.** In
   `ACodeRescueGameMode::SpawnZone`, gate the existing `SpawnBlock`
   loop behind a config flag:

   ```cpp
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|World")
   bool bUseProceduralBlocks = true;
   ```

   When false for a zone, skip the block-spawn loop and instead spawn a
   pre-made `ULevelStreamingDynamic` sub-level holding the kit-bashed
   art for that zone. Keep the procedural fallback for zones where the
   art isn't done yet.

4. **PCG for prop scattering.** UE 5.7's PCG plugin (already enabled in
   `CodeRescueUnreal.uproject`) is ideal for this. Author a PCG graph
   that takes a zone bounds + a density slider and scatters debris,
   abandoned cars, posters, broken streetlamps. Same graph runs per
   zone with different inputs.

5. **Lighting baking** — see item 9.

## Effort budget per zone

- **Anchorage Medical:** ~6–10 hours (one focused session). 70% of that
  is asset selection + retargeting; the C++ glue is small.
- **Seattle Harbor:** ~6–8 hours; similar complexity, water surfaces add
  some material work.
- **Tokyo Metro:** ~8–12 hours; underground tunnels need specific
  modular geometry that's harder to find pre-made.

## Why this is deferred from the in-session work

I'm running the project through a screen-automation MCP. Browsing
asset libraries, drag-dropping into the level, eyeballing prop placement,
and iterating on lighting — all need a human at the desk. Doing it
through pixel-coordinate clicks is theoretically possible but would take
~5x as long and miss judgment calls (does this prop read at this scale?
does this material need a tweak?) that an artist makes instinctively.

## Acceptance test

Once Anchorage is done, drop a player into the zone and walk for 30
seconds. Without seeing the labels, can a friend identify which city
this is supposed to be? If yes, the zone is done. If no, more signage
or distinctive props needed.
