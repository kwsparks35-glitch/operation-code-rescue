#!/usr/bin/env python3
"""Static verifier for the launch-language, grounding, symbol, and pickup pass."""

from __future__ import annotations

from pathlib import Path
import csv
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


main_menu_cpp = read(SRC / "CodeRescueMainMenuWidget.cpp")
main_menu_h = read(SRC / "CodeRescueMainMenuWidget.h")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
game_mode_h = read(SRC / "CodeRescueGameMode.h")
game_mode_spawning_cpp = read(SRC / "CodeRescueGameModeSpawning.cpp")
game_instance_cpp = read(SRC / "CodeRescueGameInstance.cpp")
game_instance_h = read(SRC / "CodeRescueGameInstance.h")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
terminal_cpp = read(SRC / "CodeTerminalWidget.cpp")
campaign_cpp = read(SRC / "CodeRescueCampaign.cpp")
language_station_cpp = read(SRC / "LanguageStationActor.cpp")
pickup_cpp = read(SRC / "PickupActor.cpp")
pickup_h = read(SRC / "PickupActor.h")
qa_doc = read(DOC / "QA_PLAYTEST_CHECKLIST.md")
pass_doc = read(DOC / "improvement_pass_2026-06-18/41_LANGUAGE_LAUNCH_GROUNDING_SYMBOL_PICKUP_PASS.md")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
packaged_smoke = read(PROJECT_ROOT / "Smoke_Test_Packaged_App.command")

manifest_path = DATA / "launch_language_grounding_symbol_pickup_next20_manifest.tsv"

# 1. Language-first launch.
check("StartLanguageRun" in main_menu_h and "StartLanguageRun" in main_menu_cpp,
      "main menu must define StartLanguageRun")
check("ResumeLanguageRun" in main_menu_h and "ResumeLanguageRun" in main_menu_cpp,
      "main menu must define ResumeLanguageRun")
check("CHOOSE CODING LANGUAGE" in main_menu_cpp and "LANGUAGE SELECTION" in main_menu_cpp,
      "main menu must prompt for language deployment")
check('CampaignMapName = TEXT("Entry")' in main_menu_h,
      "main menu must launch the packaged Entry map after language selection")
check("bHasSelectedLaunchLanguageThisSession" in game_instance_h,
      "game instance must track a session-only launch language gate")
check("CodeRescueLaunchLanguageMenu" in game_mode_cpp and "CreateWidget<UCodeRescueMainMenuWidget>" in game_mode_cpp,
      "game mode must show the language chooser before active play on normal startup")
check("SpawnLaunchLanguageSelectionScene" in game_mode_h and "SpawnLaunchLanguageSelectionScene();" in game_mode_cpp,
      "normal startup must spawn a visible launch-language scene before the UI overlay")
check("LaunchLanguageWorldPrompt" in game_mode_cpp and "LaunchLanguageCamera" in game_mode_cpp and "SetViewTarget" in game_mode_cpp,
      "launch-language scene must render visible world prompts through a dedicated camera")
check("CodeRescueBypassLaunchLanguageMenu" in game_mode_cpp,
      "game mode must expose an automation bypass for active-play smoke tests")
check("-CodeRescueBypassLaunchLanguageMenu" in packaged_smoke and "-CodeRescueBypassLaunchLanguageMenu" in full_qa,
      "automated smoke tests must explicitly bypass the launch-language menu")
for language in ("Java", "C", "CPlus", "Cpp", "Python", "MATLAB"):
    check(f"On{language}LanguageClicked() {{ bLaunchLanguageOnly ? StartLanguageRun(ECodingLanguage::{language})" in main_menu_cpp,
          f"{language} menu button must launch a fresh selected-language run")
    check(f"OnResume{language}Clicked() {{ ResumeLanguageRun(ECodingLanguage::{language})" in main_menu_cpp,
          f"{language} resume button must load the selected-language save")
check("UGameplayStatics::OpenLevel(GetWorld(), CampaignMapName)" in main_menu_cpp,
      "language selection must open the campaign map")
check("START SELECTED LANGUAGE" in main_menu_cpp,
      "main menu fallback start must be relabeled")

# 2. Active-play language lock.
# 2026-07-04: the 07-01 root fix legitimately spawns REAL language stations in
# the LAUNCH scene (walk-up + E deploys). They remain forbidden during active
# play, so assert the only spawn site lives inside SpawnLaunchLanguageSelectionScene.
_station_at = game_mode_cpp.find("SpawnActor<ALanguageStationActor>")
_launch_at = game_mode_cpp.find("void ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene")
_next_fn_at = game_mode_cpp.find("\nvoid ACodeRescueGameMode::", _launch_at + 10) if _launch_at >= 0 else -1
check(game_mode_cpp.count("SpawnActor<ALanguageStationActor>") == 1
      and _launch_at >= 0 and _station_at > _launch_at
      and (_next_fn_at < 0 or _station_at < _next_fn_at),
      "language stations spawn ONLY inside the launch-language scene (never during active play)")
check("ActivePlaySingleLanguageOnly" in game_mode_cpp and "LaunchLockedLanguageTrack" in game_mode_cpp,
      "game mode must tag the selected launch-language marker")
check("GI->SelectedLanguage != Language" in game_mode_cpp,
      "SpawnLanguageStation must filter out unselected languages")
