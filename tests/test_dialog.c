#include "constants/memory.h"
#include "constants/gameplay.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "gb.h"
#include "home/dialog.h"
#include "constants/hardware.h"
#include "constants/dialog.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static void test_dialog_lookups(void) {
    GBState gb;
    gb_init(&gb);

    /* Allocate test ROM buffer: 32 banks (0x80000 bytes) */
    uint8_t rom[0x80000];
    memset(rom, 0, sizeof(rom));

    /* Bank $1C is at file offset 0x1C * 0x4000 = 0x70000 */
    /* Map address $4641 is offset 0x70000 + (0x4641 - 0x4000) = 0x70641 */
    /* Map address $4741 is offset 0x70000 + (0x4741 - 0x4000) = 0x70741 */
    uint32_t ascii_base = 0x70000 + (ADDR_CodepointToTileMap - 0x4000);
    uint32_t diacr_base = 0x70000 + (ADDR_CodepointToDiacritic - 0x4000);

    rom[ascii_base + 0x20] = 0x5A; /* Codepoint 0x20 -> tile 0x5A */
    rom[ascii_base + 0x41] = 0x7B; /* Codepoint 0x41 -> tile 0x7B */

    rom[diacr_base + 0x05] = 0x12; /* Codepoint 0x05 -> diacritic 0x12 */
    rom[diacr_base + 0x10] = 0x34; /* Codepoint 0x10 -> diacritic 0x34 */

    gb_attach_rom(&gb, rom, sizeof(rom));

    /* Initial ROM bank is 5 */
    gb.rom_bank = 5;

    /* Test ReadValueInDialogsBank directly */
    uint8_t val1 = ReadValueInDialogsBank(&gb, ADDR_CodepointToTileMap, 0x20);
    TEST_ASSERT(val1 == 0x5A, "ReadValueInDialogsBank returned incorrect value");
    TEST_ASSERT(gb.rom_bank == 1, "ReadValueInDialogsBank did not restore bank 1");

    /* Test ReadTileValueFromAsciiTable */
    gb.rom_bank = 7;
    uint8_t val2 = ReadTileValueFromAsciiTable(&gb, 0x41);
    TEST_ASSERT(val2 == 0x7B, "ReadTileValueFromAsciiTable returned incorrect tile");
    TEST_ASSERT(gb.rom_bank == 1, "ReadTileValueFromAsciiTable did not restore bank 1");

    /* Test ReadTileValueFromDiacriticsTable */
    gb.rom_bank = 9;
    uint8_t val3 = ReadTileValueFromDiacriticsTable(&gb, 0x05);
    TEST_ASSERT(val3 == 0x12, "ReadTileValueFromDiacriticsTable returned incorrect diacritic");
    TEST_ASSERT(gb.rom_bank == 1, "ReadTileValueFromDiacriticsTable did not restore bank 1");

    uint8_t val4 = ReadTileValueFromDiacriticsTable(&gb, 0x10);
    TEST_ASSERT(val4 == 0x34, "ReadTileValueFromDiacriticsTable 2nd check returned incorrect diacritic");
    TEST_ASSERT(gb.rom_bank == 1, "ReadTileValueFromDiacriticsTable did not restore bank 1");
}


static int test_dlg_cb_14_called = 0;
static void mock_dlg_cb_14(GBState *gb) {
    test_dlg_cb_14_called++;
    TEST_ASSERT(gb->rom_bank == 0x14, "ROM bank not 0x14");
}

static int test_dlg_cb_1c_called = 0;
static void mock_dlg_cb_1c(GBState *gb) {
    test_dlg_cb_1c_called++;
    TEST_ASSERT(gb->rom_bank == 0x1C, "ROM bank not 0x1C");
}

static int test_dlg_cb_21_called = 0;
static void mock_dlg_cb_21(GBState *gb) {
    test_dlg_cb_21_called++;
    TEST_ASSERT(gb->rom_bank == 0x21, "ROM bank not 0x21");
}

