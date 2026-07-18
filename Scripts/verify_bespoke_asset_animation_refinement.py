#!/usr/bin/env python3
"""
Static verifier for the bespoke authored-asset and animation refinement pass.

This script intentionally avoids importing Unreal. It checks the C++ wiring,
critical content references, documentation hooks, and launcher/progress notes
so the pass can be validated from a plain shell before launching the editor.
"""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def require(condition, message, errors):
    if not condition:
        errors.append(message)


def require_tokens(name, text, tokens, errors):
    for token in tokens:
        require(token in text, f"{name}: missing token {token!r}", errors)


def object_path_exists(object_path):
    if not object_path.startswith("/Game/"):
        return False
    package_path = object_path.split(".", 1)[0]
    rel = package_path.removeprefix("/Game/")
    return (ROOT / "Content" / f"{rel}.uasset").exists()


def main():
    errors = []

    game_mode_h = read("Source/CodeRescueUnreal/CodeRescueGameMode.h")
    game_mode_cpp = read("Source/CodeRescueUnreal/CodeRescueGameMode.cpp")
    zombie_cpp = read("Source/CodeRescueUnreal/CodeZombieActor.cpp")
    doc = read("Documentation/improvement_pass_2026-05-24/24_BESPOKE_ASSET_ANIMATION_REFINEMENT_PASS.md")
    progress = read("progress.md")
    launcher = read("Run_Character_World_Demo.command")

    require_tokens("CodeRescueGameMode.h", game_mode_h, [
        "SpawnBespokeAuthoredAssetRefinementLayer",
    ], errors)

    require_tokens("CodeRescueGameMode.cpp", game_mode_cpp, [
        "SpawnBespokeAuthoredAssetRefinementLayer",
        "BespokeAuthoredAssetRefinement",
        "ImportedMeshReplacement",
        "AuthoredTextureLayer",
        "LearningLoopPreserved",
        "BespokeCharacterAnimationClip",
        "AnimationClipHook",
        "SetAnimationMode(EAnimationMode::AnimationSingleNode)",
        "SM_DoorFrame.SM_DoorFrame",
        "SM_Lamp_Wall.SM_Lamp_Wall",
        "SM_Couch.SM_Couch",
        "SM_modern_bridge_001.SM_modern_bridge_001",
        "ParallaxBacklotReplacement",
        "ThirdPerson_AnimBP.ThirdPerson_AnimBP_C",
    ], errors)

    require(
        game_mode_cpp.find("SpawnBespokeSurvivalHorrorArtLayer(Mission, CityIndex, Origin, CityLabel);")
        < game_mode_cpp.find("SpawnBespokeAuthoredAssetRefinementLayer(Mission, CityIndex, Origin, CityLabel);"),
        "CodeRescueGameMode.cpp: authored refinement layer should run after the bespoke art layer",
        errors,
    )

    require_tokens("CodeZombieActor.cpp", zombie_cpp, [
        "BespokeAnimationClipFallback",
        "anim_Dog_Trot_InPlace.anim_Dog_Trot_InPlace",
        "ANMS_ZombieFemaleWalk01Forward.ANMS_ZombieFemaleWalk01Forward",
        "Zombie_Walk_F_1_Loop_IPC.Zombie_Walk_F_1_Loop_IPC",
        "SetAnimationMode(EAnimationMode::AnimationSingleNode)",
    ], errors)

    for object_path in [
        "/Game/StarterContent/Props/SM_DoorFrame.SM_DoorFrame",
        "/Game/StarterContent/Props/SM_Lamp_Wall.SM_Lamp_Wall",
        "/Game/StarterContent/Props/SM_Couch.SM_Couch",
        "/Game/StarterContent/Props/SM_Statue.SM_Statue",
        "/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_001.SM_modern_bridge_001",
        "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Quinn/MF_Idle.MF_Idle",
        "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Walk_InPlace.MM_Walk_InPlace",
        "/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit.ZombieFemale_NurseOutfit",
        "/Game/ZombieFemale/Asset/Animations/ANMS_ZombieFemaleIdle05.ANMS_ZombieFemaleIdle05",
        "/Game/DogZombie/Animations/anim_Dog_Sit_Idle.anim_Dog_Sit_Idle",
    ]:
        require(object_path_exists(object_path), f"missing content asset for {object_path}", errors)

    require_tokens("24_BESPOKE_ASSET_ANIMATION_REFINEMENT_PASS.md", doc, [
        "Bespoke authored-asset and animation refinement pass",
        "Imported mesh replacements",
        "Authored texture treatments",
        "Looped character animation clips",
        "coding-learning loop",
    ], errors)
    require_tokens("progress.md", progress, [
        "Bespoke authored-asset and animation refinement pass",
        "SpawnBespokeAuthoredAssetRefinementLayer",
    ], errors)
    require_tokens("Run_Character_World_Demo.command", launcher, [
        "bespoke authored-asset refinement",
    ], errors)

    if errors:
        print("[cr-bespoke-refinement] FAIL")
        for error in errors:
            print(f" - {error}")
        return 1

    print("[cr-bespoke-refinement] OK imported mesh replacements, authored texture treatments, animation clips, docs, and launcher hooks are present")
    print("[cr-bespoke-refinement] Success - 0 error(s), 0 warning(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