check("GI->SelectedLanguage = Language" not in language_station_cpp,
      "language station actor must not mutate the selected language mid-run")
check("language locked at launch" in hud_cpp,
      "HUD prompt must describe launch-locked language behavior")
check("Language locked from launch" in character_cpp,
      "interact fallback must not activate language switching")

# 3. Selected-language-only runtime surfaces.
check("TEXT(\"%s track %d/%d\")" in game_instance_cpp,
      "language progress summary must report selected track only")
check("Track %s %d/%d" in hud_cpp and "Lang J" not in hud_cpp,
      "HUD must report selected-language counters only")
check("Selected Curriculum Wall Panel" in game_mode_cpp,
      "academy wall must collapse to selected language")
check("Hanging Selected Language Banner" in game_mode_cpp,
      "language banners must collapse to selected language")
check("Next100 Selected Language Mentor" in game_mode_cpp,
      "mentor row must collapse to selected language")
check("TerminalActor->Challenge.Language = GI->SelectedLanguage" in terminal_cpp,
      "terminal language must be forced from selected launch language")
check("choose a language station" not in terminal_cpp.lower(),
      "terminal text must not reference old language stations")
check("another language station" not in terminal_cpp.lower(),
      "terminal completion text must not point at another language station")
check("launch track note" in campaign_cpp and "second language station" not in campaign_cpp,
      "campaign language track text must describe launch-locked run behavior")

# 4. Symbolized world labels with essential text preserved.
check("IsEssentialGuideText" in game_mode_cpp and "SymbolForGuideText" in game_mode_cpp,
      "guide text must centralize essential-text and symbol mapping")
for symbol in ('TEXT("</>")', 'TEXT("+")', 'TEXT("!")', 'TEXT("H")', 'TEXT(">")', 'TEXT("[]")', 'TEXT("~")'):
    check(symbol in game_mode_cpp, f"symbol mapping must include {symbol}")
check("WorldInfoSymbol" in game_mode_cpp and "WorldTextEssential" in game_mode_cpp,
      "guide text must tag symbol and essential text actors")
check("PROTECTED CODING" in game_mode_cpp and "TRACK ONLY" in game_mode_cpp,
      "essential guide text whitelist must preserve non-symbolic learning/safety text")

# 5. Platform and pickup grounding.
check("Origin + FVector(0, 0, -6)" in game_mode_cpp,
      "mega zone floor must be raised to the play plane")
check("CityOffset(FVector(0, 0, -6))" in game_mode_cpp,
      "city mission floor must be raised to the play plane")
check("bSnapToGround" in pickup_h and "GroundClearance" in pickup_h,
      "pickup actor must expose ground-snap settings")
check("LineTraceSingleByChannel" in pickup_cpp and "PickupGroundSnapped" in pickup_cpp,
      "pickup actor must snap to a WorldStatic surface")
check("InitSphereRadius(220.0f)" in pickup_cpp,
      "pickup trigger radius must be widened")
check(game_mode_spawning_cpp.count("MeshComp->SetMobility(EComponentMobility::Movable);\n    MeshComp->SetStaticMesh(CubeMesh);") >= 2,
      "block helpers must set mobility before assigning cube meshes")
check(("MeshComp->SetMobility(EComponentMobility::Movable);\n    MeshComp->SetStaticMesh(Mesh);" in game_mode_spawning_cpp)
      or (game_mode_spawning_cpp.find("MeshComp->SetMobility(EComponentMobility::Movable);")
          < game_mode_spawning_cpp.find("MeshComp->SetStaticMesh(Mesh);")),
      "static mesh prop helper must set mobility before assigning meshes (2026-07-04: collision setup may sit between)")

# 6. Pickup availability.
pickup_kinds = ("Ammo", "Medkit", "Flare", "Smoke", "Stim", "Scrap", "ArmorPlate")
for kind in pickup_kinds:
    check(f"SpawnArmoryPickup(EPickupKind::{kind}" in game_mode_cpp,
          f"armory must spawn {kind}")
    check(f"SpawnPickup(EPickupKind::{kind}" in game_mode_cpp,
          f"city route must spawn {kind}")
check("CityRoutePickupAvailable" in game_mode_cpp and "TacticalArmoryAllWeaponsAvailable" in game_mode_cpp,
      "pickup spawns must be tagged for future review")

# 7. Documentation and QA.
if manifest_path.exists():
    with manifest_path.open("r", encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader(handle, delimiter="\t"))
    check(len(rows) == 20, "next-20 manifest must contain exactly 20 implementation rows")
    check(all(row.get("implemented_change") for row in rows), "every next-20 row must document an implementation")
else:
    errors.append(f"missing {manifest_path.relative_to(PROJECT_ROOT)}")
check("verify_june18_launch_grounding_symbol_pickup_pass.py" in pass_doc,
      "implementation doc must name the verifier")
check("campaign immediately starts" in qa_doc.lower() and "world information markers" in qa_doc.lower(),
      "QA checklist must cover launch-language and symbol-marker review")
check("verify_june18_launch_grounding_symbol_pickup_pass.py" in full_qa,
      "full QA must run this verifier")

if errors:
    for error in errors:
        print(f"[verify_june18_launch_grounding_symbol_pickup_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_june18_launch_grounding_symbol_pickup_pass] PASS: launch language, grounding, symbols, and pickups verified")