static int test_clear_pixels_called = 0;
static void mock_clear_pixels(GBState *gb) {
    test_clear_pixels_called++;
    TEST_ASSERT(gb->rom_bank == 0x1C, "ROM bank not 0x1C");
}

static void test_dialog_state_machine(void) {
    GBState gb;

    /* 1. OpenDialogInTable0 */
    gb_init(&gb);
    gb_write(&gb, hLinkPositionY, 0x30); /* < 0x48 -> bottom flag set (0x81) */
    OpenDialogInTable0(&gb, 0x42);
    TEST_ASSERT(gb_read(&gb, wDialogIndex) == 0x42, "wDialogIndex mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogIndexHi) == 0x00, "wDialogIndexHi mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogAskSelectionIndex) == 0x00, "wDialogAskSelectionIndex mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogOpenCloseAnimationFrame) == 0x00, "wDialogOpenCloseAnimationFrame mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogCharacterIndex) == 0x00, "wDialogCharacterIndex mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogCharacterIndexHi) == 0x00, "wDialogCharacterIndexHi mismatch");
    TEST_ASSERT(gb_read(&gb, wNameIndex) == 0x00, "wNameIndex mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogSFX) == 0x0F, "wDialogSFX mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x81, "wDialogState bottom flag mismatch");

    /* Link Y >= 0x48 -> top flag (0x01) */
    gb_write(&gb, hLinkPositionY, 0x50);
    OpenDialogInTable0(&gb, 0x10);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x01, "wDialogState top flag mismatch");

    /* 2. OpenDialogInTable1 and OpenDialogInTable2 */
    OpenDialogInTable1(&gb, 0x25);
    TEST_ASSERT(gb_read(&gb, wDialogIndex) == 0x25, "table 1 dialog index");
    TEST_ASSERT(gb_read(&gb, wDialogIndexHi) == 0x01, "table 1 dialog index hi");

    OpenDialogInTable2(&gb, 0x33);
    TEST_ASSERT(gb_read(&gb, wDialogIndex) == 0x33, "table 2 dialog index");
    TEST_ASSERT(gb_read(&gb, wDialogIndexHi) == 0x02, "table 2 dialog index hi");

    /* 3. DialogOpenAnimationStartHandler */
    gb_init(&gb);
    test_dlg_cb_14_called = 0;
    DialogOpenAnimationStartHandler(&gb, mock_dlg_cb_14);
    TEST_ASSERT(test_dlg_cb_14_called == 1, "DialogOpenAnimationStartHandler callback not called");

    /* 4. DialogOpenAnimationHandler (NOP) */
    DialogOpenAnimationHandler(&gb);

    /* 5. DialogClosingEndHandler */
    gb_init(&gb);
    gb_write(&gb, wDialogState, 0x8E);
    gb_write(&gb, hIsGBC, 0);
    DialogClosingEndHandler(&gb, mock_dlg_cb_21);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0, "wDialogState not cleared");
    TEST_ASSERT(gb_read(&gb, wDialogCooldown) == DIALOG_COOLDOWN, "cooldown not set");

    /* CGB with GAMEPLAY_WORLD and palette effect address >= 8 */
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wBGPaletteEffectAddress, 0x08);
    test_dlg_cb_21_called = 0;
    DialogClosingEndHandler(&gb, mock_dlg_cb_21);
    TEST_ASSERT(test_dlg_cb_21_called == 1, "CGB DialogClosingEndHandler callback not called");

    /* 6. DialogOpenAnimationEndHandler */
    test_dlg_cb_1c_called = 0;
    DialogOpenAnimationEndHandler(&gb, mock_dlg_cb_1c);
    TEST_ASSERT(test_dlg_cb_1c_called == 1, "DialogOpenAnimationEndHandler callback not called");

    /* 7. IncrementDialogState and IncrementDialogStateAndReturn */
    gb_write(&gb, wDialogState, 0x05);
    IncrementDialogState(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x06, "IncrementDialogState failed");
    IncrementDialogStateAndReturn(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x07, "IncrementDialogStateAndReturn failed");

    /* 8. UpdateDialogState */
    gb_write(&gb, wDialogOpenCloseAnimationFrame, 4);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wDialogState, 0x87); /* bottom flag + state 7 */
    UpdateDialogState(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogOpenCloseAnimationFrame) == 0, "animation frame not cleared");
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x8E, "UpdateDialogState failed to set closing state");

    /* Photo album mode */
    gb_write(&gb, wGameplayType, GAMEPLAY_PHOTO_ALBUM);
    gb_write(&gb, wDialogState, 0x87);
    UpdateDialogState(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0, "photo album dialog state not 0");

    /* 9. DialogFinishedHandler */
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wDialogState, 0x8C); /* state END */
    gb_write(&gb, wDialogInteractionLocked, 1);
    gb_write(&gb, hJoypadState, J_A);
    DialogFinishedHandler(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x8C, "locked dialog responded to joypad");

    gb_write(&gb, wDialogInteractionLocked, 0);
    gb_write(&gb, hJoypadState, 0);
    DialogFinishedHandler(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x8C, "no buttons pressed responded to joypad");

    gb_write(&gb, hJoypadState, J_B);
    DialogFinishedHandler(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x8E, "pressed B did not advance to closing");

    /* 10. DialogClosingBeginHandler */
    test_dlg_cb_1c_called = 0;
    DialogClosingBeginHandler(&gb, mock_dlg_cb_1c);
    TEST_ASSERT(test_dlg_cb_1c_called == 1, "DialogClosingBeginHandler callback not called");

    /* 11. DialogLetterAnimationStartHandler */
    gb_write(&gb, wDialogState, 0x06);
    gb_write(&gb, wDialogScrollDelay, 2);
    test_clear_pixels_called = 0;
    DialogLetterAnimationStartHandler(&gb, mock_clear_pixels);
    TEST_ASSERT(gb_read(&gb, wDialogScrollDelay) == 1, "delay not decremented");
    TEST_ASSERT(test_clear_pixels_called == 0, "pixels cleared before delay over");
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x06, "state incremented prematurely");

    gb_write(&gb, wDialogScrollDelay, 0);
    DialogLetterAnimationStartHandler(&gb, mock_clear_pixels);
    TEST_ASSERT(test_clear_pixels_called == 1, "clear pixels not called");
    TEST_ASSERT(gb_read(&gb, wDialogState) == 0x07, "state not incremented after clear pixels");
}


