#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "gb.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_collision.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/directions.h"
#include "constants/gameplay.h"
#include "constants/sfx.h"
#include "constants/audio.h"
#include "constants/physics.h"
#include "constants/gfx.h"
#include "constants/rooms.h"
#include "bank3/entities_bomb.h"
#include "bank3/entities_pushed_block.h"
#include "home/entities.h"
#include "home/link.h"

static uint8_t mock_physics_rom[0x4000 * 0x09];
#define ROM_BANK_8_OFFSET(addr) ((0x08 * 0x4000) + ((addr) - 0x4000))

static void init_mock_physics_rom(GBState *gb) {
    memset(mock_physics_rom, 0, sizeof(mock_physics_rom));
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x21)] = OBJ_PHYSICS_SOLID;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x00)] = OBJ_PHYSICS_NONE;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x05)] = OBJ_PHYSICS_SHALLOW_WATER;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x07)] = OBJ_PHYSICS_DEEP_WATER;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x50)] = OBJ_PHYSICS_PIT;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + OBJECT_LOWERED_BLOCK)] = OBJ_PHYSICS_OCEAN_SWITCH_BLOCK;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + (OBJ_PHYSICS_CONVEYOR + 3))] = (uint8_t)(OBJ_PHYSICS_CONVEYOR + 3);
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x30)] = 0xD1;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x31)] = 0xFF;
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + OBJECT_TORCH_UNLIT)] = OBJ_PHYSICS_SOLID;
    gb_attach_rom(gb, mock_physics_rom, sizeof(mock_physics_rom));
}

/* Test GetEntityXDistanceToLink_03 */
static void test_GetEntityXDistanceToLink(void) {
    printf("[RUN ] GetEntityXDistanceToLink_03\n");

    GBState gb;
    gb_init(&gb);

    /* Entity 2 at X = 0x40 */
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x40);

    uint8_t dir = 0xFF;
    uint8_t diff = 0xFF;

    /* Case 1: Link is to the right (X = 0x60) */
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    GetEntityXDistanceToLink_03(&gb, &dir, &diff);
    assert(dir == DIRECTION_RIGHT);
    assert(diff == 0x20);

    /* Case 2: Link is to the left (X = 0x20) */
    gb_write_hram(&gb, hLinkPositionX, 0x20);
    GetEntityXDistanceToLink_03(&gb, &dir, &diff);
    assert(dir == DIRECTION_LEFT);
    assert(diff == 0xE0); /* (uint8_t)(0x20 - 0x40) = 0xE0 */

    /* Case 3: Link is at same position (X = 0x40) */
    gb_write_hram(&gb, hLinkPositionX, 0x40);
    GetEntityXDistanceToLink_03(&gb, &dir, &diff);
    assert(dir == DIRECTION_RIGHT);
    assert(diff == 0x00);

    /* Case 4: Test _idx variant with entity 5 at X = 0x80 */
    gb_write(&gb, wEntitiesPosXTable + 0x05, 0x80);
    gb_write_hram(&gb, hLinkPositionX, 0x90);
    GetEntityXDistanceToLink_03_idx(&gb, 0x05, &dir, &diff);
    assert(dir == DIRECTION_RIGHT);
    assert(diff == 0x10);

    printf("[PASS] GetEntityXDistanceToLink_03\n");
}

/* Test GetEntityYDistanceToLink_03 */
static void test_GetEntityYDistanceToLink(void) {
    printf("[RUN ] GetEntityYDistanceToLink_03\n");

    GBState gb;
    gb_init(&gb);

    /* Entity 3 at Y = 0x50, Z = 0 */
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wEntitiesPosYTable + 0x03, 0x50);
    gb_write(&gb, wEntitiesPosZTable + 0x03, 0x00);

    uint8_t dir = 0xFF;
    uint8_t diff = 0xFF;

    /* Case 1: Link is below (Y = 0x70) */
    gb_write_hram(&gb, hLinkPositionY, 0x70);
    GetEntityYDistanceToLink_03(&gb, &dir, &diff);
    assert(dir == DIRECTION_DOWN);
    assert(diff == 0x20);

    /* Case 2: Link is above (Y = 0x30) */
    gb_write_hram(&gb, hLinkPositionY, 0x30);
    GetEntityYDistanceToLink_03(&gb, &dir, &diff);
    assert(dir == DIRECTION_UP);
    assert(diff == 0xE0); /* (uint8_t)(0x30 - 0x50) = 0xE0 */

    /* Case 3: Link at same Y (Y = 0x50) */
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    GetEntityYDistanceToLink_03(&gb, &dir, &diff);
    assert(dir == DIRECTION_DOWN);
    assert(diff == 0x00);

    /* Case 4: With entity altitude Z = 0x08 */
    /* link_y = 0x4A, ent_y = 0x50, ent_z = 0x08 -> diff = (0x4A - 0x50) + 0x08 = 0xFA + 0x08 = 0x02 */
    gb_write_hram(&gb, hLinkPositionY, 0x4A);
    gb_write(&gb, wEntitiesPosZTable + 0x03, 0x08);
    GetEntityYDistanceToLink_03(&gb, &dir, &diff);
    assert(dir == DIRECTION_DOWN);
    assert(diff == 0x02);

    printf("[PASS] GetEntityYDistanceToLink_03\n");
}

