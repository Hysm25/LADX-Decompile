#include "test_bank1.h"

#include "gb.h"
#include "bank1/file_menu.h"
#include "constants/audio.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/joypad.h"
#include "constants/memory.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void test_func_001_4954(void) {
    GBState gb;

    /* Slot 0: Y = 0x3B. Frame counter bit 3 set: normal frame */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, hFrameCounter, 0x08);
    func_001_4954(&gb);
    assert(gb_read(&gb, wOAMBuffer + 0) == 0x3B);
    assert(gb_read(&gb, wOAMBuffer + 1) == 0x18);
    assert(gb_read(&gb, wOAMBuffer + 2) == 0x00);
    assert(gb_read(&gb, wOAMBuffer + 3) == 0x00);
    assert(gb_read(&gb, wOAMBuffer + 4) == 0x3B);
    assert(gb_read(&gb, wOAMBuffer + 5) == 0x20);
    assert(gb_read(&gb, wOAMBuffer + 6) == 0x02);
    assert(gb_read(&gb, wOAMBuffer + 7) == 0x00);

    /* Frame counter bit 3 clear: flipped frame (attr 0x20) */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, hFrameCounter, 0x00);
    func_001_4954(&gb);
    assert(gb_read(&gb, wOAMBuffer + 0) == 0x3B);
    assert(gb_read(&gb, wOAMBuffer + 1) == 0x18);
    assert(gb_read(&gb, wOAMBuffer + 2) == 0x02);
    assert(gb_read(&gb, wOAMBuffer + 3) == 0x20);
    assert(gb_read(&gb, wOAMBuffer + 4) == 0x3B);
    assert(gb_read(&gb, wOAMBuffer + 5) == 0x20);
    assert(gb_read(&gb, wOAMBuffer + 6) == 0x00);
    assert(gb_read(&gb, wOAMBuffer + 7) == 0x20);

    /* Slot 1: Y = 0x53 */
    gb_write(&gb, wSaveSlot, 1);
    func_001_4954(&gb);
    assert(gb_read(&gb, wOAMBuffer + 0) == 0x53);
    assert(gb_read(&gb, wOAMBuffer + 4) == 0x53);

    /* Slot 3: Y = 0x83 */
    gb_write(&gb, wSaveSlot, 3);
    func_001_4954(&gb);
    assert(gb_read(&gb, wOAMBuffer + 0) == 0x83);
    assert(gb_read(&gb, wOAMBuffer + 4) == 0x83);
}

