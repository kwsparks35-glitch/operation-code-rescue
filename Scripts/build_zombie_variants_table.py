"""
build_zombie_variants_table.py
================================

One-shot editor automation: scans each landed Fab zombie pack under /Game/,
finds the (most plausible) skeletal mesh + AnimBlueprint per pack, and creates
or updates the DataTable at /Game/CodeRescueAssets/DT_ZombieVariants using the
FZombieVariantRow row struct from the CodeRescueUnreal C++ module.

How to run
----------
1. Open CodeRescueUnreal.uproject in UE 5.7.
2. Window → Output Log → switch the input mode dropdown to "Python".
3. Paste:
       exec(open(r"/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Scripts/build_zombie_variants_table.py").read())
4. Watch the log for the per-pack mesh/AnimBP it picked. If anything is wrong,
   open DT_ZombieVariants and tweak the row by hand — re-running the script is
   safe (it overwrites rows but does NOT add unrelated rows).
5. On the GameMode (default class or your Blueprint subclass), set
   ZombieVariantTable to /Game/CodeRescueAssets/DT_ZombieVariants.

Heuristics
----------
For each pack root we ask the asset registry for:
  * all SkeletalMesh assets   → pick the largest-LOD0-vert-count mesh (proxies
    for "the hero character", ignoring face/cloth-only props)
  * all AnimBlueprint assets  → pick the one whose AnimGraph references the
    chosen mesh's skeleton; fall back to the first one in the pack
"""

import unreal
import os

# ----------------------------------------------------------------------------
# Configuration: one entry per EZombieVariant we want to author. Order does
# NOT matter; the data-table row name comes from the variant key.
# ----------------------------------------------------------------------------
PACK_CONFIG = [
    # (variant_enum_value, content_root, display_name,
    #  health_mul, damage_mul, speed_mul, mesh_scale, zone_weights{0,1,2},
    #  name_filter_or_None)
    #
    # Zone keys: 0=Anchorage Medical, 1=Seattle Harbor, 2=Tokyo Metro
    #
    # Tuning rationale (item 5 of the roadmap; see
    # Documentation/zombie_system/05_variant_tuning.md):
    #
    #   - DogZombie: fast & light, weighted heavier in Seattle (the wider
    #     harbor area benefits from a quick attacker). Mesh ~0.55x scale
    #     so it reads as a four-legged silhouette next to humanoids.
    #   - UrbanZombie4: the workhorse humanoid. Heaviest weight in
    #     Anchorage (medical district = clinical, urban-zombie aesthetic).
    #   - BusinessSuit (Yarrawah M04): "office worker" silhouette — fits
    #     Seattle/Tokyo evac contexts more than the wilderness Anchorage.
    #   - BloatedFemale (Yarrawah F01): tank archetype. Weighted toward
    #     Tokyo (the final zone — ramps difficulty). HealthMul 1.6 in the
    #     row but the InitializeFromVariant clamp caps it at 1.30 so on
    #     Hard difficulty (1.6 mul) the EFFECTIVE health is 1.30x base
    #     instead of 1.6*1.6=2.56x.
    #   - NurseFemale: medical-themed, fits Anchorage best. Free pack.
    #   - BaseMesh (rivai): NO ANIMATIONS — only weight 0.05 per zone so
    #     it's an occasional curiosity instead of a T-pose plague.
    ("DogZombie",     "/Game/DogZombie",         "Dog Zombie",
     0.55, 0.7,  1.45, 0.55, {0: 0.6, 1: 1.6, 2: 0.6}, None),
    ("UrbanZombie4",  "/Game/UrbanZombie4",      "Urban Zombie 4",
     1.0,  1.0,  1.0,  1.0,  {0: 1.8, 1: 1.0, 2: 0.9}, None),
    ("BusinessSuit",  "/Game/YI_ModularZombies",
     "Zombie - Business Suit",
     1.1,  1.05, 0.95, 1.0,  {0: 0.4, 1: 1.6, 2: 1.4}, "zombiem04"),
    ("BloatedFemale", "/Game/YI_ModularZombies",
     "Zombie - Bloated Female",
     1.6,  1.25, 0.7,  1.10, {0: 0.6, 1: 1.0, 2: 1.7}, "zombief01"),
    ("NurseFemale",   "/Game/ZombieFemale",      "Zombie Female: Nurse",
     0.95, 1.0,  1.05, 1.0,  {0: 1.4, 1: 0.9, 2: 0.7}, None),
    # rivai BaseMesh: rarity 0.05. With other variants summing to ~3-5
    # weight per zone, BaseMesh shows up roughly 1-2% of spawns — a rare
    # static-pose oddball rather than a T-pose plague.
    ("BaseMesh",      "/Game/Zombie",            "Zombie (rivai, base mesh)",
     0.85, 0.7,  0.85, 1.0,  {0: 0.05, 1: 0.05, 2: 0.05}, None),

    # ---- #29 elites (re-use the closest-fit pack mesh for the silhouette;
    # the gameplay difference is driven by C++ behavior in
    # ACodeZombieActor::Tick keyed off the Variant enum). Lower zone
    # weights overall — elites should be rare interruptions, not the norm.
    #
    # EliteSpitter:  ranged acid attack — reuse the BloatedFemale silhouette
    #                because it reads as "swollen / bursting", and we want
    #                visual variety from the regular humanoids.
    # EliteCharger:  sprinter — reuse the BusinessSuit silhouette so it
    #                still reads as "former office worker that broke loose".
    # EliteBoomer:   AoE-on-death — reuse the UrbanZombie4 silhouette since
    #                it's the most generic shape and the explosion is the
    #                memorable part, not the body.
    ("EliteSpitter",  "/Game/YI_ModularZombies",
     "Elite — Spitter (ranged acid)",
     1.20, 1.10, 0.85, 1.10, {0: 0.20, 1: 0.30, 2: 0.45}, "zombief01"),
    ("EliteCharger",  "/Game/YI_ModularZombies",
     "Elite — Charger (sprint+knockdown)",
     1.10, 1.30, 1.50, 1.05, {0: 0.30, 1: 0.30, 2: 0.55}, "zombiem04"),
    ("EliteBoomer",   "/Game/UrbanZombie4",
     "Elite — Boomer (explodes on death)",
     1.30, 1.40, 0.80, 1.15, {0: 0.20, 1: 0.30, 2: 0.55}, None),
]

