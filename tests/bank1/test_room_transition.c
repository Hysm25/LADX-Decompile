#include "test_bank1.h"

#include "gb.h"
#include "bank1/room_transition.h"
#include "constants/dialog.h"
#include "constants/entities.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/maps.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t g_last_spawned_entity = 0;
static uint16_t mock_spawn_new_entity(GBState *gb, uint8_t entity_type) {
    g_last_spawned_entity = entity_type;
    return 3; /* Always assign slot 3 */
}

static bool g_mock_audio_called = false;
static void mock_func_01F_4003(GBState *gb) {
    g_mock_audio_called = true;
}

void test_prepare_entity_position_for_room_transition(void) {
    GBState gb;

    /* Direction: RIGHT (0). off_x = 0xA0, sign_x = 0x00, off_y = 0x00, sign_y = 0x00 */
    gb_init(&gb);
    gb_write(&gb, hMultiPurposeD, 3);
    gb_write(&gb, wRoomTransitionDirection, 0);
    gb_write(&gb, wEntitiesPosXTable + 2, 0x70);
    gb_write(&gb, wEntitiesPosXSignTable + 2, 0x00);
    gb_write(&gb, wEntitiesPosYTable + 2, 0x40);
    gb_write(&gb, wEntitiesPosYSignTable + 2, 0x00);

    PrepareEntityPositionForRoomTransition(&gb, 2);

    assert(gb_read(&gb, wEntitiesLoadOrderTable + 2) == 3);
    assert(gb_read(&gb, hMultiPurposeD) == 4);
    assert(gb_read(&gb, hMultiPurpose0) == 0xA0);
    assert(gb_read(&gb, hMultiPurpose1) == 0x00);
    assert(gb_read(&gb, hMultiPurpose2) == 0x00);
    assert(gb_read(&gb, hMultiPurpose3) == 0x00);
    /* 0x70 + 0xA0 = 0x110 -> low byte 0x10, carry into sign -> 0x01 */
    assert(gb_read(&gb, wEntitiesPosXTable + 2) == 0x10);
    assert(gb_read(&gb, wEntitiesPosXSignTable + 2) == 0x01);
    /* Y: 0x40 + 0 = 0x40, sign = 0 */
    assert(gb_read(&gb, wEntitiesPosYTable + 2) == 0x40);
    assert(gb_read(&gb, wEntitiesPosYSignTable + 2) == 0x00);

    /* Direction: LEFT (1). off_x = 0x60, sign_x = 0xFF, off_y = 0x00, sign_y = 0x00 */
    gb_init(&gb);
    gb_write(&gb, hMultiPurposeD, 0);
    gb_write(&gb, wRoomTransitionDirection, 1);
    gb_write(&gb, wEntitiesPosXTable + 1, 0x20);
    gb_write(&gb, wEntitiesPosXSignTable + 1, 0x00);
    PrepareEntityPositionForRoomTransition(&gb, 1);
    /* 0x20 + 0x60 = 0x80 (no carry), sign = 0 + 0xFF + 0 = 0xFF */
    assert(gb_read(&gb, wEntitiesPosXTable + 1) == 0x80);
    assert(gb_read(&gb, wEntitiesPosXSignTable + 1) == 0xFF);

    /* Direction: TOP (2). off_x = 0x00, sign_x = 0x00, off_y = 0x80, sign_y = 0xFF */
    gb_init(&gb);
    gb_write(&gb, hMultiPurposeD, 0);
    gb_write(&gb, wRoomTransitionDirection, 2);
    gb_write(&gb, wEntitiesPosYTable + 0, 0x30);
    gb_write(&gb, wEntitiesPosYSignTable + 0, 0x00);
    PrepareEntityPositionForRoomTransition(&gb, 0);
    /* 0x30 + 0x80 = 0xB0 (no carry), sign = 0 + 0xFF = 0xFF */
    assert(gb_read(&gb, wEntitiesPosYTable + 0) == 0xB0);
    assert(gb_read(&gb, wEntitiesPosYSignTable + 0) == 0xFF);
}

