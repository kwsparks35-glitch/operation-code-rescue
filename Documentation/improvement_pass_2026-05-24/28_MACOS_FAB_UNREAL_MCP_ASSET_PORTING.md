# MacOS Fab/Unreal MCP Asset Porting Pass

## Scope

Implemented the local MCP-server development described by the two attached
MCP/Fab/Unreal PDF documents. The goal of this pass was to make the Windows/
Linux-oriented Fab and Unreal asset-porting workflow usable on this Mac, while
preserving the game's coding-learning purpose and keeping license/authentication
boundaries honest.

## PDF-Derived Design Rules Applied

1. Use a local stdio MCP server for workstation/project access.
2. Keep Fab/Vault cache access read-only unless a user-owned local asset folder
   is explicitly staged.
3. Treat all mutating operations as dry-run-first.
4. Expose strict, typed tools and structured reports.
5. Use Unreal's own automation surfaces: Python Editor Script Plugin,
   commandlets, UAT/UBT, resave, Blueprint compile, and DataValidation.
6. Classify assets by risk: content-only, character/animation, Blueprint smart
   asset, code/plugin, binary-only, unsupported feature, or manual-review.
7. Never claim a binary-only Windows/Linux plugin is Mac-compatible without
   source code, Mac libraries, or publisher support.

## Implemented Files

- `MCP_Server_Development/fab_unreal_macos_mcp/server.py`
  - No-dependency local MCP server over JSON-RPC stdio.
  - Tools: `verify_roots`, `scan_fab_cache`, `scan_unreal_project`,
    `analyze_fab_item`, `build_porting_plan`, `write_project_manifest`,
    `stage_local_content_copy`, and `commandlet_plan`.
  - Resources: `fab://library/index`, `unreal://project/current/summary`, and
    `mcp://policy/mac-fab-unreal-sop`.
  - Prompts: `port_fab_asset_to_macos` and `forward_port_character_to_ue57`.
- `MCP_Server_Development/fab_unreal_macos_mcp/README.md`
- `MCP_Server_Development/fab_unreal_macos_mcp/claude_desktop_config.example.json`
- `MCP_Server_Development/fab_unreal_macos_mcp/mcp_roots.example.json`
- `Run_Fab_Unreal_MCP_Server.command`
- `Run_Fab_Unreal_MCP_Audit.command`
- `Scripts/mcp_fab_unreal_import_validate.py`
- `Scripts/verify_fab_unreal_mcp_porting.py`
- `Content/CodeRescueData/fab_unreal_mcp_asset_plan.json`
- `Content/CodeRescueData/fab_unreal_mcp_world_generation_queue.tsv`
- `Content/CodeRescueData/fab_unreal_mcp_import_status.tsv`
- `Saved/MCPFabUnreal/latest_asset_audit.json`
- `Saved/MCPFabUnreal/import_available_report.json`
- `Saved/MCPFabUnreal/entry_access_and_import_review.json`
- `Saved/MCPFabUnreal/unreal_asset_validation_report.json`

## Audit Result

The MCP audit found 16 local Fab/Vault entries.

Verdict counts:

- `portable`: 2
- `portable_after_retarget`: 5
- `manual_review_required`: 9

Already represented in the project:

1. Building Interior Cubemap Material Function -> `Parallax_Night_Building_Material`
2. Dog Zombie -> `DogZombie`
3. Modern Bridges -> `ModernBridges`
4. Urban Zombie 4 -> `UrbanZombie4`
5. Zombie -> `Zombie`
6. Zombie - Bloated Female -> `YI_ModularZombies`
7. Zombie - Business Suit -> `YI_ModularZombies`

## Import Execution Result

After the first audit pass, the MCP server was extended with an
`import_available_fab_assets` tool. That tool scans the plan, finds any local,
materialized, user-owned `Content` folders, stages safe local content into the
project when explicitly confirmed, and writes a status report.

Execution result:

- Total local Fab/Vault entries inspected: 16
- Already incorporated in the game environment: 7
- Newly staged from local materialized `Content` folders: 0
- Requiring Fab Launcher/Fab Window materialization or source/plugin review: 9

The following seven entries are already present and included in the game-world
environment or validation plan:

1. Building Interior Cubemap Material Function
2. Dog Zombie
3. Modern Bridges
4. Urban Zombie 4
5. Zombie
6. Zombie - Bloated Female
7. Zombie - Business Suit