void test_file_selection_interactive_and_choice(void) {
    GBState gb;

    /* Test 1: Navigation without saved files (wraps 0..2) */
    gb_init(&gb);
    gb_write(&gb, wSaveFilesCount, 0);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, hJoypadState, J_UP);
    FileSelectionInteractiveHandler(&gb);
    assert(gb_read(&gb, wSaveSlot) == 2);

    /* DOWN from 2 wraps to 0 */
    gb_write(&gb, hJoypadState, J_DOWN);
    FileSelectionInteractiveHandler(&gb);
    assert(gb_read(&gb, wSaveSlot) == 0);

    /* Test 2: Navigation with saved files (wraps 0..3) */
    gb_write(&gb, wSaveFilesCount, 1);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, hJoypadState, J_UP);
    FileSelectionInteractiveHandler(&gb);
    assert(gb_read(&gb, wSaveSlot) == 3);

    /* On slot 3, test left/right shifts arrow */
    gb_write(&gb, wIsFileSelectionArrowShifted, 0);
    gb_write(&gb, hJoypadState, J_RIGHT);
    gb_write(&gb, hFrameCounter, 0x00); /* Frame counter bit 4 clear: arrow visible */
    FileSelectionInteractiveHandler(&gb);
    assert(gb_read(&gb, wIsFileSelectionArrowShifted) == 1);
    assert(gb_read(&gb, wOAMBuffer + 8) == 0x88);
    assert(gb_read(&gb, wOAMBuffer + 9) == 0x64);
    assert(gb_read(&gb, wOAMBuffer + 10) == 0xBE);

    /* Test A button increments subtype */
    gb_write(&gb, wGameplaySubtype, 2);
    gb_write(&gb, hJoypadState, J_A);
    FileSelectionInteractiveHandler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    /* Test 3: HandleFileSelectionCommand */
    gb_init(&gb);
    gb_write(&gb, wIsFileSelectionArrowShifted, 0);
    HandleFileSelectionCommand(&gb);
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_DELETE);
    assert(gb_read(&gb, wGameplaySubtype) == 0);
    assert(gb_read(&gb, hJingle) == JINGLE_VALIDATE);

    gb_write(&gb, wIsFileSelectionArrowShifted, 1);
    HandleFileSelectionCommand(&gb);
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_COPY);

    /* Test 4: FileSelectionExecuteChoice on empty slot */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 1);
    /* Name is empty (all zeroes) */
    FileSelectionExecuteChoice(&gb);
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_NEW);
    assert(gb_read(&gb, wGameplaySubtype) == 0);

    /* Test 5: FileSelectionExecuteChoice on populated slot */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, wSaveSlotNames + 0, 'Z');
    gb_write(&gb, wGameplaySubtype, 2);
    FileSelectionExecuteChoice(&gb);
    assert(gb_read(&gb, wBGPalette) == 0);
    assert(gb_read(&gb, wOBJ0Palette) == 0);
    assert(gb_read(&gb, wOBJ1Palette) == 0);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_BASE_OVERWORLD);
    assert(gb_read(&gb, wGameplaySubtype) == 3); /* incremented */

    /* Test 6: FileSelectionLoadSavedFile */
    gb_init(&gb);
    gb_write(&gb, wSpawnPositionX, 0);
    gb_write(&gb, wHealth, 0);
    gb_write(&gb, wMaxHearts, 3);
    FileSelectionLoadSavedFile(&gb);
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_WORLD);
}

void test_file_creation_init_and_sram(void) {
    GBState gb;

    /* Test 1: FileCreationInit1Handler */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 0);
    FileCreationInit1Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_FILL_TILEMAP);
    assert(gb_read(&gb, wDBA8) == 0);
    assert(gb_read(&gb, wNameEntryCurrentChar) == 0);
    assert(gb_read(&gb, wSaveSlotNameCharIndex) == 0);

    /* Test 2: FileCreationInit2Handler for Slot 1 */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 1);
    gb_write(&gb, wGameplaySubtype, 1);
    FileCreationInit2Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_MENU_FILE_CREATION);
    assert(gb_read(&gb, wDrawCommand + 0) == (FILE_NEW_SAVE_SLOT_INDEX_BG >> 8));
    assert(gb_read(&gb, wDrawCommand + 1) == (FILE_NEW_SAVE_SLOT_INDEX_BG & 0xFF));
    assert(gb_read(&gb, wDrawCommand + 2) == 0);
    assert(gb_read(&gb, wDrawCommand + 3) == (1 + FILE_NEW_SAVE_SLOT_1_TILE));
    assert(gb_read(&gb, wDrawCommand + 4) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 2);

    /* Test 3: WriteByteToSRAM */
    gb_init(&gb);
    WriteByteToSRAM(&gb, 0xA100, 0x05, 0x42);
    assert(gb_read(&gb, 0xA105) == 0x42);
}

