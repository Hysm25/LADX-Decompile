#include "test_bank1.h"

#include "gb.h"
#include "bank1/world_handler.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/maps.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/tilesets.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef ROM_OFFSET
#define ROM_OFFSET(bank, addr) (((size_t)(bank) * 0x4000) + ((size_t)(addr) - 0x4000))
#endif

void test_world_handler_subsystem(void) {
    GBState gb;

    static uint8_t mock_world_rom[0x4000 * 0x25] = {0};
    mock_world_rom[ROM_OFFSET(BANK_OverworldRoomsFirstHalf, OverworldRoomPointers)] = 0x50;
    mock_world_rom[ROM_OFFSET(BANK_OverworldRoomsFirstHalf, OverworldRoomPointers + 1)] = 0x40;
    mock_world_rom[ROM_OFFSET(BANK_OverworldRoomsFirstHalf, 0x4050)] = ROOM_END;
    mock_world_rom[ROM_OFFSET(0x20, OverworldTilesetsTable_Addr)] = W_TILESET_KEEP;

    /* Test 1: GameplayWorldLoad0Handler overworld */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hFrameCounter, 0x02);
    GameplayWorldLoad0Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_INVENTORY);

    /* Test 2: GameplayWorldLoad0Handler debug tool 2 */
    uint8_t mock_rom[16] = {0};
    mock_rom[ROM_DebugTool2] = 1;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    GameplayWorldLoad0Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_INVENTORY_DEBUG);

    /* Test 3: GameplayWorldLoad0Handler dungeon (Tail Cave) */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, wDungeonItemFlags, 0x01); /* map flag */
    GameplayWorldLoad0Handler(&gb);
    assert(gb_read(&gb, wHasDungeonMap) == 0x01);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_MINIMAP);
    assert(gb_read(&gb, wMinimapLayout) == 0x00);

    /* Test 4: GameplayWorldLoad0Handler Windfish's Egg */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_WINDFISHS_EGG);
    GameplayWorldLoad0Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_EAGLES_TOWER_CLOUDS);

    /* Test 5: GameplayWorldLoadRoomHandler */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_world_rom, sizeof(mock_world_rom));
    gb_write(&gb, wMapEntrancePositionX, 0x30);
    gb_write(&gb, wMapEntrancePositionY, 0x40);
    gb_write(&gb, wMapEntrancePositionZ, 0x05);
    gb_write(&gb, wIsIndoor, 0);
    GameplayWorldLoadRoomHandler(&gb);
    assert(gb_read(&gb, hLinkPositionX) == 0x30);
    assert(gb_read(&gb, hLinkPositionY) == 0x40);
    assert(gb_read(&gb, hLinkPositionZ) == 0x05);
    assert(gb_read(&gb, wIsLinkInTheAir) == 2);
    assert(gb_read(&gb, wTilesetToLoad) == 0x07); /* Outdoor */
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    /* Test 6: GameplayWorldSelectTilesetHandler */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_world_rom, sizeof(mock_world_rom));
    GameplayWorldSelectTilesetHandler(&gb);
    assert(gb_read(&gb, hWorldTileset) == 0x0F);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_ROOM_SPECIFIC);
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    /* Test 7: GameplayWorldLoadRoomTilemapHandler */
    gb_init(&gb);
    gb_write(&gb, wRoomSwitchableObject, 1);
    gb_write(&gb, wSwitchButtonPressed, 1);
    GameplayWorldLoadRoomTilemapHandler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_ROOM_TILEMAP);
    assert(gb_read(&gb, wSwitchableObjectAnimationStage) == 5);
    assert(gb_read(&gb, hReplaceTiles) == REPLACE_TILES_BUTTON_PRESSED);
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    /* Test 8: GameplayWorldLoadHeartsAndRupeesHandler & ABButtonsHandler */
    gb_init(&gb);
    GameplayWorldLoadHeartsAndRupeesHandler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    GameplayWorldLoadABButtonsHandler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 2);

    /* Test 9: GameplayWorldLoad6Handler */
    gb_init(&gb);
    gb_write(&gb, rLCDC, 0);
    gb_write(&gb, wLinkMotionState, 0x02);
    GameplayWorldLoad6Handler(&gb);
    assert((gb_read(&gb, rLCDC) & LCDCF_WINON) != 0);
    assert(gb_read(&gb, wD463) == 0x02);
    assert(gb_read(&gb, wLinkMotionState) == LINK_MOTION_MAP_FADE_IN);
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    /* Test 10: CreateMinimapTilemap and func_001_5619 */
    gb_init(&gb);
    gb_write(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write(&gb, wMinimapLayout, 0);
    CreateMinimapTilemap(&gb);
    /* Verify prefix loaded at wMinimapTilemap */
    assert(gb_read(&gb, wMinimapTilemap) == Data_001_54E8[0]);
    assert(gb_read(&gb, wMinimapTilemap + 1) == Data_001_54E8[1]);

    /* Test 11: WorldHandlerEntryPoint dispatcher */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_world_rom, sizeof(mock_world_rom));
    gb_write(&gb, wGameplaySubtype, 2);
    WorldHandlerEntryPoint(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_ROOM_SPECIFIC);
}