The following nine entries were not locally materialized as importable project
`Content` folders during this pass, so they were not falsely marked imported:

1. AI for NPC MetaHuman - Dialog actions and general intelligence - by Convai
2. ASYNC PHYSICS Blueprints Library
3. LE Extended Standard Library
4. MetaHuman Animator Depth Processing Plugin
5. MetaHuman for Houdini
6. MetaHuman for Maya
7. MetaHuman Groom Advanced Kit for Houdini
8. Quest Kit Pro
9. Zombie Female Nurse

Those nine are now actionable in
`Content/CodeRescueData/fab_unreal_mcp_import_status.tsv` and
`Saved/MCPFabUnreal/import_available_report.json`. They should be materialized
through the Epic Games Launcher or Fab window, then rerun through the MCP audit
and import tool. Binary/tool/plugin entries still require honest Mac source,
Mac binaries, or publisher support before they can be treated as compatible.

Manual-review items are not marked failed. They are marked honestly because the
local cache entry is an Epic binary manifest or plugin/tooling-oriented entry
that does not expose file-level source evidence to this no-dependency server.
Those items should be refreshed/materialized through the Fab window or Epic
Games Launcher, then rescanned.

Unreal-side validation was run through `UnrealEditor-Cmd` with
`-run=PythonScript`, `-unattended`, `-NoSound`, and `-NullRHI`. Unreal completed
with 0 errors and 0 warnings. The generated
`Saved/MCPFabUnreal/unreal_asset_validation_report.json` confirms 7 already
present Fab-derived content roots and 0 asset-registry warnings for those
present roots. The report's remaining warnings are manual-review notes for
unmaterialized/plugin-style cache entries, not playability blockers.

## World And Character Integration

The generated `world-generation queue` maps Fab assets into game-world roles:

- Modern Bridges -> city traversal architecture and rescue-route crossings.
- Building Interior Cubemap Material Function -> skyline/interior material
  upgrade candidate.
- Dog Zombie, Urban Zombie 4, Zombie, Zombie - Bloated Female, and Zombie -
  Business Suit -> enemy character roster and animation validation queue.
- MetaHuman/groom/tooling entries -> survivor/mentor character pipeline after
  launcher materialization and license-safe validation.

This keeps the game intent intact: assets support visual character/world
construction while the play loop remains gamified coding education.

## macOS Compatibility Policy

- Content-only assets can usually forward-port into UE 5.7 through sandbox
  migration, resave, and validation.
- Character assets require IK/retarget/physics validation even when they already
  load.
- Blueprint-heavy assets require `CompileAllBlueprints` review for deprecated
  nodes.
- Code plugins require UAT/UBT Mac rebuilds, third-party dylib checks,
  `@rpath` repair, universal-binary validation, and codesigning review.
- Binary-only Windows/Linux plugins remain blocked until publisher source,
  publisher Mac binaries, or an internal replacement exists.

## Commands

Run the local server self-test and write the asset plan:

```sh
./Run_Fab_Unreal_MCP_Audit.command
```

Start the MCP server for a host:

```sh
./Run_Fab_Unreal_MCP_Server.command
```

Run the explicit local-materialized-content import pass:

```sh
printf '%s\n' \
  '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}' \
  '{"jsonrpc":"2.0","id":2,"method":"tools/call","params":{"name":"import_available_fab_assets","arguments":{"dry_run":false,"confirm_import":"import_available_fab_assets","target_platform":"Mac","target_engine_version":"5.7"}}}' \
  | python3 ../MCP_Server_Development/fab_unreal_macos_mcp/server.py
```

Run the standalone verifier:

```sh
python3 Scripts/verify_fab_unreal_mcp_porting.py
```

Run the entry-access and Fab import verifier:

```sh
python3 Scripts/verify_fab_import_and_entry_access.py
```

Run Unreal-side validation after a migration/import pass:

```sh
ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
"$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
  -run=PythonScript -script="$(pwd)/Scripts/mcp_fab_unreal_import_validate.py" \
  -unattended -NoSound -NullRHI
```

## Current Boundary

No remote Fab download was attempted or claimed. This implementation respects
the attached SOP's honesty boundary: it can inspect local manifests, plan
compatibility work, stage explicit local folders, run validation, and produce
Unreal command plans. It cannot and should not bypass Fab authentication,
publisher licensing, or missing source code.
