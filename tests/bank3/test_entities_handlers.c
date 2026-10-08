#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "gb.h"
#include "bank3/entities_handlers.h"
#include "bank3/entities_init_core.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_collision.h"
#include "bank3/entities_moblin.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/directions.h"
#include "constants/gameplay.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include "constants/rooms.h"
#include "constants/audio.h"
#include "constants/physics.h"

static uint8_t mock_physics_rom[0x4000 * 0x09];
#define ROM_BANK_8_OFFSET(addr) ((0x08 * 0x4000) + ((addr) - 0x4000))

static void init_mock_physics_rom(GBState *gb) {
    memset(mock_physics_rom, 0, sizeof(mock_physics_rom));
    mock_physics_rom[ROM_BANK_8_OFFSET(OverworldObjectPhysicFlags + 0x21)] = OBJ_PHYSICS_SOLID;
    gb_attach_rom(gb, mock_physics_rom, sizeof(mock_physics_rom));
}

/* Helper to setup interactive game state so ReturnIfNonInteractive_03 passes */
static void setup_interactive(GBState *gb) {
    gb_write_hram(gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(gb, wTransitionSequenceCounter, 0x04);
    gb_write(gb, wDialogState, 0x00);
    gb_write(gb, wC1A8, 0x00);
    gb_write(gb, wInventoryAppearing, 0x00);
    gb_write(gb, wRoomTransitionState, 0x00);
}

/* ===== 1. Data Tables Verification ===== */
static void test_DataTables_EntitiesHandlers(void) {
    printf("[RUN ] DataTables_EntitiesHandlers\n");

    /* FireSpriteVariants (8 bytes: 2 variants * 4 bytes) */
    assert(sizeof(FireSpriteVariants) == 8);
    /* variant 0: tile 0x34, pal 2; flipped */
    assert(FireSpriteVariants[0] == 0x34);
    assert(FireSpriteVariants[1] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    assert(FireSpriteVariants[2] == 0x34);
    assert(FireSpriteVariants[3] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));
    /* variant 1: tile 0x34, pal 4; flipped */
    assert(FireSpriteVariants[4] == 0x34);
    assert(FireSpriteVariants[5] == (OAM_GBC_PAL_4 | OAMF_PAL1));
    assert(FireSpriteVariants[6] == 0x34);
    assert(FireSpriteVariants[7] == (OAM_GBC_PAL_4 | OAMF_PAL1 | OAMF_XFLIP));

    /* Unknown020SpriteVariants (4 bytes) */
    assert(sizeof(Unknown020SpriteVariants) == 4);
    assert(Unknown020SpriteVariants[0] == 0x1E);
    assert(Unknown020SpriteVariants[1] == 0x01);
    assert(Unknown020SpriteVariants[2] == 0x1E);
    assert(Unknown020SpriteVariants[3] == 0x61);

    /* Data_003_4CA4 (4 bytes) */
    assert(sizeof(Data_003_4CA4) == 4);
    assert(Data_003_4CA4[0] == 0x00);
    assert(Data_003_4CA4[1] == 0x00);
    assert(Data_003_4CA4[2] == 0x04);
    assert(Data_003_4CA4[3] == 0x00);

    /* Data_003_4CA8 (4 bytes) */
    assert(sizeof(Data_003_4CA8) == 4);
    assert(Data_003_4CA8[0] == 0x00);
    assert(Data_003_4CA8[1] == 0x01);
    assert(Data_003_4CA8[2] == 0x03);
    assert(Data_003_4CA8[3] == 0x06);

    /* Data_003_4CAC (6 bytes) */
    assert(sizeof(Data_003_4CAC) == 6);
    assert(Data_003_4CAC[0] == 0x24);
    assert(Data_003_4CAC[1] == 0x01);
    assert(Data_003_4CAC[2] == 0x24);
    assert(Data_003_4CAC[3] == 0x01);
    assert(Data_003_4CAC[4] == 0x3E);
    assert(Data_003_4CAC[5] == 0x01);

    /* Data_003_4E05 (2 bytes) */
    assert(sizeof(Data_003_4E05) == 2);
    assert(Data_003_4E05[0] == 0x10);
    assert(Data_003_4E05[1] == 0xF0);

    /* Data_003_56EA (4 bytes) */
    assert(sizeof(Data_003_56EA) == 4);
    assert(Data_003_56EA[0] == 0x01);
    assert(Data_003_56EA[1] == 0x08);
    assert(Data_003_56EA[2] == 0x08);
    assert(Data_003_56EA[3] == 0x10);

    /* Data_003_56EE (3 bytes) */
    assert(sizeof(Data_003_56EE) == 3);
    assert(Data_003_56EE[0] == 0x01);
    assert(Data_003_56EE[1] == 0x04);
    assert(Data_003_56EE[2] == 0x04);

    /* Data_003_56F1 (17 bytes) */
    assert(sizeof(Data_003_56F1) == 17);
    /* Right */
    assert(Data_003_56F1[0] == 0x0A && Data_003_56F1[1] == 0x37 && Data_003_56F1[2] == 0x37 && Data_003_56F1[3] == 0x37);
    /* Left */
    assert(Data_003_56F1[4] == 0x01 && Data_003_56F1[5] == 0x39 && Data_003_56F1[6] == 0x39 && Data_003_56F1[7] == 0x39);
    /* Up */
    assert(Data_003_56F1[8] == 0x01 && Data_003_56F1[9] == 0x3B && Data_003_56F1[10] == 0x3B && Data_003_56F1[11] == 0x3B);
    /* Down */
    assert(Data_003_56F1[12] == 0x01 && Data_003_56F1[13] == 0x3D && Data_003_56F1[14] == 0x3D && Data_003_56F1[15] == 0x3D);
    assert(Data_003_56F1[16] == 0x01);

    /* Data_003_5701 (17 bytes) */
    assert(sizeof(Data_003_5701) == 17);
    assert(Data_003_5701[0] == 0x01 && Data_003_5701[1] == 0x10 && Data_003_5701[2] == 0x10 && Data_003_5701[3] == 0x08);
    assert(Data_003_5701[4] == 0x00 && Data_003_5701[5] == 0xF0 && Data_003_5701[6] == 0xF0 && Data_003_5701[7] == 0xF8);
    assert(Data_003_5701[8] == 0x00 && Data_003_5701[9] == 0x00 && Data_003_5701[10] == 0x00 && Data_003_5701[11] == 0x00);
    assert(Data_003_5701[12] == 0x00 && Data_003_5701[13] == 0xFF && Data_003_5701[14] == 0xFF && Data_003_5701[15] == 0xFF);
    assert(Data_003_5701[16] == 0xFF);

    /* Data_003_5711 (17 bytes) */
    assert(sizeof(Data_003_5711) == 17);
    assert(Data_003_5711[0] == 0xFF && Data_003_5711[1] == 0x00 && Data_003_5711[2] == 0x00 && Data_003_5711[3] == 0x00);
    assert(Data_003_5711[4] == 0x00 && Data_003_5711[5] == 0x00 && Data_003_5711[6] == 0x00 && Data_003_5711[7] == 0x00);
    assert(Data_003_5711[8] == 0x00 && Data_003_5711[9] == 0x00 && Data_003_5711[10] == 0x00 && Data_003_5711[11] == 0x00);
    assert(Data_003_5711[12] == 0x00 && Data_003_5711[13] == 0x00 && Data_003_5711[14] == 0x00 && Data_003_5711[15] == 0x08);
    assert(Data_003_5711[16] == 0x00);

    /* Data_003_5721 (17 bytes) */
    assert(sizeof(Data_003_5721) == 17);
    assert(Data_003_5721[0] == 0x00 && Data_003_5721[1] == 0x00 && Data_003_5721[2] == 0x00 && Data_003_5721[3] == 0x08);
    assert(Data_003_5721[4] == 0x0E && Data_003_5721[5] == 0x00 && Data_003_5721[6] == 0x00 && Data_003_5721[7] == 0x08);
    assert(Data_003_5721[8] == 0x0E && Data_003_5721[9] == 0x00 && Data_003_5721[10] == 0x00 && Data_003_5721[11] == 0x08);
    assert(Data_003_5721[12] == 0x0E && Data_003_5721[13] == 0x00 && Data_003_5721[14] == 0x00 && Data_003_5721[15] == 0x00);
    assert(Data_003_5721[16] == 0x0E);

    printf("[PASS] DataTables_EntitiesHandlers\n");
}

/* ===== 2. Entity 25/26 Stubs ===== */
static void test_Entity25_26_Stubs(void) {
    printf("[RUN ] Entity25_26_Stubs\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);

    /* Test EntityInitEntity25 with countdown 0 */
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0x01, 0x00);
    EntityInitEntity25(&gb, 0x01);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x01) == ENTITY_STATUS_DYING);
    assert(gb_read(&gb, wEntitiesPrivateCountdown3Table + 0x01) == 0x1F);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_ENEMY_DESTROYED);

    /* Test EntityInitEntity26 */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0x02, 0x00);
    EntityInitEntity26(&gb, 0x02);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x02) == ENTITY_STATUS_DYING);

    /* Test Entity25Handler */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0x03, 0x00);
    Entity25Handler(&gb, 0x03);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) == ENTITY_STATUS_DYING);

    /* Test Entity26Handler */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0x04, 0x00);
    Entity26Handler(&gb, 0x04);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x04) == ENTITY_STATUS_DYING);

    printf("[PASS] Entity25_26_Stubs\n");
}

