#include <stdio.h>
#include "bank1/test_bank1.h"

#define RUN_BANK1_TEST(fn, name) \
    do { \
        printf("[RUN ] %s\n", name); \
        fn(); \
        printf("[PASS] %s\n", name); \
    } while (0)

void run_bank1_tests(void) {
    printf("[TEST] Bank 1 Subsystems\n");

    RUN_BANK1_TEST(test_prepare_entity_position_for_room_transition, "PrepareEntityPositionForRoomTransition");
    RUN_BANK1_TEST(test_update_recent_rooms_list, "UpdateRecentRoomsList");
    RUN_BANK1_TEST(test_hide_all_sprites, "HideAllSprites");
    RUN_BANK1_TEST(test_hide_sprites, "HideSprites");
    RUN_BANK1_TEST(test_synchronize_dungeons_item_flags, "SynchronizeDungeonsItemFlags");
    RUN_BANK1_TEST(test_create_following_npc_entity, "CreateFollowingNpcEntity");
    RUN_BANK1_TEST(test_func_001_6162, "func_001_6162");
    RUN_BANK1_TEST(test_load_counter_animated_tiles, "LoadCounterAnimatedTiles");
    RUN_BANK1_TEST(test_open_dungeon_name_dialog, "OpenDungeonNameDialog");
    RUN_BANK1_TEST(test_load_tileset_0f_and_attributes, "LoadTileset0FAndAttributes");
    RUN_BANK1_TEST(test_write_dma_code_to_hram, "WriteDMACodeToHRAM");
    RUN_BANK1_TEST(test_update_minimap_entrance_arrow, "UpdateMinimapEntranceArrow");
    RUN_BANK1_TEST(test_increment_gameplay_subtype, "IncrementGameplaySubtype");
    RUN_BANK1_TEST(test_func_001_5888, "func_001_5888");
    RUN_BANK1_TEST(test_initialize_inventory_bar, "InitializeInventoryBar");
    RUN_BANK1_TEST(test_func_001_58A8, "func_001_58A8");
    RUN_BANK1_TEST(test_peach_picture_state_2, "PeachPictureState2");
    RUN_BANK1_TEST(test_peach_picture_state_3, "PeachPictureState3");
    RUN_BANK1_TEST(test_func_001_695B, "func_001_695B");
    RUN_BANK1_TEST(test_func_6A7C, "func_6A7C");
    RUN_BANK1_TEST(test_peach_picture_state_4, "PeachPictureState4");
    RUN_BANK1_TEST(test_peach_picture_state_5_and_68D9, "PeachPictureState5And68D9");
    RUN_BANK1_TEST(test_peach_picture_state_7, "PeachPictureState7");
    RUN_BANK1_TEST(test_peach_picture_state_8, "PeachPictureState8");
    RUN_BANK1_TEST(test_peach_picture_state_9, "PeachPictureState9");
    RUN_BANK1_TEST(test_file_save_fade_out_and_state_A, "FileSaveFadeOutAndStateA");
    RUN_BANK1_TEST(test_peach_picture_state_0_and_1, "PeachPictureState0And1");
    RUN_BANK1_TEST(test_peach_picture_entry_point, "PeachPictureEntryPoint");
    RUN_BANK1_TEST(test_play_validation_jingle, "PlayValidationJingle");
    RUN_BANK1_TEST(test_func_001_5A59, "func_001_5A59");
    RUN_BANK1_TEST(test_world_map_states, "WorldMapStates");
    RUN_BANK1_TEST(test_move_select_and_jingle, "MoveSelectAndJingle");
    RUN_BANK1_TEST(test_label_001_5B3F, "label_001_5B3F");
    RUN_BANK1_TEST(test_func_001_5A71, "func_001_5A71");
    RUN_BANK1_TEST(test_func_001_5C49_and_5C55, "func_001_5C49_and_5C55");
    RUN_BANK1_TEST(test_world_map_interactive_and_entry_point, "WorldMapInteractiveAndEntryPoint");
    RUN_BANK1_TEST(test_build_save_slot_hearts_draw_command, "BuildSaveSlotHeartsDrawCommand");
    RUN_BANK1_TEST(test_func_5DC0_and_save_game_to_file, "Func5DC0AndSaveGameToFile");
    RUN_BANK1_TEST(test_load_saved_file, "LoadSavedFile");
    RUN_BANK1_TEST(test_func_001_4954, "func_001_4954");
    RUN_BANK1_TEST(test_file_selection_interactive_and_choice, "FileSelectionInteractiveAndChoice");
    RUN_BANK1_TEST(test_file_creation_init_and_sram, "FileCreationInitAndSRAM");
    RUN_BANK1_TEST(test_transition_to_file_menu_reload, "TransitionToFileMenuReload");
    RUN_BANK1_TEST(test_file_creation_grid_and_entry, "FileCreationGridAndEntry");
    RUN_BANK1_TEST(test_file_deletion_and_digits, "FileDeletionAndDigits");
    RUN_BANK1_TEST(test_file_deletion_interactive_and_erase, "FileDeletionInteractiveAndErase");
    RUN_BANK1_TEST(test_file_copy_subsystem, "FileCopySubsystem");
    RUN_BANK1_TEST(test_file_save_screen_and_init, "FileSaveScreenAndInit");
    RUN_BANK1_TEST(test_game_over_subsystem, "GameOverSubsystem");
    RUN_BANK1_TEST(test_world_handler_subsystem, "WorldHandlerSubsystem");
    RUN_BANK1_TEST(test_face_shrine_mural_subsystem, "FaceShrineMuralSubsystem");
    RUN_BANK1_TEST(test_siren_instruments_subsystem, "SirenInstrumentsSubsystem");
    RUN_BANK1_TEST(test_marin_beach_subsystem, "MarinBeachSubsystem");
    RUN_BANK1_TEST(test_intro_subsystem, "IntroSubsystem");

    printf("[PASS] Bank 1 Subsystems\n\n");
}
