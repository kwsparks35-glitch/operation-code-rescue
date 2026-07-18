#!/usr/bin/env python3
"""Static verifier for the inventory/map/journal polish slice."""

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


journal_h = read(SRC / "CodeRescueObjectiveJournalWidget.h")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")
manifest = read(DATA / "inventory_map_journal_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "INVENTORY_MAP_JOURNAL_POLISH_SLICE.md")

construct_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::NativeConstruct")
refresh_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::RefreshJournal")
language_body = function_body(journal_cpp, "FString BuildLanguageSaveContinuityLine")
route_body = function_body(journal_cpp, "FString BuildRouteMapReadout")
inventory_body = function_body(journal_cpp, "FString BuildInventoryReadout")

check_all(
    journal_h,
    [
        "UTextBlock* LanguageSaveText",
        "UTextBlock* RouteMapText",
        "UTextBlock* InventoryText",
        "UTextBlock* IntelText",
    ],
    "journal header must expose distinct save, route, inventory, and intel readouts",
)
check_all(
    journal_cpp,
    [
        "#include \"CodeRescueCharacter.h\"",
        "#include \"Kismet/GameplayStatics.h\"",
        "BuildLanguageSaveContinuityLine",
        "BuildRouteMapReadout",
        "BuildInventoryReadout",
        "LanguageSaveContinuityText",
        "RouteMapReadoutText",
        "FieldInventoryReadoutText",
        "PanelSlot->SetAnchors(FAnchors(0.55f, 0.09f, 0.97f, 0.86f))",
    ],
    "journal implementation must add named inventory/map/save surfaces and expand the overlay",
)
check_all(
    construct_body,
    [
        "LanguageSaveText = WidgetTree->ConstructWidget<UTextBlock>",
        "RouteMapText = WidgetTree->ConstructWidget<UTextBlock>",
        "InventoryText = WidgetTree->ConstructWidget<UTextBlock>",
        "LanguageSlot->SetPadding",
        "RouteSlot->SetPadding",
        "InventorySlot->SetPadding",
    ],
    "construct path must build separate readable sections before the mission rows",
)
check_all(
    language_body,
    [
        "LANGUAGE SAVE",
        "GI->DoesLanguageSaveExist(GI->SelectedLanguage)",
        "GI->SaveSlotName",
        "Start screen remains available",
        "Resume %s",
    ],
    "language save readout must connect selected language, save slot, and start-screen resume behavior",
)
check_all(
    route_body,
    [
        "ROUTE MAP",
        "SolvedTerminalIds.Contains",
        "RescuedSurvivorNames.Contains",
        "GetCaseFileCollectionSummary",
        "terminal needs code",
        "cyan survivor marker",
        "helipad extraction/debrief",
        "Next marker",
    ],
    "route map readout must expose terminal, survivor, case-file, and next-marker state",
)
check_all(
    inventory_body,
    [
        "FIELD INVENTORY",
        "GetActiveWeaponName",
        "GetActiveWeaponTacticalRole",
        "GetRadioScannerCharges",
        "GetFlashlightBatteries",
        "GetBypassKits",
        "GetThrowableCountForSlot(0)",
        "GetAmmoPouchCapacityBonus",
        "GetScrap",
        "ResearchPoints",
    ],
    "inventory readout must expose active weapon and saved field-kit resources",
)
check_all(
    refresh_body,
    [
        "UGameplayStatics::GetPlayerPawn(GetWorld(), 0)",
        "BuildLanguageSaveContinuityLine(GI)",
        "BuildRouteMapReadout(ActiveMission, GI, CompletedCount, Missions.Num())",
        "BuildInventoryReadout(Character, GI)",
        "LanguageSaveText->SetAutoWrapText(true)",
        "RouteMapText->SetAutoWrapText(true)",
        "InventoryText->SetAutoWrapText(true)",
        "GI && GI->bHighContrastHUD",
    ],
    "refresh path must update all new sections from live save/player state and accessibility settings",
)
check_all(
    manifest,
    [
        "LanguageSaveContinuityText",
        "RouteMapReadoutText",
        "FieldInventoryReadoutText",
        "SurvivorIntelDossierText",
        "SelectedLanguage",
        "SolvedTerminalIds",
        "Player resource save fields",
    ],
    "inventory/map/journal manifest must document each runtime surface and persisted state",
)
check(
    "inventory map and journal polish" in creative_plan
    and "verify_inventory_map_journal_polish_slice_pass.py plus verify_minimap_route_readability_slice_pass.py plus verify_objective_journal_accessibility_slice_pass.py plus packaged smoke plus manual UI pass" in creative_plan,
    "creative development plan must route inventory map and journal polish through the new verifier",
)
check_all(
    visual_targets,
    [
        "InventoryMapJournal",
        "Language save continuity",
        "field inventory",
        "survivor intel dossier",
    ],
    "visual regression targets must include the polished journal overlay",
)
check_all(
    human_qa,
    [
        "InventoryMapJournalPolish",
        "Open J during fresh, solved-terminal, and rescued-survivor states",
        "language save, route map, field inventory, survivor intel",
    ],
    "human QA checklist must include the polished inventory/map/journal flow",
)
check_all(
    access_manifest,
    [
        "InventoryMapJournalAccessibility",
        "LanguageSaveContinuityText + RouteMapReadoutText + FieldInventoryReadoutText",
        "text-first inventory, route, save, and intel summaries",
    ],
    "accessibility manifest must document text-first inventory/map/journal coverage",
)
check_all(
    onboarding,
    [
        "journal field ops readout",
        "language save, route map, field inventory, survivor intel",
    ],
    "first-ten-minutes checklist must guide reviewers to the polished journal",
)
check("verify_inventory_map_journal_polish_slice_pass.py" in full_qa,
      "full QA must run the inventory map journal verifier")
check("verify_inventory_map_journal_polish_slice_pass.py" in local_ci,
      "local CI must run the inventory map journal verifier")
check("Inventory map and journal polish slice" in progress,
      "progress log must document the inventory map and journal polish slice")
check_all(
    slice_doc,
    [
        "Inventory Map And Journal Polish Slice",
        "LANGUAGE SAVE",
        "ROUTE MAP",
        "FIELD INVENTORY",
        "Survivor Intel",
        "Verification",
    ],
    "slice doc must explain player impact, implementation, QA, and remaining art hooks",
)

if errors:
    for error in errors:
        print(f"[verify_inventory_map_journal_polish_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_inventory_map_journal_polish_slice_pass] PASS: inventory map journal polish verified")