void test_transition_to_file_menu_reload(void) {
    GBState gb;
    gb_init(&gb);

    /* Populate SRAM for Slot 1 */
    gb_write(&gb, 0xA454 + 0, 'L');
    gb_write(&gb, 0xA454 + 1, 'I');
    gb_write(&gb, 0xA454 + 2, 'N');
    gb_write(&gb, 0xA454 + 3, 'K');
    gb_write(&gb, 0xA454 + 4, '1');
    gb_write(&gb, 0xA45F, 24);
    gb_write(&gb, 0xA460, 3);
    gb_write(&gb, 0xA45C, 0);
    gb_write(&gb, 0xA45D, 2);

    /* Populate SRAM for Slot 2 */
    gb_write(&gb, 0xA801 + 0, 'Z');
    gb_write(&gb, 0xA801 + 1, 'E');
    gb_write(&gb, 0xA801 + 2, 'L');
    gb_write(&gb, 0xA801 + 3, 'D');
    gb_write(&gb, 0xA801 + 4, 'A');
    gb_write(&gb, 0xA80C, 40);
    gb_write(&gb, 0xA80D, 5);
    gb_write(&gb, 0xA809, 0);
    gb_write(&gb, 0xA80A, 0);

    /* Populate SRAM for Slot 3 */
    gb_write(&gb, 0xABAE + 0, 'M');
    gb_write(&gb, 0xABAE + 1, 'A');
    gb_write(&gb, 0xABAE + 2, 'R');
    gb_write(&gb, 0xABAE + 3, 'I');
    gb_write(&gb, 0xABAE + 4, 'N');
    gb_write(&gb, 0xABB9, 80);
    gb_write(&gb, 0xABBA, 10);
    gb_write(&gb, 0xABB6, 0);
    gb_write(&gb, 0xABB7, 7);

    TransitionToFileMenu(&gb, 1);
    assert(gb_read(&gb, wForceFileSelectionScreenMusic) == 1);

    /* Slot 1 checks */
    assert(gb_read(&gb, wSaveSlot1Name + 0) == 'L');
    assert(gb_read(&gb, wSaveSlot1Name + 4) == '1');
    assert(gb_read(&gb, wFile1Health) == 24);
    assert(gb_read(&gb, wFile1MaxHearts) == 3);
    assert(gb_read(&gb, wFile1DeathCountHigh) == 0);
    assert(gb_read(&gb, wFile1DeathCountLow) == 2);

    /* Slot 2 checks */
    assert(gb_read(&gb, wSaveSlot2Name + 0) == 'Z');
    assert(gb_read(&gb, wSaveSlot2Name + 4) == 'A');
    assert(gb_read(&gb, wFile2Health) == 40);
    assert(gb_read(&gb, wFile2MaxHearts) == 5);
    assert(gb_read(&gb, wFile2DeathCountHigh) == 0);
    assert(gb_read(&gb, wFile2DeathCountLow) == 0);

    /* Slot 3 checks */
    assert(gb_read(&gb, wSaveSlot3Name + 0) == 'M');
    assert(gb_read(&gb, wSaveSlot3Name + 4) == 'N');
    assert(gb_read(&gb, wFile3Health) == 80);
    assert(gb_read(&gb, wFile3MaxHearts) == 10);
    assert(gb_read(&gb, wFile3DeathCountHigh) == 0);
    assert(gb_read(&gb, wFile3DeathCountLow) == 7);

    /* Mode and display checks */
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SELECT);
    assert(gb_read(&gb, wGameplaySubtype) == 0);
    assert(gb_read(&gb, hBaseScrollY) == 0);
    assert(gb_read(&gb, hBaseScrollX) == 0);
    assert(gb_read(&gb, wBGPalette) == 0);
}

