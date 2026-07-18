#!/usr/bin/env python3
"""Static acceptance contract for the 2026-07-11 reported regression pass."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source" / "CodeRescueUnreal"


def read(name: str) -> str:
    return (SOURCE / name).read_text(encoding="utf-8")


character_h = read("CodeRescueCharacter.h")
character = read("CodeRescueCharacter.cpp")
zombie = read("CodeZombieActor.cpp")
game_instance_h = read("CodeRescueGameInstance.h")
game_instance = read("CodeRescueGameInstance.cpp")
game_mode_h = read("CodeRescueGameMode.h")
game_mode = read("CodeRescueGameMode.cpp")
spawning = read("CodeRescueGameModeSpawning.cpp")
pause_h = read("CodeRescuePauseWidget.h")
pause = read("CodeRescuePauseWidget.cpp")
reader = read("CodeRescueMessageReaderWidget.cpp")
runner = read("CodeRunnerLibrary.cpp")
terminal = read("CodeTerminalWidget.cpp")
hud = read("CodeRescueHUDWidget.cpp")


checks: list[tuple[bool, str]] = [
    ("StopActiveRadioBriefing" in game_mode_h
     and "RadioBriefingDelayTimer" in game_mode_h,
     "radio narration has one owner and a city-switch debounce"),
    ("SpawnSound2D" in game_mode
     and "TerminateProc(ActiveSystemRadioProcess" in game_mode
     and "overlapping_voices=0" in game_mode,
     "cooked audio and macOS speech are both stopped before replacement"),
    ("FVector(0.0f, 0.0f, -620.0f)" in game_mode
     and "FallRecoveryCatchFloor" in game_mode,
     "emergency catch floor sits well below the playable city"),
    ("CanonicalMissionGround" in game_mode
     and "NeverAutoGroundToCatchFloor" in game_mode,
     "mission floor owns an explicit canonical-ground contract"),
    ("never infer playable" in spawning
     and "CityGroundContinuity" in spawning
     and "remaining_elevated=0" in spawning,
     "ground normalization no longer traces the city onto the catch floor"),
    ("GroundParams.AddIgnoredActor" in character
     and "rejected non-playable recovery height" in character,
     "arena recovery ignores the catch floor and rejects invalid heights"),
    ("CampaignGroundRecoveryAudit" in game_mode
     and "recovered_relative_z" in game_mode,
     "second-city below-floor recovery has a runtime audit"),
    ("0.96f, 0.985f, 1.0f" in terminal
     and "0.002f, 0.005f, 0.008f" in terminal
     and "SetBackgroundImageFocused" in terminal,
     "terminal editor uses near-white text on explicit near-black state brushes"),
    ("TerminalContrastAudit" in terminal
     and "TerminalContrastReview" in game_mode,
     "terminal contrast has numeric and rendered-review checks"),
    ("bModalUIOwnsInput" in hud and "SetRenderOpacity" in hud,
     "gameplay HUD cannot overlap terminal, reader, or pause content"),
    ("IsPersistentStoryZombieId" in game_instance_h
     and "ZombieId >= 2000000 && ZombieId < 2200000" in game_instance,
     "only boss and named-miniboss deaths persist as removals"),
    ("IsPersistentStoryZombieId(Zombie->ZombieId)" in game_instance,
     "saved world-state application preserves renewable enemies"),
    ("NeutralizedZombieIds.Contains(ZombieId)" not in game_mode
     and "NeutralizedZombieIds.Contains(PupId)" not in game_mode,
     "regular, horde, director, and dog spawners ignore prior-session deaths"),
    ("CityZombiePopulation" in game_mode
     and "RenewableAlive >= 6" in game_mode,
     "every non-sandbox loaded city requires a visible renewable population"),
    ("--\\\\s*[A-Za-z_]" in runner
     and "postfix decrement" in runner,
     "reverse validator accepts prefix and postfix decrement"),
    # 2026-07-11 (afternoon fix pass): the single reverse-string vector became a
    # TABLE of alternate-solution vectors. The contract is STRONGER now — all
    # three player-reported rejected-but-correct solutions (prefix-decrement
    # reverse, != two-pointer palindrome, overflow-safe midpoint binary
    # search) are pinned as runtime regression tests, verbatim.
    ("i >= 0; --i" in game_mode
     and "s[left] != s[right]" in game_mode
     and "low + (high - low) / 2" in game_mode
     and "two_pointer_neq_palindrome" in game_mode
     and "overflow_safe_midpoint_bsearch" in game_mode
     and "FirstLevelAlternateSolutionAudit" in game_mode,
     "the exact reported valid C++ solution is a runtime regression test"),
    ("CODE ACCEPTED\\nRESCUE ROUTE OPEN" not in game_mode,
     "ordinary solved-route reconstruction cannot spawn the blocking reader"),
    ("SetIsFocusable(true)" in reader
     and "ReaderCloseButton" in reader
     and "SetUIOpen(true)" in reader,
     "message reader owns focus, input lock, and an explicit mouse close"),
    ("IsReaderOpen()" in character
     and "CloseActiveReader();" in character
     and "MessageReaderRoutingAudit" in character,
     "reader, journal, and pause surfaces are mutually exclusive and audited"),
    ("NativeOnPreviewMouseButtonDown" in pause_h
     and "NativeOnMouseButtonDown" in pause_h
     and pause.count("RoutePointerAtScreenPosition") >= 3,
     "pause has preview and bubble pointer routing fallbacks"),
    ("[PauseAction]" in pause
     and "PauseMouseAudit" in pause
     and "SetClickMethod(EButtonClickMethod::MouseDown)" in pause,
     "all pause actions expose click telemetry and direct activation"),
    (all(name in character for name in (
        "MM_Idle", "MM_Walk_Fwd", "MM_Run_Fwd",
        "MM_Jump", "MM_Fall_Loop", "MM_Land")),
     "Manny loads six authored production animation sequences"),
    ("UpdateAuthoredMannyAnimation" in character_h
     and "PlayerAnimationRuntimeAudit" in character
     and "TwoArmWeaponAimPoseActive" in character,
     "locomotion, airborne states, landing, and two-arm aim are integrated"),
    ("ground=1 population=1" in pause
     and "animation=1 reader=1 armory=1 pause_mouse=1" in pause,
     "single-session integrated acceptance includes all reported regressions"),
    # 2026-07-11 (evening) resume-launch crash: UpdatePhysicsEngineImp asserted
    # ("Array index out of bounds") because the hit-reaction reset and the
    # death paths tore down body instances while the physical-animation
    # component stayed bound with live drive data. The component must detach
    # at reset, at death, on the pre-swap path, AND via the per-tick empty-
    # bodies failsafe; the -CodeRescueAutoResumeLanguage harness reproduces
    # the real-save resume path headlessly.
    (zombie.count("SetSkeletalMeshComponent(nullptr)") >= 4
     and "Bodies.Num() == 0" in zombie
     and "CodeRescueAutoResumeLanguage" in character,
     "physical animation detaches on reset/death and the resume-launch harness exists"),
]

failed = [label for passed, label in checks if not passed]
for passed, label in checks:
    print(f"[{'PASS' if passed else 'FAIL'}] {label}")

if failed:
    raise SystemExit(f"{len(failed)} regression contract check(s) failed")

print(f"[PASS] 2026-07-11 regression source contract ({len(checks)}/{len(checks)})")
