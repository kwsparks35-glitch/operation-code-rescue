# May 27 Tactical Arsenal, MCP, and Runtime Pass

Current package note: this pass was superseded by the May 27 Unreal systems
character/world package documented in
`Documentation/improvement_pass_2026-05-27/32_UNREAL_SYSTEMS_CHARACTER_WORLD_PASS.md`.
The tactical arsenal/runtime work below remains included in that newer build.

## Gameplay changes

- The player now starts with a full original survival-horror arsenal. In short, all weapons are immediately available in normal gameplay and can be cycled with mouse wheel, bracket keys, or gamepad left shoulder.
- Direct quick-select now uses 1-0 for the first ten arsenal slots, while wheel/bracket cycling still reaches every available weapon and utility.
- The old single shared reserve was replaced with per-weapon reserve ammo while preserving the legacy total ammo value for saves and HUD compatibility.
- Weapons now have tactical roles in code and HUD: sidearm economy, shotgun crowd control, SMG suppression, precision pierce, magnum stopping power, bolt piercing, rocket burst, incendiary area denial, flash stun, and no-ammo knife fallback.
- Grenade-style weapons now use real area effects instead of inert stats. Flash grenades intentionally do low damage but still interrupt enemies through the existing stagger path.
- Visible armory staging was added near the open level entry. It displays every weapon archetype, tactical gear crates, ammo, medkit support, and control labels so the level starts with clear access instead of barrier friction.

## Runtime stability

- Code validation subprocesses now run through a timed process wrapper.
- The runtime validator timeout is 8 seconds. When user code, compilers, interpreters, or test harnesses hang, the process tree is terminated and the terminal gets a clear timeout message instead of freezing the game or commandlet.
- This specifically targets the previous curriculum verifier stall seen during the Mac package audit.
- If MATLAB batch mode exists on the Mac but does not respond in time, the terminal now marks the local MATLAB CLI as unavailable for the session and falls back to the in-engine MATLAB-compatible validator so gameplay can continue.

## MCP and Unreal constituent access matrix

- The local Fab/Unreal macOS MCP server was advanced to version 0.2.0.
- The server now exposes an Unreal constituent access matrix covering MetaHuman Character Design, MetaHuman for Maya and Houdini DCC handoff, Chaos Interactive and Async Physics, AI for NPC and enemy characters, Extended Standard Libraries and Plugins, Quest and Mission Kits, plus Niagara/PCG/World Partition/Sequencer/Control Rig/IK Retargeter/Groom systems.
- The matrix is deliberately honest: it records local hooks, validation commands, macOS caveats, and which systems require authenticated Fab/Epic/DCC materialization. It does not claim to download or license remote assets.
- The matrix is available through the MCP tool `unreal_constituent_matrix`, the resource `unreal://project/current/constituent-access-matrix`, the project scan, and the generated Fab MCP asset plan.

## Mac demo app rebuild and handoff status

Fresh package for review:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

Package evidence:

- Size: `1.9G`
- Timestamp from `stat`: `May 27 12:01:58 2026`
- Null smoke log: `Saved/Logs/PackagedSmoke_null.log`
- Render smoke log: `Saved/Logs/PackagedSmoke_render.log`

Passed validation:

- `python3 Scripts/verify_may27_tactical_arsenal_mcp_runtime.py`
- `python3 Scripts/verify_fab_unreal_mcp_porting.py`
- `python3 Scripts/verify_may27_gameplay_access_pass.py`
- `python3 Scripts/verify_fab_import_and_entry_access.py`
- `python3 Scripts/verify_audit_implementation_closure.py`
- `python3 -m py_compile ../MCP_Server_Development/fab_unreal_macos_mcp/server.py Scripts/verify_may27_tactical_arsenal_mcp_runtime.py Scripts/verify_fab_unreal_mcp_porting.py`
- `./Recompile_Module.command`
- `Scripts/verify_curriculum_validator_shapes.py` commandlet
- `Scripts/verify_runtime_step_smoke_contracts.py` commandlet
- `Scripts/verify_camera_perspectives_and_character_roster.py` commandlet
- `Scripts/verify_character_world_assets.py` commandlet
- `Scripts/verify_production_track_completion.py` commandlet
- `./Package_Mac_App.command`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`
- `git diff --check`

Packaged smoke notes:

- Null smoke launched the packaged app with `-NullRHI -nosound`, loaded
  `/Engine/Maps/Entry`, and exited cleanly.
- Render smoke launched the packaged app with Metal/CoreAudio, loaded
  `/Engine/Maps/Entry`, and exited cleanly.
- The scanner only allowed the known immediate-quit navigation/crowd diagnostics
  and unattended macOS CoreAudio sample-rate query warning. No errors, fatals,
  stale asset references, load errors, linker warnings, or missing object
  warnings were reported.