void test_file_creation_grid_and_entry(void) {
    GBState gb;

    /* Test 1: DrawSaveSlotName */
    gb_init(&gb);
    gb_write(&gb, wDrawCommandsSize, 0);
    gb_write(&gb, wSaveSlot1Name + 0, 'A');
    gb_write(&gb, wSaveSlot1Name + 1, 0);
    gb_write(&gb, wSaveSlot1Name + 2, 0);
    gb_write(&gb, wSaveSlot1Name + 3, 0);
    gb_write(&gb, wSaveSlot1Name + 4, 0);
    DrawSaveSlotName(&gb, 0x984A, wSaveSlot1Name);
    assert(gb_read(&gb, wDrawCommandsSize) == 0x10);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 1) == 0x4A);
    assert(gb_read(&gb, wDrawCommand + 2) == 0x04);
    assert(gb_read(&gb, wDrawCommand + 8) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 9) == 0x2A);
    assert(gb_read(&gb, wDrawCommand + 10) == 0x04);
    assert(gb_read(&gb, wDrawCommand + 16) == 0x00);

    /* Test 2: func_001_4CDA */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, wSaveSlotNameCharIndex, 0);
    gb_write(&gb, wNameEntryCurrentChar, 0); /* 'A' (0x42) */
    func_001_4CDA(&gb);
    assert(gb_read(&gb, wSaveSlot1Name) == 0x42);

    /* Test 3: func_001_4C8A (A button and B button) */
    gb_write(&gb, hJoypadState, J_A);
    gb_write(&gb, hFrameCounter, 0x10); /* underline cursor visible */
    func_001_4C8A(&gb);
    assert(gb_read(&gb, wSaveSlotNameCharIndex) == 1);
    assert(gb_read(&gb, wOAMBuffer + 4) == 0x23);
    assert(gb_read(&gb, wOAMBuffer + 6) == 0xE0);

    gb_write(&gb, hJoypadState, J_B);
    func_001_4C8A(&gb);
    assert(gb_read(&gb, wSaveSlotNameCharIndex) == 0);

    /* Test 4: func_001_4BF5 (Grid navigation) */
    gb_init(&gb);
    gb_write(&gb, wNameEntryCurrentChar, 0);
    gb_write(&gb, hJoypadState, J_RIGHT);
    func_001_4BF5(&gb);
    assert(gb_read(&gb, wNameEntryCurrentChar) == 1);
    assert(gb_read(&gb, wOAMBuffer + 0) == (0x38 + 0x0B));
    assert(gb_read(&gb, wOAMBuffer + 1) == (0x1C + 0x04));
    assert(gb_read(&gb, wOAMBuffer + 2) == 0xE0);

    /* Left from 0 wraps to 63 */
    gb_write(&gb, wNameEntryCurrentChar, 0);
    gb_write(&gb, hJoypadState, J_LEFT);
    func_001_4BF5(&gb);
    assert(gb_read(&gb, wNameEntryCurrentChar) == 63);

    /* Up from 0 wraps to 48 */
    gb_write(&gb, wNameEntryCurrentChar, 0);
    gb_write(&gb, hJoypadState, J_UP);
    func_001_4BF5(&gb);
    assert(gb_read(&gb, wNameEntryCurrentChar) == 48);

    /* Test 5: FileCreationInteractiveHandler with "ZELDA" secret name */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    /* "ZELDA" in NameEntryCharmap: 0x5B, 0x46, 0x4D, 0x45, 0x42 */
    gb_write(&gb, wSaveSlotNames + 0, 0x5B);
    gb_write(&gb, wSaveSlotNames + 1, 0x46);
    gb_write(&gb, wSaveSlotNames + 2, 0x4D);
    gb_write(&gb, wSaveSlotNames + 3, 0x45);
    gb_write(&gb, wSaveSlotNames + 4, 0x42);
    gb_write(&gb, hJoypadState, J_START);
    FileCreationInteractiveHandler(&gb);

    /* Music track should trigger easter egg */
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_FILE_SELECT_ZELDA);
    /* SRAM SaveGame1.main name written */
    assert(gb_read(&gb, 0xA454 + 0) == 0x5B);
    assert(gb_read(&gb, 0xA454 + 4) == 0x42);
    /* SRAM SaveGame1.main health written (0x18) */
    assert(gb_read(&gb, 0xA45F) == 0x18);
    /* SRAM SaveGame1.main max hearts written (0x03) */
    assert(gb_read(&gb, 0xA460) == 0x03);
    /* SRAM SaveGame1.main death count written (0) */
    assert(gb_read(&gb, 0xA45C) == 0);
    assert(gb_read(&gb, 0xA45D) == 0);
    /* Returns to file select screen */
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SELECT);

    /* Test 6: FileCreationEntryPoint dispatcher */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 0);
    FileCreationEntryPoint(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    FileCreationEntryPoint(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 2);
}

