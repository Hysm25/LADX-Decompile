#include "test_bank1.h"

#include "gb.h"
#include "bank1/world_map.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void test_play_validation_jingle(void) {
    GBState gb;
    gb_init(&gb);
    PlayValidationJingle(&gb);
    assert(gb_read(&gb, hJingle) == JINGLE_VALIDATE);
}

void test_func_001_5A59(void) {
    GBState gb;
    gb_init(&gb);

    /* Room 0x06 has special table entry 0x3E -> lookup 0x3E is 0x5E */
    gb_write(&gb, hMapRoom, 0x06);
    gb_write(&gb, hLinkPositionY, 0x50); /* Place dialog at top */
    func_001_5A59(&gb);
    assert(gb_read(&gb, wDialogIndex) == 0x5E);
    assert((gb_read(&gb, wDialogState) & 0x7F) == 1);
}

void test_world_map_states(void) {
    GBState gb;
    gb_init(&gb);

    /* Test State 2 */
    gb_write(&gb, wGameplaySubtype, 2);
    WorldMapState2Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_WORLD_MAP);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    /* Test State 3 */
    WorldMapState3Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_WORLD_MAP_TILEMAP);
    assert(gb_read(&gb, wPaletteUnknownE) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 4);

    /* Test State 4 (counter != 4) */
    gb_write(&gb, wTransitionSequenceCounter, 2);
    WorldMapState4Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 4);

    /* Test State 4 (counter == 4) */
    gb_write(&gb, wTransitionSequenceCounter, 4);
    WorldMapState4Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 5);
    assert(gb_read(&gb, hJingle) == JINGLE_VALIDATE);

    /* Test State 1 with Owl marker check */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, wDB54, 0x16); /* room 0x16: MapSpecialLocationNamesTable has 0x06 (Owl reminder) */
    gb_write(&gb, wC5A2, 0);
    gb_write(&gb, (uint16_t)(wOverworldRoomStatus + 0x16), OW_ROOM_STATUS_OWL_TALKED);
    gb_write(&gb, rLCDC, LCDCF_WINON | LCDCF_ON);

    WorldMapState1Handler(&gb);
    assert(gb_read(&gb, hVolumeRight) == 3);
    assert(gb_read(&gb, hVolumeLeft) == 0x30);
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    assert(gb_read(&gb, wDBB4) == 0x16);
    assert(gb_read(&gb, wC1B1) == 1); /* Owl talked -> 1 */
    assert(gb_read(&gb, wC1B4) == 0x16);
    assert((gb_read(&gb, rLCDC) & LCDCF_WINON) == 0);
    assert((gb_read(&gb, wLCDControl) & LCDCF_WINON) == 0);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_WORLD_MAP);

    /* Test State 1 with dungeon icon (room 0x0E: 0x17 -> category 1 -> icon 2) */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, wDB54, 0x0E);
    WorldMapState1Handler(&gb);
    assert(gb_read(&gb, wC1B1) == 2);

    /* Test State 0 on CGB */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wGameplaySubtype, 0);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, wDB54, 0x00);
    gb_write(&gb, rSVBK, 0);
    gb_write(&gb, wBGPal1, 0x77);

    WorldMapState0Handler(&gb);
    /* Subtype was 0 -> State0 increments to 1 -> State1 increments to 2 */
    assert(gb_read(&gb, wGameplaySubtype) == 2);
    /* Verify palette copied to bank 3 */
    gb_write(&gb, rSVBK, 3);
    assert(gb_read(&gb, wBGPal1) == 0x77);
    assert(gb_read(&gb, wIsFileSelectionArrowShifted) == 1);
    gb_write(&gb, rSVBK, 0);
}

void test_move_select_and_jingle(void) {
    GBState gb;
    gb_init(&gb);

    playMoveSelectionJingle(&gb);
    assert(gb_read(&gb, hJingle) == JINGLE_MOVE_SELECTION);

    gb_init(&gb);
    gb_write(&gb, hJoypadState, J_LEFT);
    MoveSelect(&gb);
    assert(gb_read(&gb, hJingle) == 0);

    gb_write(&gb, hJoypadState, J_UP);
    MoveSelect(&gb);
    assert(gb_read(&gb, hJingle) == JINGLE_MOVE_SELECTION);
}

