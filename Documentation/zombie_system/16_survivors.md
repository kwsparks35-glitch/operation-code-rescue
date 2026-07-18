# Item 16 — Real survivor variety

**Status:** DEFERRED — needs MetaHuman / character pack assets and VO
that aren't in the project yet.

## Current state

`ASurvivorActor` spawns as a primitive cube body + sphere head. It has
the hooks for a real character:

```cpp
USkeletalMesh* ProfessionalSurvivorMesh;
TSubclassOf<UAnimInstance> ProfessionalSurvivorAnimClass;
UNiagaraSystem* RescueBeaconVFX;
```

When `ProfessionalSurvivorMesh` is set, `BeginPlay` swaps the primitive
body for the skeletal mesh (per the existing pre-roadmap code). So the
plumbing is there — just no assets assigned.

## Recommended approach

### Step 1 — get four distinct character meshes

Cheapest source: **MetaHumans**. Free, photoreal, skeleton-compatible
with the bundled UE Mannequin AnimBPs. Workflow:

1. Editor → Window → Quixel Bridge.
2. Browse MetaHumans, pick four distinct presets (different ages,
   sexes, ethnicities — narrative variety is the goal).
3. Click each one's "Add to Project" — it lands at
   `/Game/MetaHumans/<Name>/`.
4. UE auto-imports the LODs, skeleton, and a base AnimBP.

Alternative cheap source: any character pack from Fab Marketplace. The
RamsterZ pack we already have actually ships a Mannequin-skeleton
female base mesh; you could deatomize it into a "Nurse Survivor" if you
want to reuse instead of downloading more.

### Step 2 — wire 4 BP subclasses

For each MetaHuman, create a `BP_Survivor_<Name>` blueprint subclassing
`ASurvivorActor`:

1. Content Browser → Right-click → Blueprint Class → pick
   `ASurvivorActor`.
2. Open the BP. In Class Defaults, set
   `ProfessionalSurvivorMesh = MetaHuman face/body skeletal mesh`,
   `ProfessionalSurvivorAnimClass = ABP_Manny_Female` (or whatever
   ships).
3. Set `SurvivorName` and `Story` to identifying strings — these show
   up in the rescue VO/HUD.
4. Optionally set `RescueBeaconVFX` to a Niagara system from the
   project.

### Step 3 — survivor-specific VO

Currently rescue is silent. Add a `USoundBase* RescueLine` to
`ASurvivorActor` and play it in `Rescue()` via
`UGameplayStatics::PlaySoundAtLocation`. Author one short line per
survivor:

- "Thank god you came. Let's go!"
- "I knew someone would make it through."
- "You're real. You're really here."
- "I thought I was the last one."

Free TTS (e.g. ElevenLabs free tier) is enough for prototyping.

### Step 4 — assign per-spawn in GameMode

Replace the single fallback in
`ACodeRescueGameMode::SpawnWorld`'s survivor loop:

```cpp
TArray<TSubclassOf<ASurvivorActor>> SurvivorClasses = {
    BP_Survivor_Anchorage_Doctor,
    BP_Survivor_Seattle_Engineer,
    BP_Survivor_Tokyo_Student,
    BP_Survivor_Tokyo_Officer,
};
for (int32 i = 0; ...)
{
    UClass* C = SurvivorClasses[i].Get();
    ASurvivorActor* S = GetWorld()->SpawnActor<ASurvivorActor>(C, ...);
}
```

The class array can be exposed on `ACodeRescueGameMode`'s defaults so
no recompile is needed when adding new survivors.

## Why deferred

MetaHumans requires browsing the Quixel Bridge tab and clicking through
a download flow — same problem class as the zombie packs but without
the Python-script automation we built for those. Doable, just ~30 min
per survivor of editor work.

Plus the VO step requires an external tool (ElevenLabs / a real voice
actor / record-yourself).

## Files that already support this

- `Source/CodeRescueUnreal/SurvivorActor.{h,cpp}` — has the mesh
  + AnimBP fields and the swap-on-BeginPlay logic.
- `Source/CodeRescueUnreal/CodeRescueGameMode.h` —
  `SurvivorActorClass` slot for a single override; needs to be widened
  to `TArray<TSubclassOf<>>` per the recipe above.

## Acceptance test

Walk up to each of the 4 survivors. They should:

1. Look distinct from each other (different MetaHuman / character pack).
2. Show their `SurvivorName` in the rescue prompt (HUD work in item 13
   already uses the actor type for the prompt; passing the name through
   would be a small extension).
3. Speak a unique line on rescue.