/* Test GetEntityDirectionToLink_03 */
static void test_GetEntityDirectionToLink(void) {
    printf("[RUN ] GetEntityDirectionToLink_03\n");

    GBState gb;
    gb_init(&gb);

    /* Entity 1 at (0x40, 0x40) */
    gb_write(&gb, wActiveEntityIndex, 0x01);
    gb_write(&gb, wEntitiesPosXTable + 0x01, 0x40);
    gb_write(&gb, wEntitiesPosYTable + 0x01, 0x40);
    gb_write(&gb, wEntitiesPosZTable + 0x01, 0x00);

    /* Case 1: Link is to the right (dx = +0x30, dy = +0x10) */
    gb_write_hram(&gb, hLinkPositionX, 0x70);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    uint8_t dir = GetEntityDirectionToLink_03(&gb);
    assert(dir == DIRECTION_RIGHT);
    assert(gb_read_hram(&gb, hMultiPurpose0) == DIRECTION_RIGHT);
    assert(gb_read_hram(&gb, hMultiPurpose1) == DIRECTION_DOWN);

    /* Case 2: Link is to the left (dx = -0x30, dy = -0x10) */
    gb_write_hram(&gb, hLinkPositionX, 0x10);
    gb_write_hram(&gb, hLinkPositionY, 0x30);
    dir = GetEntityDirectionToLink_03(&gb);
    assert(dir == DIRECTION_LEFT);

    /* Case 3: Link is predominantly down (dx = +0x10, dy = +0x30) */
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x70);
    dir = GetEntityDirectionToLink_03(&gb);
    assert(dir == DIRECTION_DOWN);

    /* Case 4: Link is predominantly up (dx = -0x10, dy = -0x30) */
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x10);
    dir = GetEntityDirectionToLink_03(&gb);
    assert(dir == DIRECTION_UP);

    /* Case 5: 45-degree diagonal (dx = 0x20, dy = 0x20) -> vertical priority per assembly */
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    gb_write_hram(&gb, hLinkPositionY, 0x60);
    dir = GetEntityDirectionToLink_03(&gb);
    assert(dir == DIRECTION_DOWN);

    printf("[PASS] GetEntityDirectionToLink_03\n");
}

/* Test GetVectorTowardsLink and ApplyVectorTowardsLink */
static void test_GetVectorTowardsLink(void) {
    printf("[RUN ] GetVectorTowardsLink\n");

    GBState gb;
    gb_init(&gb);

    /* Entity 0 at (0x40, 0x40) */
    gb_write(&gb, wActiveEntityIndex, 0x00);
    gb_write(&gb, wEntitiesPosXTable, 0x40);
    gb_write(&gb, wEntitiesPosYTable, 0x40);
    gb_write(&gb, wEntitiesPosZTable, 0x00);

    /* Case 1: Zero length cancel */
    uint8_t vx = 0xFF, vy = 0xFF;
    GetVectorTowardsLink_with_length(&gb, 0x00, &vy, &vx);
    assert(vy == 0x00);
    assert(vx == 0x00);
    assert(gb_read_hram(&gb, hMultiPurpose0) == 0x00);

    /* Case 2: Pure horizontal right (dx = 0x20, dy = 0) */
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    GetVectorTowardsLink_with_length(&gb, 0x18, &vy, &vx);
    assert(vx == 0x18);
    assert(vy == 0x00);

    /* Case 3: Pure horizontal left (dx = -0x20, dy = 0) */
    gb_write_hram(&gb, hLinkPositionX, 0x20);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    GetVectorTowardsLink_with_length(&gb, 0x18, &vy, &vx);
    assert(vx == (uint8_t)(-0x18));
    assert(vy == 0x00);

    /* Case 4: Pure vertical down (dx = 0, dy = 0x20) */
    gb_write_hram(&gb, hLinkPositionX, 0x40);
    gb_write_hram(&gb, hLinkPositionY, 0x60);
    GetVectorTowardsLink_with_length(&gb, 0x10, &vy, &vx);
    assert(vx == 0x00);
    assert(vy == 0x10);

    /* Case 5: Pure vertical up (dx = 0, dy = -0x20) */
    gb_write_hram(&gb, hLinkPositionX, 0x40);
    gb_write_hram(&gb, hLinkPositionY, 0x20);
    GetVectorTowardsLink_with_length(&gb, 0x10, &vy, &vx);
    assert(vx == 0x00);
    assert(vy == (uint8_t)(-0x10));

    /* Case 6: 45-degree angle (dx = 0x20, dy = 0x20, length = 0x10) */
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    gb_write_hram(&gb, hLinkPositionY, 0x60);
    GetVectorTowardsLink_with_length(&gb, 0x10, &vy, &vx);
    assert(vx == 0x10);
    assert(vy == 0x10);

    /* Case 7: ApplyVectorTowardsLink */
    ApplyVectorTowardsLink(&gb, 0x00);
    assert(gb_read(&gb, wEntitiesSpeedXTable) == 0x10);
    assert(gb_read(&gb, wEntitiesSpeedYTable) == 0x10);

    printf("[PASS] GetVectorTowardsLink\n");
}

/* Test AddEntitySpeedToPos_03 */
static void test_AddEntitySpeedToPos(void) {
    printf("[RUN ] AddEntitySpeedToPos_03\n");

    GBState gb;
    gb_init(&gb);

    /* Setup entity 1 */
    uint16_t bc = 0x01;
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);
    gb_write(&gb, wEntitiesSpeedXAccTable + bc, 0x00);

    /* Case 1: Speed = 0 does nothing */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x00);
    AddEntitySpeedToPos_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x40);
    assert(gb_read(&gb, wEntitiesSpeedXAccTable + bc) == 0x00);

    /* Case 2: Speed = 0x08 (+0.5 px/frame) */
    /* Frame 1: frac = 0x80, acc becomes 0x80, carry = 0, int_part = 0 -> pos = 0x40 */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x08);
    AddEntitySpeedToPos_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedXAccTable + bc) == 0x80);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x40);

    /* Frame 2: frac = 0x80, acc overflows to 0x00, carry = 1, int_part = 0 -> pos = 0x41 */
    AddEntitySpeedToPos_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedXAccTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x41);

    /* Case 3: Speed = 0xF0 (-1.0 px/frame = -16 sixteenths) */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0xF0);
    AddEntitySpeedToPos_03(&gb, bc);
    /* frac = 0x00, acc remains 0x00, carry = 0, int_part = 0xFF (-1) -> pos = 0x40 */
    assert(gb_read(&gb, wEntitiesSpeedXAccTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x40);

    printf("[PASS] AddEntitySpeedToPos_03\n");
}

