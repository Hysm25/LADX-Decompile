#include "home/entities.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "gb.h"
#include "home/gameplay.h"
#include "constants/gameplay.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/memory.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static void test_check_present_save_screen(void) {
    GBState gb;

    /* Base valid setup */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wDialogState, 0x00);
    gb_write(&gb, wC167, 0x00);
    gb_write(&gb, wRoomTransitionState, 0x00);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_WORLD_INTERACTIVE);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wD474, 0x00);
    gb_write(&gb, wD464, 0x00);
    gb_write(&gb, wC16C, 0x55);

    /* Test valid presentation */
    bool presented = CheckPresentSaveScreen(&gb);
    TEST_ASSERT(presented == true, "Save screen should be presented");
    TEST_ASSERT(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SAVE, "wGameplayType not set to GAMEPLAY_FILE_SAVE");
    TEST_ASSERT(gb_read(&gb, wGameplaySubtype) == 0, "wGameplaySubtype not cleared");
    TEST_ASSERT(gb_read(&gb, wTransitionSequenceCounter) == 0, "wTransitionSequenceCounter not cleared");
    TEST_ASSERT(gb_read(&gb, wC16C) == 0, "wC16C not cleared");
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0, "wDialogState not cleared");

    /* Condition 1: wTransitionSequenceCounter != 4 */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x02);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented despite transition counter != 4");

    /* Condition 2: wDialogState != 0 */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wDialogState, 0x01);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented despite dialog active");

    /* Condition 3: wC167 != 0 */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wC167, 0x02);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented despite wC167 != 0");

    /* Condition 4: wRoomTransitionState != 0 */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wRoomTransitionState, 0x01);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented despite room transition scrolling");

    /* Condition 5: wGameplayType >= GAMEPLAY_INVENTORY */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wGameplayType, GAMEPLAY_INVENTORY);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented during inventory");

    /* Condition 6: Button combo missing Select */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented without Select pressed");

    /* Condition 7: wD474 != 0 */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wD474, 0x01);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented despite wD474 != 0");

    /* Condition 8: wD464 != 0 */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wD464, 0x01);
    TEST_ASSERT(CheckPresentSaveScreen(&gb) == false, "Presented despite wD464 != 0");
}

static int mock_dialog_calls = 0;
static int mock_palettes_calls = 0;

static void mock_execute_dialog(GBState *gb) {
    mock_dialog_calls++;
    TEST_ASSERT(gb->rom_bank == 0x0F, "Dialog executed in wrong bank");
}

static void mock_load_bg_palettes(GBState *gb) {
    mock_palettes_calls++;
    TEST_ASSERT(gb->rom_bank == 0x24, "LoadBGPalettes executed in wrong bank");
}

static void test_return_from_gameplay_handler(void) {
    GBState gb;

    /* DMG mode: executes dialog in bank 0x0F, returns without palette call */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 0x00);
    mock_dialog_calls = 0;
    mock_palettes_calls = 0;

    returnFromGameplayHandler(&gb, mock_execute_dialog, mock_load_bg_palettes);
    TEST_ASSERT(mock_dialog_calls == 1, "Dialog handler not called on DMG");
    TEST_ASSERT(mock_palettes_calls == 0, "Palette handler called on DMG");

    /* CGB mode: executes dialog in bank 0x0F, then loads palettes in bank 0x24 */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 0x01);
    mock_dialog_calls = 0;
    mock_palettes_calls = 0;

    returnFromGameplayHandler(&gb, mock_execute_dialog, mock_load_bg_palettes);
    TEST_ASSERT(mock_dialog_calls == 1, "Dialog handler not called on CGB");
    TEST_ASSERT(mock_palettes_calls == 1, "Palette handler not called on CGB");
    TEST_ASSERT(gb.rom_bank == 0x24, "Final bank not 0x24 on CGB");
}

static void test_enable_sram(void) {
    GBState gb;
    gb_init(&gb);

    gb.sram_bank = 2;
    gb.sram_enabled = false;

    EnableSRAM(&gb);
    TEST_ASSERT(gb.sram_bank == 0, "SRAM bank not reset to 0");
    TEST_ASSERT(gb.sram_enabled == true, "SRAM not enabled");
}

