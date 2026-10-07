#include "test_bank1.h"

#include "gb.h"
#include "bank1/save.h"
#include "constants/directions.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/link.h"
#include "constants/maps.h"
#include "constants/memory.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void test_build_save_slot_hearts_draw_command(void) {
    GBState gb;
    gb_init(&gb);

    /* Test 1: Slot 0, 3 full hearts (health = 24), max hearts = 5 */
    gb_write(&gb, wDrawCommandsSize, 0);
    gb_write(&gb, hMultiPurpose4, 0); /* Slot 0 */
    gb_write(&gb, hMultiPurpose2, 24); /* Health = 3 full hearts */
    gb_write(&gb, hMultiPurpose3, 5);  /* Max health = 5 containers */

    BuildSaveSlotHeartsDrawCommand(&gb);
    assert(gb_read(&gb, wDrawCommandsSize) == 0x14);

    /* Row 1 header */
    assert(gb_read(&gb, wDrawCommand + 0) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xCB);
    assert(gb_read(&gb, wDrawCommand + 2) == 0x06);

    /* Verify 5 hearts drawn with tile 0xAE */
    for (int i = 0; i < 5; i++) {
        assert(gb_read(&gb, wDrawCommand + 3 + i) == 0xAE);
    }
    /* Remaining 2 tiles in row 1 are empty (0x7E) */
    assert(gb_read(&gb, wDrawCommand + 3 + 5) == 0x7E);
    assert(gb_read(&gb, wDrawCommand + 3 + 6) == 0x7E);

    /* Test 2: Slot 1, 10 hearts (spans row 1 and row 2) */
    gb_init(&gb);
    gb_write(&gb, wDrawCommandsSize, 0);
    gb_write(&gb, hMultiPurpose4, 1); /* Slot 1 */
    gb_write(&gb, hMultiPurpose2, 80); /* 10 hearts */
    gb_write(&gb, hMultiPurpose3, 10);

    BuildSaveSlotHeartsDrawCommand(&gb);
    /* Row 1 destination */
    assert(gb_read(&gb, wDrawCommand + 0) == 0x99);
    assert(gb_read(&gb, wDrawCommand + 1) == 0x2B);
    /* All 7 tiles in row 1 are hearts (0xAE) */
    for (int i = 0; i < 7; i++) {
        assert(gb_read(&gb, wDrawCommand + 3 + i) == 0xAE);
    }
    /* Row 2 destination header (offset 10..12) */
    assert(gb_read(&gb, wDrawCommand + 10) == 0x99);
    assert(gb_read(&gb, wDrawCommand + 11) == 0x4B);
    assert(gb_read(&gb, wDrawCommand + 12) == 0x06);
    /* 3 hearts in row 2 */
    for (int i = 0; i < 3; i++) {
        assert(gb_read(&gb, wDrawCommand + 13 + i) == 0xAE);
    }
    /* Remaining 4 tiles in row 2 are 0x7E */
    for (int i = 3; i < 7; i++) {
        assert(gb_read(&gb, wDrawCommand + 13 + i) == 0x7E);
    }
}

void test_func_5DC0_and_save_game_to_file(void) {
    GBState gb;
    gb_init(&gb);

    /* Test 1: func_5DC0 with no names */
    func_5DC0(&gb);
    assert(gb_read(&gb, wSaveFilesCount) == 0);

    /* Slot 0 populated: 'L', 'I', 'N', 'K', 0 */
    gb_write(&gb, wSaveSlotNames + 0, 'L');
    func_5DC0(&gb);
    assert(gb_read(&gb, wSaveFilesCount) == 1);

    /* Slot 2 populated: slot 2 starts at index 10 */
    gb_write(&gb, wSaveSlotNames + 10, 'Z');
    func_5DC0(&gb);
    assert(gb_read(&gb, wSaveFilesCount) == 5); /* 1 | 4 */

    /* Slot 1 populated: slot 1 starts at index 5 */
    gb_write(&gb, wSaveSlotNames + 5, 'M');
    func_5DC0(&gb);
    assert(gb_read(&gb, wSaveFilesCount) == 7); /* 1 | 2 | 4 */

    /* Test 2: SaveGameToFile */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    gb_write(&gb, wHealth, 0); /* 0 health should reset to starting health */
    gb_write(&gb, wMaxHearts, 6); /* MaxHeartsToStartingHealthTable[6] == 40 */
    gb_write(&gb, wOverworldRoomStatus + 0x10, 0x55);
    gb_write(&gb, wColorDungeonItemFlags + 1, 0xAA);
    gb_write(&gb, wColorDungeonRoomStatus + 2, 0x77);
    gb_write(&gb, wTunicType, 2);
    gb_write(&gb, wPhotos1, 0x11);
    gb_write(&gb, wPhotos2, 0x22);

    SaveGameToFile(&gb);
    assert(gb_read(&gb, wHealth) == 40);

    /* Verify SRAM destination 0xA105 (SaveGame1.main) */
    assert(gb_read(&gb, 0xA105 + 0x10) == 0x55);
    /* DX1 offset: 0xA105 + 0x380 = 0xA485 */
    assert(gb_read(&gb, 0xA485 + 1) == 0xAA);
    /* DX2 offset: 0xA485 + 5 = 0xA48A */
    assert(gb_read(&gb, 0xA48A + 2) == 0x77);
    /* DX3 offset: 0xA48A + 0x20 = 0xA4AA */
    assert(gb_read(&gb, 0xA4AA + 0) == 2);    /* wTunicType */
    assert(gb_read(&gb, 0xA4AA + 1) == 0x11); /* wPhotos1 */
    assert(gb_read(&gb, 0xA4AA + 2) == 0x22); /* wPhotos2 */

    /* Test 3: Save slot 1 (dest: 0xA4B2) */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 1);
    gb_write(&gb, wHealth, 20);
    gb_write(&gb, wOverworldRoomStatus + 0x05, 0x99);
    SaveGameToFile(&gb);
    assert(gb_read(&gb, 0xA4B2 + 0x05) == 0x99);
}