/* Test AddEntityZSpeedToPos_03 */
static void test_AddEntityZSpeedToPos(void) {
    printf("[RUN ] AddEntityZSpeedToPos_03\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x02;
    gb_write(&gb, wEntitiesPosZTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedZAccTable + bc, 0x00);

    /* Case 1: Speed = 0 does nothing */
    gb_write(&gb, wEntitiesSpeedZTable + bc, 0x00);
    AddEntityZSpeedToPos_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesPosZTable + bc) == 0x10);

    /* Case 2: Speed = 0x10 (+1.0 px/frame) */
    gb_write(&gb, wEntitiesSpeedZTable + bc, 0x10);
    AddEntityZSpeedToPos_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesPosZTable + bc) == 0x11);

    /* Case 3: Speed = 0xF0 (-1.0 px/frame) */
    gb_write(&gb, wEntitiesSpeedZTable + bc, 0xF0);
    AddEntityZSpeedToPos_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesPosZTable + bc) == 0x10);

    printf("[PASS] AddEntityZSpeedToPos_03\n");
}

/* Test UpdateEntityPosWithSpeed_03 */
static void test_UpdateEntityPosWithSpeed(void) {
    printf("[RUN ] UpdateEntityPosWithSpeed_03\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x03;
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write(&gb, wEntitiesSpeedXAccTable + bc, 0x00);
    gb_write(&gb, wEntitiesSpeedYAccTable + bc, 0x00);

    /* Set speed X = +1 px (0x10), speed Y = -1 px (0xF0) */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0xF0);

    UpdateEntityPosWithSpeed_03(&gb, bc);

    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x21);
    assert(gb_read(&gb, wEntitiesPosYTable + bc) == 0x2F);

    printf("[PASS] UpdateEntityPosWithSpeed_03\n");
}

/* Test StartIgnoringHitsForEntity and ConfigureEntityRecoil */
static void test_RecoilAndIgnoringHits(void) {
    printf("[RUN ] ConfigureEntityRecoil & StartIgnoringHitsForEntity\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x04;
    gb_write(&gb, wActiveEntityIndex, bc);

    /* Test StartIgnoringHitsForEntity */
    gb_write(&gb, wEntitiesPowerRecoilingTable + bc, 0xFF);
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x00);

    StartIgnoringHitsForEntity(&gb);

    assert(gb_read(&gb, wEntitiesPowerRecoilingTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x0A);

    /* Test ConfigureEntityRecoil */
    /* Position Link right and below entity: (dx = 0x20, dy = 0x20) */
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x50);

    ConfigureEntityRecoil(&gb, bc, 0x20);

    /* Vector towards link is (+0x20, +0x20). Recoil should be negated: (-0x20, -0x20) = (0xE0, 0xE0) */
    assert(gb_read(&gb, wEntitiesRecoilVelocityX + bc) == 0xE0);
    assert(gb_read(&gb, wEntitiesRecoilVelocityY + bc) == 0xE0);
    assert(gb_read(&gb, wEntitiesPowerRecoilingTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x0A);

    printf("[PASS] ConfigureEntityRecoil & StartIgnoringHitsForEntity\n");
}

/* Test func_003_6C6B */
static void test_func_003_6C6B(void) {
    printf("[RUN ] func_003_6C6B\n");

    GBState gb;
    gb_init(&gb);

    /* (frame ^ c) & 1 == 0 -> returns false */
    gb_write_hram(&gb, hFrameCounter, 0x04);
    assert(func_003_6C6B(&gb, 0x02) == false); /* (4 ^ 2) = 6, bit 0 is 0 */

    /* (frame ^ c) & 1 == 1 -> returns true */
    gb_write_hram(&gb, hFrameCounter, 0x05);
    assert(func_003_6C6B(&gb, 0x02) == true);  /* (5 ^ 2) = 7, bit 0 is 1 */

    gb_write_hram(&gb, hFrameCounter, 0x00);
    assert(func_003_6C6B(&gb, 0x01) == true);  /* (0 ^ 1) = 1, bit 0 is 1 */
    assert(func_003_6C6B(&gb, 0x00) == false); /* (0 ^ 0) = 0, bit 0 is 0 */

    printf("[PASS] func_003_6C6B\n");
}

/* Test func_003_6CC0 */
static void test_func_003_6CC0(void) {
    printf("[RUN ] func_003_6CC0\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x03;

    /* Case 1: Harmless entity flag set -> returns true */
    gb_write(&gb, wEntitiesPhysicsFlagsTable + bc, ENTITY_PHYSICS_HARMLESS);
    gb_write_hram(&gb, hLinkAnimationState, 0x00);
    assert(func_003_6CC0(&gb, bc) == true);

    /* Case 2: Harmless flag clear, Link falling into pit (state 0x4E or 0x4F) -> returns true */
    gb_write(&gb, wEntitiesPhysicsFlagsTable + bc, 0x00);
    gb_write_hram(&gb, hLinkAnimationState, 0x4E);
    assert(func_003_6CC0(&gb, bc) == true);
    gb_write_hram(&gb, hLinkAnimationState, 0x4F);
    assert(func_003_6CC0(&gb, bc) == true);

    /* Case 3: Harmless flag clear, normal Link animation (e.g. 0x00 or 0x50) -> returns false */
    gb_write_hram(&gb, hLinkAnimationState, 0x00);
    assert(func_003_6CC0(&gb, bc) == false);
    gb_write_hram(&gb, hLinkAnimationState, 0x4D);
    assert(func_003_6CC0(&gb, bc) == false);
    gb_write_hram(&gb, hLinkAnimationState, 0x50);
    assert(func_003_6CC0(&gb, bc) == false);

    printf("[PASS] func_003_6CC0\n");
}

/* Test label_003_6FA7 */
static void test_label_003_6FA7(void) {
    printf("[RUN ] label_003_6FA7\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x01;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);

    /* Link to the right (X = 0x60) -> speed_x = +magnitude, speed_y = 0 */
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    label_003_6FA7(&gb, 0x10);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x10);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    label_003_6FA7(&gb, 0x18);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x18);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    /* Link to the left (X = 0x20) -> speed_x = -magnitude, speed_y = 0 */
    gb_write_hram(&gb, hLinkPositionX, 0x20);
    label_003_6FA7(&gb, 0x10);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0xF0); /* (uint8_t)(-0x10) */
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    label_003_6FA7(&gb, 0x18);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0xE8); /* (uint8_t)(-0x18) */
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    printf("[PASS] label_003_6FA7\n");
}

