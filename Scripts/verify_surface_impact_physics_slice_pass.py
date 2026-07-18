#!/usr/bin/env python3
"""Static verifier for throwable surface-impact physics feedback."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing body for {signature}")
        return ""
    depth = 0
    for idx in range(brace, len(source)):
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                _cr_body = source[brace : idx + 1]  # 2026-07-04 BuildWidgetTreeNow migration
                if "::NativeConstruct" in signature and "BuildWidgetTreeNow();" in _cr_body:
                    return function_body(source, signature.replace("::NativeConstruct", "::BuildWidgetTreeNow"))
                return _cr_body
    errors.append(f"unterminated function {signature}")
    return ""


throwable_h = read(SRC / "ThrowableActor.h")
throwable_cpp = read(SRC / "ThrowableActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
config = read(PROJECT_ROOT / "Config/DefaultEngine.ini")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SURFACE_IMPACT_PHYSICS_SLICE.md")

impact_body = function_body(throwable_cpp, "void AThrowableActor::OnThrowableImpact")
resolve_body = function_body(throwable_cpp, "EPhysicalSurface AThrowableActor::ResolveImpactSurface")
color_body = function_body(throwable_cpp, "FLinearColor AThrowableActor::GetSurfaceImpactColor")
scale_body = function_body(throwable_cpp, "float AThrowableActor::GetSurfaceImpactImpulseScale")
tag_body = function_body(throwable_cpp, "FName AThrowableActor::GetSurfaceImpactTag")
yard_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPhysicsTraversalYard")

check_all(
    throwable_h,
    [
        "SurfaceImpactImpulseStrength",
        "MinSurfaceImpactSpeed",
        "SurfaceImpactCooldown",
        "LastSurfaceImpactTime",
        "OnThrowableImpact",
        "ResolveImpactSurface",
        "GetSurfaceImpactColor",
        "GetSurfaceImpactImpulseScale",
        "GetSurfaceImpactTag",
    ],
    "throwable header must expose surface-impact tuning and helpers",
)
check_all(
    throwable_cpp,
    [
        "bReturnMaterialOnMove = true",
        "OnComponentHit.AddDynamic",
        "CodeRescueSurfaceImpact",
        "PhysicalMaterialSurfaceReaction",
    ],
    "throwable constructor/source must subscribe to hit events and log surface impact feedback",
)
check_all(
    resolve_body,
    [
        "Hit.PhysMaterial",
        "UGameplayStatics::GetSurfaceType",
        "SurfaceConcrete",
        "SurfaceMetal",
        "SurfaceWood",
        "SurfaceGlass",
        "SurfaceFlesh",
        "SurfaceDirt",
        "SurfaceType1",
        "SurfaceType6",
    ],
    "surface resolver must use physical material first and tag fallback second",
)
check_all(
    color_body + scale_body + tag_body,
    [
        "SurfaceImpact_Concrete",
        "SurfaceImpact_Metal",
        "SurfaceImpact_Wood",
        "SurfaceImpact_Glass",
        "SurfaceImpact_Flesh",
        "SurfaceImpact_Dirt",
        "SurfaceType1",
        "SurfaceType2",
        "SurfaceType3",
        "SurfaceType4",
        "SurfaceType5",
        "SurfaceType6",
    ],
    "surface helper functions must provide per-surface colors, impulse scales, and tags",
)
check_all(
    impact_body,
    [
        "SurfaceImpactCooldown",
        "MinSurfaceImpactSpeed",
        "ResolveImpactSurface",
        "GetSurfaceImpactImpulseScale",
        "AddImpulseAtLocation",
        "GlowLight->SetLightColor",
        "SurfaceImpactFeedback",
        "PhysicalMaterialSurfaceReaction",
        "SurfaceImpactResponder",
    ],
    "impact handler must throttle, branch by surface, apply impulse, and expose feedback tags",
)
check_all(
    yard_body,
    [
        "TagSurfaceImpact",
        "SurfaceImpactTraining",
        "PhysicalMaterialSurfaceReaction",
        "SurfaceImpactTrainingProp",
        "SurfaceConcrete",
        "SurfaceMetal",
        "SurfaceWood",
        "SURFACE IMPACT RANGE",
        "concrete dust, metal sparks, wood chip reactions",
    ],
    "physics yard must include tagged surface-impact training props and readable in-world label",
)
check_all(
    config,
    [
        '+PhysicalSurfaces=(Type=SurfaceType1,Name="Concrete")',
        '+PhysicalSurfaces=(Type=SurfaceType2,Name="Metal")',
        '+PhysicalSurfaces=(Type=SurfaceType3,Name="Wood")',
        '+PhysicalSurfaces=(Type=SurfaceType4,Name="Glass")',
        '+PhysicalSurfaces=(Type=SurfaceType5,Name="Flesh")',
        '+PhysicalSurfaces=(Type=SurfaceType6,Name="Dirt")',
    ],
    "engine physics settings must define the compact surface set from the deep dive",
)
check("verify_surface_impact_physics_slice_pass.py" in full_qa,
      "full QA must run the surface-impact verifier")
check("verify_surface_impact_physics_slice_pass.py" in local_ci,
      "local CI must run the surface-impact verifier")
check("Surface-specific throwable impact feedback" in progress,
      "progress log must document the surface-impact slice")
check("GAME_PHYSICS_DEEPDIVE" in slice_doc and "physical materials" in slice_doc,
      "slice doc must map the work to the physical-materials guidance")
check("WORLD_DEVELOPMENT_DEEPDIVE" in slice_doc,
      "slice doc must explain the world/playability purpose of surface reactions")

if errors:
    for error in errors:
        print(f"[verify_surface_impact_physics_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_surface_impact_physics_slice_pass] PASS: surface-impact physics feedback verified")
