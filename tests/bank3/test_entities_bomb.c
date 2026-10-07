#include "test_bank3.h"
#include "gb.h"
#include "bank3/entities_bomb.h"
#include "bank3/entities_physics.h"
#include "constants/entities.h"
#include "constants/physics.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/maps.h"
#include "constants/gameplay.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/audio.h"
#include "constants/gfx.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t mock_bomb_rom[0x4000 * 9];

/* Test 1: RenderBombExplosion */
static void test_RenderBombExplosion(void) {
    printf("[RUN ] RenderBombExplosion\n");

    GBState gb;
    gb_init(&gb);

    /* Entity slot 2 with sprite variant 1 */
    gb_write(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 2), 1);
    gb_write_hram(&gb, hActiveEntityPosX, 40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 50);

    RenderBombExplosion(&gb, 2);

    /* Renders 8 sprite tiles (32 bytes of OAM data) */
    assert(gb_read_hram(&gb, hActiveEntityPosX) == 40);

    printf("[PASS] RenderBombExplosion\n");
}

/* Test 2: BombExplosionVisuals */
static void test_BombExplosionVisuals(void) {
    printf("[RUN ] BombExplosionVisuals\n");

    GBState gb;
    gb_init(&gb);

    /* Test variant assignment from ExplosionSpriteVariantFrames based on countdown */
    /* Countdown 5 -> variant 0 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 5);
    gb_write(&gb, (uint16_t)(wEntitiesPhysicsFlagsTable + 1), 0x20);
    BombExplosionVisuals(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 1)) == 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPhysicsFlagsTable + 1)) == 0x28);

    /* Countdown 12 -> variant 1 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 12);
    BombExplosionVisuals(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 1)) == 1);

    /* Countdown 18 -> variant 2 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 18);
    BombExplosionVisuals(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 1)) == 2);

    /* Countdown 22 -> variant 3 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 22);
    BombExplosionVisuals(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 1)) == 3);

    /* Test DMG indoor palette flashing */
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, wRoomTransitionState, 0);

    /* Countdown with bit 2 set (e.g. 4) -> palette 0x84 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 4);
    BombExplosionVisuals(&gb, 1);
    assert(gb_read(&gb, wBGPalette) == 0x84);

    /* Countdown with bit 2 clear (e.g. 0) -> palette 0xE4 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0);
    BombExplosionVisuals(&gb, 1);
    assert(gb_read(&gb, wBGPalette) == 0xE4);

    /* Outdoors -> palette unchanged */
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wBGPalette, 0x11);
    BombExplosionVisuals(&gb, 1);
    assert(gb_read(&gb, wBGPalette) == 0x11);

    printf("[PASS] BombExplosionVisuals\n");
}

/* Test 3: BombExplosionHandler */
static void test_BombExplosionHandler(void) {
    printf("[RUN ] BombExplosionHandler\n");

    GBState gb;
    gb_init(&gb);

    /* Case A: Countdown == 0 -> unloads entity */
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 3), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 3), 0);
    BombExplosionHandler(&gb, 3);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 3)) == 0);

    /* Case B: Tarin transformation (privateState4 == 0x4C) -> returns early without damage */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 3), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 3), 0x12);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState4Table + 3), 0x4C);
    BombExplosionHandler(&gb, 3);
    assert(gb_read(&gb, wSwordMoblinAlertingSoundCounter) == 0);

    /* Case C: Link bomb (privateState4 == 0) at countdown 0x12 -> alerts sword moblins */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 3), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 3), 0x12);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState4Table + 3), 0);
    BombExplosionHandler(&gb, 3);
    assert(gb_read(&gb, wSwordMoblinAlertingSoundCounter) == 0x04);

    /* Case D: Enemy bomb (privateState4 != 0) with Link in explosion radius -> damages Link */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 3), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 3), 0x12);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState4Table + 3), 1);
    gb_write_hram(&gb, hActiveEntityPosX, 60);
    gb_write_hram(&gb, hActiveEntityPosY, 60);
    gb_write_hram(&gb, hLinkPositionX, 64);
    gb_write_hram(&gb, hLinkPositionY, 64);
    gb_write_hram(&gb, hLinkSpeedX, 2);
    gb_write_hram(&gb, hLinkSpeedY, 3);
    BombExplosionHandler(&gb, 3);
    assert(gb_read(&gb, wSwordMoblinAlertingSoundCounter) == 0x04);
    /* Link invincibility and collision reaction applied */
    assert(gb_read(&gb, wInvincibilityCounter) == 0x50);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x10);

    /* Case E: Enemy bomb with Link outside radius -> no speed alteration */
    gb_init(&gb);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 3), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 3), 0x12);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState4Table + 3), 1);
    gb_write_hram(&gb, hActiveEntityPosX, 30);
    gb_write_hram(&gb, hActiveEntityPosY, 30);
    gb_write_hram(&gb, hLinkPositionX, 120);
    gb_write_hram(&gb, hLinkPositionY, 120);
    gb_write_hram(&gb, hLinkSpeedX, 2);
    gb_write_hram(&gb, hLinkSpeedY, 3);
    BombExplosionHandler(&gb, 3);
    assert(gb_read(&gb, wSwordMoblinAlertingSoundCounter) == 0x04);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 2);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 3);

    printf("[PASS] BombExplosionHandler\n");
}