static int test_exec_handler_called = 0;
static void mock_exec_handler(GBState *gb) {
    (void)gb;
    test_exec_handler_called++;
}

static int test_choice_marker_called = 0;
static void mock_choice_marker(GBState *gb) {
    test_choice_marker_called++;
    TEST_ASSERT(gb->rom_bank == 0x17, "ROM bank not 0x17 in choice marker");
}

static int test_dialog_arrow_called = 0;
static void mock_dialog_arrow(GBState *gb) {
    test_dialog_arrow_called++;
    TEST_ASSERT(gb->rom_bank == 0x17, "ROM bank not 0x17 in dialog arrow");
}

static void test_execute_and_choice_routines(void) {
    GBState gb;

    /* 1. ExecuteDialog */
    gb_init(&gb);
    gb_write(&gb, wDialogState, 0);
    void (*handlers[15])(GBState *) = {0};
    handlers[5] = mock_exec_handler;
    test_exec_handler_called = 0;
    ExecuteDialog(&gb, handlers);
    TEST_ASSERT(test_exec_handler_called == 0, "ExecuteDialog ran while state 0");

    /* State 6 (DIALOG_LETTER_IN_1) -> index 5 */
    gb_write(&gb, wDialogState, DIALOG_LETTER_IN_1);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wDialogCharacterIndexHi, 0);
    gb_write(&gb, wDialogCharacterIndex, 0x15);
    ExecuteDialog(&gb, handlers);
    TEST_ASSERT(test_exec_handler_called == 1, "Handler 5 not called");
    TEST_ASSERT(gb_read(&gb, hDialogBackgroundTile) == DIALOG_BG_TILE_DARK, "BG tile not dark");
    TEST_ASSERT(gb_read(&gb, wDialogNextCharPosition) == 0x15, "Position not 0x15");

    /* Credits mode and character index > 0x20 wrapping */
    gb_write(&gb, wGameplayType, GAMEPLAY_CREDITS);
    gb_write(&gb, wDialogCharacterIndex, 0x25); /* (0x25 & 0x0F) | 0x10 = 0x15 */
    test_exec_handler_called = 0;
    ExecuteDialog(&gb, handlers);
    TEST_ASSERT(test_exec_handler_called == 1, "Handler 5 not called in credits");
    TEST_ASSERT(gb_read(&gb, hDialogBackgroundTile) == DIALOG_BG_TILE_LIGHT, "BG tile not light in credits");
    TEST_ASSERT(gb_read(&gb, wDialogNextCharPosition) == 0x15, "Wrapped position mismatch");

    /* 2. func_23E4 (DMG) */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wDialogState, DIALOG_OPENING_2); /* state 2, top box -> de = 2 */
    gb_write(&gb, wBGOriginHigh, 0x00);
    gb_write(&gb, wBGOriginLow, 0x00);
    /* For de = 2: bc_offset = 0 (dest wD500), bg_high_offset = 0x98, bg_low_offset = 0x21 (src 0x9821) */
    gb_write(&gb, 0x9821, 0x5A);
    func_23E4(&gb);
    TEST_ASSERT(gb_read(&gb, wD500) == 0x5A, "func_23E4 did not copy tile to wD500");

    /* 3. func_23E4 (GBC) */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wDialogState, DIALOG_OPENING_2 | DIALOG_BOX_BOTTOM_FLAG); /* bottom box -> de = 5 */
    gb_write(&gb, wBGOriginHigh, 0x00);
    gb_write(&gb, wBGOriginLow, 0x00);
    /* For de = 5: bc_offset = 0 (dest wD500), bg_high_offset = 0x99, bg_low_offset = 0x41 (src 0x9941) */
    /* VRAM bank 0 tile */
    gb_write(&gb, 0x9941, 0x33);
    /* VRAM bank 1 attribute */
    gb_write(&gb, rVBK, 1);
    gb_write(&gb, 0x9941, 0x04);
    gb_write(&gb, rVBK, 0);

    func_23E4(&gb);
    /* WRAM bank 1 has tile */
    gb_write(&gb, rSVBK, 1);
    TEST_ASSERT(gb_read(&gb, wD500) == 0x33, "GBC func_23E4 tile mismatch");
    /* WRAM bank 2 has attribute */
    gb_write(&gb, rSVBK, 2);
    TEST_ASSERT(gb_read(&gb, wD500) == 0x04, "GBC func_23E4 attr mismatch");
    gb_write(&gb, rSVBK, 1);

    /* 4. DialogScrollingStartHandler & DialogScrollingEndHandler */
    DialogScrollingStartHandler(&gb);
    DialogScrollingEndHandler(&gb);

    /* 5. SkipDialog */
    gb_init(&gb);
    gb_write(&gb, wDialogState, 0x07);
    SkipDialog(&gb);
    TEST_ASSERT(gb_read(&gb, wDialogAskSelectionIndex) == 0x02, "wDialogAskSelectionIndex not 2");
    TEST_ASSERT((gb_read(&gb, wDialogState) & 0x0F) == DIALOG_CLOSING_1, "state not closing");

    /* 6. DialogChoiceHandler */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_CHOICE);
    gb_write(&gb, wDialogAskSelectionIndex, 0);
    gb_write(&gb, hJoypadState, J_RIGHT);
    gb_write(&gb, hFrameCounter, 0x10); /* bit 4 set -> call marker */
    test_choice_marker_called = 0;
    DialogChoiceHandler(&gb, mock_choice_marker);
    TEST_ASSERT(gb_read(&gb, wDialogAskSelectionIndex) == 1, "selection not toggled to 1");
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_MOVE_SELECTION, "selection jingle not played");
    TEST_ASSERT(test_choice_marker_called == 1, "choice marker not called");

    /* Press A -> advances to closing */
    gb_write(&gb, hJoypadState, J_A);
    DialogChoiceHandler(&gb, mock_choice_marker);
    TEST_ASSERT((gb_read(&gb, wDialogState) & 0x0F) == DIALOG_CLOSING_1, "A button did not advance choice");

    /* 7. DrawDialogArrowTrampoline */
    test_dialog_arrow_called = 0;
    DrawDialogArrowTrampoline(&gb, mock_dialog_arrow);
    TEST_ASSERT(test_dialog_arrow_called == 1, "DrawDialogArrowTrampoline failed");
}