void test_update_recent_rooms_list(void) {
    GBState gb;
    gb_init(&gb);

    for (int i = 0; i < 6; i++) {
        gb_write(&gb, wRecentRooms + i, 0xFF);
    }
    gb_write(&gb, wRecentRoomsIndex, 0);

    /* Room already in list -> does nothing */
    gb_write(&gb, wRecentRooms + 2, 0x12);
    gb_write(&gb, hMapRoom, 0x12);
    UpdateRecentRoomsList(&gb);
    assert(gb_read(&gb, wRecentRoomsIndex) == 0);

    /* Room not in list -> adds room, advances index, clears evicted room flag */
    gb_write(&gb, hMapRoom, 0x34);
    gb_write(&gb, wRecentRooms + 1, 0x77); /* evicted room */
    gb_write(&gb, wEntitiesClearedRooms + 0x77, 0x5A);
    UpdateRecentRoomsList(&gb);
    assert(gb_read(&gb, wRecentRoomsIndex) == 1);
    assert(gb_read(&gb, wRecentRooms + 1) == 0x34);
    assert(gb_read(&gb, wEntitiesClearedRooms + 0x77) == 0);

    /* Wraparound at 6 */
    gb_write(&gb, wRecentRoomsIndex, 5);
    gb_write(&gb, hMapRoom, 0x88);
    gb_write(&gb, wRecentRooms + 0, 0x99);
    gb_write(&gb, wEntitiesClearedRooms + 0x99, 0xFF);
    UpdateRecentRoomsList(&gb);
    assert(gb_read(&gb, wRecentRoomsIndex) == 0);
    assert(gb_read(&gb, wRecentRooms + 0) == 0x88);
    assert(gb_read(&gb, wEntitiesClearedRooms + 0x99) == 0);
}

void test_hide_all_sprites(void) {
    GBState gb;

    /* DMG */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 0);
    for (int i = 0; i < 40; i++) {
        gb_write(&gb, wOAMBuffer + i * 4, 0x10);
    }
    HideAllSprites(&gb);
    assert(gb.sram_enabled == false);
    for (int i = 0; i < 40; i++) {
        assert(gb_read(&gb, wOAMBuffer + i * 4) == 0xF4);
    }

    /* CGB */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    HideAllSprites(&gb);
    assert(gb.sram_enabled == false);
}

void test_hide_sprites(void) {
    GBState gb;

    /* 1. Inventory appearing */
    gb_init(&gb);
    gb_write(&gb, wInventoryAppearing, 1);
    gb_write(&gb, wWindowY, 0x20); /* d = 0x20 + 8 = 0x28 */
    /* Sprite 0: Y = 0x20 (< 0x28, not hidden) */
    gb_write(&gb, wOAMBuffer + 0, 0x20);
    /* Sprite 1: Y = 0x30 (>= 0x28, hidden) */
    gb_write(&gb, wOAMBuffer + 4, 0x30);
    HideSprites(&gb);
    assert(gb_read(&gb, wOAMBuffer + 0) == 0x20);
    assert(gb_read(&gb, wOAMBuffer + 4) == 0x00);

    /* 2. Dialog box at bottom (DIALOG_BOX_BOTTOM_FLAG set) */
    gb_init(&gb);
    gb_write(&gb, wInventoryAppearing, 0);
    gb_write(&gb, wWindowY, 0x80); /* not 0 */
    gb_write(&gb, wDialogState, DIALOG_BOX_BOTTOM_FLAG | DIALOG_LETTER_IN_1);
    /* d = 0x58 */
    /* Entry 9 (first tested): Y = 0x50 (< 0x58, not hidden) */
    gb_write(&gb, wOAMBuffer + 9 * 4, 0x50);
    /* Entry 10: Y = 0x60 (>= 0x58, hidden) */
    gb_write(&gb, wOAMBuffer + 10 * 4, 0x60);
    HideSprites(&gb);
    assert(gb_read(&gb, wOAMBuffer + 9 * 4) == 0x50);
    assert(gb_read(&gb, wOAMBuffer + 10 * 4) == 0x00);

    /* 3. Dialog box at top, Dialog04F Piece of Heart exception */
    gb_init(&gb);
    gb_write(&gb, wInventoryAppearing, 0);
    gb_write(&gb, wWindowY, 0x80);
    gb_write(&gb, wDialogState, DIALOG_LETTER_IN_1); /* top box, d = 0x3E */
    gb_write(&gb, wDialogIndex, Dialog04F);
    gb_write(&gb, wDialogIndexHi, 0);

    /* Entry 9: Y = 0x20 (< 0x3E -> hide candidate), Tile = 0x9B (Piece of Heart) -> preserved */
    gb_write(&gb, wOAMBuffer + 9 * 4 + 0, 0x20);
    gb_write(&gb, wOAMBuffer + 9 * 4 + 2, 0x9B);

    /* Entry 10: Y = 0x20 (< 0x3E -> hide candidate), Tile = 0x30 (regular sprite) -> hidden */
    gb_write(&gb, wOAMBuffer + 10 * 4 + 0, 0x20);
    gb_write(&gb, wOAMBuffer + 10 * 4 + 2, 0x30);

    HideSprites(&gb);
    assert(gb_read(&gb, wOAMBuffer + 9 * 4 + 0) == 0x20);
    assert(gb_read(&gb, wOAMBuffer + 10 * 4 + 0) == 0x00);
}

