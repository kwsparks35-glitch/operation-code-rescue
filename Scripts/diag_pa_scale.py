# diag_pa_scale.py — measure mesh bounds vs physics-asset body sizes for the
# reimported characters (freeze report 2026-07-16: PA-scale suspicion).
import unreal

PATHS = (
    "/Game/CodeRescueArt/CharactersV3/ZombieShamblerV3/ZombieShamblerV3",
    "/Game/CodeRescueArt/CharactersV3/SurvivorMayaV3/SurvivorMayaV3",
    "/Game/CodeRescueArt/CharactersV4/SurvivorKennyV4/SurvivorKennyV4",
    "/Game/CodeRescueArt/CharactersV2/SurvivorKenny",
)
for p in PATHS:
    m = unreal.load_asset(p)
    if not m:
        unreal.log_error("[PADiag] MISSING " + p)
        continue
    b = m.get_bounds()
    ext = b.box_extent
    line = "[PADiag] {0}: mesh_bounds_r={1:.1f} box=({2:.0f},{3:.0f},{4:.0f})".format(
        m.get_name(), b.sphere_radius, ext.x, ext.y, ext.z)
    pa = m.get_editor_property("physics_asset")
    if pa:
        try:
            setups = pa.get_editor_property("skeletal_body_setups")
            n = len(setups)
            detail = []
            for s in setups[:4]:
                if not s:
                    continue
                agg = s.get_editor_property("agg_geom")
                for cap in agg.get_editor_property("sphyl_elems"):
                    detail.append("capsule r={0:.1f} l={1:.1f}".format(
                        cap.get_editor_property("radius"), cap.get_editor_property("length")))
                for sp in agg.get_editor_property("sphere_elems"):
                    detail.append("sphere r={0:.1f}".format(sp.get_editor_property("radius")))
            line += " PA={0} bodies={1} [{2}]".format(pa.get_name(), n, "; ".join(detail[:4]))
        except Exception as e:
            line += " PA={0} (introspect fail {1})".format(pa.get_name(), e)
    else:
        line += " PA=NONE"
    unreal.log_error(line)

# rig height: pelvis->head component-space Z for scale ground truth
for p in PATHS:
    m = unreal.load_asset(p)
    if not m:
        continue
    try:
        opts = m.get_editor_property("positive_bounds_extension")
        neg = m.get_editor_property("negative_bounds_extension")
        unreal.log_error("[PADiag] {0}: bounds_ext pos=({1:.0f},{2:.0f},{3:.0f}) neg=({4:.0f},{5:.0f},{6:.0f})".format(
            m.get_name(), opts.x, opts.y, opts.z, neg.x, neg.y, neg.z))
    except Exception:
        pass