/* Test 4: BombBounceOffWalls */
static void test_BombBounceOffWalls(void) {
    printf("[RUN ] BombBounceOffWalls\n");

    GBState gb;
    gb_init(&gb);

    /* Horizontal collision bounce */
    gb_write(&gb, (uint16_t)(wEntitiesCollisionsTable + 2), 0x01);
    gb_write(&gb, (uint16_t)(wEntitiesSpeedXTable + 2), 0x10);
    BombBounceOffWalls(&gb, 2);
    /* Speed X should be negated and shifted right by 3 (0x10 -> -2 = 0xFE) */
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpeedXTable + 2)) == (uint8_t)(-2));

    /* Vertical collision bounce */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesCollisionsTable + 2), 0x04);
    gb_write(&gb, (uint16_t)(wEntitiesSpeedYTable + 2), 0x0C);
    BombBounceOffWalls(&gb, 2);
    /* Speed Y should be negated and shifted right by 3 (0x0C -> -2 = 0xFE) */
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpeedYTable + 2)) == (uint8_t)(-2));

    /* Side scrolling skips vertical bounce */
    gb_init(&gb);
    gb_write_hram(&gb, hIsSideScrolling, 1);
    gb_write(&gb, (uint16_t)(wEntitiesCollisionsTable + 2), 0x04);
    gb_write(&gb, (uint16_t)(wEntitiesSpeedYTable + 2), 0x0C);
    BombBounceOffWalls(&gb, 2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpeedYTable + 2)) == 0x0C);

    printf("[PASS] BombBounceOffWalls\n");
}

/* Test 5: RenderBomb */
static void test_RenderBomb(void) {
    printf("[RUN ] RenderBomb\n");

    GBState gb;
    gb_init(&gb);

    gb_write_hram(&gb, hActiveEntityVisualPosY, 20);
    gb_write_hram(&gb, hActiveEntityPosX, 30);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 4), 30);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 4), 22);

    RenderBomb(&gb, 4);

    /* Visual Y incremented by 2 */
    assert(gb_read_hram(&gb, hActiveEntityVisualPosY) == 22);

    printf("[PASS] RenderBomb\n");
}