void test_load_saved_file(void) {
    GBState gb;

    /* Test 1: New game initialization when wSpawnPositionX is 0 */
    gb_init(&gb);
    /* In a new game file, SRAM has wSpawnPositionX = 0, health = 24 */
    gb_write(&gb, 0xA105 + (wHealth - wOverworldRoomStatus), 24);
    gb_write(&gb, 0xA105 + (wMaxHearts - wOverworldRoomStatus), 3);
    gb_write(&gb, 0xA105 + (wSpawnPositionX - wOverworldRoomStatus), 0);

    LoadSavedFile(&gb);
    assert(gb_read(&gb, wHealth) == 24); /* 3 hearts = 24 */
    assert(gb_read(&gb, wMaxArrows) == 0x30);
    assert(gb_read(&gb, wMaxBombs) == 0x30);
    assert(gb_read(&gb, wMaxMagicPowder) == 0x20);
    assert(gb_read(&gb, wMapEntranceRoom) == ROOM_INDOOR_B_MARIN_HOUSE);
    assert(gb_read(&gb, hMapRoom) == ROOM_INDOOR_B_MARIN_HOUSE);
    assert(gb_read(&gb, wDB54) == ROOM_INDOOR_B_MARIN_HOUSE);
    assert(gb_read(&gb, wIsIndoor) == 1);
    assert(gb_read(&gb, hMapId) == MAP_HOUSE);
    assert(gb_read(&gb, wMapEntrancePositionX) == 0x50);
    assert(gb_read(&gb, wMapEntrancePositionY) == 0x60);
    assert(gb_read(&gb, hLinkDirection) == DIRECTION_DOWN);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_INVENTORY);
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_WORLD);

    /* Test 2: Loading existing save from SRAM slot 0 with indoor spawn */
    gb_init(&gb);
    gb_write(&gb, wSaveSlot, 0);
    /* Populate SRAM for slot 0 (SaveGame1.main: 0xA105) */
    gb_write(&gb, 0xA105 + 0x20, 0x7E); /* room status */
    /* DX1: 0xA105 + 0x380 = 0xA485 */
    gb_write(&gb, 0xA485 + 2, 0x33);
    /* DX2: 0xA485 + 5 = 0xA48A */
    gb_write(&gb, 0xA48A + 3, 0x44);
    /* DX3: 0xA48A + 0x20 = 0xA4AA */
    gb_write(&gb, 0xA4AA + 0, 1);    /* tunic */
    gb_write(&gb, 0xA4AA + 1, 0x88); /* photo1 */
    gb_write(&gb, 0xA4AA + 2, 0x99); /* photo2 */

    /* Set up spawn variables in SRAM room status / WRAM */
    gb_write(&gb, 0xA105 + (wSpawnPositionX - wOverworldRoomStatus), 0x48);
    gb_write(&gb, 0xA105 + (wSpawnPositionY - wOverworldRoomStatus), 0x52);
    gb_write(&gb, 0xA105 + (wSpawnMapRoom - wOverworldRoomStatus), 0x37);
    gb_write(&gb, 0xA105 + (wSpawnMapId - wOverworldRoomStatus), 0x05);
    gb_write(&gb, 0xA105 + (wSpawnIndoorRoom - wOverworldRoomStatus), 0x12);
    gb_write(&gb, 0xA105 + (wSpawnIsIndoor - wOverworldRoomStatus), 0x01);

    LoadSavedFile(&gb);
    /* Check loaded SRAM data */
    assert(gb_read(&gb, wOverworldRoomStatus + 0x20) == 0x7E);
    assert(gb_read(&gb, wColorDungeonItemFlags + 2) == 0x33);
    assert(gb_read(&gb, wColorDungeonRoomStatus + 3) == 0x44);
    assert(gb_read(&gb, wTunicType) == 1);
    assert(gb_read(&gb, wPhotos1) == 0x88);
    assert(gb_read(&gb, wPhotos2) == 0x99);

    /* Check spawn positions and directions */
    assert(gb_read(&gb, wMapEntrancePositionX) == 0x48);
    assert(gb_read(&gb, wMapEntrancePositionY) == 0x52);
    assert(gb_read(&gb, hMapRoom) == 0x37);
    assert(gb_read(&gb, wMapEntranceRoom) == 0x37);
    assert(gb_read(&gb, hMapId) == 0x05);
    assert(gb_read(&gb, wIndoorRoom) == 0x12);
    assert(gb_read(&gb, wIsIndoor) == 1);
    assert(gb_read(&gb, hLinkDirection) == DIRECTION_UP);
    assert(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_STANDING_UP);

    /* Test 3: wDBD1 != 0 skips SRAM loading and restores health from max hearts if 0 */
    gb_init(&gb);
    gb_write(&gb, wDBD1, 1);
    gb_write(&gb, wHealth, 0);
    gb_write(&gb, wMaxHearts, 6); /* MaxHeartsToStartingHealthTable[6] == 40 */
    gb_write(&gb, wSpawnPositionX, 0x48);
    LoadSavedFile(&gb);
    assert(gb_read(&gb, wDBD1) == 0);
    assert(gb_read(&gb, wHealth) == 40);
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_WORLD);
}