void test_label_001_5B3F(void) {
    GBState gb;
    gb_init(&gb);

    /* Room 0x35: Y=3, X=5 -> e = 3*8 + 0x14 = 0x2C, d = 5*8 + 0x14 = 0x3C */
    gb_write(&gb, wDBB4, 0x35);
    gb_write(&gb, hFrameCounter, 0x00); /* Bit 4 = 0 -> render arrows */

    label_001_5B3F(&gb);

    uint16_t oam = (uint16_t)(wDynamicOAMBuffer + 0x50);
    assert(gb_read(&gb, oam + 0) == 0x2C);
    assert(gb_read(&gb, oam + 1) == 0x3C);
    assert(gb_read(&gb, oam + 2) == 0xF0);
    assert(gb_read(&gb, oam + 3) == 0x00);

    assert(gb_read(&gb, oam + 4) == 0x2C);
    assert(gb_read(&gb, oam + 5) == 0x44);
    assert(gb_read(&gb, oam + 6) == 0xF0);
    assert(gb_read(&gb, oam + 7) == 0x20);

    /* Arrows present */
    uint16_t arrow = (uint16_t)(wDynamicOAMBuffer + 0x58);
    assert(gb_read(&gb, arrow + 0) == (uint8_t)(0x2C + 4));
    assert(gb_read(&gb, arrow + 1) == (uint8_t)(0x3C + 0xF6));
    assert(gb_read(&gb, arrow + 2) == 0xF6);

    /* Test bit 4 = 1 -> skip arrows */
    gb_init(&gb);
    gb_write(&gb, wDBB4, 0x35);
    gb_write(&gb, hFrameCounter, 0x10);
    label_001_5B3F(&gb);
    assert(gb_read(&gb, oam + 0) == 0x2C);
    assert(gb_read(&gb, arrow + 0) == 0x00);
}

void test_func_001_5A71(void) {
    GBState gb;
    gb_init(&gb);

    /* Case 1: Dialog active -> cursor does not move */
    gb_write(&gb, wDBB4, 0x10);
    gb_write(&gb, wDialogState, 1);
    gb_write(&gb, hPressedButtonsMask, J_RIGHT);
    func_001_5A71(&gb);
    assert(gb_read(&gb, wDBB4) == 0x10);

    /* Case 2: Move right to unvisited room -> blocked by fog of war, plays bump */
    gb_init(&gb);
    gb_write(&gb, wDBB4, 0x10);
    gb_write(&gb, (uint16_t)(wOverworldRoomStatus + 0x10), 1);
    gb_write(&gb, (uint16_t)(wOverworldRoomStatus + 0x11), 0); /* unvisited */
    gb_write(&gb, hPressedButtonsMask, J_RIGHT);
    gb_write(&gb, hJoypadState, J_RIGHT);
    func_001_5A71(&gb);
    assert(gb_read(&gb, wDBB4) == 0x10); /* restored */
    assert(gb_read(&gb, hJingle) == JINGLE_BUMP);

    /* Case 3: Move right to visited room -> allowed, plays selection jingle */
    gb_init(&gb);
    gb_write(&gb, wDBB4, 0x10);
    gb_write(&gb, (uint16_t)(wOverworldRoomStatus + 0x11), 1); /* visited */
    gb_write(&gb, hPressedButtonsMask, J_RIGHT);
    gb_write(&gb, hJoypadState, J_RIGHT);
    func_001_5A71(&gb);
    assert(gb_read(&gb, wDBB4) == 0x11);
    assert(gb_read(&gb, hJingle) == JINGLE_MOVE_SELECTION);

    /* Case 4: Free movement mode allows unvisited room */
    gb_init(&gb);
    gb_write(&gb, wDBB4, 0x10);
    gb_write(&gb, wFreeMovementMode, 1);
    gb_write(&gb, hPressedButtonsMask, J_RIGHT);
    gb_write(&gb, hJoypadState, J_RIGHT);
    func_001_5A71(&gb);
    assert(gb_read(&gb, wDBB4) == 0x11);
    assert(gb_read(&gb, hJingle) == JINGLE_MOVE_SELECTION);

    /* Case 5: Auto-repeat counter test */
    gb_init(&gb);
    gb_write(&gb, wDBB4, 0x20);
    gb_write(&gb, wFreeMovementMode, 1);
    gb_write(&gb, wC182, 0x17);
    gb_write(&gb, hPressedButtonsMask, J_RIGHT);
    func_001_5A71(&gb);
    assert(gb_read(&gb, wC182) == 0x15); /* loops back to 0x15 */
}

