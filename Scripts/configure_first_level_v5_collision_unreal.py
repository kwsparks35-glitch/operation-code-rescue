"""Persist doorway-preserving collision settings on the V5 access buildings."""

import unreal


ASSET_NAMES = (
    "AccessibleMarketV5",
    "AccessibleClinicV5",
    "OpenStreetCafeV5",
)


def sequence_len(value):
    try:
        return len(value)
    except Exception:
        return 0


trace_flag = None
for candidate in ("CTF_USE_COMPLEX_AS_SIMPLE", "USE_COMPLEX_AS_SIMPLE"):
    trace_flag = getattr(unreal.CollisionTraceFlag, candidate, None)
    if trace_flag is not None:
        break
if trace_flag is None:
    raise RuntimeError(
        "UE Python does not expose the complex-as-simple CollisionTraceFlag; values={}".format(
            [name for name in dir(unreal.CollisionTraceFlag) if not name.startswith("_")]
        )
    )

configured = []
for name in ASSET_NAMES:
    path = "/Game/CodeRescueArt/FirstLevelV5/{0}/{0}/StaticMeshes/{0}.{0}".format(name)
    mesh = unreal.load_asset(path)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError("Missing V5 access StaticMesh: " + path)
    body_setup = mesh.get_editor_property("body_setup")
    if body_setup is None:
        raise RuntimeError("Missing BodySetup: " + path)

    body_setup.set_editor_property("collision_trace_flag", trace_flag)
    body_setup.modify()
    mesh.modify()
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)

    agg_geom = body_setup.get_editor_property("agg_geom")
    simple_shapes = 0
    for property_name in ("box_elems", "sphere_elems", "sphyl_elems", "convex_elems"):
        simple_shapes += sequence_len(agg_geom.get_editor_property(property_name))
    unreal.log(
        "[FirstLevelV5Collision] {} trace={} simple_shapes={} triangles={}".format(
            name,
            body_setup.get_editor_property("collision_trace_flag"),
            simple_shapes,
            mesh.get_num_triangles(0),
        )
    )
    configured.append(name)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log(
    "[FirstLevelV5Collision] COMPLETE PASS buildings={} mode=complex_as_simple doorway_holes=preserved".format(
        len(configured)
    )
)