void test_file_save_screen_and_init(void) {
    GBState gb;

    /* Test 1: func_001_4794 prefix validation and recovery */
    gb_init(&gb);
    /* Corrupt slot 0 prefix */
    gb_write(&gb, 0xA100, 0x00);
    gb_write(&gb, 0xA105, 0x55);
    func_001_4794(&gb, 0);
    /* Prefix should now be 1, 3, 5, 7, 9 */
    assert(gb_read(&gb, 0xA100) == 1);
    assert(gb_read(&gb, 0xA101) == 3);
    assert(gb_read(&gb, 0xA102) == 5);
    assert(gb_read(&gb, 0xA103) == 7);
    assert(gb_read(&gb, 0xA104) == 9);
    /* Main area wiped */
    assert(gb_read(&gb, 0xA105) == 0);

    /* Test 2: InitSaveFiles with DebugTool */
    gb_init(&gb);
    uint8_t dummy_rom[0x100] = { 0 };
    dummy_rom[0x0003] = 1; /* Enable debug save creation */
    gb_attach_rom(&gb, dummy_rom, sizeof(dummy_rom));
    InitSaveFiles(&gb);
    assert(gb_read(&gb, 0xA453) == 0x01); /* sword level 1 */
    assert(gb_read(&gb, 0xA460) == 0x0A); /* 10 hearts */

    /* Test 3: FileSaveDelay1 and FileSaveDelay2 */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    FileSaveDelay1(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_SAVE_MENU);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    FileSaveDelay2(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_MENU_FILE_SAVE);
    assert(gb_read(&gb, wWindowY) == 0xFF);
    assert(gb_read(&gb, wPaletteUnknownE) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 4);

    /* Test 4: func_001_412A navigation */
    gb_init(&gb);
    gb_write(&gb, wC13F, 0);
    gb_write(&gb, hJoypadState, J_DOWN);
    func_001_412A(&gb);
    assert(gb_read(&gb, wC13F) == 1);
    assert(gb_read(&gb, wOAMBuffer + 0x18) == 0x58);
    assert(gb_read(&gb, wOAMBuffer + 0x19) == 0x24);

    /* Test 5: FileSaveInteractive Return to Game */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 5);
    gb_write(&gb, wC13F, 0); /* Return to game */
    gb_write(&gb, hJoypadState, J_A);
    FileSaveInteractive(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 6);
    assert(gb_read(&gb, hJingle) == JINGLE_VALIDATE);

    /* Test 6: FileSaveInteractive Save and Quit */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 5);
    gb_write(&gb, wC13F, 1); /* Save and Quit */
    gb_write(&gb, hJoypadState, J_A);
    FileSaveInteractive(&gb);
    assert(gb_read(&gb, rLCDC) == 0xC7);
    assert(gb_read(&gb, wLCDControl) == 0xC7);

    /* Test 7: LCDOn configuration */
    gb_init(&gb);
    LCDOn(&gb);
    assert(gb_read(&gb, rLCDC) == 0xC7);
    assert(gb_read(&gb, rWX) == 0x07);
    assert(gb_read(&gb, rWY) == 0x80);

    /* Test 8: FileSaveEntryPoint dispatcher */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    FileSaveEntryPoint(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}