static int test_break_arrow_called = 0;
static void mock_break_arrow(GBState *gb) {
    (void)gb;
    test_break_arrow_called++;
}

static void test_dialog_scrolling_and_character_rendering(void) {
    GBState gb;

    /* 1. DialogBeginScrolling */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_SCROLLING_1); /* 0x0A, top box */
    gb_write(&gb, hDialogBackgroundTile, 0x7E);
    gb_write(&gb, wBGOriginHigh, 0x00);
    gb_write(&gb, wBGOriginLow, 0x00);
    /* Set up row 1 (0x9862) and row 2 (0x9882) */
    gb_write(&gb, 0x9862, 0x11);
    gb_write(&gb, 0x9882, 0x22);
    DialogBeginScrolling(&gb);
    /* Row 0 (0x9842) should now have row 1 data (0x11) */
    TEST_ASSERT(gb_read(&gb, 0x9842) == 0x11, "BeginScrolling row 0 mismatch");
    /* Row 1 (0x9862) should now have row 2 data (0x22) */
    TEST_ASSERT(gb_read(&gb, 0x9862) == 0x22, "BeginScrolling row 1 mismatch");
    /* Row 2 (0x9882) should now have BG tile (0x7E) */
    TEST_ASSERT(gb_read(&gb, 0x9882) == 0x7E, "BeginScrolling row 2 BG tile mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogScrollDelay) == 8, "BeginScrolling delay not 8");
    TEST_ASSERT(gb_read(&gb, wDialogState) == DIALOG_SCROLLING_2, "BeginScrolling state not 0x0B");

    /* 2. DialogFinishScrolling */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_SCROLLING_2);
    gb_write(&gb, hDialogBackgroundTile, 0x7E);
    gb_write(&gb, wBGOriginHigh, 0x00);
    gb_write(&gb, wBGOriginLow, 0x00);
    gb_write(&gb, 0x9842, 0x33);
    gb_write(&gb, 0x9862, 0x44);
    DialogFinishScrolling(&gb);
    TEST_ASSERT(gb_read(&gb, 0x9822) == 0x33, "FinishScrolling row -1 mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9842) == 0x44, "FinishScrolling row 0 mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9862) == 0x7E, "FinishScrolling row 1 BG tile mismatch");
    TEST_ASSERT(gb_read(&gb, wDialogState) == DIALOG_LETTER_IN_1, "FinishScrolling state not LETTER_IN_1");
    TEST_ASSERT(gb_read(&gb, wDialogScrollDelay) == 0, "FinishScrolling delay not 0");

    /* 3. DialogBreakHandler */
    /* Case 3a: Non-zero char index -> builds fill draw command directly */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_BREAK); /* 0x09 */
    gb_write(&gb, wDialogCharacterIndex, 0x07);
    gb_write(&gb, hDialogBackgroundTile, 0x7F);
    gb_write(&gb, wBGOriginHigh, 0x00);
    gb_write(&gb, wBGOriginLow, 0x00);
    DialogBreakHandler(&gb, mock_break_arrow);
    TEST_ASSERT(gb_read(&gb, wDrawCommand) == 0x98, "DrawCommand dest hi not 0x98");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 1) == 0x22, "DrawCommand dest lo not 0x22");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 2) == (DC_FILL_ROW | 0x0F), "DrawCommand len mismatch");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 3) == 0x7F, "DrawCommand tile mismatch");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 4) == 0x00, "DrawCommand terminator not 0");
    TEST_ASSERT(gb_read(&gb, wDialogState) == (DIALOG_BREAK + 1), "BreakHandler state not incremented");

    /* Case 3b: Zero char index with @ terminator */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_BREAK);
    gb_write(&gb, wDialogCharacterIndex, 0x20); /* 0x20 & 0x1F == 0 */
    gb_write(&gb, wDialogNextChar, 0xFF); /* @ */
    DialogBreakHandler(&gb, mock_break_arrow);
    TEST_ASSERT(gb_read(&gb, wDrawCommand) == 0, "DrawCommand not cleared on @");
    TEST_ASSERT((gb_read(&gb, wDialogState) & 0x0F) == DIALOG_END, "State not DIALOG_END on @");

    /* Case 3c: Zero char index with <ask> choice */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_BREAK);
    gb_write(&gb, wDialogCharacterIndex, 0x20);
    gb_write(&gb, wDialogNextChar, 0xFE); /* <ask> */
    DialogBreakHandler(&gb, mock_break_arrow);
    TEST_ASSERT(gb_read(&gb, wDrawCommand) == 0, "DrawCommand not cleared on <ask>");
    TEST_ASSERT((gb_read(&gb, wDialogState) & 0x0F) == DIALOG_CHOICE, "State not DIALOG_CHOICE on <ask>");
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_DIALOG_BREAK, "Jingle not played on <ask>");

    /* Case 3d: Waiting for button press and A button */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_BREAK);
    gb_write(&gb, wDialogCharacterIndex, 0x20);
    gb_write(&gb, wDialogNextChar, 'X');
    gb_write(&gb, wDialogIsWaitingForButtonPress, 0);
    gb_write(&gb, hJoypadState, J_A);
    test_break_arrow_called = 0;
    DialogBreakHandler(&gb, mock_break_arrow);
    TEST_ASSERT(test_break_arrow_called == 1, "Break arrow trampoline not called");
    TEST_ASSERT(gb_read(&gb, wDialogIsWaitingForButtonPress) == 1, "wDialogIsWaitingForButtonPress not 1");
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_DIALOG_BREAK, "Break jingle not played");
    TEST_ASSERT(gb_read(&gb, wDrawCommand) == 0x98, "DrawCommand not built on A press");
    TEST_ASSERT(gb_read(&gb, wDialogState) == (DIALOG_BREAK + 1), "State not incremented on A press");

    /* Case 3e: B button skip in GAMEPLAY_WORLD_MAP */
    gb_init(&gb);
    gb_write(&gb, wDialogState, DIALOG_BREAK);
    gb_write(&gb, wDialogCharacterIndex, 0x20);
    gb_write(&gb, wDialogNextChar, 'X');
    gb_write(&gb, wDialogIsWaitingForButtonPress, 1);
    gb_write(&gb, hJoypadState, J_B);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD_MAP);
    DialogBreakHandler(&gb, mock_break_arrow);
    TEST_ASSERT(gb_read(&gb, wDialogAskSelectionIndex) == 0x02, "SkipDialog not called on B in world map");
    TEST_ASSERT((gb_read(&gb, wDialogState) & 0x0F) == DIALOG_CLOSING_1, "State not closing on B in world map");

    /* 4. DialogLetterAnimationEndHandler & DialogDrawNextCharacterHandler */
    gb_init(&gb);
    uint8_t mock_rom[0x80000]; /* 512KB ROM */
    memset(mock_rom, 0, sizeof(mock_rom));

    /* Bank $1C DialogPointerTable at 0x4001: pointer to 0x4500 */
    mock_rom[0x1C * 0x4000 + 1] = 0x00;
    mock_rom[0x1C * 0x4000 + 2] = 0x45;
    /* Bank $1C DialogBankTable at 0x4741: bank $1D */
    mock_rom[0x1C * 0x4000 + (0x4741 - 0x4000)] = 0x1D;
    /* Bank $1C CodepointToTileMap at 0x4641: map 'A' (0x41) to tile 2 */
    mock_rom[0x1C * 0x4000 + (0x4641 - 0x4000) + 'A'] = 0x02;
    /* Bank 1 FontTiles at 0x5000 + (2 * 16) = 0x5020: 16 bytes of font data */
    for (int i = 0; i < 16; i++) {
        mock_rom[0x01 * 0x4000 + (0x5020 - 0x4000) + i] = (uint8_t)(0xA0 + i);
    }
    /* Bank $1D dialog text at 0x4500: "A", "B", "@" */
    mock_rom[0x1D * 0x4000 + (0x4500 - 0x4000) + 0] = 'A';
    mock_rom[0x1D * 0x4000 + (0x4500 - 0x4000) + 1] = 'B';
    mock_rom[0x1D * 0x4000 + (0x4500 - 0x4000) + 2] = 0xFF;

    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb.rom_bank = 1;
    gb_write(&gb, wCurrentBank, 1);
    gb_write(&gb, wDialogIndex, 0);
    gb_write(&gb, wDialogIndexHi, 0);
    gb_write(&gb, wDialogCharacterIndex, 0);
    gb_write(&gb, wDialogCharacterIndexHi, 0);
    gb_write(&gb, wDialogNextCharPosition, 0);
    gb_write(&gb, wDialogState, DIALOG_LETTER_IN_2);
    gb_write(&gb, wDialogSFX, WAVE_SFX_TEXT_PRINT);
    gb_write(&gb, wBGOriginHigh, 0x00);
    gb_write(&gb, wBGOriginLow, 0x00);

    DialogLetterAnimationEndHandler(&gb);

    /* Verify character index advanced to 1 */
    TEST_ASSERT(gb_read(&gb, wDialogCharacterIndex) == 1, "Character index not advanced to 1");
    /* Verify wDialogNextChar is 'B' */
    TEST_ASSERT(gb_read(&gb, wDialogNextChar) == 'B', "Next char not 'B'");
    /* Verify wDrawCommand has character tile placement and font data */
    TEST_ASSERT(gb_read(&gb, wDrawCommand) == 0x98, "Char tile dest hi not 0x98");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 1) == 0x42, "Char tile dest lo not 0x42");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 2) == 0, "Char tile length not 0");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 3) == 0xD0, "Char tile index not 0xD0");
    /* wDrawCommand + 4..6: OAM Y=0x8D, X=0x00, len=0x0F */
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 4) == 0x8D, "Font tile OAM Y not 0x8D");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 5) == 0x00, "Font tile OAM X not 0x00");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 6) == 0x0F, "Font tile len not 0x0F");
    /* Font data at wDrawCommand + 7..22 */
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 7) == 0xA0, "Font data byte 0 mismatch");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 22) == 0xAF, "Font data byte 15 mismatch");
    TEST_ASSERT(gb_read(&gb, wDrawCommand + 23) == 0x00, "Draw command terminator not 0");
    /* Dialog state should now be LETTER_IN_1 */
    TEST_ASSERT(gb_read(&gb, wDialogState) == DIALOG_LETTER_IN_1, "State not reset to LETTER_IN_1");
    /* SFX triggered for text print */
    TEST_ASSERT(gb_read(&gb, hWaveSfx) == WAVE_SFX_TEXT_PRINT, "Text print SFX not triggered");
}

#define RUN_DIALOG_TEST(fn, name) \
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

void run_dialog_tests(void) {
    printf("[TEST] Dialog\n");
    RUN_DIALOG_TEST(test_dialog_lookups, "DialogLookups");
    RUN_DIALOG_TEST(test_dialog_state_machine, "DialogStateMachine");
    RUN_DIALOG_TEST(test_execute_and_choice_routines, "DialogExecuteAndChoice");
    RUN_DIALOG_TEST(test_dialog_scrolling_and_character_rendering, "DialogScrollingAndRendering");

    if (failures == 0) {
        printf("[PASS] Dialog\n\n");
    } else {
        printf("[FAIL] Dialog (%d failures)\n\n", failures);
    }
}