void test_synchronize_dungeons_item_flags(void) {
    GBState gb;

    /* 1. Overworld -> does nothing */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hMapId, 2);
    for (int i = 0; i < 5; i++) {
        gb_write(&gb, wCurrentDungeonItemFlags + i, 0x10 + i);
    }
    SynchronizeDungeonsItemFlags(&gb);
    for (int i = 0; i < 5; i++) {
        assert(gb_read(&gb, wDungeonItemFlags + 2 * 5 + i) == 0);
    }

    /* 2. Dungeon 2 (hMapId = 2) */
    gb_write(&gb, wIsIndoor, 1);
    SynchronizeDungeonsItemFlags(&gb);
    for (int i = 0; i < 5; i++) {
        assert(gb_read(&gb, wDungeonItemFlags + 2 * 5 + i) == 0x10 + i);
    }

    /* 3. Non-dungeon cave (hMapId >= MAP_CAVE_B) -> does nothing */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_CAVE_B);
    SynchronizeDungeonsItemFlags(&gb);
    for (int i = 0; i < 5; i++) {
        assert(gb_read(&gb, wDungeonItemFlags + i) == 0);
    }

    /* 4. Color Dungeon (hMapId = MAP_COLOR_DUNGEON = 0xFF) */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_COLOR_DUNGEON);
    for (int i = 0; i < 5; i++) {
        gb_write(&gb, wCurrentDungeonItemFlags + i, 0x40 + i);
    }
    SynchronizeDungeonsItemFlags(&gb);
    for (int i = 0; i < 5; i++) {
        assert(gb_read(&gb, wColorDungeonItemFlags + i) == 0x40 + i);
    }
}

void test_create_following_npc_entity(void) {
    GBState gb;

    /* 1. Indoor exclusions */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hIsSideScrolling, 1);
    gb_write(&gb, wIsRoosterFollowingLink, 1);
    g_last_spawned_entity = 0;
    CreateFollowingNpcEntity(&gb, mock_spawn_new_entity);
    assert(g_last_spawned_entity == 0);

    /* 2. Rooster spawn */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wIsRoosterFollowingLink, 1);
    gb_write(&gb, hLinkPositionX, 0x40);
    gb_write(&gb, hLinkPositionY, 0x30);
    gb_write(&gb, hLinkPositionZ, 0x05);
    gb_write(&gb, wC13B, 0x02);
    /* Existing rooster in slot 1 */
    gb_write(&gb, wEntitiesTypeTable + 1, ENTITY_ROOSTER);
    gb_write(&gb, wEntitiesStatusTable + 1, 0x01);
    CreateFollowingNpcEntity(&gb, mock_spawn_new_entity);
    assert(gb_read(&gb, wEntitiesStatusTable + 1) == 0); /* old rooster deactivated */
    assert(g_last_spawned_entity == ENTITY_ROOSTER);
    assert(gb_read(&gb, wEntitiesPosXTable + 3) == 0x40);
    assert(gb_read(&gb, wEntitiesPosYTable + 3) == 0x32); /* 0x30 + 0x02 */
    assert(gb_read(&gb, wEntitiesPosZTable + 3) == 0x05);

    /* 3. Ghost trigger (state 2 -> state 1) */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wIsGhostFollowingLink, 2);
    gb_write(&gb, hMapRoom, ROOM_SECTION_OW_GHOST_TRIGGER);
    gb_write(&gb, wHasInstrument4, 0x02);
    gb_write(&gb, wPowerBraceletLevel, 1);
    CreateFollowingNpcEntity(&gb, mock_spawn_new_entity);
    assert(gb_read(&gb, wIsGhostFollowingLink) == 1);

    /* Ghost spawn (state 1) */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wIsGhostFollowingLink, 1);
    gb_write(&gb, hLinkPositionX, 0x55);
    gb_write(&gb, hLinkPositionY, 0x66);
    gb_write(&gb, wC13B, 0x04);
    CreateFollowingNpcEntity(&gb, mock_spawn_new_entity);
    assert(g_last_spawned_entity == ENTITY_GHOST);
    assert(gb_read(&gb, wEntitiesPosXTable + 3) == 0x55);
    assert(gb_read(&gb, wEntitiesPosYTable + 3) == 0x6A);
    assert(gb_read(&gb, wEntitiesPrivateState1Table + 3) == 1);
    assert(gb_read(&gb, hJingle) == JINGLE_GHOST_PRESENCE);

    /* 4. Marin spawn */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wIsMarinFollowingLink, 1);
    gb_write(&gb, hLinkPositionX, 0x20);
    gb_write(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, hLinkPositionZ, 0x00);
    gb_write(&gb, hLinkDirection, 2);
    gb_write(&gb, wC13B, 0x00);
    gb_write(&gb, wDB10, 1);
    CreateFollowingNpcEntity(&gb, mock_spawn_new_entity);
    assert(g_last_spawned_entity == ENTITY_MARIN_AT_THE_SHORE);
    assert(gb_read(&gb, wEntitiesPosXTable + 3) == 0x40); /* 0x20 + 0x20 */
    assert(gb_read(&gb, wEntitiesPosYTable + 3) == 0x50); /* 0x40 + 0x10 */
    assert(gb_read(&gb, wEntitiesPrivateState4Table + 3) == 1);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + 3) == 0x0C);
    for (int i = 0; i < 0x10; i++) {
        assert(gb_read(&gb, wLinkPositionXHistory + i) == 0x20);
        assert(gb_read(&gb, wLinkPositionYHistory + i) == 0x40);
        assert(gb_read(&gb, wLinkPositionZHistory + i) == 0x00);
        assert(gb_read(&gb, wLinkDirectionHistory + i) == 2);
    }

    /* 5. Bow-Wow spawn */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wIsBowWowFollowingLink, BOW_WOW_FOLLOWING);
    gb_write(&gb, hLinkPositionX, 0x18);
    gb_write(&gb, hLinkPositionY, 0x28);
    gb_write(&gb, hLinkPositionZ, 0x00);
    CreateFollowingNpcEntity(&gb, mock_spawn_new_entity);
    assert(g_last_spawned_entity == ENTITY_BOW_WOW);
    assert(gb_read(&gb, wEntitiesPosXTable + 3) == 0x18);
    assert(gb_read(&gb, wEntitiesPosYTable + 3) == 0x28);
    assert(gb_read(&gb, wEntitiesPosZTable + 3) == 0x00);
}