void test_file_deletion_and_digits(void) {
    GBState gb;

    /* Test 1: CopyDigitsToFileScreenBG */
    gb_init(&gb);
    gb_write(&gb, wDrawCommandsSize, 0);
    /* high_b = 0x45 (tens=4, units=5), low_c = 0x03 (hundreds=3) */
    CopyDigitsToFileScreenBG(&gb, 0x98E7, 0x45, 0x03);
    assert(gb_read(&gb, wDrawCommandsSize) == 6);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xE7);
    assert(gb_read(&gb, wDrawCommand + 2) == 0x02);
    assert(gb_read(&gb, wDrawCommand + 3) == Data_001_4F3B[3]);
    assert(gb_read(&gb, wDrawCommand + 4) == Data_001_4F3B[4]);
    assert(gb_read(&gb, wDrawCommand + 5) == Data_001_4F3B[5]);
    assert(gb_read(&gb, wDrawCommand + 6) == 0x00);

    /* Test 2: CopyDeathCountsToBG */
    gb_init(&gb);
    gb_write(&gb, wSaveFilesCount, 0x05); /* files 1 and 3 active */
    gb_write(&gb, wFile1DeathCountHigh, 0x00);
    gb_write(&gb, wFile1DeathCountLow, 0x00);
    gb_write(&gb, wFile3DeathCountHigh, 0x12);
    gb_write(&gb, wFile3DeathCountLow, 0x00);
    CopyDeathCountsToBG(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    assert(gb_read(&gb, wDrawCommandsSize) == 12); /* 2 files * 6 bytes */

    /* Test 3: DrawSaveSlot names 1, 2, 3 */
    gb_init(&gb);
    DrawSaveSlot1Name(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xC5);
    DrawSaveSlot2Name(&gb);
    assert(gb_read(&gb, wDrawCommand + 16 + 0) == 0x99);
    assert(gb_read(&gb, wDrawCommand + 16 + 1) == 0x25);
    DrawSaveSlot3Name(&gb);
    assert(gb_read(&gb, wDrawCommand + 32 + 0) == 0x99);
    assert(gb_read(&gb, wDrawCommand + 32 + 1) == 0x85);

    /* Test 4: DrawSaveSlot max hearts */
    gb_init(&gb);
    gb_write(&gb, wSaveFilesCount, 0x07);
    gb_write(&gb, wFile1Health, 24);
    gb_write(&gb, wFile1MaxHearts, 3);
    DrawSaveSlot1MaxHearts(&gb);
    assert(gb_read(&gb, hMultiPurpose4) == 0);
    assert(gb_read(&gb, hMultiPurpose2) == 24);
    assert(gb_read(&gb, hMultiPurpose3) == 3);

    /* Test 5: FileDeletion state handlers */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    FileDeletionState0Handler(&gb);
    assert(gb_read(&gb, wPaletteDataFlags) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    FileDeletionState1Handler(&gb);
    assert(gb_read(&gb, wPaletteDataFlags) == 2);
    assert(gb_read(&gb, wGameplaySubtype) == 2);

    FileDeletionState2Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_FILL_TILEMAP);
    assert(gb_read(&gb, wSaveSlot) == 0);
    assert(gb_read(&gb, wCreditsScratch0) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    FileDeletionState3Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_MENU_FILE_ERASE);
    assert(gb_read(&gb, wGameplaySubtype) == 4);

    FileDeletionState4Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 5);

    FileDeletionState5Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 6);

    FileDeletionState6Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 7);

    FileDeletionState7Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 8);

    FileDeletionState8Handler(&gb);
    assert(gb_read(&gb, wPaletteDataFlags) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 9);

    FileDeletionState9Handler(&gb);
    assert(gb_read(&gb, wPaletteDataFlags) == 2);
    assert(gb_read(&gb, wGameplaySubtype) == 10);
}

