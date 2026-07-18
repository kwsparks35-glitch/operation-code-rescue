# Survivor Archetype Roster Slice

This slice closes the June 25 P0 survivor-archetype guidance by turning the rescued survivor from a generic objective into a role-specific runtime roster member. The campaign now resolves a survivor profile from each mission lesson family, and the survivor actor carries that profile through world markers, HUD prompts, journal dossier text, rescue subtitles, companion handoff, and selected-language save continuity.

## Implementation

- Added `FCodeRescueSurvivorArchetypeProfile` and `FCodeRescueCampaign::GetSurvivorArchetypeProfile`.
- Mapped lesson families to readable survivor roles: Power-Grid Apprentice, Systems Mechanic, Radio-Code Cleanup Specialist, Archive Integrity Analyst, Drone Timing Coordinator, Data Medic, Network Engineer, and Supply-Cache Analyst.
- Added archetype fields and helpers to `ASurvivorActor`: title, icon label, field need, rescue skill, dossier hook, accent color, `ConfigureArchetypeFromMission`, `GetSurvivorArchetypeSummary`, and `GetInteractionPrompt`.
- Configured each spawned survivor from its city mission before save-state restoration, then tagged the live actor and helper markers with `SurvivorArchetypeRosterRuntime`, `SurvivorRoleReadableNameplate`, and `SelectedLanguageSurvivorHandoff`.
- Updated HUD hover prompts, immediate rescue confirmation, survivor relief-camp signage, rescue subtitles, dispatch handoff, companion handoff, and journal survivor dossier text to use the same archetype profile.

## Player Result

The player now sees who they are saving, why that survivor needs the coding lesson, and what field value the survivor adds after rescue. The start screen and selected-language save model remain unchanged: survivor rescue state still persists by selected language through existing `RescuedSurvivorNames`, while the archetype is deterministically rebuilt from the mission on load.

## Validation

Added `Content/CodeRescueData/survivor_archetype_roster_manifest.tsv` and `Scripts/verify_survivor_archetype_roster_slice_pass.py`, then wired the verifier into local CI and full QA. The creative inclusion plan, human QA checklist, visual regression targets, and this progress log now name the survivor roster slice as the P0 survivor-archetype implementation path.

Manual QA should inspect a fresh route, a terminal-solved route, and a rescued/reloaded route. The survivor marker, prompt, relief-camp card, journal dossier, rescue subtitle, dispatch line, and companion handoff should all agree on survivor role, icon, need, and selected-language save continuity.