/* ===== 3. EntityBurningHandler ===== */
static void test_EntityBurningHandler(void) {
    printf("[RUN ] EntityBurningHandler\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);

    /* Case A: Countdown != 0: Burning flicker animation and physics */
    uint16_t bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x20);
    gb_write(&gb, wEntitiesSpriteVariantTable + bc, 0x05);
    gb_write_hram(&gb, hFrameCounter, 0x00); /* 0 >> 3 & 1 = 0 */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x10);

    EntityBurningHandler(&gb, bc);

    /* Sprite variant should be restored to entity variant table value */
    assert(gb_read_hram(&gb, hActiveEntitySpriteVariant) == 0x05);
    /* Speed cleared */
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x00);

    /* Frame counter test: frame 8 -> variant 1 */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x10);
    gb_write_hram(&gb, hFrameCounter, 0x08);
    EntityBurningHandler(&gb, bc);
    assert(gb_read_hram(&gb, hActiveEntitySpriteVariant) == 0x05);

    /* Case B: Countdown == 0, non-Gibdo: Enemy destroyed */
    gb_init(&gb);
    setup_interactive(&gb);
    bc = 0x03;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);

    EntityBurningHandler(&gb, bc);

    assert(gb_read(&gb, wEntitiesPrivateCountdown3Table + bc) == 0x1F);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DYING);
    assert(gb_read(&gb, wEntitiesPhysicsFlagsTable + bc) == 0x04);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_ENEMY_DESTROYED);

    /* Case C: Countdown == 0, Gibdo: Transforms into Stalfos */
    gb_init(&gb);
    setup_interactive(&gb);
    bc = 0x04;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GIBDO);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_GIBDO);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);

    EntityBurningHandler(&gb, bc);

    /* Replaced with evasive stalfos, marked active */
    assert(gb_read(&gb, wEntitiesTypeTable + bc) == ENTITY_STALFOS_EVASIVE);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);

    printf("[PASS] EntityBurningHandler\n");
}