void test_file_deletion_interactive_and_erase(void) {
    GBState gb;

    /* Test 1: CopyQuitOkTilemap and CopyReturnToMenuTilemap */
    gb_init(&gb);
    CopyQuitOkTilemap(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x99);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xE4);
    assert(gb_read(&gb, wDrawCommand + 2) == 0x0D);

    gb_write(&gb, wDrawCommandsSize, 0);
    CopyReturnToMenuTilemap(&gb);
    assert(gb_read(&gb, wDrawCommandsSize) == 17);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x99);
    assert(gb_read(&gb, wDrawCommand + 3) == 0x11);

    /* Test 2: BlankSaveSlotNameDrawCommand */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 1);
    BlankSaveSlotNameDrawCommand(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x99);
    assert(gb_read(&gb, wDrawCommand + 1) == 0x05);
    assert(gb_read(&gb, wDrawCommand + 8) == 0x00);

    /* Test 3: func_001_4F0C (QUIT/OK cursor arrow) */
    gb_init(&gb);
    gb_write(&gb, wCreditsScratch0, 0);
    gb_write(&gb, hJoypadState, J_RIGHT);
    gb_write(&gb, hFrameCounter, 0x00); /* visible */
    func_001_4F0C(&gb);
    assert(gb_read(&gb, wCreditsScratch0) == 1);
    assert(gb_read(&gb, wOAMBuffer + 12) == 0x88);
    assert(gb_read(&gb, wOAMBuffer + 13) == 0x6C); /* OK position */
    assert(gb_read(&gb, wOAMBuffer + 14) == 0xBE);

    /* Test 4: FileDeletionState10Handler navigation */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, hJoypadState, J_DOWN);
    FileDeletionState10Handler(&gb);
    assert(gb_read(&gb, wSaveSlot) == 1);

    /* Slot 3 with A returns to file menu */
    gb_write(&gb, wSaveSlot, 3);
    gb_write(&gb, hJoypadState, J_A);
    FileDeletionState10Handler(&gb);
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SELECT);

    /* Slot 1 with A advances to confirmation */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 1);
    gb_write(&gb, wGameplaySubtype, 10);
    gb_write(&gb, hJoypadState, J_A);
    FileDeletionState10Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 11);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x99); /* Quit/Ok tilemap copied */

    /* Test 5: FileDeletionState11Handler B cancels */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 11);
    gb_write(&gb, hJoypadState, J_B);
    FileDeletionState11Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 10);

    /* Test 6: FileDeletionState11Handler A with OK erases SRAM */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, wCreditsScratch0, 1); /* OK chosen */
    /* Fill SRAM slot 1 with non-zero dummy data */
    for (uint16_t i = 0; i < 0x03A8; i++) {
        gb_write(&gb, 0xA105 + i, 0xEE);
    }
    gb_write(&gb, hJoypadState, J_A);
    FileDeletionState11Handler(&gb);
    /* SRAM slot 1 must now be all 0x00 */
    assert(gb_read(&gb, 0xA105) == 0);
    assert(gb_read(&gb, 0xA105 + 0x03A7) == 0);
    /* Returned to file select screen */
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SELECT);

    /* Test 7: FileDeletionEntryPoint dispatcher */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    FileDeletionEntryPoint(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}