void test_func_001_6162(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wGameplayType, 2);
    gb_write(&gb, wGameplaySubtype, 3);
    gb_write(&gb, wOBJ0Palette, 0xFF);
    gb_write(&gb, wOBJ1Palette, 0xFF);
    gb_write(&gb, wBGPalette, 0xFF);
    gb_write(&gb, rBGP, 0xE4);
    gb_write(&gb, rOBP0, 0xE4);
    gb_write(&gb, rOBP1, 0xE4);
    gb_write(&gb, hBaseScrollY, 0x10);
    gb_write(&gb, hBaseScrollX, 0x20);
    gb_write(&gb, wSwitchBlocksState, 1);
    gb_write(&gb, wSwitchableObjectAnimationStage, 2);
    gb_write(&gb, hButtonsInactiveDelay, 0);

    g_mock_audio_called = false;
    func_001_6162(&gb, mock_func_01F_4003);

    assert(g_mock_audio_called == true);
    assert(gb_read(&gb, wGameplayType) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 0);
    assert(gb_read(&gb, wOBJ0Palette) == 0);
    assert(gb_read(&gb, wOBJ1Palette) == 0);
    assert(gb_read(&gb, wBGPalette) == 0);
    assert(gb_read(&gb, rBGP) == 0);
    assert(gb_read(&gb, rOBP0) == 0);
    assert(gb_read(&gb, rOBP1) == 0);
    assert(gb_read(&gb, hBaseScrollY) == 0);
    assert(gb_read(&gb, hBaseScrollX) == 0);
    assert(gb_read(&gb, wSwitchBlocksState) == 0);
    assert(gb_read(&gb, wSwitchableObjectAnimationStage) == 0);
    assert(gb_read(&gb, hButtonsInactiveDelay) == 0x18);
}

void test_load_counter_animated_tiles(void) {
    GBState gb;
    gb_init(&gb);

    /* Allocate dummy ROM for bank $0F */
    uint8_t dummy_rom[0x4000 * 16];
    memset(dummy_rom, 0, sizeof(dummy_rom));
    /* Fill offset $5730 in bank $0F with 0xAA, and $5810 with 0xBB */
    memset(&dummy_rom[0x0F * 0x4000 + (0x5730 - 0x4000)], 0xAA, 0x10);
    memset(&dummy_rom[0x0F * 0x4000 + (0x5810 - 0x4000)], 0xBB, 0x10);
    gb_attach_rom(&gb, dummy_rom, sizeof(dummy_rom));

    /* Dialog ID = 0xB3: low nibble = 3 (addr 0x5730), high nibble = 0xB (addr 0x5810) */
    gb_write(&gb, wTextDebuggerDialogId, 0xB3);
    LoadCounterAnimatedTiles(&gb);

    /* Verify low nibble copied to 0x96D0 */
    for (int i = 0; i < 0x10; i++) {
        assert(gb_read(&gb, 0x96D0 + i) == 0xAA);
    }
    /* Verify high nibble copied to 0x96C0 */
    for (int i = 0; i < 0x10; i++) {
        assert(gb_read(&gb, 0x96C0 + i) == 0xBB);
    }
    assert(gb_read(&gb, 0x9909) == 0x6C);
    assert(gb_read(&gb, 0x990A) == 0x6D);
}