/* Test func_003_7565 */
static void test_func_003_7565(void) {
    printf("[RUN ] func_003_7565\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);

    /* Link to the right (X = 0x60, Y = 0x30) */
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    gb_write_hram(&gb, hLinkPositionY, 0x30);

    func_003_7565_with_length(&gb, 0x14);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x14);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    /* Default length 0x12 */
    func_003_7565(&gb);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x12);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    printf("[PASS] func_003_7565\n");
}

/* Test func_003_6F93 and func_003_6F5C */
static void test_func_003_6F93_and_6F5C(void) {
    printf("[RUN ] func_003_6F93 & func_003_6F5C\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x01;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    gb_write_hram(&gb, hLinkPositionY, 0x40);

    /* Case 1: ENTITY_ROLLING_BONES_BAR uses label_003_6FA7 with magnitude 0x10 */
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ROLLING_BONES_BAR);
    func_003_6F93(&gb);

    assert(gb_read_hram(&gb, hJingle) == JINGLE_BUMP);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x0C);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x10);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    /* Case 2: func_003_6F5C clears ignore hits countdown */
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x0A);
    func_003_6F5C(&gb, bc);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x00);

    printf("[PASS] func_003_6F93 & func_003_6F5C\n");
}

/* Test func_003_6DDF */
static void test_func_003_6DDF(void) {
    printf("[RUN ] func_003_6DDF\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    gb_write_hram(&gb, hLinkPositionY, 0x40);

    /* Normal top-down mode */
    gb_write_hram(&gb, hIsSideScrolling, 0x00);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOLDORM);
    func_003_6DDF(&gb, bc);

    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x10);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x18); /* Moldorm uses length 0x18 */
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

    /* Side scrolling mode */
    gb_write_hram(&gb, hIsSideScrolling, 0x01);
    gb_write_hram(&gb, hLinkPhysicsModifier, 0x00);
    func_003_6DDF(&gb, bc);

    /* Link is to the right (dir_x = 0) -> Data_003_6E0C[0] = 0x0C */
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x0C);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0xF4);
    assert(gb_read_hram(&gb, hLinkPhysicsModifier) == 0x00);

    printf("[PASS] func_003_6DDF\n");
}

/* Test ReturnIfNonInteractive_03 */
static void test_ReturnIfNonInteractive(void) {
    printf("[RUN ] ReturnIfNonInteractive_03\n");

    GBState gb;
    gb_init(&gb);

    /* Normal interactive setup */
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wDialogState, 0x00);
    gb_write(&gb, wC1A8, 0x00);
    gb_write(&gb, wInventoryAppearing, 0x00);
    gb_write(&gb, wRoomTransitionState, 0x00);

    assert(ReturnIfNonInteractive_03(&gb, false) == false);

    /* Inactive entity check */
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_DISABLED);
    assert(ReturnIfNonInteractive_03(&gb, false) == true);
    assert(ReturnIfNonInteractive_03(&gb, true) == false);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);

    /* Gameplay types */
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD_MAP);
    assert(ReturnIfNonInteractive_03(&gb, false) == true);

    gb_write(&gb, wGameplayType, GAMEPLAY_CREDITS);
    assert(ReturnIfNonInteractive_03(&gb, false) == false);

    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);

    /* Transition counter */
    gb_write(&gb, wTransitionSequenceCounter, 0x03);
    assert(ReturnIfNonInteractive_03(&gb, false) == true);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);

    /* Dialog / inventory / transition blocks */
    gb_write(&gb, wDialogState, 0x01);
    assert(ReturnIfNonInteractive_03(&gb, false) == true);
    gb_write(&gb, wDialogState, 0x00);

    gb_write(&gb, wC1A8, 0x01);
    assert(ReturnIfNonInteractive_03(&gb, false) == true);
    gb_write(&gb, wC1A8, 0x00);

    gb_write(&gb, wInventoryAppearing, 0x01);
    assert(ReturnIfNonInteractive_03(&gb, false) == true);
    gb_write(&gb, wInventoryAppearing, 0x00);

    gb_write(&gb, wRoomTransitionState, 0x01);
    assert(ReturnIfNonInteractive_03(&gb, false) == true);
    gb_write(&gb, wRoomTransitionState, 0x00);

    assert(ReturnIfNonInteractive_03(&gb, false) == false);

    printf("[PASS] ReturnIfNonInteractive_03\n");
}

/* Test ApplyRecoilIfNeeded_03 */
static void test_ApplyRecoilIfNeeded(void) {
    printf("[RUN ] ApplyRecoilIfNeeded_03\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x03;

    /* When countdown is 0 -> no recoil applied */
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x00);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    ApplyRecoilIfNeeded_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x20);

    /* When countdown > 0 -> recoil applied */
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x05);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x00);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x00);
    gb_write(&gb, wEntitiesRecoilVelocityX + bc, 0x10); /* +1 pixel per frame */
    gb_write(&gb, wEntitiesRecoilVelocityY + bc, 0x00);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_ALLOW_OUT_OF_BOUNDS);

    ApplyRecoilIfNeeded_03(&gb, bc);

    /* Countdown decremented from 5 to 4 */
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x04);
    /* Position updated by +1 pixel (from 0x20 to 0x21) */
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x21);
    /* Speeds restored to original (0) */
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x00);

    printf("[PASS] ApplyRecoilIfNeeded_03\n");
}