/* ===== 4. EntityFallHandler ===== */
static void test_EntityFallHandler(void) {
    printf("[RUN ] EntityFallHandler\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);

    /* Case A: Color dungeon color shell */
    uint16_t bc = 0x01;
    gb_write_hram(&gb, hMapId, MAP_COLOR_DUNGEON);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_COLOR_SHELL_RED);

    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x06);

    /* Green shell */
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_COLOR_SHELL_GREEN);
    gb_write(&gb, wEntitiesStateTable + bc, 0x00);
    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x06);

    /* Blue shell */
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_COLOR_SHELL_BLUE);
    gb_write(&gb, wEntitiesStateTable + bc, 0x00);
    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x06);

    /* Outside Color Dungeon: normal fall */
    gb_write_hram(&gb, hMapId, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Case B: Countdown == 0: Entity unloading and room flags */
    gb_init(&gb);
    setup_interactive(&gb);
    bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesOptions1Table + bc, 0x00);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);

    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wD460) == 0x01);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Excluded from kill all: does not set wD460 */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wD460) == 0x00);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Wrecking ball: sets respawn coordinates */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_WRECKING_BALL);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wWreckingBallRoom) == 0x16);
    assert(gb_read(&gb, wWreckingBallPosX) == 0x50);
    assert(gb_read(&gb, wWreckingBallPosY) == 0x27);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Case C: Countdown >= 0x40 (Octorok/Moblin variant animation) */
    gb_init(&gb);
    setup_interactive(&gb);
    bc = 0x03;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x50);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_UP);

    EntityFallHandler(&gb, bc);
    /* EntityVariantForDirection_03 for UP is 2; SetEntityVariantForDirection_03 cycles inertia */
    assert(gb_read(&gb, wEntitiesInertiaTable + bc) == 0x03);

    /* Case D: Countdown < 0x40: Falling pit shrinkage animation and movement towards hole */
    gb_init(&gb);
    setup_interactive(&gb);
    bc = 0x04;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x3F); /* 63 -> variant 3 */
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x20);
    gb_write_hram(&gb, hLinkPositionX, 0x40);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write(&gb, wEntitiesFallingTargetXTable + bc, 0x70);
    gb_write(&gb, wEntitiesFallingTargetYTable + bc, 0x80);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x60);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x60);

    EntityFallHandler(&gb, bc);

    /* Falling jingle played on countdown 0x3F */
    assert(gb_read_hram(&gb, hJingle) == JINGLE_ITEM_FALLING);
    /* Variant 3 set */
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 0x03);
    /* Link's original coordinates restored */
    assert(gb_read_hram(&gb, hLinkPositionX) == 0x40);
    assert(gb_read_hram(&gb, hLinkPositionY) == 0x50);

    /* Variant 2 (countdown 0x20): Visual pos Y + 4 */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x20);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x10);
    EntityFallHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 0x02);
    assert(gb_read_hram(&gb, hActiveEntityVisualPosY) == 0x14);

    printf("[PASS] EntityFallHandler\n");
}

