#!/usr/bin/env python3
"""Static save/back-compat contract checks for the demo-readiness pass."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(msg)


types = read(SRC / "CodeRescueTypes.h")
save = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")

for token in (
    "Easy      = 0",
    "Normal    = 1",
    "Hard      = 2",
    "Story     = 3",
    "Survival  = 4",
    "Nightmare = 5",
):
    check(token in types, f"difficulty enum must preserve explicit value {token}")

for field in (
    "bHasPlayerMagazineAmmo",
    "bHasHeadshotCount",
    "bHasPlayerStamina",
    "bHasPlayerResources",
    "bHasWorldState",
    "bHasRunScoreboard",
    "bHasWeaponQuickSlotState",
    "bHasOperatorIdentityState",
):
    check(field in save, f"save back-compat flag missing: {field}")

for field in (
    "OperatorCallsign",
    "OperatorRoleTitle",
    "OperatorProfileNote",
):
    check(field in save, f"operator identity save field missing: {field}")
    check(field in gi_h, f"GameInstance operator identity field missing: {field}")
    check(f"Save->{field}" in gi_cpp, f"SavePersistentRun/LoadPersistentRun must serialize {field}")
check("InitializeOperatorIdentityForLanguage" in gi_h and "GetOperatorIdentitySummary" in gi_h,
      "GameInstance must expose operator identity initialization and summary helpers")
check("Save->bHasOperatorIdentityState = bHasOperatorIdentityState" in gi_cpp,
      "SavePersistentRun must serialize operator identity back-compat flag")
check("InitializeOperatorIdentityForLanguage(SelectedLanguage)" in gi_cpp,
      "LoadPersistentRun must initialize operator identity for older saves")

for field in (
    "ActiveWeapon",
    "WeaponMagazines",
    "WeaponReserveAmmo",
):
    check(field in save, f"weapon quick-slot save field missing: {field}")

for field in (
    "LastActiveWeapon",
    "LastWeaponMagazines",
    "LastWeaponReserveAmmo",
    "bHasWeaponQuickSlotState",
):
    check(field in gi_h, f"GameInstance weapon quick-slot cache field missing: {field}")
check("Save->ActiveWeapon = LastActiveWeapon" in gi_cpp and "Save->WeaponMagazines = LastWeaponMagazines" in gi_cpp,
      "SavePersistentRun must serialize weapon quick-slot active/magazine state")
check("LastActiveWeapon = Save->bHasWeaponQuickSlotState ? Save->ActiveWeapon" in gi_cpp,
      "LoadPersistentRun must gate weapon quick-slot restore behind bHasWeaponQuickSlotState")

for field in (
    "SubtitleScale",
    "UITextScale",
    "bHighContrastHUD",
    "bReducedMotion",
    "bSimplifiedInputHints",
    "AimAssistScale",
    "ControlProfileName",
    "ControlProfileExportCount",
):
    check(field in save and field in gi_h, f"accessibility save/runtime field missing: {field}")
    check(f"Save->{field}" in gi_cpp or f"Save->b{field}" in gi_cpp,
          f"SavePersistentRun/LoadPersistentRun must serialize {field}")

check("0.8.0-demo-readiness" in save, "SaveVersion must document the demo-readiness schema")
check("GetDifficultyDisplayName" in gi_h and "GetDifficultyDisplayName" in gi_cpp,
      "GameInstance must expose difficulty display names")
check("GetAccessibilitySummary" in gi_h and "GetAccessibilitySummary" in gi_cpp,
      "GameInstance must expose accessibility summary")
check("GetControlProfileSummary" in gi_h and "GetControlProfileSummary" in gi_cpp,
      "GameInstance must expose control profile summary")
check("ExportControlProfileReviewFile" in gi_h and "runtime_controls_profile.json" in gi_cpp,
      "GameInstance must export a reviewable runtime control profile")

if errors:
    for error in errors:
        print(f"[verify_save_compatibility_pass] FAIL: {error}")
    sys.exit(1)
print("[verify_save_compatibility_pass] PASS: save compatibility contract intact")