void test_open_dungeon_name_dialog(void) {
    GBState gb;

    /* 1. Motion state not default */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, 1);
    gb_write(&gb, wFreeMovementMode, 0);
    gb_write(&gb, hMapId, 2);
    OpenDungeonNameDialog(&gb);
    assert(gb_read(&gb, wDialogIndex) == 0);

    /* 2. Free movement mode active */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wFreeMovementMode, 1);
    gb_write(&gb, hMapId, 2);
    OpenDungeonNameDialog(&gb);
    assert(gb_read(&gb, wDialogIndex) == 0);

    /* 3. Successful open */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wFreeMovementMode, 0);
    gb_write(&gb, hMapId, 2);
    OpenDungeonNameDialog(&gb);
    assert(gb_read(&gb, wDialogIndex) == 0x56 + 2);
}

void test_load_tileset_0f_and_attributes(void) {
    GBState gb;

    /* 1. DMG mode: fills checkerboard in bank 0, does not touch bank 1 attributes */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 0);
    LoadTileset0F(&gb);

    /* Row 0: alternating AE, AF */
    assert(gb_read(&gb, 0x9800) == 0xAE);
    assert(gb_read(&gb, 0x9801) == 0xAF);
    assert(gb_read(&gb, 0x9802) == 0xAE);
    assert(gb_read(&gb, 0x9813) == 0xAF); /* col 19 */
    assert(gb_read(&gb, 0x9814) == 0x00); /* col 20 not touched */
    assert(gb_read(&gb, 0x981F) == 0x00); /* col 31 not touched */

    /* Row 1: alternating AF, AE (inverted due to bit 5) */
    assert(gb_read(&gb, 0x9820) == 0xAF);
    assert(gb_read(&gb, 0x9821) == 0xAE);
    assert(gb_read(&gb, 0x9833) == 0xAE); /* col 19 */
    assert(gb_read(&gb, 0x9834) == 0x00); /* col 20 not touched */

    /* 2. GBC mode with GAMEPLAY_WORLD */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    LoadTileset0F(&gb);

    assert(gb_read(&gb, rVBK) == 0); /* bank restored */
    /* Check VRAM bank 1 attributes */
    gb_write(&gb, rVBK, 1);
    for (int i = 0; i < 0x400; i++) {
        assert(gb_read(&gb, 0x9800 + i) == 0x05);
    }
    gb_write(&gb, rVBK, 0);

    /* 3. GBC mode with other gameplay type */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wGameplayType, 0);
    func_001_6D11(&gb);
    gb_write(&gb, rVBK, 1);
    for (int i = 0; i < 0x400; i++) {
        assert(gb_read(&gb, 0x9800 + i) == 0x06);
    }
}

void test_write_dma_code_to_hram(void) {
    GBState gb;
    gb_init(&gb);

    WriteDMACodeToHRAM(&gb);

    static const uint8_t expected[10] = {
        0x3E, 0xC0, 0xE0, 0x46, 0x3E, 0x28, 0x3D, 0x20, 0xFD, 0xC9
    };
    for (int i = 0; i < 10; i++) {
        assert(gb_read(&gb, hDMARoutine + i) == expected[i]);
    }
}

void test_update_minimap_entrance_arrow(void) {
    GBState gb;

    /* 1. ROM_DebugTool2 enabled -> returns */
    gb_init(&gb);
    uint8_t dummy_rom[0x4000];
    memset(dummy_rom, 0, sizeof(dummy_rom));
    dummy_rom[ROM_DebugTool2] = 1;
    gb_attach_rom(&gb, dummy_rom, sizeof(dummy_rom));
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    UpdateMinimapEntranceArrowAndReturn(&gb);
    assert(gb_read(&gb, vBGMap1 + 0x20B + MINIMAP_ARROW_TAIL_CAVE) == 0);

    /* 2. Outdoors (wIsIndoor == 0) -> returns */
    dummy_rom[ROM_DebugTool2] = 0;
    gb_write(&gb, wIsIndoor, 0);
    UpdateMinimapEntranceArrowAndReturn(&gb);
    assert(gb_read(&gb, vBGMap1 + 0x20B + MINIMAP_ARROW_TAIL_CAVE) == 0);

    /* 3. Non-dungeon map (hMapId >= 8) -> returns */
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, 8);
    UpdateMinimapEntranceArrowAndReturn(&gb);
    assert(gb_read(&gb, vBGMap1 + 0x20B + MINIMAP_ARROW_TAIL_CAVE) == 0);

    /* 4. Tail Cave (hMapId = 0) normal */
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, hIsSideScrolling, 0);
    UpdateMinimapEntranceArrowAndReturn(&gb);
    assert(gb_read(&gb, vBGMap1 + 0x20B + MINIMAP_ARROW_TAIL_CAVE) == 0xA3);

    /* 5. Tail Cave side-scrolling -> writes 0x7F */
    gb_write(&gb, hIsSideScrolling, 1);
    UpdateMinimapEntranceArrowAndReturn(&gb);
    assert(gb_read(&gb, vBGMap1 + 0x20B + MINIMAP_ARROW_TAIL_CAVE) == 0x7F);

    /* 6. Color Dungeon (hMapId = 0xFF) */
    gb_write(&gb, hMapId, MAP_COLOR_DUNGEON);
    gb_write(&gb, hIsSideScrolling, 0);
    UpdateMinimapEntranceArrowAndReturn(&gb);
    assert(gb_read(&gb, vBGMap1 + 0x20B + MINIMAP_ARROW_COLOR_DUNGEON) == 0xA3);
}