/* Test CheckLinkCollisionWithEnemy */
static void test_CheckLinkCollisionWithEnemy(void) {
    printf("[RUN ] CheckLinkCollisionWithEnemy\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x02;

    /* Setup entity 2 at X=0x40, Y=0x40 */
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    /* Hitbox: offset X=0, radius X=4, offset Y=0, radius Y=4 */
    uint16_t hb = (uint16_t)(wEntitiesHitboxPositionTable + (bc << 2));
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x04);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x04);

    /* Case 1: Link in the air (Z > 0) -> no collision */
    gb_write_hram(&gb, hLinkPositionZ, 0x05);
    gb_write_hram(&gb, hLinkPositionX, 0x38);
    gb_write_hram(&gb, hLinkPositionY, 0x38);
    assert(CheckLinkCollisionWithEnemy(&gb, bc) == false);
    gb_write_hram(&gb, hLinkPositionZ, 0x00);

    /* Case 2: Link non-interactive -> no collision */
    gb_write(&gb, wLinkMotionState, LINK_MOTION_TYPE_NON_INTERACTIVE);
    assert(CheckLinkCollisionWithEnemy(&gb, bc) == false);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);

    /* Case 3: Link far away (X = 0x80) -> no collision */
    gb_write_hram(&gb, hLinkPositionX, 0x80);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    assert(CheckLinkCollisionWithEnemy(&gb, bc) == false);

    /* Case 4: Link overlapping entity hitbox */
    /* diff_x = abs(0x40 + 0 - 0x38 - 8) = abs(0) = 0 < 4 + 4 = 8 */
    /* diff_y = abs(0x40 + 0 - 0x38 - 8) = abs(0) = 0 < 4 + 4 = 8 */
    gb_write_hram(&gb, hLinkPositionX, 0x38);
    gb_write_hram(&gb, hLinkPositionY, 0x38);
    gb_write(&gb, wEntitiesPhysicsFlagsTable + bc, 0x00); /* Normal enemy */
    gb_write_hram(&gb, hLinkAnimationState, 0x00);

    /* Normal enemy hurts Link */
    gb_write(&gb, wInvincibilityCounter, 0x00);
    bool col = CheckLinkCollisionWithEnemy(&gb, bc);
    assert(col == true);
    assert(gb_read(&gb, wInvincibilityCounter) == 0x50);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_LINK_HURT);

    /* Case 5: Harmless entity */
    gb_write(&gb, wEntitiesPhysicsFlagsTable + bc, ENTITY_PHYSICS_HARMLESS);
    gb_write(&gb, wInvincibilityCounter, 0x00);
    col = CheckLinkCollisionWithEnemy(&gb, bc);
    assert(col == true);
    /* Harmless entity does NOT hurt Link */
    assert(gb_read(&gb, wInvincibilityCounter) == 0x00);

    printf("[PASS] CheckLinkCollisionWithEnemy\n");
}

static void test_BombBounceOffWalls(void) {
    printf("[RUN ] BombBounceOffWalls\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x03;

    /* Case 1: Side scrolling with X and Y collisions */
    gb_write_hram(&gb, hIsSideScrolling, 0x01);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x01 | 0x04);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x20);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x20);

    BombBounceOffWalls(&gb, bc);

    /* Speed X bounced: (~0x20 + 1) >> 3 = -32 >> 3 = -4 = 0xFC */
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0xFC);
    /* Speed Y untouched because side scrolling returns early */
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x20);

    /* Case 2: Top-down with both X and Y collisions */
    gb_write_hram(&gb, hIsSideScrolling, 0x00);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x02 | 0x08);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x18);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x18);

    BombBounceOffWalls(&gb, bc);

    /* Both X and Y bounced: -24 >> 3 = -3 = 0xFD */
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0xFD);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0xFD);

    /* Case 3: Top-down with only Y collision */
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x04);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x10);

    BombBounceOffWalls(&gb, bc);

    /* Speed X untouched, Speed Y bounced: -16 >> 3 = -2 = 0xFE */
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x10);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0xFE);

    /* Case 4: No collision -> neither changes */
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x00);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x10);

    BombBounceOffWalls(&gb, bc);

    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x10);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x10);

    printf("[PASS] BombBounceOffWalls\n");
}

static void test_func_003_51C9(void) {
    printf("[RUN ] func_003_51C9\n");

    GBState gb;
    gb_init(&gb);

    /* Active entity at PosY = 0x3F, PosX = 0x47 */
    /* top = 0x3F - 0x0F = 0x30, left = 0x47 - 0x07 = 0x40 */
    /* de_offset = (0x30 & 0xF0) | ((0x40 >> 4) & 0x0F) = 0x34 */
    gb_write_hram(&gb, hActiveEntityPosY, 0x3F);
    gb_write_hram(&gb, hActiveEntityPosX, 0x47);
    gb_write(&gb, wDrawCommandsSize, 0x00);

    static const uint8_t test_tiles[4] = { 0xF8, 0xF9, 0xFA, 0xFB };
    func_003_51C9(&gb, 0, test_tiles, 0xA6);

    /* Room object updated */
    assert(gb_read(&gb, wRoomObjects + 0x34) == 0xA6);
    assert(gb_read(&gb, wDDD8) == 0xA6);

    /* Draw commands size incremented by 10 */
    assert(gb_read(&gb, wDrawCommandsSize) == 10);

    /* Check draw command tiles */
    uint8_t bg_high = gb_read_hram(&gb, hIntersectedObjectBGAddressHigh);
    uint8_t bg_low = gb_read_hram(&gb, hIntersectedObjectBGAddressLow);

    assert(gb_read(&gb, wDrawCommand + 0) == bg_high);
    assert(gb_read(&gb, wDrawCommand + 1) == bg_low);
    assert(gb_read(&gb, wDrawCommand + 2) == 0x81);
    assert(gb_read(&gb, wDrawCommand + 3) == 0xF8);
    assert(gb_read(&gb, wDrawCommand + 4) == 0xF9);
    assert(gb_read(&gb, wDrawCommand + 5) == bg_high);
    assert(gb_read(&gb, wDrawCommand + 6) == (uint8_t)(bg_low + 1));
    assert(gb_read(&gb, wDrawCommand + 7) == 0x81);
    assert(gb_read(&gb, wDrawCommand + 8) == 0xFA);
    assert(gb_read(&gb, wDrawCommand + 9) == 0xFB);
    assert(gb_read(&gb, wDrawCommand + 10) == 0x00); /* Terminator */

    printf("[PASS] func_003_51C9\n");
}

