#!/usr/bin/env python3
"""Static verifier for the city radio and survivor bark cadence slice."""

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


subs_h = read(SRC / "CodeRescueSubtitlesWidget.h")
subs_cpp = read(SRC / "CodeRescueSubtitlesWidget.cpp")
mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
manifest = read(DATA / "city_radio_bark_cadence_manifest.tsv")
audio_manifest = read(DATA / "audio_coverage_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "CITY_RADIO_BARK_CADENCE_SLICE.md")

construct_body = function_body(subs_cpp, "void UCodeRescueSubtitlesWidget::NativeConstruct")
push_body = function_body(subs_cpp, "void UCodeRescueSubtitlesWidget::Push")
enabled_body = function_body(subs_cpp, "bool UCodeRescueSubtitlesWidget::AreSubtitlesEnabledInSettings")
radio_line_body = function_body(mode_cpp, "FString BuildRadioRouteCadenceLine")
briefing_body = function_body(mode_cpp, "void ACodeRescueGameMode::SpeakRadioBriefing")

check_all(
    subs_h,
    [
        "static TArray<FSubtitleEntry> PendingQueue",
        "AreSubtitlesEnabledInSettings",
    ],
    "subtitle header must declare the pending queue and settings gate",
)
check_all(
    subs_cpp,
    [
        "UCodeRescueSubtitlesWidget::PendingQueue",
        "MaxPendingSubtitleLines = 8",
    ],
    "subtitle implementation must define capped pending subtitle storage",
)
check_all(
    construct_body,
    [
        "PendingQueue.Num() > 0",
        "AreSubtitlesEnabledInSettings()",
        "Queue.Append(PendingQueue)",
        "PendingQueue.Reset()",
    ],
    "subtitle construct path must flush pending lines only when subtitles are enabled",
)
check_all(
    push_body,
    [
        "if (!ActiveInstance)",
        "PendingQueue.Add",
        "MaxPendingSubtitleLines",
        "PendingQueue.RemoveAt",
        "ActiveInstance->Queue.Add",
        "GI->bSubtitlesEnabled",
    ],
    "subtitle push path must buffer early lines and still honor live subtitle settings",
)
check_all(
    enabled_body,
    [
        "GetGameInstance<UCodeRescueGameInstance>",
        "return GI->bSubtitlesEnabled",
        "return true",
    ],
    "subtitle settings helper must read saved subtitle preference",
)
check_all(
    radio_line_body,
    [
        "[Radio Relay]:",
        "GI->GetLanguageName()",
        "SolvedTerminalIds.Contains(Mission.TerminalId)",
        "RescuedSurvivorNames.Contains(Mission.SurvivorName)",
        "Extraction beacon live",
        "Survivor route open",
        "Terminal route locked",
        "solve protected terminal",
        "follow survivor ping",
        "reach the helipad and debrief",
        "Mission.TerminalTitle",
        "Mission.SurvivorName",
        "Mission.LandmarkName",
    ],
    "radio cadence helper must expose language, save phase, terminal, survivor, landmark, and next step",
)
check_all(
    briefing_body,
    [
        "UCodeRescueSubtitlesWidget::Push(Mission.RadioBriefing, 12.0f)",
        "BuildRadioRouteCadenceLine(Mission, GetGameInstance<UCodeRescueGameInstance>())",
        "UCodeRescueSubtitlesWidget::Push",
        "UseCookedRadioVoice",
        "NoRadioVoice",
        "/usr/bin/say",
    ],
    "radio briefing must queue text-first briefing/cadence before cooked audio or system voice fallback",
)
check_all(
    survivor_cpp,
    [
        "BuildSurvivorRescueLine",
        "BuildExtractionDispatchLine",
        "UCodeRescueSubtitlesWidget::Push",
    ],
    "existing survivor barks must continue to use the safer subtitle queue",
)
check_all(
    manifest,
    [
        "PendingSubtitleQueue",
        "CityRadioBriefing",
        "RadioRelayCadence",
        "SurvivorBarkHandoff",
        "subtitle overlay exists",
        "selected language",
        "save continuity",
    ],
    "city radio bark cadence manifest must document runtime surfaces and accessibility contracts",
)
check_all(
    audio_manifest,
    [
        "city_radio_bark_cadence",
        "runtime.radio_route_cadence",
        "pending_subtitle_queue",
        "route_phase_language_terminal_survivor_landmark_next_step",
    ],
    "audio coverage manifest must include the runtime radio cadence coverage row",
)
check(
    "city radio and survivor barks" in creative_plan
    and "verify_city_radio_bark_cadence_slice_pass.py plus verify_survivor_rescue_dialogue_handoff_slice_pass.py plus audio manifest plus packaged smoke plus manual audio/subtitle review" in creative_plan,
    "creative development plan must route city radio and survivor barks through the new verifier",
)
check_all(
    visual_targets,
    [
        "CityRadioBarkCadence",
        "Radio Relay subtitle",
        "terminal/survivor/extraction phase",
    ],
    "visual regression targets must include radio relay subtitle review",
)
check_all(
    human_qa,
    [
        "CityRadioBarkCadence",
        "Start a fresh city and inspect the queued radio briefing",
        "terminal locked, survivor route open, and extraction beacon live",
    ],
    "human QA checklist must include city radio cadence state review",
)
check_all(
    access_manifest,
    [
        "CityRadioBarkCadenceAccessibility",
        "PendingQueue + Radio Relay subtitle",
        "subtitle toggle, subtitle scale, and high-contrast styling",
    ],
    "accessibility manifest must document pending radio subtitles and style settings",
)
check_all(
    onboarding,
    [
        "radio relay names language, terminal, survivor, landmark, and next step",
        "queued radio briefing survives HUD startup",
    ],
    "first-ten-minutes onboarding must include radio relay expectations",
)
check("verify_city_radio_bark_cadence_slice_pass.py" in full_qa,
      "full QA must run the city radio bark cadence verifier")
check("verify_city_radio_bark_cadence_slice_pass.py" in local_ci,
      "local CI must run the city radio bark cadence verifier")
check("City radio bark cadence slice" in progress,
      "progress log must document the city radio bark cadence slice")
check_all(
    slice_doc,
    [
        "City Radio Bark Cadence Slice",
        "PendingQueue",
        "BuildRadioRouteCadenceLine",
        "SpeakRadioBriefing",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "Validation",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, source guidance, validation, and QA",
)

if errors:
    for error in errors:
        print(f"[verify_city_radio_bark_cadence_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_city_radio_bark_cadence_slice_pass] PASS: city radio bark cadence slice verified")