static int mock_sync_calls = 0;
static void mock_sync_func(GBState *gb) {
    mock_sync_calls++;
    TEST_ASSERT(gb->rom_bank == 0x01, "Sync function not executed in bank 1");
}

static void test_synchronize_dungeons_trampoline(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wCurrentBank, 0x06);
    gb.rom_bank = 0x06;

    mock_sync_calls = 0;
    SynchronizeDungeonsItemFlags_trampoline(&gb, mock_sync_func);
    TEST_ASSERT(mock_sync_calls == 1, "Sync function was not called");
    TEST_ASSERT(gb.rom_bank == 0x06, "Current bank not restored");
}

static void test_get_random_byte(void) {
    GBState gb;
    gb_init(&gb);

    /* sum = 0x10 + 0x20 + 0x30 = 0x60 (0110 0000b) -> rrca is 0x30 (0011 0000b) */
    gb_write(&gb, hFrameCounter, 0x10);
    gb_write(&gb, wRandomSeed, 0x20);
    gb_write(&gb, rLY, 0x30);

    uint8_t rnd = GetRandomByte(&gb);
    TEST_ASSERT(rnd == 0x30, "Random byte did not match expected rrca value");
    TEST_ASSERT(gb_read(&gb, wRandomSeed) == 0x30, "wRandomSeed not updated");

    /* sum = 0x01 + 0x00 + 0x00 = 0x01 (0000 0001b) -> rrca is 0x80 (1000 0000b) */
    gb_write(&gb, hFrameCounter, 0x01);
    gb_write(&gb, wRandomSeed, 0x00);
    gb_write(&gb, rLY, 0x00);

    rnd = GetRandomByte(&gb);
    TEST_ASSERT(rnd == 0x80, "Random byte bit 0 rotation failed");
    TEST_ASSERT(gb_read(&gb, wRandomSeed) == 0x80, "wRandomSeed not updated for bit 0 rotation");
}

static void test_read_joypad_state(void) {
    GBState gb;

    /* 1. Map transition active -> early return, joypad untouched */
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 0x01);
    gb_write(&gb, hPressedButtonsMask, 0x55);
    gb_write(&gb, hJoypadState, 0xAA);
    gb.joypad_input = J_A;
    ReadJoypadState(&gb);
    TEST_ASSERT(gb_read(&gb, hPressedButtonsMask) == 0x55, "Buttons modified during room transition");
    TEST_ASSERT(gb_read(&gb, hJoypadState) == 0xAA, "JoypadState modified during room transition");

    /* 2. World gameplay, not interactive -> clear joypad */
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 0x00);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wGameplaySubtype, 0x00); /* not interactive */
    gb_write(&gb, hPressedButtonsMask, 0x55);
    gb_write(&gb, hJoypadState, 0xAA);
    ReadJoypadState(&gb);
    TEST_ASSERT(gb_read(&gb, hPressedButtonsMask) == 0x00, "Mask not cleared for non-interactive world");
    TEST_ASSERT(gb_read(&gb, hJoypadState) == 0x00, "State not cleared for non-interactive world");

    /* 3. World interactive, transition counter != 4 -> clear joypad */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_WORLD_INTERACTIVE);
    gb_write(&gb, wTransitionSequenceCounter, 0x02);
    gb_write(&gb, hPressedButtonsMask, 0x55);
    ReadJoypadState(&gb);
    TEST_ASSERT(gb_read(&gb, hPressedButtonsMask) == 0x00, "Mask not cleared when transition counter != 4");

    /* 4. World interactive, wPaletteUnknownE != 0 -> clear joypad */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_WORLD_INTERACTIVE);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wPaletteUnknownE, 0x01);
    gb_write(&gb, hPressedButtonsMask, 0x55);
    ReadJoypadState(&gb);
    TEST_ASSERT(gb_read(&gb, hPressedButtonsMask) == 0x00, "Mask not cleared when wPaletteUnknownE != 0");

    /* 5. World interactive, pass out animation exception: motion pass out and physics modifier == 4 */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_WORLD_INTERACTIVE);
    gb_write(&gb, wTransitionSequenceCounter, 0x00); /* normally would block */
    gb_write(&gb, wLinkMotionState, LINK_MOTION_PASS_OUT);
    gb_write(&gb, hLinkPhysicsModifier, 0x04);
    gb.joypad_input = J_B;
    ReadJoypadState(&gb);
    TEST_ASSERT(gb_read(&gb, hPressedButtonsMask) == J_B, "Pass out exception did not read joypad");
    TEST_ASSERT(gb_read(&gb, hJoypadState) == J_B, "Pass out exception did not set newly pressed button");

    /* 6. Normal interactive world read: previous button held, new button pressed */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_WORLD_INTERACTIVE);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wPaletteUnknownE, 0x00);
    gb_write(&gb, hPressedButtonsMask, J_A);
    gb.joypad_input = J_A | J_START;
    ReadJoypadState(&gb);
    TEST_ASSERT(gb_read(&gb, hPressedButtonsMask) == (J_A | J_START), "Held buttons mask incorrect");
    TEST_ASSERT(gb_read(&gb, hJoypadState) == J_START, "Newly pressed joypad state incorrect");
    TEST_ASSERT(gb_read(&gb, rP1) == (J_BUTTONS | J_DPAD), "rP1 not reset to J_BUTTONS | J_DPAD");

    /* 7. Non-world gameplay (e.g. file save menu) reads joypad directly */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_FILE_SAVE);
    gb.joypad_input = J_UP | J_SELECT;
    ReadJoypadState(&gb);
    TEST_ASSERT(gb_read(&gb, hPressedButtonsMask) == (J_UP | J_SELECT), "Non-world menu joypad read failed");
    TEST_ASSERT(gb_read(&gb, hJoypadState) == (J_UP | J_SELECT), "Non-world menu newly pressed failed");
}

