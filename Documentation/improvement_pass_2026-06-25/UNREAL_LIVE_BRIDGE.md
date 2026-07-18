# Connecting Claude to Unreal Engine — Live Bridge (2026-06-25)

You asked Claude to "establish a connection with Unreal Engine ... to actively test, create,
build, and implement." Here is the honest, working answer.

## What is *not* possible

Claude's sandbox is an **isolated ARM64 Linux VM with ~8 GB free disk, no GPU, allow-listed
network, and a per-command time cap.** Unreal Engine 5.7 needs 100 GB+, a GPU, an Epic license,
and a multi-hour source build. **UE cannot run inside Claude's sandbox.** Any claim otherwise
would be false.

## What *is* possible — three real tiers

Your project folder is **mounted** into Claude's sandbox, so Claude can read and write files
inside the project. That single fact enables a genuine connection to the Unreal Editor running on
*your* Mac.

### Tier 1 — Async file bridge (preferred; built this pass)

A small Python bridge runs **inside your editor** and watches a folder:

```
Saved/ClaudeBridge/inbox/    Claude drops <id>.json command files here
Saved/ClaudeBridge/outbox/   the bridge writes <id>.json results here
Saved/ClaudeBridge/outbox/_bridge_status.json   heartbeat while the editor is open
```

Files added this pass:

- `Content/Python/claude_unreal_bridge.py` — the bridge (game-thread Slate post-tick poller).
- `Content/Python/init_unreal.py` — auto-starts the bridge on editor launch.
- `Run_Claude_Unreal_Bridge.command` — opens the editor (bridge auto-starts).

**How to use it:** double-click `Run_Claude_Unreal_Bridge.command` (or just open the `.uproject`),
leave the editor open, and tell Claude. Claude writes a command; you keep working; Claude reads the
result on its next turn. Supported actions today:

| action | does |
|---|---|
| `ping` | engine version, project, world (proves the link) |
| `exec` | run arbitrary editor Python (`RESULT = ...` is returned; stdout captured) |
| `console` | run a console command |
| `screenshot` | `HighResShot` to `Saved/Screenshots/` — **Claude can then read the PNG and "see" the editor** |
| `save_all` | save dirty packages |
| `list_actors` | enumerate level actors (label + class) |
| `data_validation` | best-effort asset scan (full gate stays in `Run_Full_QA_Audit.command`) |

Example command Claude would drop as `inbox/0007.json`:

```json
{ "id": "0007", "action": "screenshot", "args": { "width": 1600, "height": 900 } }
```

This is how Claude can **create** (spawn/edit actors and assets via `exec`), **test** (PIE console
commands, screenshots), and **inspect** the live editor across turns.

### Tier 2 — Computer-use (interactive GUI)

For things that need a human-style GUI session (clicking the editor, pressing Play, dragging in the
viewport), Claude can use desktop computer-use to drive the editor directly on your Mac with your
approval. Best for one-off interactive checks; the file bridge is better for repeatable operations.

### Tier 3 — Build & package scripts (compiling C++)

Compiling the C++ module is a toolchain job, not an in-editor Python job. Keep using the project's
existing scripts on your Mac — they already work and are the source of truth for builds:

- `Recompile_Module.command` / open the `.uproject` and let it rebuild
- `Run_Full_QA_Audit.command`, `Run_Local_CI_Readiness.command`
- `Package_Mac_App.command`, `Smoke_Test_Packaged_App.command`

Claude can author and edit these and the C++ they compile (as it did this pass), then read the
resulting logs from `Saved/Logs/` through the mount to diagnose failures.

## Recommended workflow

1. Open the editor via `Run_Claude_Unreal_Bridge.command`.
2. Compile any new C++ (Tier 3) — e.g. after this pass's `CodeRescueUITheme` + widget changes.
3. Ask Claude to verify visually: Claude drops a `screenshot` command (Tier 1) and reads the PNG.
4. Iterate: Claude edits source/assets, you recompile, Claude re-checks.

## Security

The `exec` action runs arbitrary Python in your editor — that is what lets Claude drive it, but it
means you are trusting the command author. The bridge only reads from this project's
`Saved/ClaudeBridge/inbox`. To disable, delete `Content/Python/init_unreal.py` (or the whole
`Content/Python/` bridge). Nothing in the bridge bypasses Epic/Fab authentication or licensing.