/* ===== 5. EntityThrownHandler ===== */
static void test_EntityThrownHandler(void) {
    printf("[RUN ] EntityThrownHandler\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);

    /* Case A: Thrown moving entity -> does not become stunned */
    uint16_t bc = 0x01;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x00);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);

    EntityThrownHandler(&gb, bc);

    assert(gb_read(&gb, wAttackDamageType) == DAMAGE_TYPE_THROW_AT);
    /* Moving entity should not be stunned */
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);

    /* Case B: Thrown entity that has stopped moving -> becomes stunned */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x00);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x00);

    EntityThrownHandler(&gb, bc);

    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_STUNNED);
    assert(gb_read(&gb, wEntitiesPrivateCountdown2Table + bc) == 0xFF);
    assert(gb_read(&gb, wEntitiesSpeedZTable + bc) == 0x00);

    /* Case C: Thrown Genie boss collision */
    gb_init(&gb);
    init_mock_physics_rom(&gb);
    setup_interactive(&gb);
    bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GENIE);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write(&gb, wRoomObjects + 0x23, 0x21); /* Solid wall tile */
    gb_write(&gb, wEntitiesPrivateState4Table + bc, 0x00);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);

    EntityThrownHandler(&gb, bc);

    /* 1st hit: hurt flash and wave sound, counter = 1 */
    assert(gb_read(&gb, wEntitiesFlashCountdownTable + bc) == 0x20);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_BOSS_HURT);
    assert(gb_read(&gb, wEntitiesPrivateState4Table + bc) == 0x01);

    /* 2nd hit: counter = 2 */
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    EntityThrownHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesPrivateState4Table + bc) == 0x02);

    /* 3rd hit: reaches 3 -> jumps to genie2 */
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    EntityThrownHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesPrivateState4Table + bc) == 0x03);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x01);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x80);
    assert(gb_read(&gb, wEntitiesPrivateState3Table + bc) == 0x00);

    printf("[PASS] EntityThrownHandler\n");
}