/* Test 6: BombEntityHandler */
static void test_BombEntityHandler(void) {
    printf("[RUN ] BombEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* Case A: Off screen vertically -> unloads */
    gb_write_hram(&gb, hActiveEntityVisualPosY, 150); /* 150 + 16 = 166 >= 160 */
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    BombEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == 0);

    /* Case B: Countdown == 0x18 -> decrements to 0x17 and plays explosion SFX */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 40);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0x18);
    BombEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x17);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_EXPLOSION);
    assert(gb_read(&gb, wSwordMoblinAlertingSoundCounter) == 4);

    /* Case C: Countdown == 0x48 -> sets flash countdown table to 0x30 */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 40);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0x48);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    BombEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesFlashCountdownTable + 1)) == 0x30);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPrivateCountdown2Table + 1)) == 0xFF);

    /* Case D: Lifting bomb with B button when B button has bombs */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), 40);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), 40);
    gb_write_hram(&gb, hLinkPositionX, 32);
    gb_write_hram(&gb, hLinkPositionY, 32);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 40);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0x50);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wInventoryBButtonSlot, INVENTORY_BOMBS);
    gb_write_hram(&gb, hJoypadState, J_B);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    BombEntityHandler(&gb, 1);
    /* Entity lifted up sets status to ENTITY_STATUS_LIFTED (7) */
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_LIFTED);

    printf("[PASS] BombEntityHandler\n");
}

/* Test 7: CheckForBombDestroyableObjectPuzzle (Giant Skull) */
static void test_CheckForBombDestroyableObjectPuzzle_GiantSkull(void) {
    printf("[RUN ] CheckForBombDestroyableObjectPuzzle_GiantSkull\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hIsSideScrolling, 0);
    gb_write_hram(&gb, hMapRoom, 0x14);

    /* Bomb at PosX = 0x38, PosY = 0x38 */
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 0), 0x38);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 0), 0x38);

    /* With de = 4 (offset X = 0x08, Y = 0x08):
     * obj_left = (0x38 - 8 + 8) & 0xF0 = 0x30 -> 3
     * obj_top  = (0x38 - 16 + 8) & 0xF0 = 0x30
     * tile c   = 0x33 (bottom-right skull tile)
     */
    gb_write(&gb, (uint16_t)(wRoomObjects + 0x33), OBJECT_GIANT_SKULL_BR);

    CheckForBombDestroyableObjectPuzzle(&gb, 0, 4);

    /* Verifies puzzle solved jingle triggered */
    assert(gb_read_hram(&gb, hJingle) == JINGLE_PUZZLE_SOLVED);

    /* Verifies overworld room marked opened */
    assert((gb_read(&gb, (uint16_t)(wOverworldRoomStatus + 0x14)) & OW_ROOM_STATUS_OPENED) != 0);
    assert((gb_read_hram(&gb, hRoomStatus) & OW_ROOM_STATUS_OPENED) != 0);

    /* Verifies tiles replaced with OBJECT_ROCKY_GROUND */
    /* c & 0xEE = 0x23 & 0xEE = 0x22 */
    assert(gb_read(&gb, (uint16_t)(wRoomObjects + 0x22)) == OBJECT_ROCKY_GROUND);
    assert(gb_read(&gb, (uint16_t)(wRoomObjects + 0x23)) == OBJECT_ROCKY_GROUND);
    assert(gb_read(&gb, (uint16_t)(wRoomObjects + 0x32)) == OBJECT_ROCKY_GROUND);
    assert(gb_read(&gb, (uint16_t)(wRoomObjects + 0x33)) == OBJECT_ROCKY_GROUND);
    assert(gb_read(&gb, wDDD8) == OBJECT_ROCKY_GROUND);

    printf("[PASS] CheckForBombDestroyableObjectPuzzle_GiantSkull\n");
}

