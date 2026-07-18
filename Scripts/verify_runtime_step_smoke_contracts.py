"""
Verify runtime-step gameplay contracts in an editor commandlet.

This is the active smoke complement to immediate-quit boot logs: it spawns the
player and a terminal, cycles every camera, applies representative movement
input, and verifies that an unsolved terminal remains unsolved until validation
marks it complete.

Run from the project root:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/verify_runtime_step_smoke_contracts.py" \
        -unattended -NoSound -NullRHI
"""

import unreal


PLAYER_CLASS = "/Script/CodeRescueUnreal.CodeRescueCharacter"
TERMINAL_CLASS = "/Script/CodeRescueUnreal.CodingTerminalActor"


def fail(message):
    unreal.log_error(f"[cr-runtime-step] {message}")
    raise RuntimeError(message)


def load_class(label, path):
    cls = unreal.load_class(None, path)
    if not cls:
        fail(f"missing class {label}: {path}")
    return cls


def get_editor_world():
    subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    if subsystem:
        world = subsystem.get_editor_world()
        if world:
            return world
    try:
        return unreal.EditorLevelLibrary.get_editor_world()
    except Exception:
        fail("could not resolve editor world")


def destroy_actor(actor):
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if subsystem:
        subsystem.destroy_actor(actor)
    else:
        unreal.EditorLevelLibrary.destroy_actor(actor)


def main():
    unreal.log("[cr-runtime-step] === runtime step smoke verification START ===")
    player_class = load_class("player", PLAYER_CLASS)
    terminal_class = load_class("terminal", TERMINAL_CLASS)
    spawned = []

    try:
        player = unreal.EditorLevelLibrary.spawn_actor_from_class(
            player_class,
            unreal.Vector(0.0, 0.0, 220.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        if not player:
            fail("could not spawn player")
        spawned.append(player)

        for index in range(6):
            player.select_camera_perspective(index)
            actual = player.get_camera_perspective_index()
            label = player.get_camera_perspective_label()
            if actual != index:
                fail(f"camera did not select expected index {index}; got {actual}")
            unreal.log(f"[cr-runtime-step] camera {index}: {label}")

        player.add_movement_input(unreal.Vector(1.0, 0.0, 0.0), 1.0, False)
        player.add_movement_input(unreal.Vector(0.0, 1.0, 0.0), 0.6, False)
        unreal.log("[cr-runtime-step] movement input accepted")

        terminal = unreal.EditorLevelLibrary.spawn_actor_from_class(
            terminal_class,
            unreal.Vector(450.0, 0.0, 120.0),
            unreal.Rotator(0.0, 180.0, 0.0),
        )
        if not terminal:
            fail("could not spawn terminal")
        spawned.append(terminal)

        if not terminal.get_actor_enable_collision():
            fail("newly spawned terminal should start interactable")
        terminal.mark_solved()
        if terminal.get_actor_enable_collision():
            fail("terminal mark_solved did not disable interaction collision")
        unreal.log("[cr-runtime-step] terminal solve contract accepted")

    finally:
        for actor in spawned:
            try:
                destroy_actor(actor)
            except Exception:
                pass

    unreal.log("[cr-runtime-step] === runtime step smoke verification PASSED ===")


main()