/* ===== 6. EntityStunnedHandler ===== */
static void test_EntityStunnedHandler(void) {
    printf("[RUN ] EntityStunnedHandler\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);

    /* Case A: Countdown2 == 0: Wakes up from stun */
    uint16_t bc = 0x01;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesPrivateCountdown2Table + bc, 0x00);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_STUNNED);
    gb_write(&gb, wEntitiesSpeedZTable + bc, 0x15);

    EntityStunnedHandler(&gb, bc);

    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesSpeedZTable + bc) == 0x00);

    /* Case B: Countdown2 >= 0x38: No horizontal shaking */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesPrivateCountdown2Table + bc, 0x40);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);

    EntityStunnedHandler(&gb, bc);
    /* Position unchanged */
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x20);

    /* Case C: Countdown2 < 0x38: Shaking with Data_003_4E05 speeds */
    /* Countdown 0x04 -> variant 1 -> speed -16 (0xF0) */
    gb_write(&gb, wEntitiesPrivateCountdown2Table + bc, 0x04);
    EntityStunnedHandler(&gb, bc);
    /* ClearEntitySpeed clears speed at end */
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x00);

    /* Helper for colliding Link and entity */
#define SETUP_COLLISION(gb_ptr, ent_bc) do { \
    gb_write_hram(gb_ptr, hActiveEntityPosX, 0x40); \
    gb_write_hram(gb_ptr, hActiveEntityVisualPosY, 0x40); \
    gb_write(gb_ptr, wEntitiesPosXTable + (ent_bc), 0x40); \
    gb_write(gb_ptr, wEntitiesPosYTable + (ent_bc), 0x40); \
    uint16_t hb = (uint16_t)(wEntitiesHitboxPositionTable + ((ent_bc) << 2)); \
    gb_write(gb_ptr, hb + 0, 0x00); \
    gb_write(gb_ptr, hb + 1, 0x04); \
    gb_write(gb_ptr, hb + 2, 0x00); \
    gb_write(gb_ptr, hb + 3, 0x04); \
    gb_write_hram(gb_ptr, hLinkPositionX, 0x38); \
    gb_write_hram(gb_ptr, hLinkPositionY, 0x38); \
    gb_write_hram(gb_ptr, hLinkPositionZ, 0x00); \
} while(0)

    /* Case D: Power Bracelet B button lift */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wInventoryItems_BButtonSlot, INVENTORY_POWER_BRACELET);
    gb_write_hram(&gb, hJoypadState, J_B);
    gb_write(&gb, wC3CF, 0x00);
    SETUP_COLLISION(&gb, bc);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_UP);

    EntityStunnedHandler(&gb, bc);

    /* Should be lifted! */
    assert(gb_read(&gb, wC3CF) == 0x01);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_LIFTED);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_LIFT_UP);
    assert(gb_read(&gb, wC15D) == DIRECTION_UP);

    /* Case E: Power Bracelet A button lift */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wInventoryItems_AButtonSlot, INVENTORY_POWER_BRACELET);
    gb_write_hram(&gb, hJoypadState, J_A);
    gb_write(&gb, wC3CF, 0x00);
    SETUP_COLLISION(&gb, bc);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_DOWN);

    EntityStunnedHandler(&gb, bc);

    assert(gb_read(&gb, wC3CF) == 0x01);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_LIFTED);

    printf("[PASS] EntityStunnedHandler\n");
}