static void test_get_intersected_object_bg_address(void) {
    GBState gb;
    gb_init(&gb);

    /* top = 0x18 + 0x08 = 0x20 -> row = 4. left = 0x10 + 0x00 = 0x10 -> col = 2 */
    /* address = 0x9800 + 4 * 32 + 2 = 0x9882 */
    gb_write(&gb, hIntersectedObjectTop, 0x18);
    gb_write(&gb, hBaseScrollY, 0x08);
    gb_write(&gb, hIntersectedObjectLeft, 0x10);
    gb_write(&gb, hBaseScrollX, 0x00);

    uint16_t addr = GetIntersectedObjectBGAddress(&gb);
    TEST_ASSERT(addr == 0x9882, "Calculated BG address mismatch");
    TEST_ASSERT(gb_read(&gb, hIntersectedObjectBGAddressHigh) == 0x98, "High byte mismatch");
    TEST_ASSERT(gb_read(&gb, hIntersectedObjectBGAddressLow) == 0x82, "Low byte mismatch");
}

static int table_call_idx = -1;
static void jump_target_0(GBState *gb) {
    (void)gb;
    table_call_idx = 0;
}
static void jump_target_1(GBState *gb) {
    (void)gb;
    table_call_idx = 1;
}
static void jump_target_2(GBState *gb) {
    (void)gb;
    table_call_idx = 2;
}

static void test_table_jump(void) {
    GBState gb;
    gb_init(&gb);

    const JumpTableFunc table[] = {
        jump_target_0,
        jump_target_1,
        jump_target_2,
    };

    table_call_idx = -1;
    TableJump(&gb, 0, table);
    TEST_ASSERT(table_call_idx == 0, "TableJump index 0 failed");

    table_call_idx = -1;
    TableJump(&gb, 1, table);
    TEST_ASSERT(table_call_idx == 1, "TableJump index 1 failed");

    table_call_idx = -1;
    TableJump(&gb, 2, table);
    TEST_ASSERT(table_call_idx == 2, "TableJump index 2 failed");
}