void test_func_001_5C49_and_5C55(void) {
    GBState gb;
    gb_init(&gb);

    /* Test 1: Preservation of wEntitiesPhysicsFlagsTable */
    gb_write(&gb, wEntitiesPhysicsFlagsTable, 0x42);
    gb_write(&gb, wC1B1, 2); /* Dungeon icon */
    gb_write(&gb, wC1B2, 0);
    gb_write(&gb, wC1B3, 0);
    gb_write(&gb, wC1B4, 0x12); /* Room (1, 2) -> Top-right corner (X=0x78, Y=0x28) */

    func_001_5C49(&gb);
    assert(gb_read(&gb, wEntitiesPhysicsFlagsTable) == 0x42);
    assert(gb_read(&gb, hActiveEntityPosX) == 0x78);
    assert(gb_read(&gb, hActiveEntityVisualPosY) == 0x28);

    /* Check written OAM sprite tiles for dungeon icon (variant 1: tiles 0x24, 0x26) */
    assert(gb_read(&gb, wDynamicOAMBuffer + 2) == 0x24);
    assert(gb_read(&gb, wDynamicOAMBuffer + 6) == 0x26);

    /* Test 2: Room (8, 9) -> Bottom-left corner (X=0x28, Y=0x78) */
    gb_init(&gb);
    gb_write(&gb, wC1B1, 1); /* Owl icon */
    gb_write(&gb, wC1B4, 0x89);
    func_001_5C49(&gb);
    assert(gb_read(&gb, hActiveEntityPosX) == 0x28);
    assert(gb_read(&gb, hActiveEntityVisualPosY) == 0x78);
}

void test_world_map_interactive_and_entry_point(void) {
    GBState gb;
    gb_init(&gb);

    /* Test 1a: Interactive handler with A button on Bottle Grotto (room 0x24) -> opens dialog 0x57 */
    gb_write(&gb, wDBB4, 0x24);
    gb_write(&gb, hJoypadState, J_A);
    WorldMapInteractiveHandler(&gb);
    assert(gb_read(&gb, wDialogIndex) == 0x57);
    assert(gb_read(&gb, wDialogState) == 0x81); /* room < 0x70 -> 0x81 */

    /* Test 1b: Interactive handler with A button on Goponga Swamp (room 0x34) -> opens dialog 0x76 */
    gb_init(&gb);
    gb_write(&gb, wDBB4, 0x34);
    gb_write(&gb, hJoypadState, J_A);
    WorldMapInteractiveHandler(&gb);
    assert(gb_read(&gb, wDialogIndex) == 0x76);
    assert(gb_read(&gb, wDialogState) == 0x81);

    /* Test 2: Interactive handler with B button -> exits map (advances subtype) */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 5);
    gb_write(&gb, hJoypadState, J_B);
    WorldMapInteractiveHandler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 6);
    assert(gb_read(&gb, wPaletteUnknownE) == 1);

    /* Test 3: WorldMapEntryPoint when subtype != 5 clears inputs */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    gb_write(&gb, hJoypadState, J_B);
    gb_write(&gb, hPressedButtonsMask, J_B);
    WorldMapEntryPoint(&gb);
    assert(gb_read(&gb, hJoypadState) == 0);
    assert(gb_read(&gb, hPressedButtonsMask) == 0);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_WORLD_MAP);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}
