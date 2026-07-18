#!/usr/bin/env python3
"""
Static verification for the bespoke survival-horror art/UI implementation pass.

This intentionally avoids importing Unreal so it can run quickly from the
project root before a full editor smoke test:

    python3 Scripts/verify_bespoke_survival_horror_art_ui.py
"""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


CHECKS = {
    "Source/CodeRescueUnreal/CodeRescueGameMode.h": [
        "SpawnBespokeSurvivalHorrorArtLayer",
    ],
    "Source/CodeRescueUnreal/CodeRescueGameMode.cpp": [
        "SpawnBespokeSurvivalHorrorArtLayer",
        "BespokeSurvivalHorrorArt",
        "PolishedArtPass",
        "BespokeAnimatedProp",
        "URotatingMovementComponent",
        "Bespoke Weathered Courtyard Stone",
        "Bespoke Animated Lantern",
        "Bespoke Animated Code Glyph",
        "Bespoke Safe Room",
        "Bespoke Threat Gate",
    ],
    "Source/CodeRescueUnreal/CodeRescueMainMenuWidget.cpp": [
        "BespokeMainMenuBackdrop",
        "BespokeMainMenuPanel",
        "FIELD TERMINAL",
    ],
    "Source/CodeRescueUnreal/CodeTerminalWidget.cpp": [
        "BespokeTerminalBackdrop",
        "BespokeTerminalPanelFrame",
        "FIELD CODING TERMINAL",
    ],
    "Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp": [
        "BespokeSurvivalHUDTopVignette",
        "BespokeSurvivalHUDStatusPanel",
        "BespokeSurvivalHUDObjectivePanel",
    ],
    "Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp": [
        "ArmoryPauseBackdrop",  # renamed by the 2026-07-10 armory pause redesign
        "FirstLevelFieldArmoryFrame",  # 2026-07-11 refresh: armory redesign renamed the pause frame
        "FIELD ARMORY",  # 2026-07-11 refresh: pause surface is the field armory now
    ],
    "Source/CodeRescueUnreal/CodeRescueVictoryWidget.cpp": [
        "BespokeVictoryBackdrop",
        "BespokeVictoryPanelFrame",
    ],
    "Source/CodeRescueUnreal/CodeRescueDeathWidget.cpp": [
        "BespokeDeathBackdrop",
        "BespokeDeathPanelFrame",
    ],
    "Documentation/improvement_pass_2026-05-24/23_BESPOKE_SURVIVAL_HORROR_ART_UI_PASS.md": [
        "Bespoke survival-horror art and UI pass",
        "no direct franchise assets",
        "animated lanterns",
        "polished UI screens",
    ],
    "progress.md": [
        "Bespoke survival-horror art and UI pass",
    ],
    "Run_Character_World_Demo.command": [
        "bespoke survival-horror art/UI pass",
    ],
}


def fail(message: str) -> None:
    raise SystemExit(f"[cr-bespoke-art-ui] ERROR: {message}")


def main() -> None:
    for relative, tokens in CHECKS.items():
        path = ROOT / relative
        if not path.exists():
            fail(f"missing expected file: {relative}")
        text = path.read_text(encoding="utf-8", errors="ignore")
        for token in tokens:
            if token not in text:
                fail(f"{relative} missing token: {token}")

    game_mode = (ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp").read_text(encoding="utf-8", errors="ignore")
    function_index = game_mode.find("void ACodeRescueGameMode::SpawnBespokeSurvivalHorrorArtLayer")
    # 2026-07-11 pattern refresh: the layer chain moved one scope deeper
    # (development-showcase gating); wiring order is unchanged.
    wire_token = (
        "SpawnNext100DevelopmentLayer(Mission, CityIndex, Origin, CityLabel);\n"
        "        SpawnBespokeSurvivalHorrorArtLayer(Mission, CityIndex, Origin, CityLabel);"
    )
    if function_index < 0 or wire_token not in game_mode:
        fail("bespoke world layer call/function not found")

    print("[cr-bespoke-art-ui] OK bespoke world layer, animated props, UI polish, docs, and launcher tokens are present")
    print("[cr-bespoke-art-ui] Success - 0 error(s), 0 warning(s)")


if __name__ == "__main__":
    main()
