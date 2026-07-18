#!/bin/zsh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

ENGINE_ROOT="$("$SCRIPT_DIR/Scripts/find_unreal_mac.sh")"
UPROJECT="$SCRIPT_DIR/CodeRescueUnreal.uproject"
EDITOR_CMD="$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd"
EDITOR="$ENGINE_ROOT/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
FULL_SMOKE_LOG="$SCRIPT_DIR/Saved/Logs/HeadlessFullQASmoke.log"

if [[ ! -x "$EDITOR" ]]; then
  EDITOR="$ENGINE_ROOT/Binaries/Mac/UnrealEditor"
fi

run_commandlet() {
  local script_path="$1"
  shift
  local extra_args=("$@")
  echo ""
  echo "== Unreal verifier: ${script_path:t} =="
  "$EDITOR_CMD" "$UPROJECT" \
    "${extra_args[@]}" \
    -run=pythonscript -script="$SCRIPT_DIR/$script_path" \
    -unattended -NoSound -NullRHI
}

echo "=============================================="
echo " Code Rescue Unreal - Full QA Audit"
echo "=============================================="
echo "Project: $UPROJECT"
echo "Engine:  $ENGINE_ROOT"
echo ""

# </dev/null keeps the inner script's interactive close prompt from blocking
# the audit when both run inside the same Terminal tty.
./Recompile_Module.command < /dev/null