void test_increment_gameplay_subtype(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wGameplaySubtype, 5);
    IncrementGameplaySubtype(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 6);

    IncrementGameplaySubtypeAndReturn(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 7);
}

void test_func_001_5888(void) {
    GBState gb;
    gb_init(&gb);

    for (int i = 0; i < 0x0C; i++) {
        gb_write(&gb, wRoomTransitionState + i, 0xAA);
    }
    gb_write(&gb, wRoomTransitionState - 1, 0x55);
    gb_write(&gb, wRoomTransitionState + 0x0C, 0x55);

    func_001_5888(&gb);

    assert(gb_read(&gb, wRoomTransitionState - 1) == 0x55);
    for (int i = 0; i < 0x0C; i++) {
        assert(gb_read(&gb, wRoomTransitionState + i) == 0x00);
    }
    assert(gb_read(&gb, wRoomTransitionState + 0x0C) == 0x55);
}

void test_initialize_inventory_bar(void) {
    GBState gb;
    gb_init(&gb);

    InitializeInventoryBar(&gb);

    assert(gb_read(&gb, wWindowY) == 0x80);
    assert(gb_read(&gb, rWX) == 0x07);
    assert(gb_read(&gb, wSubscreenScrollIncrement) == 0x08);
    assert(gb_read(&gb, wInventoryAppearing) == 0x00);
}

void test_func_001_58A8(void) {
    GBState gb;

    /* 1. DMG mode */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wDB54, 0x56);
    gb_write(&gb, hFrameCounter, 0x08);

    func_001_58A8(&gb);

    assert(gb_read(&gb, wDynamicOAMBuffer + 0x6C) == 0x40);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x6D) == 0x48);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x6E) == 0x3E);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x6F) == 0x10);

    /* 2. GBC mode with frame bit 3 set */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wDB54, 0x56);
    gb_write(&gb, hFrameCounter, 0x08);

    func_001_58A8(&gb);

    assert(gb_read(&gb, wDynamicOAMBuffer + 0x6F) == 0x03);

    /* 3. GBC mode with frame bit 3 cleared */
    gb_write(&gb, hFrameCounter, 0x00);
    func_001_58A8(&gb);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x6F) == 0x00);
}

void test_peach_picture_state_2(void) {
    GBState gb;

    /* 1. Eagles Tower */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_EAGLES_TOWER);
    gb_write(&gb, wGameplaySubtype, 2);
    PeachPictureState2Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_EAGLES_TOWER_TOP);
    assert(gb_read(&gb, wC13F) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    /* 2. Schule House */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, hMapRoom, ROOM_INDOOR_B_SCHULE_HOUSE);
    gb_write(&gb, wGameplaySubtype, 2);
    PeachPictureState2Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_SCHULE_PAINTING);
    assert(gb_read(&gb, wC13F) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    /* 3. Christine */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, hMapRoom, 0x00);
    gb_write(&gb, wGameplaySubtype, 2);
    PeachPictureState2Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_CHRISTINE);
    assert(gb_read(&gb, wC13F) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}

void test_peach_picture_state_3(void) {
    GBState gb;

    /* 1. Eagles Tower Collapse */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_EAGLES_TOWER);
    gb_write(&gb, wGameplaySubtype, 3);
    PeachPictureState3Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_EAGLES_TOWER_COLLAPSE);
    assert(gb_read(&gb, wWindowY) == 0xFF);
    assert(gb_read(&gb, hBaseScrollX) == 0);
    assert(gb_read(&gb, hBaseScrollY) == 0);
    assert(gb_read(&gb, wTransitionSequenceCounter) == 0);
    assert(gb_read(&gb, wC16C) == 0);
    for (int i = 0; i < 8; i++) {
        assert(gb_read(&gb, wD210 + i) == 0);
    }
    assert(gb_read(&gb, wPaletteUnknownE) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 4);

    /* 2. Schule Painting */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, hMapRoom, ROOM_INDOOR_B_SCHULE_HOUSE);
    gb_write(&gb, wGameplaySubtype, 3);
    PeachPictureState3Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_SCHULE_PAINTING);

    /* 3. Peach */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, hMapRoom, 0x00);
    gb_write(&gb, wGameplaySubtype, 3);
    PeachPictureState3Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_PEACH);
}