static int test_dispatch_called = -1;
static void test_cb_intro(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_INTRO; }
static void test_cb_credits(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_CREDITS; }
static void test_cb_file_sel(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_FILE_SELECT; }
static void test_cb_file_new(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_FILE_NEW; }
static void test_cb_file_del(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_FILE_DELETE; }
static void test_cb_file_cpy(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_FILE_COPY; }
static void test_cb_file_sav(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_FILE_SAVE; }
static void test_cb_world_map(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_WORLD_MAP; }
static void test_cb_peach(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_CUTSCENE; }
static void test_cb_marin(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_MARIN_BEACH; }
static void test_cb_shrine(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_WF_MURAL; }
static void test_cb_world(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_WORLD; }
static void test_cb_inventory(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_INVENTORY; }
static void test_cb_album(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_PHOTO_ALBUM; }
static void test_cb_photo(GBState *gb) { (void)gb; test_dispatch_called = GAMEPLAY_PHOTO_DIZZY_LINK; }

static void test_gameplay_dispatchers(void) {
    GBState gb;
    gb_init(&gb);

    GameplayCallbacks cbs = {
        .intro = test_cb_intro,
        .end_credits = test_cb_credits,
        .file_selection = test_cb_file_sel,
        .file_creation = test_cb_file_new,
        .file_deletion = test_cb_file_del,
        .file_copy = test_cb_file_cpy,
        .file_save = test_cb_file_sav,
        .world_map = test_cb_world_map,
        .peach_picture = test_cb_peach,
        .marin_beach = test_cb_marin,
        .face_shrine_mural = test_cb_shrine,
        .world = test_cb_world,
        .inventory = test_cb_inventory,
        .photo_album = test_cb_album,
        .photo_picture = test_cb_photo,
    };

    /* Test all gameplay types */
    uint8_t types[] = {
        GAMEPLAY_INTRO, GAMEPLAY_CREDITS, GAMEPLAY_FILE_SELECT, GAMEPLAY_FILE_NEW,
        GAMEPLAY_FILE_DELETE, GAMEPLAY_FILE_COPY, GAMEPLAY_FILE_SAVE, GAMEPLAY_WORLD_MAP,
        GAMEPLAY_CUTSCENE, GAMEPLAY_MARIN_BEACH, GAMEPLAY_WF_MURAL, GAMEPLAY_WORLD,
        GAMEPLAY_INVENTORY, GAMEPLAY_PHOTO_ALBUM, GAMEPLAY_PHOTO_DIZZY_LINK, GAMEPLAY_PHOTO_BRIDGE
    };

    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        gb_write(&gb, wGameplayType, types[i]);
        test_dispatch_called = -1;
        jumpToGameplayHandler(&gb, &cbs);
        if (types[i] >= GAMEPLAY_PHOTO_DIZZY_LINK) {
            TEST_ASSERT(test_dispatch_called == GAMEPLAY_PHOTO_DIZZY_LINK, "Photo picture dispatch failed");
        } else {
            TEST_ASSERT(test_dispatch_called == types[i], "jumpToGameplayHandler type dispatch failed");
        }
    }

    /* Test ExecuteGameplayHandler triggering save screen */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_WORLD_INTERACTIVE);
    gb_write(&gb, hPressedButtonsMask, J_A | J_B | J_START | J_SELECT);

    test_dispatch_called = -1;
    ExecuteGameplayHandler(&gb, &cbs);
    TEST_ASSERT(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SAVE, "Save screen not presented by ExecuteGameplayHandler");
    TEST_ASSERT(test_dispatch_called == GAMEPLAY_FILE_SAVE, "Dispatch did not route to FileSaveHandler");
}

static int mock_animate_calls = 0;
static void mock_animate_entities(GBState *gb) {
    (void)gb;
    mock_animate_calls++;
}

static void test_animate_entities_and_restore_bank(void) {
    GBState gb;
    gb_init(&gb);

    /* AnimateEntitiesAndRestoreBank17: sets bank $03, calls animate, switches to bank $17 */
    mock_animate_calls = 0;
    AnimateEntitiesAndRestoreBank17(&gb, mock_animate_entities);
    TEST_ASSERT(mock_animate_calls == 1, "AnimateEntities not called for bank 17");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x17, "Current bank not 0x17");

    /* AnimateEntitiesAndRestoreBank01 */
    mock_animate_calls = 0;
    AnimateEntitiesAndRestoreBank01(&gb, mock_animate_entities);
    TEST_ASSERT(mock_animate_calls == 1, "AnimateEntities not called for bank 01");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x01, "Current bank not 0x01");

    /* AnimateEntitiesAndRestoreBank02 */
    mock_animate_calls = 0;
    AnimateEntitiesAndRestoreBank02(&gb, mock_animate_entities);
    TEST_ASSERT(mock_animate_calls == 1, "AnimateEntities not called for bank 02");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x02, "Current bank not 0x02");
}

