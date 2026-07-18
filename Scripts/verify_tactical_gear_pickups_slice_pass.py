#!/usr/bin/env python3
"""Static verifier for the expanded tactical gear pickup slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
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


pickup_h = read(SRC / "PickupActor.h")
pickup_cpp = read(SRC / "PickupActor.cpp")
character_h = read(SRC / "CodeRescueCharacter.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
terminal_h = read(SRC / "CodeTerminalWidget.h")
terminal_cpp = read(SRC / "CodeTerminalWidget.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")

manifest = read(DATA / "tactical_gear_pickups_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "TACTICAL_GEAR_PICKUPS_SLICE.md")

new_pickup_kinds = [
    "RadioScanner",
    "FlashlightBattery",
    "AmmoPouch",
    "BypassKit",
]
for kind in new_pickup_kinds:
    check(f"EPickupKind::{kind}" in pickup_cpp or f"{kind} UMETA" in pickup_h, f"pickup kind {kind} must exist")
    check(f"SpawnArmoryPickup(EPickupKind::{kind}" in gamemode_cpp, f"armory must spawn {kind}")
    check(f"SpawnPickup(EPickupKind::{kind}" in gamemode_cpp, f"city route must spawn {kind}")
    check(f"SpawnCreativePickup(EPickupKind::{kind}" in gamemode_cpp, f"creative gear hub must spawn {kind}")

check_all(
    pickup_cpp,
    [
        "AddRadioScannerCharges",
        "AddFlashlightBatteries",
        "AddAmmoPouch",
        "AddBypassKits",
        "SavePersistentRun",
        "radio scanner charge",
        "flashlight battery",
        "ammo pouch capacity",
        "bypass kit",
    ],
    "pickup actor must grant and autosave the expanded field-kit resources",
)

check_all(
    character_h + character_cpp,
    [
        "RadioScannerCharges",
        "FlashlightBatteries",
        "BypassKits",
        "AmmoPouchCapacityBonus",
        "FieldFlashlight",
        "ToggleFlashlight",
        "UseRadioScanner",
        "TrySpendBypassKit",
        "GetFieldKitSummary",
        "RestorePlayerResourcesDetailed",
        "EKeys::L",
        "EKeys::Z",
    ],
    "character must expose active scanner, flashlight, bypass, ammo-pouch, and restore contracts",
)
check("FieldFlashlight->SetIntensity(5200.0f)" in character_cpp, "flashlight must turn on a real light component")
check("Radio scan:" in character_cpp and "RadioScannerCharges = FMath::Max" in character_cpp, "radio scanner must consume charges and report guidance")
check("MaxAmmo = BaseMaxAmmo + AmmoPouchCapacityBonus" in character_cpp, "ammo pouch must increase reserve capacity")

check_all(
    save_h + gi_h + gi_cpp,
    [
        "PlayerArmorPlates",
        "PlayerFlares",
        "PlayerSmokes",
        "PlayerStims",
        "PlayerScrap",
        "PlayerRadioScannerCharges",
        "PlayerFlashlightBatteries",
        "PlayerBypassKits",
        "PlayerAmmoPouchCapacityBonus",
        "bHasPlayerTacticalGear",
        "LastPlayerRadioScannerCharges",
        "LastPlayerFlashlightBatteries",
        "LastPlayerBypassKits",
        "RestorePlayerResourcesDetailed",
    ],
    "save schema and game instance must persist the full tactical kit",
)

check_all(
    terminal_h + terminal_cpp,
    [
        "BypassButton",
        "OnBypassClicked",
        "USE BYPASS KIT [Ctrl+B]",
        "TrySpendBypassKit(1)",
        "BypassScore = 60",
        "RecordTerminalSolved",
        "RecordLanguageSolve",
        "RevealSolvedTerminalRescueRoute",
        "clean-solve rewards are disabled",
    ],
    "terminal UI must provide an explicit bypass-kit route assist",
)

check_all(
    hud_cpp,
    [
        "GetFieldKitSummary",
        "Scan:",
        "Light:",
        "Bypass:",
        "Pouch +",
        "L flashlight",
        "Z scanner",
    ],
    "HUD must surface expanded tactical gear and controls",
)

check_all(
    manifest,
    [
        "RadioScanner",
        "FlashlightBattery",
        "AmmoPouch",
        "BypassKit",
        "selected language save slot",
        "CodeTerminalWidget",
    ],
    "tactical gear manifest must document resource effects, save behavior, and terminal hook",
)
check_all(
    creative_plan,
    [
        "tactical gear pickups",
        "verify_tactical_gear_pickups_slice_pass.py plus packaged smoke plus terminal bypass UI check",
    ],
    "creative plan must move tactical gear pickups out of manual-only validation",
)
check_all(
    onboarding,
    [
        "scanner, flashlight, ammo pouch, bypass kit",
        "L flashlight",
        "Z scanner",
    ],
    "onboarding must teach expanded tactical gear",
)
check_all(
    visual_manifest,
    [
        "TacticalGearPickups",
        "scanner, flashlight, ammo pouch, bypass kit",
    ],
    "visual regression targets must include expanded gear pickup review",
)
check_all(
    human_qa,
    [
        "TacticalGearPickups",
        "Ctrl+B bypass kit",
        "selected language",
    ],
    "human QA checklist must cover scanner, flashlight, bypass, and save behavior",
)
check_all(
    accessibility_manifest,
    [
        "TacticalGearPickupAccessibility",
        "text-first pickup labels",
        "L flashlight and Z scanner",
    ],
    "accessibility manifest must document tactical gear labels and controls",
)
check_all(
    full_qa + local_ci,
    [
        "verify_tactical_gear_pickups_slice_pass.py",
    ],
    "QA scripts must run the tactical gear verifier",
)
check_all(
    progress,
    [
        "Tactical gear pickups slice",
        "RadioScanner",
        "FlashlightBattery",
        "AmmoPouch",
        "BypassKit",
    ],
    "progress log must summarize the tactical gear implementation",
)
check_all(
    slice_doc,
    [
        "Tactical Gear Pickups Slice",
        "TOP_50_RECOMMENDATIONS.pdf",
        "scanner",
        "flashlight",
        "ammo pouch",
        "bypass kit",
        "selected language save slot",
        "verify_tactical_gear_pickups_slice_pass.py",
    ],
    "slice documentation must describe source guidance, runtime behavior, persistence, and verification",
)

if errors:
    print("Tactical gear pickups verifier FAILED:")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("Tactical gear pickups verifier passed.")