static void test_PushedBlockEntityHandler(void) {
    printf("[RUN ] PushedBlockEntityHandler\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;

    /* Setup active entity 2 */
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityPosY, 0x3F);
    gb_write_hram(&gb, hActiveEntityPosX, 0x47);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x3F);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x47);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x3F);

    /* Link collided with pushed block */
    gb_write_hram(&gb, hLinkPositionX, 0x3F);
    gb_write_hram(&gb, hLinkPositionY, 0x37);
    gb_write_hram(&gb, hLinkFinalPositionX, 0x40);
    gb_write_hram(&gb, hLinkFinalPositionY, 0x30);
    gb_write(&gb, wIsIndoor, 0x01);
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x20); /* Next increment reaches 0x21 */
    gb_write(&gb, wRoomEvent, TRIGGER_PUSH_SINGLE_BLOCK);

    PushedBlockEntityHandler(&gb, bc);

    /* Link position restored to final position */
    assert(gb_read_hram(&gb, hLinkPositionX) == 0x40);
    assert(gb_read_hram(&gb, hLinkPositionY) == 0x30);
    assert(gb_read(&gb, wIsLinkPushing) == 0x03);

    /* Entity unloaded after inertia reaches 0x21 */
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Indoor tile 0xA6 placed at room objects */
    assert(gb_read(&gb, wRoomObjects + 0x34) == 0xA6);
    assert(gb_read(&gb, wDDD8) == 0xA6);

    /* Single block push resolved */
    assert(gb_read(&gb, wRoomEventEffectExecuted) == 0x01);
    assert(gb_read(&gb, hJingle) == JINGLE_PUZZLE_SOLVED);

    /* Test outdoor block push */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityPosY, 0x3F);
    gb_write_hram(&gb, hActiveEntityPosX, 0x47);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x3F);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x47);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x3F);
    gb_write(&gb, wIsIndoor, 0x00); /* Outdoor */
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x20);
    gb_write(&gb, wRoomEvent, TRIGGER_PUSH_SINGLE_BLOCK);

    PushedBlockEntityHandler(&gb, bc);

    /* Outdoor tile 0xC4 placed */
    assert(gb_read(&gb, wRoomObjects + 0x34) == 0xC4);
    assert(gb_read(&gb, wDDD8) == 0xC4);

    printf("[PASS] PushedBlockEntityHandler\n");
}

static void test_EntityBackgroundTables(void) {
    printf("[RUN ] EntityBackgroundTables\n");

    /* Verify EntityCollisionPointsX */
    assert(EntityCollisionPointsX[0] == 13);
    assert(EntityCollisionPointsX[1] == 2);
    assert(EntityCollisionPointsX[2] == 8);
    assert(EntityCollisionPointsX[3] == 8);
    assert(EntityCollisionPointsX[8] == 16);
    assert(EntityCollisionPointsX[9] == -1);

    /* Verify EntityCollisionPointsY */
    assert(EntityCollisionPointsY[0] == 8);
    assert(EntityCollisionPointsY[2] == 2);
    assert(EntityCollisionPointsY[3] == 13);
    assert(EntityCollisionPointsY[10] == -1);
    assert(EntityCollisionPointsY[11] == 16);

    /* Verify CollisionsTableFlagPerDirection */
    assert(CollisionsTableFlagPerDirection[0] == 0x01);
    assert(CollisionsTableFlagPerDirection[1] == 0x02);
    assert(CollisionsTableFlagPerDirection[2] == 0x04);
    assert(CollisionsTableFlagPerDirection[3] == 0x08);

    /* Verify EntityOnConveyorMovement */
    assert(EntityOnConveyorMovementX[2] == -1);
    assert(EntityOnConveyorMovementX[3] == 1);
    assert(EntityOnConveyorMovementY[0] == 1);
    assert(EntityOnConveyorMovementY[1] == -1);

    /* Verify SwitchBlockLoweredStatePerObject */
    assert(SwitchBlockLoweredStatePerObject[0] == 0x00);
    assert(SwitchBlockLoweredStatePerObject[1] == 0x02);

    /* Verify FineCollisionShapes */
    assert(FineCollisionShapes[0] == 1);
    assert(FineCollisionShapes[1] == 0);

    printf("[PASS] EntityBackgroundTables\n");
}

static void test_func_003_7E0E(void) {
    printf("[RUN ] func_003_7E0E (Entity tile & physics flags)\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 3;
    gb_write(&gb, wEntitiesPosXTable + bc, 0x41);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x37);
    gb_write(&gb, wIsIndoor, 0x00);

    gb_write(&gb, wRoomObjects + 0x34, OBJECT_SHORT_GRASS);

    func_003_7E0E(&gb, bc);

    assert(gb_read_hram(&gb, hMultiPurpose4) == 0x40);
    assert(gb_read_hram(&gb, hIntersectedObjectLeft) == 0x40);
    assert(gb_read_hram(&gb, hMultiPurpose5) == 0x30);
    assert(gb_read_hram(&gb, hIntersectedObjectTop) == 0x30);
    assert(gb_read_hram(&gb, hObjectUnderEntity) == OBJECT_SHORT_GRASS);

    printf("[PASS] func_003_7E0E (Entity tile & physics flags)\n");
}