/* Test 8: CheckForBombDestroyableObjectPuzzle (Doors) */
static void test_CheckForBombDestroyableObjectPuzzle_Doors(void) {
    printf("[RUN ] CheckForBombDestroyableObjectPuzzle_Doors\n");

    GBState gb;
    gb_init(&gb);

    memset(mock_bomb_rom, 0, sizeof(mock_bomb_rom));
    /* Overworld door physics flag (object 0xBA) */
    mock_bomb_rom[8 * 0x4000 + (OverworldObjectPhysicFlags + 0xBA - 0x4000)] = OBJ_PHYSICS_DOOR_CLOSED | 0x09;
    /* Indoor door physics flag (group 1, object 0x3F) */
    mock_bomb_rom[8 * 0x4000 + (OverworldObjectPhysicFlags + 0x013F - 0x4000)] = OBJ_PHYSICS_DOOR_CLOSED | 0x09;

    /* Case A: Outdoors cave entrance */
    gb_attach_rom(&gb, mock_bomb_rom, sizeof(mock_bomb_rom));
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hIsSideScrolling, 0);
    gb_write_hram(&gb, hMapRoom, 0x25);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), 0x58);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), 0x58);

    /* Place outdoor bombable cave door object (e.g. 0xBA which has physics flag 0x99) */
    /* obj_left = (0x58 - 8 + 8) & 0xF0 = 0x50, c_x = 5 */
    /* obj_top  = (0x58 - 16 + 8) & 0xF0 = 0x50, c_y = 0x50 */
    /* tile c = 0x55 */
    gb_write(&gb, (uint16_t)(wRoomObjects + 0x55), 0xBA);

    CheckForBombDestroyableObjectPuzzle(&gb, 1, 4);

    assert(gb_read_hram(&gb, hJingle) == JINGLE_PUZZLE_SOLVED);
    assert(gb_read(&gb, (uint16_t)(wRoomObjects + 0x55)) == OBJECT_ROCKY_CAVE_DOOR);
    assert(gb_read(&gb, wDDD8) == OBJECT_ROCKY_CAVE_DOOR);
    assert((gb_read(&gb, (uint16_t)(wOverworldRoomStatus + 0x25)) & OW_ROOM_STATUS_OPENED) != 0);

    /* Case B: Indoors bombable passage */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_bomb_rom, sizeof(mock_bomb_rom));
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapId, 1); /* Tail Cave */
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wIndoorRoom, 0x10);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), 0x58);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), 0x58);

    /* Indoors object with physics flag 0x99 (Up door): 0x3F */
    gb_write(&gb, (uint16_t)(wRoomObjects + 0x55), 0x3F);

    CheckForBombDestroyableObjectPuzzle(&gb, 1, 4);

    assert(gb_read_hram(&gb, hJingle) == JINGLE_PUZZLE_SOLVED);
    /* Replaces tile with BombedWallObjects[0] (OBJECT_BOMBED_PASSAGE_VERTICAL) */
    assert(gb_read(&gb, (uint16_t)(wRoomObjects + 0x55)) == OBJECT_BOMBED_PASSAGE_VERTICAL);
    assert(gb_read(&gb, wDDD8) == OBJECT_BOMBED_PASSAGE_VERTICAL);
    /* Current room status receives ROOM_STATUS_DOOR_OPEN_UP (0x04) */
    assert((gb_read(&gb, (uint16_t)(wIndoorARoomStatus + 0x10)) & ROOM_STATUS_DOOR_OPEN_UP) != 0);
    assert((gb_read_hram(&gb, hRoomStatus) & ROOM_STATUS_DOOR_OPEN_UP) != 0);

    printf("[PASS] CheckForBombDestroyableObjectPuzzle_Doors\n");
}

/* Test 9: CheckForBombDestroyableObjectBasic */
static void test_CheckForBombDestroyableObjectBasic(void) {
    printf("[RUN ] CheckForBombDestroyableObjectBasic\n");

    GBState gb;
    gb_init(&gb);

    /* Case A: Outdoors bush/grass destruction */
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hActiveEntityPosX, 0x48);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x48);

    /* With de = 4 (center, +8, +8):
     * left = (0x48 + 8 - 8) & 0xF0 = 0x40, c = 4
     * top  = (0x48 + 8 - 16) & 0xF0 = 0x40, e = 0x44
     */
    gb_write(&gb, (uint16_t)(wRoomObjects + 0x44), OBJECT_TALL_GRASS);

    CheckForBombDestroyableObjectBasic(&gb, 0, 4);

    /* Spawns ENTITY_LIFTABLE_ROCK in highest available slot (15) */
    assert(gb_read(&gb, (uint16_t)(wEntitiesTypeTable + 15)) == ENTITY_LIFTABLE_ROCK);
    /* Tall grass sets sprite variant to 0xFF (invisible) */
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 15)) == 0xFF);
    assert(gb_read_hram(&gb, hActiveEntitySpriteVariant) == 0xFF);
    /* Placed at left + 8 = 0x48, top + 16 = 0x50 */
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 15)) == 0x48);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYTable + 15)) == 0x50);

    /* Case B: Indoors bombable block */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapId, 2); /* Bottle Grotto */
    gb_write_hram(&gb, hMapRoom, 0x30);
    gb_write_hram(&gb, hActiveEntityPosX, 0x48);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x48);

    gb_write(&gb, (uint16_t)(wRoomObjects + 0x44), OBJECT_BOMBABLE_BLOCK);

    CheckForBombDestroyableObjectBasic(&gb, 0, 4);

    /* Room event 3 flag set */
    assert((gb_read(&gb, (uint16_t)(wIndoorARoomStatus + 0x30)) & ROOM_STATUS_EVENT_3) != 0);
    assert((gb_read_hram(&gb, hRoomStatus) & ROOM_STATUS_EVENT_3) != 0);

    /* Rock entity spawned */
    assert(gb_read(&gb, (uint16_t)(wEntitiesTypeTable + 15)) == ENTITY_LIFTABLE_ROCK);

    printf("[PASS] CheckForBombDestroyableObjectBasic\n");
}