echo ""
echo "== Static verifiers =="
python3 Scripts/verify_bespoke_survival_horror_art_ui.py
python3 Scripts/verify_bespoke_asset_animation_refinement.py
python3 Scripts/verify_audit_implementation_closure.py
python3 Scripts/verify_fab_unreal_mcp_porting.py
python3 Scripts/verify_fab_import_and_entry_access.py
python3 Scripts/verify_june01_rescue_survivability_pass.py
python3 Scripts/verify_squad_personality_tactical_readability_slice_pass.py
python3 Scripts/verify_companion_gesture_readability_slice_pass.py
python3 Scripts/verify_june12_next100_improvement_pass.py
python3 Scripts/verify_june12_squad_command_status_pass.py
python3 Scripts/verify_june12_us_city_identity_pass.py
python3 Scripts/verify_june13_arena_confinement_pass.py
python3 Scripts/verify_maple_sinister_narration_pass.py
python3 Scripts/verify_june12_city_realization_pass.py
python3 Scripts/verify_june18_public_hardening_pass.py
python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py
python3 Scripts/verify_tactical_gear_pickups_slice_pass.py
python3 Scripts/verify_interior_mission_spaces_slice_pass.py
python3 Scripts/verify_weather_lighting_identity_slice_pass.py
python3 Scripts/verify_regional_city_kit_identity_slice_pass.py
python3 Scripts/verify_challenge_room_concept_art_slice_pass.py
python3 Scripts/verify_city_street_grid_storefront_shell_slice_pass.py
python3 Scripts/verify_june19_playability_readability_fix_pass.py
python3 Scripts/verify_human_scale_route_access_slice_pass.py
python3 Scripts/verify_launch_language_start_screen_save_pass.py
python3 Scripts/verify_selected_language_terminal_flow_slice_pass.py
python3 Scripts/verify_first_session_route_preview_slice_pass.py
python3 Scripts/verify_protected_learning_zone_ai_slice_pass.py
python3 Scripts/verify_playable_operator_identity_slice_pass.py
python3 Scripts/verify_survivor_archetype_roster_slice_pass.py
python3 Scripts/verify_survivor_gesture_readability_slice_pass.py
python3 Scripts/verify_standard_direct_pursuit_zombie_slice_pass.py
python3 Scripts/verify_friendly_safehouse_npc_service_slice_pass.py
python3 Scripts/verify_friendly_npc_gesture_readability_slice_pass.py
python3 Scripts/verify_save_slots_language_backup_ux_slice_pass.py
python3 Scripts/verify_end_state_language_run_continuity_slice_pass.py
python3 Scripts/verify_death_replay_save_quit_flow_slice_pass.py
python3 Scripts/verify_onboarding_input_glyph_slice_pass.py
python3 Scripts/verify_pause_difficulty_matrix_slice_pass.py
python3 Scripts/verify_expanded_accessibility_options_slice_pass.py
python3 Scripts/verify_ui_text_scale_settings_slice_pass.py
python3 Scripts/verify_settings_audio_accessibility_slice_pass.py
python3 Scripts/verify_mono_audio_accessibility_slice_pass.py
python3 Scripts/verify_settings_color_vision_slice_pass.py
python3 Scripts/verify_settings_color_vision_live_refresh_slice_pass.py
python3 Scripts/verify_subtitle_accessibility_live_refresh_slice_pass.py
python3 Scripts/verify_damage_feedback_accessibility_slice_pass.py
python3 Scripts/verify_health_damage_survivability_slice_pass.py
python3 Scripts/verify_headshot_feedback_reduced_motion_slice_pass.py
python3 Scripts/verify_hud_vitals_theme_accessibility_slice_pass.py
python3 Scripts/verify_weapon_quick_slot_armory_slice_pass.py
python3 Scripts/verify_minimap_route_readability_slice_pass.py
python3 Scripts/verify_objective_route_toast_clarity_slice_pass.py
python3 Scripts/verify_field_checklist_hud_slice_pass.py
python3 Scripts/verify_objective_journal_accessibility_slice_pass.py
python3 Scripts/verify_inventory_map_journal_polish_slice_pass.py
python3 Scripts/verify_fail_safe_objective_board_slice_pass.py
python3 Scripts/verify_survivor_intel_dossier_slice_pass.py
python3 Scripts/verify_survivor_intel_archive_slice_pass.py
python3 Scripts/verify_city_radio_bark_cadence_slice_pass.py
python3 Scripts/verify_reactive_threat_audio_music_slice_pass.py
python3 Scripts/verify_city_ambient_zone_audio_slice_pass.py
python3 Scripts/verify_visualized_sound_cues_accessibility_slice_pass.py
python3 Scripts/verify_threat_audio_captions_slice_pass.py
python3 Scripts/verify_threat_compass_hud_slice_pass.py
python3 Scripts/verify_terminal_language_track_ux_pass.py
python3 Scripts/verify_terminal_diegetic_restyle_slice_pass.py
python3 Scripts/verify_terminal_post_solve_debrief_slice_pass.py
python3 Scripts/verify_terminal_practice_run_slice_pass.py
python3 Scripts/verify_terminal_reward_choice_kiosk_slice_pass.py
python3 Scripts/verify_persistent_learning_debrief_slice_pass.py
python3 Scripts/verify_challenge_replay_journal_slice_pass.py
python3 Scripts/verify_language_profile_recap_slice_pass.py
python3 Scripts/verify_skill_tree_progression_clarity_slice_pass.py
python3 Scripts/verify_creative_physics_world_slice_pass.py
python3 Scripts/verify_collision_channel_gameplay_contract_slice_pass.py
python3 Scripts/verify_fixed_timestep_physics_contract_slice_pass.py
python3 Scripts/verify_coding_world_response_slice_pass.py
python3 Scripts/verify_language_breach_encounter_slice_pass.py
python3 Scripts/verify_zombie_death_physics_slice_pass.py
python3 Scripts/verify_zombie_physical_animation_hit_reaction_slice_pass.py
python3 Scripts/verify_surface_impact_physics_slice_pass.py
python3 Scripts/verify_physics_lane_combat_encounter_slice_pass.py
python3 Scripts/verify_destructible_cover_physics_slice_pass.py
python3 Scripts/verify_jeep_surface_vehicle_physics_slice_pass.py
python3 Scripts/verify_encounter_director_ai_slice_pass.py
python3 Scripts/verify_adaptive_encounter_director_pressure_slice_pass.py
python3 Scripts/verify_stealth_avoidance_slice_pass.py
python3 Scripts/verify_player_first_person_animation_slice_pass.py
python3 Scripts/verify_distinct_weapon_presentation_slice_pass.py
python3 Scripts/verify_combat_juice_weapon_feel_slice_pass.py
python3 Scripts/verify_animation_budget_runtime_slice_pass.py
python3 Scripts/verify_retarget_control_rig_slots_slice_pass.py
python3 Scripts/verify_ik_foot_grounding_review_slice_pass.py
python3 Scripts/verify_maya_character_cleanup_slice_pass.py
python3 Scripts/verify_zombie_family_variants_slice_pass.py
python3 Scripts/verify_zombie_motion_readability_slice_pass.py
python3 Scripts/verify_mac_rendering_aa_readiness_slice_pass.py
python3 Scripts/verify_mac_hair_card_compatibility_slice_pass.py
python3 Scripts/verify_mac_feature_capability_gate_slice_pass.py
python3 Scripts/verify_mac_asset_import_budget_gate_slice_pass.py
python3 Scripts/verify_editor_data_validation_contract_pass.py
python3 Scripts/verify_character_promotion_validation_contract_pass.py
python3 Scripts/verify_physics_promotion_validation_contract_pass.py
python3 Scripts/verify_world_promotion_validation_contract_pass.py
python3 Scripts/verify_runtime_data_layer_migration_slice_pass.py
python3 Scripts/verify_houdini_modular_city_output_slice_pass.py
python3 Scripts/verify_mac_compatibility_validation_contract_pass.py
python3 Scripts/verify_secondary_motion_signal_slice_pass.py
python3 Scripts/verify_rescue_extraction_presentation_slice_pass.py
python3 Scripts/verify_survivor_rescue_dialogue_handoff_slice_pass.py
python3 Scripts/verify_case_file_collectibles_slice_pass.py
python3 Scripts/verify_environmental_storytelling_slice_pass.py
python3 Scripts/verify_world_bible_lore_slice_pass.py
python3 Scripts/verify_creative_development_implementation_ledger.py
python3 Scripts/verify_route_guidance_drone_slice_pass.py
python3 Scripts/verify_helipad_extraction_ready_slice_pass.py
python3 Scripts/verify_expanded_extraction_set_pieces_slice_pass.py
python3 Scripts/verify_extraction_debrief_fast_travel_slice_pass.py
python3 Scripts/verify_fast_travel_evac_route_readability_slice_pass.py
python3 Scripts/verify_sequencer_intro_boss_reveal_slice_pass.py
python3 Scripts/verify_boss_reveal_presentation_slice_pass.py
python3 Scripts/verify_boss_phase_telegraph_slice_pass.py
python3 Scripts/verify_elite_warden_miniboss_slice_pass.py
python3 Scripts/verify_objective_focus_beacon_slice_pass.py
python3 Scripts/verify_radio_scan_rescue_beacon_slice_pass.py
python3 Scripts/verify_save_compatibility_pass.py
python3 Scripts/verify_asset_budget_pass.py
python3 Scripts/verify_demo_readiness_pass.py
python3 Scripts/verify_external_demo_preflight_slice_pass.py
python3 Scripts/apply_control_remap_profile.py
python3 Scripts/profile_city_layers_static.py
python3 Scripts/generate_nohuman_next20_evidence.py
python3 Scripts/verify_next20_nohuman_improvement_pass.py
python3 Scripts/generate_nohuman_next20_round2_evidence.py
python3 Scripts/verify_next20_round2_nohuman_improvement_pass.py