static int mock_link_entry_calls = 0;
static void mock_link_entry(GBState *gb) {
    (void)gb;
    mock_link_entry_calls++;
}

static uint8_t mock_motion_allowed_yes(GBState *gb) { (void)gb; return 1; }
static uint8_t mock_motion_allowed_no(GBState *gb) { (void)gb; return 0; }

static void test_link_motion_handlers(void) {
    GBState gb;
    gb_init(&gb);

    /* LinkMotionTeleportUpHandler */
    mock_link_entry_calls = 0;
    LinkMotionTeleportUpHandler(&gb, mock_link_entry);
    TEST_ASSERT(mock_link_entry_calls == 1, "LinkMotionTeleportUpHandler entry not called");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x19, "Bank not 0x19");

    /* LinkMotionPassOutHandler */
    mock_link_entry_calls = 0;
    LinkMotionPassOutHandler(&gb, mock_link_entry);
    TEST_ASSERT(mock_link_entry_calls == 1, "LinkMotionPassOutHandler entry not called");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x01, "Bank not 0x01");

    /* LinkMotionDefaultHandler when disallowed */
    mock_link_entry_calls = 0;
    gb_write(&gb, wCurrentBank, 0x05);
    LinkMotionDefaultHandler(&gb, mock_motion_allowed_no, mock_link_entry);
    TEST_ASSERT(mock_link_entry_calls == 0, "LinkMotionDefaultHandler called when not allowed");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x05, "Bank changed when not allowed");

    /* LinkMotionDefaultHandler when allowed */
    mock_link_entry_calls = 0;
    LinkMotionDefaultHandler(&gb, mock_motion_allowed_yes, mock_link_entry);
    TEST_ASSERT(mock_link_entry_calls == 1, "LinkMotionDefaultHandler entry not called when allowed");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x02, "Bank not 0x02");
}

#define RUN_GAMEPLAY_TEST(fn, name) \
    do { \
        int _prev = failures; \
        printf("[RUN ] %s\n", name); \
        fn(); \
        if (failures == _prev) { \
            printf("[PASS] %s\n", name); \
        } else { \
            printf("[FAIL] %s\n", name); \
        } \
    } while (0)

void run_gameplay_tests(void) {
    printf("[TEST] Gameplay\n");
    RUN_GAMEPLAY_TEST(test_gameplay_dispatchers, "GameplayDispatchers");
    RUN_GAMEPLAY_TEST(test_animate_entities_and_restore_bank, "AnimateEntitiesAndRestoreBank");
    RUN_GAMEPLAY_TEST(test_link_motion_handlers, "LinkMotionHandlers");
    RUN_GAMEPLAY_TEST(test_check_present_save_screen, "CheckPresentSaveScreen");
    RUN_GAMEPLAY_TEST(test_return_from_gameplay_handler, "ReturnFromGameplayHandler");
    RUN_GAMEPLAY_TEST(test_enable_sram, "EnableSRAM");
    RUN_GAMEPLAY_TEST(test_synchronize_dungeons_trampoline, "SynchronizeDungeonsTrampoline");
    RUN_GAMEPLAY_TEST(test_get_random_byte, "GetRandomByte");
    RUN_GAMEPLAY_TEST(test_read_joypad_state, "ReadJoypadState");
    RUN_GAMEPLAY_TEST(test_get_intersected_object_bg_address, "GetIntersectedObjectBGAddress");
    RUN_GAMEPLAY_TEST(test_table_jump, "TableJump");

    if (failures == 0) {
        printf("[PASS] Gameplay\n\n");
    } else {
        printf("[FAIL] Gameplay (%d failures)\n\n", failures);
    }
}