void test_file_copy_subsystem(void) {
    GBState gb;

    /* Test 1: FileCopyState2Handler through State5Handler */
    gb_init(&gb);
    FileCopyState2Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_FILL_TILEMAP);
    assert(gb_read(&gb, wIntroTimer) == 0);
    assert(gb_read(&gb, wIntroSubTimer) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    FileCopyState3Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_MENU_FILE_COPY);
    assert(gb_read(&gb, wGameplaySubtype) == 2);

    FileCopyState4Handler(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xC4);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    gb_write(&gb, wDrawCommandsSize, 0);
    FileCopyState5Handler(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xCD);
    assert(gb_read(&gb, wGameplaySubtype) == 4);

    /* Test 2: Arrow positioning */
    gb_init(&gb);
    gb_write(&gb, wIntroTimer, 1);
    func_001_5094(&gb);
    assert(gb_read(&gb, wOAMBuffer + 0) == (uint8_t)(Data_001_48E4[1] + 5));
    assert(gb_read(&gb, wOAMBuffer + 1) == 0x14);
    assert(gb_read(&gb, wOAMBuffer + 2) == 0xBE);

    gb_write(&gb, wIntroSubTimer, 2);
    func_001_51CE(&gb);
    assert(gb_read(&gb, wOAMBuffer + 8) == (uint8_t)(Data_001_48E4[2] + 5));
    assert(gb_read(&gb, wOAMBuffer + 9) == 0x5C);
    assert(gb_read(&gb, wOAMBuffer + 10) == 0xBE);

    /* Test 3: FileCopyState8Handler empty vs non-empty slot */
    gb_init(&gb);
    gb_write(&gb, wIntroTimer, 0);
    gb_write(&gb, wGameplaySubtype, 8);
    /* Slot 1 empty (all zeroes) */
    gb_write(&gb, hJoypadState, J_A);
    FileCopyState8Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 8); /* does not advance */

    /* Slot 1 with valid name */
    gb_write(&gb, wSaveSlot1Name, 'L');
    gb_write(&gb, hJoypadState, J_A);
    FileCopyState8Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 9); /* advances to destination select */

    /* Test 4: FileCopyState9Handler navigation and cancel */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 9);
    gb_write(&gb, wIntroSubTimer, 0);
    gb_write(&gb, hJoypadState, J_DOWN);
    FileCopyState9Handler(&gb);
    assert(gb_read(&gb, wIntroSubTimer) == 1);

    /* B button cancels back to 8 */
    gb_write(&gb, hJoypadState, J_B);
    FileCopyState9Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 8);

    /* A button advances to confirmation (10) */
    gb_write(&gb, wGameplaySubtype, 9);
    gb_write(&gb, wIntroSubTimer, 1);
    gb_write(&gb, hJoypadState, J_A);
    FileCopyState9Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 10);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x99); /* Quit/Ok tilemap copied */

    /* Test 5: FileCopyStateAHandler B cancels back to 9 */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 10);
    gb_write(&gb, hJoypadState, J_B);
    FileCopyStateAHandler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 9);

    /* Test 6: FileCopyStateAHandler executes copy in SRAM */
    gb_init(&gb);
    gb_write(&gb, wIntroTimer, 0);    /* src: slot 0 (0xA100) */
    gb_write(&gb, wIntroSubTimer, 1); /* dst: slot 1 (0xA4AD) */
    gb_write(&gb, wCreditsScratch0, 1); /* OK selected */
    /* Write test pattern in slot 0 */
    for (uint16_t i = 0; i < 0x03AD; i++) {
        gb_write(&gb, 0xA100 + i, (uint8_t)(i & 0xFF));
    }
    gb_write(&gb, hJoypadState, J_A);
    FileCopyStateAHandler(&gb);
    /* Destination slot 1 must now contain identical pattern */
    for (uint16_t i = 0; i < 0x03AD; i++) {
        assert(gb_read(&gb, 0xA4AD + i) == (uint8_t)(i & 0xFF));
    }
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SELECT);

    /* Test 7: FileCopyEntryPoint dispatcher */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    FileCopyEntryPoint(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}