DT_PATH        = "/Game/CodeRescueAssets/DT_ZombieVariants"
DT_PACKAGE_DIR = "/Game/CodeRescueAssets"
DT_NAME        = "DT_ZombieVariants"
ROW_STRUCT_PKG = "/Script/CodeRescueUnreal"
ROW_STRUCT_NAME = "ZombieVariantRow"

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
asset_lib      = unreal.EditorAssetLibrary
asset_tools    = unreal.AssetToolsHelpers.get_asset_tools()
log = unreal.log


def list_assets_under(path: str, class_name: str):
    """Return sorted asset paths of `class_name` under content `path`.
    UE 5.7 dropped AssetData.object_path; reconstruct from package+asset name."""
    if not asset_lib.does_directory_exist(path):
        log(f"[zv] WARNING: missing pack root {path}")
        return []
    filt = unreal.ARFilter(
        class_names=[class_name],
        package_paths=[path],
        recursive_paths=True,
    )
    out = []
    for d in asset_registry.get_assets(filt):
        # Reconstruct "/Game/.../Foo.Foo" form. package_name is the on-disk
        # path; asset_name is the in-package object name (typically same as
        # the file's basename). Combined this matches the legacy object_path.
        pkg = str(d.package_name)
        name = str(d.asset_name)
        out.append(f"{pkg}.{name}")
    out.sort()
    return out


def pick_skeletal_mesh(pack_root: str, name_filter: str = None):
    """Pick the SkeletalMesh that's most likely the hero character.
    name_filter is a lowercase substring required in the asset name; used
    to disambiguate the two Yarrawah variants sharing /Game/YI_ModularZombies/."""
    candidates = list_assets_under(pack_root, "SkeletalMesh")
    if name_filter:
        candidates = [p for p in candidates if name_filter in p.split(".")[-1].lower()]
    if not candidates:
        return None
    # Heuristic: prefer assets whose name contains 'zomb', 'body', or matches
    # the pack folder name. Fall back to the first sorted entry (deterministic).
    pack_leaf = pack_root.rstrip("/").split("/")[-1].lower()
    keywords = ["zomb", "body", "char", pack_leaf]
    scored = []
    for path in candidates:
        name = path.split(".")[-1].lower()
        score = 0
        for kw in keywords:
            if kw and kw in name:
                score += 1
        scored.append((score, path))
    scored.sort(key=lambda x: (-x[0], x[1]))
    return scored[0][1]


