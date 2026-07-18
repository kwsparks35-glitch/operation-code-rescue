# Operation Code Rescue — Project State Review (2026-06-25)

A grounded assessment of everything built to date, how it is built, what is strong, and where the
real opportunities for continued improvement lie. This review is the basis for the *Top 50
Recommendations* and the three release-readiness deep-dives (Character Animation, Game Physics,
World Development) produced in the same pass.

## 1. Executive summary

Operation Code Rescue is a **first-person, post-apocalyptic survival-horror game that teaches real
coding** (Java, C, C++, Python, MATLAB) through terminal puzzles, set inside a 465-city rescue
campaign. It is a **mature, unusually complete C++ prototype**: ~31,700 lines of C++ across ~90
files, a full UMG UI suite, deep progression and accessibility systems, day/night + weather,
achievements, leaderboards, a mod loader, 230 voiced radio briefings, and an extensive automated
QA/release toolchain. It compiles, runs, and packages on macOS.

The honest framing — which the project's own docs already use — is that this is a **strong
foundation, not yet a shipped commercial AAA FPS.** The gap to a "fully functional game release"
is concentrated in four areas: **character animation, physics, hand-crafted world/art, and UX
polish.** This pass attacks UX directly and specifies the path for the other three.

## 2. How it is built (architecture)

- **Engine:** Unreal Engine 5.7, single C++ runtime module `CodeRescueUnreal` (depends on Core,
  Engine, UMG/Slate, AIModule, NavigationSystem, Niagara, Json). Editor plugins enabled: PCG,
  GeometryScripting, ModelingToolsEditorMode, PythonScriptPlugin, EditorScriptingUtilities, Niagara.
- **World generation:** there is **no binary `.umap` level** for gameplay — the world is generated
  **from C++ at runtime** in the engine entry map (`ACodeRescueGameMode` and its spawn layers).
  This is the project's defining technical choice: it made a 465-city campaign tractable solo, but
  it is also the root of the "world feels procedural, not art-directed" gap.
- **Player:** `ACodeRescueCharacter` (first-person), with a custom polled-input control path rather
  than raw axis bindings (a deliberate macOS-stability decision documented across passes).
- **Persistence:** `UCodeRescueGameInstance` holds run state, language selection, difficulty,
  accessibility flags, and progression counters; `UCodeRescueSaveGame` provides save/load.
- **Platform reality:** Apple Silicon / Metal. This drives several constraints surfaced in the
  deep-dives (no MetaHuman Animator on Mac yet, Groom hair strands unsupported on Metal, Lumen
  software ray tracing on M1–M3, Motion Matching still experimental in 5.7).

## 3. Systems inventory

| Domain | What exists |
|---|---|
| **Combat** | Hitscan shooting, reload, ammo, headshots, throwables, barricades, damage-direction feedback, low-health vignette |
| **Enemies** | `ACodeZombieActor` (+ Fab packs: dog, female, urban, modular), `ABossZombieActor`, hordes |
| **AI** | `ACodeRescueAIController` (navmesh chase + direct fallback); friendly NPCs, survivors, rescue squad with formations |
| **Characters** | Player, survivors, companion, friendly NPCs — currently **primitive fallback meshes + unassigned skeletal/anim slots** |
| **Coding loop** | `ACodingTerminalActor` + `CodeTerminalWidget` with real external validators for Java/C/C++/Python/MATLAB; curriculum DB |
| **World** | Runtime-procedural city districts, arena confinement, fast travel, helipad extraction, jeep vehicle, set-pieces |
| **Progression** | Skill tree, crafting, research points, mastery streaks, achievements, local leaderboards |
| **UI** | 18 UMG widgets (HUD, terminal, main menu, pause, settings, save slots, minimap, objective journal, fast travel, tutorial, subtitles, death/victory, skill tree, damage feedback) |
| **Accessibility** | Subtitles + scale, high-contrast HUD, reduced motion, simplified hints, aim assist, colorblind mode |
| **Atmosphere/Audio** | Day/night + weather; 230 "Maple" voiced radio briefings; music hooks |
| **Sandbox/Mods** | Sandbox game mode; `CodeRescueModLoader` |
| **Tooling** | QA audit, release-readiness gates, packaging, smoke tests, visual regression, performance profiling, a Fab/Unreal macOS MCP server |

## 4. Scale & maturity indicators

- ~**31,700** lines of C++ across ~**90** files; **143** Markdown docs; a **122 KB** `progress.md`.
- **40+** dated improvement-pass folders since April 2026 — a sustained, iterative development cadence.
- `CodeRescueGameMode.cpp` alone is **~12,000 lines** — the single largest file and the clearest
  technical-debt target (it owns world gen, spawning, city identity, set-pieces, and more).
- Confirmed to compile, run, and package on Mac; release tooling produces a packaged app and
  support bundle.

## 5. Strengths

1. **A genuinely novel premise** — teaching real, validated coding inside a survival-horror loop —
   with the hard part (real language validators, a 465-city curriculum spine) already working.
2. **Breadth of systems** usually absent from prototypes: progression, accessibility, achievements,
   leaderboards, mods, weather, vehicles.
3. **Engineering discipline:** automated QA gates, release manifests, smoke tests, and meticulous
   documentation make the project safe to iterate on.
4. **Accessibility-first instincts** — the flags already exist; this pass made them actually drive
   the UI.

## 6. Gaps & risks (the real improvement surface)

1. **Character animation is unrealized.** Skeletal/anim slots exist but are unassigned; actors fall
   back to primitive geometry. No anim blueprints, locomotion blendspaces, retargeting, or facial
   animation. *This is the single biggest visual-quality gap.* → see `CHARACTER_ANIMATION_*.pdf`.
2. **Physics is rudimentary.** Hitscan combat, no ragdoll/physical animation, throwables without
   real impulse, a jeep on floating-pawn movement rather than Chaos Vehicles. → `GAME_PHYSICS_*.pdf`.
3. **World is procedural, not authored.** Functional but lacks art-directed, hand-crafted spaces,
   landmark variety, and a modular kit + PCG pipeline that scales with quality. → `WORLD_*.pdf`.
4. **UX was programmer-built and inconsistent** (addressed this pass via the design-system overhaul;
   rollout to HUD/terminal remains).
5. **Technical debt:** the 12k-line `CodeRescueGameMode.cpp` should be split by ownership.
6. **No human playtest / no signing & notarization / no hosted CI** — these remain the documented
   boundaries to an actual release.

## 7. Verification status

- **Automated gates:** passing per the latest passes (QA audit, package integrity, smoke).
- **Human validation:** still outstanding — a first-ten-minutes hands-on pass, per-difficulty feel,
  and audio/UI comfort need a person.
- **Distribution:** Apple Developer signing/notarization and hosted Mac CI remain external.

## 8. Conclusion

The project has comprehensively solved *breadth* and *systems*; the remaining work is *fidelity*
(characters, physics, world art) and *finish* (UX rollout, a human playtest loop, signing). The
recommendations and deep-dives in this pass are sequenced to convert this strong foundation into a
releasable game without discarding what already works — especially the runtime-procedural world,
which should be **evolved into an authored + PCG hybrid**, not thrown away.