static void test_ApplyEntityCollisionWithObject(void) {
    printf("[RUN ] ApplyEntityCollisionWithObject\n");

    GBState gb;
    gb_init(&gb);
    init_mock_physics_rom(&gb);

    uint16_t bc = 1;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesHitboxFlagsTable + bc, 0x00);
    gb_write_hram(&gb, hMultiPurpose0, 0x00);

    /* Position entity in tile 0x22: X=0x28, Y=0x30 */
    gb_write(&gb, wEntitiesPosXTable + bc, 0x28);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write(&gb, wIsIndoor, 0x00);

    /* Case 1: Solid wall at tile 0x22 */
    gb_write(&gb, wRoomObjects + 0x22, 0x21);
    bool no_col = ApplyEntityCollisionWithObject(&gb, bc, 0);
    assert(!no_col);
    assert((gb_read(&gb, wEntitiesCollisionsTable + bc) & 0x01) != 0);

    /* Case 2: No collision (OBJ_PHYSICS_NONE) at tile 0x22 */
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x00);
    gb_write(&gb, wRoomObjects + 0x22, 0x00);
    no_col = ApplyEntityCollisionWithObject(&gb, bc, 0);
    assert(no_col);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* Case 3: Water entity (Fish) on shallow water */
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FISH);
    gb_write(&gb, wRoomObjects + 0x22, 0x05);
    no_col = ApplyEntityCollisionWithObject(&gb, bc, 0);
    assert(no_col);

    /* Case 4: Switch block interaction */
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wRoomObjects + 0x22, OBJECT_LOWERED_BLOCK);
    gb_write(&gb, wSwitchBlocksState, 0x00);
    no_col = ApplyEntityCollisionWithObject(&gb, bc, 0);
    assert(no_col);

    gb_write(&gb, wSwitchBlocksState, 0x02);
    no_col = ApplyEntityCollisionWithObject(&gb, bc, 0);
    assert(!no_col);

    printf("[PASS] ApplyEntityCollisionWithObject\n");
}

static void test_ApplyEntityInteractionWithBackground(void) {
    printf("[RUN ] ApplyEntityInteractionWithBackground\n");

    GBState gb;
    gb_init(&gb);
    init_mock_physics_rom(&gb);

    uint16_t bc = 2;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write_hram(&gb, hActiveEntityPosX, 0x2E);
    gb_write_hram(&gb, hActiveEntityPosY, 0x30);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write(&gb, wEntitiesOptions1Table + bc, 0x00);

    /* Case 1: Entity moving right with collision against wall */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x08);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x00);
    /* For bc=2, PosX=0x30, sub_x=0x28, point=0x28+13=0x35 -> tile col 3.
     * PosY=0x30, sub_y=0x20, point=0x20+8=0x28 -> tile row 2 (0x20).
     * tile_idx = 0x23! */
    gb_write(&gb, wRoomObjects + 0x23, 0x21);

    ApplyEntityInteractionWithBackground(&gb, bc);

    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x2E);
    assert((gb_read(&gb, wEntitiesCollisionsTable + bc) & 0x01) != 0);

    /* Case 2: Water splash when falling into deep water */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write(&gb, wEntitiesGroundStatusTable + bc, ENTITY_GROUND_STATUS_NORMAL);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_SPLASH_IN_WATER);
    gb_write(&gb, wEntitiesSpeedZTable + bc, 0xE0);
    gb_write(&gb, wEntitiesPrivateCountdown3Table + bc, 0x00);
    gb_write_hram(&gb, hIsSideScrolling, 0x00);
    gb_write(&gb, wRoomObjects + 0x11, 0x07);

    ApplyEntityInteractionWithBackground(&gb, bc);

    /* Non-water entity (Octorok) unloads and splashes */
    assert(gb_read_hram(&gb, hJingle) == JINGLE_WATER_SPLASH);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Water entity (Fish) survives and gets ground status 2 */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FISH);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write(&gb, wRoomObjects + 0x11, 0x07);
    ApplyEntityInteractionWithBackground(&gb, bc);
    assert(gb_read(&gb, wEntitiesGroundStatusTable + bc) == 0x02);

    /* Case 3: Pit interaction */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GEL);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x24);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x28);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x05);
    /* For bc=2, PosX=0x24, PosX-1=0x23 -> left=0x20. PosY=0x28, PosY-7=0x21 -> top=0x20. tile_idx = 0x22 */
    gb_write(&gb, wRoomObjects + 0x22, 0x50);

    ApplyEntityInteractionWithBackground(&gb, bc);

    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_FALLING);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x04);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x48);

    /* Case 4: Conveyor belt movement */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write_hram(&gb, hFrameCounter, 0x00);
    /* PosX=0x40, PosX-1=0x3F -> left=0x30, col=3. PosY=0x40, PosY-7=0x39 -> top=0x30, row=3. tile_idx=0x33 */
    gb_write(&gb, wRoomObjects + 0x33, OBJ_PHYSICS_CONVEYOR + 3);

    ApplyEntityInteractionWithBackground(&gb, bc);

    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x41);

    printf("[PASS] ApplyEntityInteractionWithBackground\n");
}