/* ===== 7. EntityGetLiftedUp ===== */
static void test_EntityGetLiftedUp(void) {
    printf("[RUN ] EntityGetLiftedUp\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);

    uint16_t bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);

    /* Case A: Already carrying something (wC3CF != 0) -> lift rejected */
    gb_write(&gb, wC3CF, 0x01);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_STUNNED);
    gb_write(&gb, wEntitiesPrivateCountdown2Table + bc, 0x40);

    EntityGetLiftedUp(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_STUNNED);

    /* Case B: Out of range of Link -> lift rejected */
    gb_write(&gb, wC3CF, 0x00);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_STUNNED);
    gb_write(&gb, wEntitiesPrivateCountdown2Table + bc, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x10);
    gb_write_hram(&gb, hLinkPositionY, 0x10);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x80);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x80);

    EntityGetLiftedUp(&gb, bc);
    assert(gb_read(&gb, wC3CF) == 0x00);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_STUNNED);

    /* Case C: Successful lift */
    SETUP_COLLISION(&gb, bc);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_LEFT);

    EntityGetLiftedUp(&gb, bc);

    assert(gb_read(&gb, wC3CF) == 0x01);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_LIFTED);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_LIFT_UP);
    assert(gb_read(&gb, wEntitiesLiftedTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x02);
    assert(gb_read(&gb, wC15D) == DIRECTION_LEFT);

    printf("[PASS] EntityGetLiftedUp\n");
}