run_commandlet "Scripts/verify_graduated_campaign_world.py"
run_commandlet "Scripts/verify_next100_implementation.py"
run_commandlet "Scripts/verify_curriculum_validator_shapes.py" -AllowExternalCodeValidation
run_commandlet "Scripts/export_production_track_manifests.py"
run_commandlet "Scripts/verify_production_track_completion.py"
run_commandlet "Scripts/verify_character_world_assets.py"
run_commandlet "Scripts/verify_camera_perspectives_and_character_roster.py"
run_commandlet "Scripts/verify_code_rescue_data_validation_unreal.py"
run_commandlet "Scripts/verify_character_promotion_validation_unreal.py"
run_commandlet "Scripts/verify_physics_promotion_validation_unreal.py"
run_commandlet "Scripts/verify_world_promotion_validation_unreal.py"
run_commandlet "Scripts/verify_mac_compatibility_validation_unreal.py"
run_commandlet "Scripts/verify_runtime_step_smoke_contracts.py"

echo ""
echo "== Headless runtime smoke =="
"$EDITOR" "$UPROJECT" \
  -game -NullRHI -NoSound -Unattended -NoRadioVoice -CodeRescueBypassLaunchLanguageMenu \
  -ExecCmds="Quit" -log -AbsLog="$FULL_SMOKE_LOG"

echo ""
echo "== Smoke log scan =="
python3 Scripts/scan_audit_warnings.py "$FULL_SMOKE_LOG"
python3 Scripts/verify_runtime_log_contracts.py "$FULL_SMOKE_LOG"

echo ""
echo "Full QA audit completed successfully."
echo "Smoke log: $FULL_SMOKE_LOG"
