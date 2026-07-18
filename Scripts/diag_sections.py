# diag_sections.py — LOD0 section/triangle census: my re-imported assets (V3/V4)
# vs the ORIGINAL July-4 bridge import (V2) that demonstrably rendered.
import unreal

sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
for path in ("/Game/CodeRescueArt/CharactersV4/SurvivorKennyV4/SurvivorKennyV4",
             "/Game/CodeRescueArt/CharactersV3/SurvivorMayaV3/SurvivorMayaV3",
             "/Game/CodeRescueArt/CharactersV2/SurvivorKenny",
             "/Game/CodeRescueArt/CharactersV2/ZombieBruteV2/ZombieBruteV2"):
    mesh = unreal.load_asset(path)
    if not mesh:
        unreal.log_error("[SectionDiag] MISSING " + path)
        continue
    try:
        num_sections = sub.get_num_sections(mesh, 0)
    except Exception as exc:
        num_sections = -1
    tris = []
    for s in range(max(0, num_sections)):
        try:
            tris.append(str(sub.get_section_num_triangles(mesh, 0, s)))
        except Exception:
            tris.append("?")
    b = mesh.get_bounds()
    unreal.log_error("[SectionDiag] {0}: sections={1} tris=[{2}] bounds_r={3:.1f}".format(
        mesh.get_name(), num_sections, ",".join(tris) if tris else "-", b.sphere_radius))