/* ===== 8. EntityLiftedHandler and func_003_5795 ===== */
static void test_EntityLiftedHandler_and_func_003_5795(void) {
    printf("[RUN ] EntityLiftedHandler_and_func_003_5795\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);

    uint16_t bc = 0x03;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BOMB);
    gb_write(&gb, wEntitiesFlashCountdownTable + bc, 0x10);

    /* Test bomb lifting: flash countdown cleared */
    gb_write(&gb, wEntitiesLiftedTable + bc, 0x04); /* At max phase */
    EntityLiftedHandler(&gb, bc);
    assert(gb_read(&gb, wLiftedEntityType) == ENTITY_BOMB);
    assert(gb_read(&gb, wEntitiesFlashCountdownTable + bc) == 0x00);

    /* Test lift animation delay selection */
    /* Normal lift: Data_003_56EA */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wPowerBraceletLevel, 0x01);
    gb_write(&gb, wTunicType, 0x00);
    gb_write(&gb, wActivePowerUp, 0x00);
    gb_write(&gb, wEntitiesLiftedTable + bc, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);

    EntityLiftedHandler(&gb, bc);

    /* Incremented lifted value to 1, wrote Data_003_56EA[0] = 1 to countdown */
    assert(gb_read(&gb, wEntitiesLiftedTable + bc) == 0x01);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x01);

    /* Next phase: table[1] = 8 */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    EntityLiftedHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesLiftedTable + bc) == 0x02);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x08);

    /* Fast lift (e.g. piece of power): Data_003_56EE */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wActivePowerUp, ACTIVE_POWER_UP_PIECE_OF_POWER);
    gb_write(&gb, wEntitiesLiftedTable + bc, 0x01);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);

    EntityLiftedHandler(&gb, bc);

    /* table[1] from Data_003_56EE = 4 */
    assert(gb_read(&gb, wEntitiesLiftedTable + bc) == 0x02);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x04);

    /* ===== func_003_5795 Overhead Positioning Tests ===== */
    /* Direction RIGHT (0), lift phase e = 2:
       index = (0 << 2) + 2 = 2
       IsCarrying = Data_003_56F1[2] = 0x37
       X offset = Data_003_5701[2] = 0x10 (+16)
       Y offset = Data_003_5711[2] = 0x00
       Z offset = Data_003_5721[2] = 0x00
    */
    gb_init(&gb);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write_hram(&gb, hLinkPositionX, 0x40);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write_hram(&gb, hLinkPositionZ, 0x04);
    gb_write_hram(&gb, hIsSideScrolling, 0x00);
    gb_write(&gb, wC13B, 0x02);

    func_003_5795(&gb, bc, 0x02);

    assert(gb_read(&gb, wIsCarryingLiftedObject) == 0x37);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x50); /* 0x40 + 0x10 */
    assert(gb_read(&gb, wEntitiesPosYTable + bc) == 0x52); /* 0x50 + 0x00 + 0x02 */
    assert(gb_read(&gb, wEntitiesPosZTable + bc) == 0x04); /* 0x04 + 0x00 */

    /* Direction LEFT (1), lift phase e = 1:
       index = (1 << 2) + 1 = 5
       IsCarrying = Data_003_56F1[5] = 0x39
       X offset = Data_003_5701[5] = 0xF0 (-16)
       Y offset = Data_003_5711[5] = 0x00
       Z offset = Data_003_5721[5] = 0x00
    */
    gb_write_hram(&gb, hLinkDirection, DIRECTION_LEFT);
    func_003_5795(&gb, bc, 0x01);
    assert(gb_read(&gb, wIsCarryingLiftedObject) == 0x39);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x30); /* 0x40 - 0x10 */

    /* Side scrolling mode: subtracts Z offset from Entity Y */
    /* Direction DOWN (3), lift phase e = 3:
       index = (3 << 2) + 3 = 15
       Y offset = Data_003_5711[15] = 0x08
       Z offset = Data_003_5721[15] = 0x00
    */
    gb_write_hram(&gb, hIsSideScrolling, 0x01);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_DOWN);
    func_003_5795(&gb, bc, 0x03);
    /* entity_y = LinkY (0x50) + 8 + C13B (2) - 0 = 0x5A */
    assert(gb_read(&gb, wEntitiesPosYTable + bc) == 0x5A);

    printf("[PASS] EntityLiftedHandler_and_func_003_5795\n");
}

/* ===== 9. EntityBecomeStunned ===== */
static void test_EntityBecomeStunned(void) {
    printf("[RUN ] EntityBecomeStunned\n");

    GBState gb;
    gb_init(&gb);

    uint16_t bc = 0x05;
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPrivateCountdown2Table + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedZTable + bc, 0x20);

    EntityBecomeStunned(&gb, bc);

    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_STUNNED);
    assert(gb_read(&gb, wEntitiesPrivateCountdown2Table + bc) == 0xFF);
    assert(gb_read(&gb, wEntitiesSpeedZTable + bc) == 0x00);

    /* NULL state safety */
    EntityBecomeStunned(NULL, bc);

    printf("[PASS] EntityBecomeStunned\n");
}

/* Suite runner */
void test_bank3_entities_handlers(void) {
    printf("[SUITE] Bank 3 Universal Entity State Handlers\n");

    test_DataTables_EntitiesHandlers();
    test_Entity25_26_Stubs();
    test_EntityBurningHandler();
    test_EntityFallHandler();
    test_EntityThrownHandler();
    test_EntityStunnedHandler();
    test_EntityGetLiftedUp();
    test_EntityLiftedHandler_and_func_003_5795();
    test_EntityBecomeStunned();

    printf("[SUITE PASS] Bank 3 Universal Entity State Handlers\n\n");
}
