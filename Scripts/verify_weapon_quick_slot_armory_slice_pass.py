#!/usr/bin/env python3
"""Static verifier for the weapon quick-slot armory slice."""

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


save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
character_h = read(SRC / "CodeRescueCharacter.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/weapon_quick_slot_armory_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "WEAPON_QUICK_SLOT_ARMORY_SLICE.md")

reset_run = function_body(gi_cpp, "void UCodeRescueGameInstance::ResetRun")
save_internal = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_run = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
capture_world = function_body(gi_cpp, "void UCodeRescueGameInstance::CaptureWorldStateFromLevel")
apply_world = function_body(gi_cpp, "void UCodeRescueGameInstance::ApplyWorldStateToLevel")
quick_summary = function_body(character_cpp, "FString ACodeRescueCharacter::GetWeaponQuickSlotSummary")
restore_slots = function_body(character_cpp, "void ACodeRescueCharacter::RestoreWeaponQuickSlotState")
hud_construct = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")
hud_update = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
armory = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnTacticalArmoryLayer")

check_all(
    save_h,
    [
        "EWeaponType ActiveWeapon",
        "TArray<int32> WeaponMagazines",
        "TArray<int32> WeaponReserveAmmo",
        "bool bHasWeaponQuickSlotState",
    ],
    "save game must persist quick-slot state with a back-compat flag",
)
check_all(
    gi_h,
    [
        "LastActiveWeapon",
        "LastWeaponMagazines",
        "LastWeaponReserveAmmo",
        "bHasWeaponQuickSlotState",
    ],
    "game instance must cache weapon quick-slot state",
)
check_all(
    reset_run,
    [
        "LastActiveWeapon = EWeaponType::Pistol",
        "LastWeaponMagazines.Reset()",
        "LastWeaponReserveAmmo.Reset()",
        "bHasWeaponQuickSlotState = false",
    ],
    "fresh language runs must reset cached weapon quick-slot state",
)
check_all(
    save_internal,
    [
        "Save->ActiveWeapon = LastActiveWeapon",
        "Save->WeaponMagazines = LastWeaponMagazines",
        "Save->WeaponReserveAmmo = LastWeaponReserveAmmo",
        "Save->bHasWeaponQuickSlotState = bHasWeaponQuickSlotState",
    ],
    "save path must serialize selected-language weapon state",
)
check_all(
    load_run,
    [
        "Save->bHasWeaponQuickSlotState ? Save->ActiveWeapon",
        "Save->bHasWeaponQuickSlotState ? Save->WeaponMagazines",
        "Save->bHasWeaponQuickSlotState ? Save->WeaponReserveAmmo",
        "bHasWeaponQuickSlotState = Save->bHasWeaponQuickSlotState",
    ],
    "load path must restore new saves while falling back for older saves",
)
check_all(
    capture_world,
    [
        "LastActiveWeapon = Character->ActiveWeapon",
        "LastWeaponMagazines = Character->WeaponMagazines",
        "LastWeaponReserveAmmo = Character->WeaponReserveAmmo",
        "bHasWeaponQuickSlotState = true",
    ],
    "world capture must copy live weapon quick-slot state",
)
check_all(
    apply_world,
    [
        "bHasWeaponQuickSlotState",
        "Character->RestoreWeaponQuickSlotState",
        "LastActiveWeapon",
        "LastWeaponMagazines",
        "LastWeaponReserveAmmo",
    ],
    "world apply must restore quick-slot state after resources",
)
check_all(
    character_h,
    [
        "GetWeaponQuickSlotSummary",
        "RestoreWeaponQuickSlotState",
    ],
    "character header must expose quick-slot summary and restore helper",
)
check_all(
    quick_summary,
    [
        "QUICK SLOTS:",
        "FMath::Min(10, WeaponLoadout.Num())",
        "KeyLabel = (i == 9) ? TEXT(\"0\")",
        "Marker = (static_cast<int32>(ActiveWeapon) == i)",
        "WeaponReserveAmmo",
    ],
    "quick-slot summary must show first ten keys, active marker, and ammo",
)
check_all(
    restore_slots,
    [
        "EnsureWeaponStateInitialized()",
        "SavedMagazines",
        "SavedReserveAmmo",
        "ActiveWeapon = WeaponLoadout.IsValidIndex",
        "SyncActiveWeaponStateFromLoadout()",
        "RefreshLegacyAmmoFromWeaponReserves()",
    ],
    "weapon restore helper must safely apply saved arrays and active slot",
)
check_all(
    hud_construct,
    [
        "WeaponFont.Size = 13",
        "WeaponSlot->SetSize(FVector2D(860.0f, 52.0f))",
    ],
    "HUD construct must size the weapon strip for two-line quick-slot text",
)
check_all(
    hud_update,
    [
        "GetWeaponQuickSlotSummary()",
        "ACTIVE %d/%d %s",
        "Wheel/[ ] all %d",
    ],
    "HUD update must show quick-slot row plus active weapon controls",
)
check_all(
    armory,
    [
        "QUICK SLOT BOARD",
        "1 Handgun | 2 Pump | 3 Rifle",
        "active slot and ammo save inside this language profile",
        "KEY %s\\n%s",
        "WHEEL\\n%s",
    ],
    "armory must teach the same quick-slot map as the HUD",
)
check_all(
    manifest,
    [
        "Quick-slot HUD strip",
        "Selected-language weapon save",
        "Selected-language weapon resume",
        "Visible armory board",
        "Back-compat flag",
    ],
    "manifest must document the implemented quick-slot surfaces",
)
check_all(
    plan,
    [
        "weapon quick slots and visible armory",
        "verify_weapon_quick_slot_armory_slice_pass.py",
        "verify_may27_tactical_arsenal_mcp_runtime.py",
        "manual weapon save/resume review",
    ],
    "creative plan must point the weapon row at this verifier",
)
check_all(
    qa,
    [
        "WeaponQuickSlotArmory",
        "1-0 quick slots",
        "magazine/reserve values persist",
    ],
    "human QA checklist must cover quick-slot armory persistence",
)
check_all(
    visual,
    [
        "WeaponQuickSlotArmory",
        "QUICK SLOT BOARD",
        "HUD QUICK SLOTS row",
    ],
    "visual regression targets must include quick-slot HUD and armory board",
)
check("verify_weapon_quick_slot_armory_slice_pass.py" in full_qa,
      "full QA must run the weapon quick-slot armory verifier")
check("verify_weapon_quick_slot_armory_slice_pass.py" in local_ci,
      "local CI must run the weapon quick-slot armory verifier")
check("Weapon quick-slot armory slice" in progress,
      "progress log must document the weapon quick-slot armory slice")
check_all(
    doc,
    [
        "Weapon Quick-Slot Armory Slice",
        "bHasWeaponQuickSlotState",
        "GetWeaponQuickSlotSummary",
        "RestoreWeaponQuickSlotState",
        "QUICK SLOT BOARD",
        "selected coding-language save",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, verification, and QA",
)

if errors:
    for error in errors:
        print(f"[verify_weapon_quick_slot_armory_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_weapon_quick_slot_armory_slice_pass] PASS: weapon quick-slot armory flow is implemented and documented")