def _safe_asset_data(path: str):
    """get_asset_by_object_path's signature varies across UE versions; on
    5.7 it expects a SoftObjectPath. Wrap and silently degrade to None so
    one missing asset doesn't take down the whole script."""
    if not path:
        return None
    try:
        sop = unreal.SoftObjectPath(path)
        return asset_registry.get_asset_by_object_path(sop)
    except Exception:
        try:
            return asset_registry.get_asset_by_object_path(path)
        except Exception:
            return None


def _safe_tag(data, tag_name: str):
    if data is None:
        return None
    try:
        val = data.get_tag_value(tag_name)
        return str(val) if val else None
    except Exception:
        return None


def pick_animbp(pack_root: str, target_skeleton_path: str = None,
                name_filter: str = None):
    """Pick a locomotion AnimBlueprint.

    Picker policy (in order):
      1. Apply name_filter if provided (Yarrawah disambiguation).
      2. Skip post-process AnimBPs — names containing 'postprocess',
         'post_process', 'pp_', 'cosmetic'. These run as overlays on top of
         the main locomotion AnimBP and don't drive walk/run on their own;
         picking one (the bug we hit on NurseFemale's first pass) leaves the
         zombie in T-pose forever.
      3. Strongly prefer names with locomotion keywords: 'zomb', 'walk',
         'run', 'idle', 'locomotion', 'anim_bp' (without 'post').
      4. Tie-break by preferring AnimBPs whose TargetSkeleton tag matches
         the chosen mesh's skeleton (so a generic ABP_Manny won't beat the
         pack's own zombie AnimBP).
      5. Fall back to the first sorted candidate.
    """
    candidates = list_assets_under(pack_root, "AnimBlueprint")
    if name_filter:
        candidates = [p for p in candidates if name_filter in p.split(".")[-1].lower()]
    # Reject post-process AnimBPs outright — they're overlays, not locomotion.
    POSTPROC_TOKENS = ("postprocess", "post_process", "_pp", "pp_", "cosmetic")
    candidates = [p for p in candidates
                  if not any(tok in p.split(".")[-1].lower() for tok in POSTPROC_TOKENS)]
    if not candidates:
        return None

    LOCOMOTION_KEYWORDS = ("zomb", "walk", "run", "idle", "locomotion",
                           "anim_bp", "animblueprint", "abp_z", "abp_m04",
                           "abp_f01", "abp_dog", "abp_urban")
    scored = []
    for path in candidates:
        name = path.split(".")[-1].lower()
        score = 0
        for kw in LOCOMOTION_KEYWORDS:
            if kw in name:
                score += 2
        # Skeleton-match bonus on top of the keyword score.
        if target_skeleton_path:
            data = _safe_asset_data(path)
            tag_val = _safe_tag(data, "TargetSkeleton")
            if tag_val and target_skeleton_path in tag_val:
                score += 5
        scored.append((score, path))
    scored.sort(key=lambda x: (-x[0], x[1]))
    return scored[0][1]


def get_skeleton_path_for_mesh(mesh_path: str):
    """Read the Skeleton tag off a SkeletalMesh without fully loading it."""
    if not mesh_path:
        return None
    return _safe_tag(_safe_asset_data(mesh_path), "Skeleton")


def find_row_struct():
    """Resolve the ZombieVariantRow user-defined struct from C++ module."""
    full_path = f"{ROW_STRUCT_PKG}.{ROW_STRUCT_NAME}"
    s = unreal.load_object(None, full_path)
    if s is None:
        log(f"[zv] FATAL: cannot find row struct {full_path}. Did the C++ "
            f"module compile? Make sure the editor finished hot-reload.")
    return s