void test_func_001_695B(void) {
    GBState gb;

    /* 1. wD215 == 0 -> returns, shake is 0 */
    gb_init(&gb);
    gb_write(&gb, wScreenShakeVertical, 5);
    gb_write(&gb, wD215, 0);
    func_001_695B(&gb);
    assert(gb_read(&gb, wScreenShakeVertical) == 0);
    assert(gb_read(&gb, wD215) == 0);

    /* 2. wD215 == 5 -> decrements to 4, (4 & 4) != 0 -> shake is 0x00 */
    gb_write(&gb, wD215, 5);
    func_001_695B(&gb);
    assert(gb_read(&gb, wD215) == 4);
    assert(gb_read(&gb, wScreenShakeVertical) == 0x00);

    /* 3. wD215 == 4 -> decrements to 3, (3 & 4) == 0 -> shake is 0xFE */
    func_001_695B(&gb);
    assert(gb_read(&gb, wD215) == 3);
    assert(gb_read(&gb, wScreenShakeVertical) == 0xFE);
}

void test_func_6A7C(void) {
    GBState gb;

    /* 1. Not Eagles Tower -> early return */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, hActiveEntityPosX, 0x12);
    func_6A7C(&gb);
    assert(gb_read(&gb, hActiveEntityPosX) == 0x12);

    /* 2. Eagles Tower */
    gb_write(&gb, hMapId, MAP_EAGLES_TOWER);
    gb_write(&gb, wD214, 0);
    gb_write(&gb, wD211, 0x10);
    gb_write(&gb, wD213, 0);
    gb_write(&gb, wScreenShakeVertical, 0);
    func_6A7C(&gb);
    assert(gb_read(&gb, hActiveEntityPosX) == 0x48);
    assert(gb_read(&gb, hActiveEntityVisualPosY) == 0x30);
    assert(gb_read(&gb, wOAMNextAvailableSlot) == 0);
}

void test_peach_picture_state_4(void) {
    GBState gb;
    gb_init(&gb);

    /* When wTransitionSequenceCounter != 4 */
    gb_write(&gb, wGameplaySubtype, 4);
    gb_write(&gb, wTransitionSequenceCounter, 2);
    PeachPictureState4Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 4);

    /* When wTransitionSequenceCounter == 4 */
    gb_write(&gb, wTransitionSequenceCounter, 4);
    PeachPictureState4Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 5);
    assert(gb_read(&gb, wD210) == 0x80);
}

void test_peach_picture_state_5_and_68D9(void) {
    GBState gb;

    /* 1. Eagles Tower -> immediately sets subtype 7 */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_EAGLES_TOWER);
    gb_write(&gb, wGameplaySubtype, 5);
    PeachPictureState5Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 7);

    /* 2. Other map, no button pressed -> does not advance */
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, wGameplaySubtype, 5);
    gb_write(&gb, hJoypadState, 0);
    PeachPictureState5Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 5);

    /* 3. Button A pressed -> sets jingle and calls func_001_68D9 */
    gb_write(&gb, hJoypadState, J_A);
    PeachPictureState5Handler(&gb);
    assert(gb_read(&gb, hJingle) == JINGLE_VALIDATE);
    assert(gb_read(&gb, wGameplaySubtype) == 6);
    assert(gb_read(&gb, wTransitionSequenceCounter) == 0);
    assert(gb_read(&gb, wC16C) == 0);
}

void test_peach_picture_state_7(void) {
    GBState gb;
    gb_init(&gb);

    /* 1. wD210 > 1 -> decrements, updates shake */
    gb_write(&gb, wGameplaySubtype, 7);
    gb_write(&gb, wD210, 5);
    PeachPictureState7Handler(&gb);
    assert(gb_read(&gb, wD210) == 4);
    assert(gb_read(&gb, wScreenShakeVertical) == 0xFE);
    assert(gb_read(&gb, wGameplaySubtype) == 7);

    /* 2. wD210 == 1 -> reaches 0, resets shake, sets wD210=0x20, advances to state 8 */
    gb_write(&gb, wD210, 1);
    PeachPictureState7Handler(&gb);
    assert(gb_read(&gb, wScreenShakeVertical) == 0);
    assert(gb_read(&gb, wD210) == 0x20);
    assert(gb_read(&gb, wGameplaySubtype) == 8);
}

