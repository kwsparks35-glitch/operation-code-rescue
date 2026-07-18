# Item 1 — Smarter AnimBP picker

**Status:** DONE

## Problem

The first version of `Scripts/build_zombie_variants_table.py`'s `pick_animbp`
heuristic returned the first `AnimBlueprint` asset it found under each pack
folder. For the RamsterZ "Zombie Female: Nurse" pack this picked
`/Game/ZombieFemale/Demo/Characters/Mannequins/Rigs/ABP_MannyPostProcess` —
the bundled UE Mannequin's *post-process* AnimBP. Post-process AnimBPs are
overlays that run on top of a real locomotion AnimBP; they don't drive
walk/run blendspaces themselves. Result: NurseFemale zombies stood in
T-pose with a subtle pose correction running on top.

## Fix

`Scripts/build_zombie_variants_table.py` `pick_animbp` now:

1. Hard-rejects any AnimBP whose name contains a post-process token
   (`postprocess`, `post_process`, `_pp`, `pp_`, `cosmetic`).
2. Scores remaining candidates by locomotion keywords (`zomb`, `walk`,
   `run`, `idle`, `locomotion`, `anim_bp`, `abp_z`, `abp_m04`, `abp_f01`,
   `abp_dog`, `abp_urban`).
3. Adds a +5 bonus to any candidate whose `TargetSkeleton` asset-tag
   matches the chosen mesh's skeleton — so a generic mannequin AnimBP
   can't beat a pack's own zombie-targeted one.
4. Falls back to the first sorted candidate only when all scored equally.

## Files touched

- `Scripts/build_zombie_variants_table.py` — `pick_animbp()` rewrite

## How to verify

1. In the editor: bottom dropdown → **Python**, paste:
   ```
   exec(open(r"…/Scripts/build_zombie_variants_table.py").read())
   ```
2. In the Output Log filter on `[zv]`. For each pack, look at the
   `animBP =` line. Acceptable picks per pack:
   - `DogZombie` — None (the pack has no AnimBP) is correct.
   - `UrbanZombie4` — `ThirdPerson_AnimBP` is fine; pack ships only this one.
   - `BusinessSuit` / `BloatedFemale` — anything under
     `/Game/YI_ModularZombies` whose name contains the corresponding
     `zombiem04` or `zombief01`.
   - `NurseFemale` — should NOT be `ABP_MannyPostProcess`. If it still is,
     the pack ships no other AnimBP and you'll need to hand-author one
     (or copy `ThirdPerson_AnimBP` from UrbanZombie4 and retarget).
   - `BaseMesh` — None is correct (rivai pack has no AnimBP).
3. Open `/Game/CodeRescueAssets/DT_ZombieVariants` in the Content Browser
   and inspect the `AnimBPClass` field on each row.

## Open follow-ups

If a pack genuinely ships only a post-process AnimBP (RamsterZ Nurse may
fall here), the picker correctly leaves that row's AnimBPClass blank and
the variant's mesh will static-pose. The right fix is to hand-author or
copy a locomotion AnimBP for that pack — see item 16's doc for the
broader "real characters" plan.