def get_or_create_data_table():
    """Get DT_ZombieVariants, creating it if missing."""
    if asset_lib.does_asset_exist(DT_PATH):
        existing = asset_lib.load_asset(DT_PATH)
        if existing:
            log(f"[zv] reusing existing data table {DT_PATH}")
            return existing
    if not asset_lib.does_directory_exist(DT_PACKAGE_DIR):
        asset_lib.make_directory(DT_PACKAGE_DIR)
    row_struct = find_row_struct()
    if row_struct is None:
        return None
    factory = unreal.DataTableFactory()
    factory.struct = row_struct
    new_dt = asset_tools.create_asset(
        asset_name=DT_NAME,
        package_path=DT_PACKAGE_DIR,
        asset_class=unreal.DataTable,
        factory=factory,
    )
    if new_dt:
        log(f"[zv] created new data table {DT_PATH}")
    return new_dt


def upsert_row(dt, row_name: str, payload: dict):
    """Write one row by JSON-ifying the payload and pasting into the table."""
    # The Python API for DataTable rows is finicky; the simplest reliable
    # path is to drive it via the existing "import from JSON" helper. We
    # build a single-row JSON and import-merge into the table.
    import json
    row_json = json.dumps([{"Name": row_name, **payload}], indent=2)
    # Editor utility: replace any existing row by deleting then re-importing.
    try:
        unreal.DataTableFunctionLibrary.add_data_table_row_from_json_string(
            dt, row_name, json.dumps(payload))
    except Exception as exc:
        # Older API path: write the whole table from JSON instead.
        log(f"[zv] add_data_table_row_from_json_string failed ({exc}); "
            f"falling back to fill_data_table_from_json_string")
        full_json = unreal.DataTableFunctionLibrary.get_data_table_as_json(dt) \
            if hasattr(unreal.DataTableFunctionLibrary, "get_data_table_as_json") \
            else "[]"
        try:
            existing = json.loads(full_json) if full_json else []
        except Exception:
            existing = []
        existing = [r for r in existing if r.get("Name") != row_name]
        existing.append({"Name": row_name, **payload})
        unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(
            dt, json.dumps(existing))


def build_payload(variant_key, display_name, mesh_path, animbp_path,
                  hp_mul, dmg_mul, spd_mul, scale, zone_weights):
    return {
        "Variant": f"EZombieVariant::{variant_key}",
        # FText serializes from a plain string in DataTable JSON; the
        # structured {"SourceString": ...} form is rejected by
        # add_data_table_row_from_json_string in UE 5.7.
        "DisplayName": display_name,
        # SoftObjectPath/SoftClassPath round-trip via "AssetPathName" form.
        "SkeletalMesh": mesh_path or "",
        "AnimBPClass": (animbp_path + "_C") if animbp_path else "",
        "HealthMultiplier": hp_mul,
        "DamageMultiplier": dmg_mul,
        "SpeedMultiplier":  spd_mul,
        "MeshScale":        scale,
        "ZoneWeights":      {str(k): v for k, v in zone_weights.items()},
    }


def main():
    log("[zv] === build_zombie_variants_table.py START ===")
    dt = get_or_create_data_table()
    if dt is None:
        log("[zv] FATAL: could not create or load data table; aborting")
        return

    for entry in PACK_CONFIG:
        (variant, root, display, hp, dmg, spd, scale, weights, name_filter) = entry
        log(f"[zv] -- {variant}: scanning {root}"
            + (f" (filter='{name_filter}')" if name_filter else "")
            + " --")
        mesh = pick_skeletal_mesh(root, name_filter=name_filter)
        skel = get_skeleton_path_for_mesh(mesh)
        anim = pick_animbp(root, skel, name_filter=name_filter)
        log(f"[zv]    mesh   = {mesh}")
        log(f"[zv]    skel   = {skel}")
        log(f"[zv]    animBP = {anim}")
        payload = build_payload(variant, display, mesh, anim,
                                hp, dmg, spd, scale, weights)
        upsert_row(dt, variant, payload)
        log(f"[zv]    upserted row '{variant}'")

    asset_lib.save_loaded_asset(dt)
    log(f"[zv] === DONE. Saved {DT_PATH}. Now assign this table to "
        f"ACodeRescueGameMode::ZombieVariantTable. ===")


main()