void test_peach_picture_state_8(void) {
    GBState gb;
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, wGameplaySubtype, 8);

    /* 1. wD210 > 1 -> decrements wD210, stays in state 8 */
    gb_write(&gb, wD210, 5);
    PeachPictureState8Handler(&gb);
    assert(gb_read(&gb, wD210) == 4);
    assert(gb_read(&gb, wGameplaySubtype) == 8);

    /* 2. wD210 == 1 -> explosion, resets timers, wD213 goes from 0 to 1 */
    gb_write(&gb, wD210, 1);
    gb_write(&gb, wD211, 0x10);
    gb_write(&gb, wD213, 0);
    PeachPictureState8Handler(&gb);
    assert(gb_read(&gb, wD210) == 0x30);
    assert(gb_read(&gb, wD214) == 0x30);
    assert(gb_read(&gb, wD215) == 0x18);
    assert(gb_read(&gb, wD211) == 0x18);
    assert(gb_read(&gb, wD213) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 8);

    /* 3. wD210 == 1 with wD213 == 3 -> wD213 becomes 4, advances to state 9 with wD210 = 0x80 */
    gb_write(&gb, wD210, 1);
    gb_write(&gb, wD213, 3);
    PeachPictureState8Handler(&gb);
    assert(gb_read(&gb, wD213) == 4);
    assert(gb_read(&gb, wD210) == 0x80);
    assert(gb_read(&gb, wGameplaySubtype) == 9);
}

void test_peach_picture_state_9(void) {
    GBState gb;
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, wGameplaySubtype, 9);

    /* 1. wD210 > 1 -> decrements, stays in state 9 */
    gb_write(&gb, wD210, 3);
    PeachPictureState9Handler(&gb);
    assert(gb_read(&gb, wD210) == 2);
    assert(gb_read(&gb, wGameplaySubtype) == 9);

    /* 2. wD210 == 1 -> decrements to 0, advances to state 10 (0x0A) */
    gb_write(&gb, wD210, 1);
    gb_write(&gb, wTransitionSequenceCounter, 5);
    gb_write(&gb, wC16C, 2);
    PeachPictureState9Handler(&gb);
    assert(gb_read(&gb, wD210) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 0x0A);
    assert(gb_read(&gb, wTransitionSequenceCounter) == 0);
    assert(gb_read(&gb, wC16C) == 0);
}

void test_file_save_fade_out_and_state_A(void) {
    GBState gb;
    gb_init(&gb);

    /* 1. Transition counter != 4 -> does nothing */
    gb_write(&gb, wTransitionSequenceCounter, 2);
    FileSaveFadeOut(&gb);
    assert(gb_read(&gb, wGameplayType) == 0);

    /* 2. Transition counter == 4 on CGB */
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, wIsIndoor, 0);

    /* Set byte in bank 3 wBGPal1 */
    gb_write(&gb, rSVBK, 3);
    gb_write(&gb, wBGPal1, 0x55);
    gb_write(&gb, wIsFileSelectionArrowShifted, 0xFF);

    PeachPictureStateAHandler(&gb);

    /* Check palette copied to bank 2 */
    gb_write(&gb, rSVBK, 2);
    assert(gb_read(&gb, wBGPal1) == 0x55);
    gb_write(&gb, rSVBK, 0);

    /* Check return to world gameplay */
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_WORLD);
    assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_WORLD_LOAD_2);
    assert(gb_read(&gb, hVolumeRight) == 7);
    assert(gb_read(&gb, hVolumeLeft) == 0x70);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_BASE_OVERWORLD_DUP);
    assert(gb_read(&gb, wWindowY) == 0x80);
}

void test_peach_picture_state_0_and_1(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wGameplaySubtype, 0);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);

    /* Set byte in bank 1 wBGPal1 */
    gb_write(&gb, rSVBK, 0);
    gb_write(&gb, wBGPal1, 0x33);

    PeachPictureState0Handler(&gb);

    /* Check palette copied to bank 3 */
    gb_write(&gb, rSVBK, 3);
    assert(gb_read(&gb, wBGPal1) == 0x33);
    gb_write(&gb, rSVBK, 0);

    /* Check state 1 execution */
    assert(gb_read(&gb, wGameplaySubtype) == 2);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_0F);
    assert(gb_read(&gb, hVolumeRight) == 3);
    assert(gb_read(&gb, hVolumeLeft) == 0x30);
    assert(gb_read(&gb, wScrollXOffset) == 0);
}

void test_peach_picture_entry_point(void) {
    GBState gb;
    gb_init(&gb);

    /* Subtype 2 dispatches to state 2 */
    gb_write(&gb, wGameplaySubtype, 2);
    gb_write(&gb, hMapId, MAP_EAGLES_TOWER);
    PeachPictureEntryPoint(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_EAGLES_TOWER_TOP);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}