/* Test ApplySwordIntersectionWithObjects, label_003_51F5, and Data_003_69A2 */
static void test_ApplySwordIntersectionWithObjects(void) {
    printf("[RUN ] ApplySwordIntersectionWithObjects & label_003_51F5\n");

    /* 1. Verify Data_003_69A2 byte table fidelity */
    const uint8_t expected_torch_tiles[8] = { 0x6C, 0x74, 0x6D, 0x75, 0x00, 0x00, 0x00, 0x00 };
    assert(memcmp(Data_003_69A2, expected_torch_tiles, sizeof(Data_003_69A2)) == 0);

    /* 2. Test label_003_51F5 draw command generation */
    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wDrawCommandsSize, 0x00);
    gb_write_hram(&gb, hIntersectedObjectTop, 0x10);
    gb_write_hram(&gb, hIntersectedObjectLeft, 0x20);
    gb_write_hram(&gb, hIsGBC, 0x00);

    label_003_51F5(&gb, Data_003_69A2);

    assert(gb_read(&gb, wDrawCommandsSize) == 10);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 1) == 0x44);
    assert(gb_read(&gb, wDrawCommand + 2) == 0x81);
    assert(gb_read(&gb, wDrawCommand + 3) == 0x6C);
    assert(gb_read(&gb, wDrawCommand + 4) == 0x74);
    assert(gb_read(&gb, wDrawCommand + 5) == 0x98);
    assert(gb_read(&gb, wDrawCommand + 6) == 0x45);
    assert(gb_read(&gb, wDrawCommand + 7) == 0x81);
    assert(gb_read(&gb, wDrawCommand + 8) == 0x6D);
    assert(gb_read(&gb, wDrawCommand + 9) == 0x75);
    assert(gb_read(&gb, wDrawCommand + 10) == 0x00);

    /* 3. Test ApplySwordIntersectionWithObjects on OBJECT_TORCH_LIT */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    uint16_t bc = 0x02;
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x38);
    /* PosX=0x30 -> left=0x30, col=3; PosY=0x38, PosY-8=0x30 -> top=0x30, row=3 -> tile_idx=0x33 */
    gb_write(&gb, wRoomObjects + 0x33, OBJECT_TORCH_LIT);

    bool res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == false);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0);

    /* 4. Test ApplySwordIntersectionWithObjects on OBJECT_TORCH_UNLIT with Magic Rod Fireball indoors */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x38);
    gb_write(&gb, wRoomObjects + 0x33, OBJECT_TORCH_UNLIT);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MAGIC_ROD_FIREBALL);
    gb_write(&gb, wIsIndoor, 0x01);
    gb_write_hram(&gb, hIsGBC, 0x01);
    gb_write(&gb, wC1A2, 0x02);
    gb_write(&gb, wC3CD, 0x10);

    res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == false);
    assert(gb_read(&gb, wRoomObjects + 0x33) == OBJECT_TORCH_LIT);
    assert(gb_read(&gb, wDDD8) == OBJECT_TORCH_LIT);
    assert(gb_read(&gb, wC1A2) == 0x03);
    assert(gb_read(&gb, wC3CD) == 0x0C);
    assert(gb_read(&gb, wBGPaletteTransitionEffect) == 0x40);
    assert(gb_read(&gb, wDDD7) == 0x0B);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_BURSTING_FLAME);

    /* 5. Test solid wall collision with coordinate rollback */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x32);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x3A);
    gb_write_hram(&gb, hActiveEntityPosX, 0x28);
    gb_write_hram(&gb, hActiveEntityPosY, 0x20);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    /* PosX=0x32 -> col=3, PosY=0x3A - 8 = 0x32 -> row=3 -> tile_idx=0x33 */
    gb_write(&gb, wRoomObjects + 0x33, 0x21); /* OBJ_PHYSICS_SOLID */

    res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == true);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x01);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x28);
    assert(gb_read(&gb, wEntitiesPosYTable + bc) == 0x20);

    /* 6. Test passable tile (OBJ_PHYSICS_NONE) */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x32);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x3A);
    gb_write(&gb, wRoomObjects + 0x33, 0x00); /* OBJ_PHYSICS_NONE */

    res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == false);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* 7. Test out-of-bounds tile (physics == 0xFF) */
    /* Non-boomerang: unloads entity */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x32);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x3A);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wRoomObjects + 0x33, 0x31); /* mock physics 0xFF */

    res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == false);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == 0); /* unloaded */

    /* Boomerang: collides instead of unload */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x32);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x3A);
    gb_write_hram(&gb, hActiveEntityPosX, 0x28);
    gb_write_hram(&gb, hActiveEntityPosY, 0x20);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BOOMERANG);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x05);
    gb_write(&gb, wRoomObjects + 0x33, 0x31); /* mock physics 0xFF */

    res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == true);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x01);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x28);

    /* 8. Test directional ledge physics (0xD1 = ledge direction 1) */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x32);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x3A);
    gb_write(&gb, wRoomObjects + 0x33, 0x30); /* mock physics 0xD1 */
    gb_write(&gb, wEntitiesThrownDirectionTable + bc, 0x01); /* matching ledge */
    gb_write(&gb, wEntitiesPosZTable + bc, 0x08); /* airborne: Z > 0 */
    gb_write(&gb, wEntitiesUnknowTableJ + bc, 0x02);

    res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == false);
    assert(gb_read(&gb, wEntitiesUnknowTableJ + bc) == 0x03);

    /* Ledge ground impact (Z == 0) -> collision */
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityPosY, 0x30);
    res = ApplySwordIntersectionWithObjects(&gb, bc);
    assert(res == true);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x01);

    printf("[PASS] ApplySwordIntersectionWithObjects & label_003_51F5\n");
}

void test_bank3_entities_physics(void) {
    test_GetEntityXDistanceToLink();
    test_GetEntityYDistanceToLink();
    test_GetEntityDirectionToLink();
    test_GetVectorTowardsLink();
    test_AddEntitySpeedToPos();
    test_AddEntityZSpeedToPos();
    test_UpdateEntityPosWithSpeed();
    test_RecoilAndIgnoringHits();
    test_func_003_6C6B();
    test_func_003_6CC0();
    test_label_003_6FA7();
    test_func_003_7565();
    test_func_003_6F93_and_6F5C();
    test_func_003_6DDF();
    test_ReturnIfNonInteractive();
    test_ApplyRecoilIfNeeded();
    test_CheckLinkCollisionWithEnemy();
    test_BombBounceOffWalls();
    test_func_003_51C9();
    test_PushedBlockEntityHandler();
    test_EntityBackgroundTables();
    test_func_003_7E0E();
    test_ApplyEntityCollisionWithObject();
    test_ApplyEntityInteractionWithBackground();
    test_ApplySwordIntersectionWithObjects();
}