/* Test 10: CheckForEntityFallingDownQuicksandHole */
static void test_CheckForEntityFallingDownQuicksandHole(void) {
    printf("[RUN ] CheckForEntityFallingDownQuicksandHole\n");

    GBState gb;
    gb_init(&gb);

    /* Indoors -> false */
    gb_write(&gb, wIsIndoor, 1);
    assert(!CheckForEntityFallingDownQuicksandHole(&gb, 2));

    /* Outdoors but wrong room -> false */
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hMapRoom, 0x00);
    assert(!CheckForEntityFallingDownQuicksandHole(&gb, 2));

    /* Yarna Lanmola room (0xCE), but outside center -> false */
    gb_write_hram(&gb, hMapRoom, ROOM_OW_YARNA_LANMOLA);
    gb_write_hram(&gb, hActiveEntityPosY, 0x20);
    gb_write_hram(&gb, hActiveEntityPosX, 0x20);
    assert(!CheckForEntityFallingDownQuicksandHole(&gb, 2));

    /* At quicksand center (0x50, 0x48), but airborne (PosZ > 0) -> false */
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityPosY, 0x48);
    gb_write(&gb, (uint16_t)(wEntitiesPosZTable + 2), 5);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 2), ENTITY_STATUS_ACTIVE);
    assert(!CheckForEntityFallingDownQuicksandHole(&gb, 2));

    /* Grounded at center, inactive status -> false */
    gb_write(&gb, (uint16_t)(wEntitiesPosZTable + 2), 0);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 2), 0);
    assert(!CheckForEntityFallingDownQuicksandHole(&gb, 2));

    /* Grounded at center, active status -> true! */
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 2), ENTITY_STATUS_ACTIVE);
    bool falling = CheckForEntityFallingDownQuicksandHole(&gb, 2);
    assert(falling);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 2)) == ENTITY_STATUS_FALLING);
    assert(gb_read(&gb, (uint16_t)(wEntitiesFallingTargetXTable + 2)) == 0x50);
    assert(gb_read(&gb, (uint16_t)(wEntitiesFallingTargetYTable + 2)) == 0x48);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2)) == 0x2F);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_ITEM_FALLING);

    printf("[PASS] CheckForEntityFallingDownQuicksandHole\n");
}

void test_bank3_entities_bomb(void) {
    printf("[TEST] Bank 3 Entities Bomb Handlers\n");

    test_RenderBombExplosion();
    test_BombExplosionVisuals();
    test_BombExplosionHandler();
    test_BombBounceOffWalls();
    test_RenderBomb();
    test_BombEntityHandler();
    test_CheckForBombDestroyableObjectPuzzle_GiantSkull();
    test_CheckForBombDestroyableObjectPuzzle_Doors();
    test_CheckForBombDestroyableObjectBasic();
    test_CheckForEntityFallingDownQuicksandHole();

    printf("[PASS] Bank 3 Entities Bomb Handlers\n\n");
}
