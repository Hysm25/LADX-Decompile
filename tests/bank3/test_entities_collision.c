#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "gb.h"
#include "bank3/entities_collision.h"
#include "bank3/entities_physics.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/directions.h"
#include "constants/gameplay.h"
#include "constants/inventory.h"
#include "constants/sfx.h"
#include "constants/audio.h"
#include "bank3/entities_arrow.h"

/* Test EntityDamagesForGroup table values */
static void test_EntityDamagesForGroup(void) {
    printf("[RUN ] EntityDamagesForGroup table\n");

    assert(sizeof(EntityDamagesForGroup) == 53);
    assert(EntityDamagesForGroup[0x00] == 0x04);
    assert(EntityDamagesForGroup[0x02] == 0x08);
    assert(EntityDamagesForGroup[0x04] == 0x18);
    assert(EntityDamagesForGroup[0x15] == 0x0C);
    assert(EntityDamagesForGroup[0x16] == 0x00);
    assert(EntityDamagesForGroup[0x1F] == 0x20);
    assert(EntityDamagesForGroup[0x34] == 0x08);

    printf("[PASS] EntityDamagesForGroup table\n");
}

/* Test ApplyLinkCollisionWithEnemy: Cheep-Cheep Jumping */
static void test_ApplyLinkCollision_CheepCheep(void) {
    printf("[RUN ] ApplyLinkCollisionWithEnemy (Cheep-Cheep)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);

    gb_write_hram(&gb, hActiveEntityType, ENTITY_CHEEP_CHEEP_JUMPING);
    gb_write_hram(&gb, hActiveEntityPosY, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);

    /* Case 1: Link is above Cheep-Cheep (Y = 0x20 -> DIRECTION_UP) */
    gb_write_hram(&gb, hLinkPositionY, 0x20);
    gb_write(&gb, wEntitiesStateTable + bc, 0x01);
    gb_write(&gb, wIsLinkInTheAir, 0x00);
    gb_write_hram(&gb, hLinkSpeedY, 0x00);
    gb_write_hram(&gb, hWaveSfx, 0x00);

    ApplyLinkCollisionWithEnemy(&gb, bc);

    assert(gb_read(&gb, wEntitiesStateTable + bc) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wIsLinkInTheAir) == 0x02);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0xF0);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_FLOOR_SWITCH);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0); /* No damage taken */

    printf("[PASS] ApplyLinkCollisionWithEnemy (Cheep-Cheep)\n");
}

/* Test ApplyLinkCollisionWithEnemy: Goomba Stomp and Damage */
static void test_ApplyLinkCollision_Goomba(void) {
    printf("[RUN ] ApplyLinkCollisionWithEnemy (Goomba)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    gb_write_hram(&gb, hActiveEntityType, ENTITY_GOOMBA);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00); /* 4 damage */

    /* Case 1: Airborne and falling in top-down mode (velocity Z negative = falling) */
    gb_write(&gb, wIsLinkInTheAir, 0x01);
    gb_write_hram(&gb, hIsSideScrolling, 0x00);
    gb_write_hram(&gb, hLinkVelocityZ, 0xF8); /* Falling */
    gb_write_hram(&gb, hLinkCountdown, 0x00);
    gb_write(&gb, wEntitiesStateTable + bc, 0x01);

    ApplyLinkCollisionWithEnemy(&gb, bc);

    assert(gb_read_hram(&gb, hLinkCountdown) == 0x02);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x02);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x30);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_FLOOR_SWITCH);
    assert(gb_read_hram(&gb, hLinkVelocityZ) == 0x10);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0); /* No damage */

    /* Case 2: Airborne and falling in side-scrolling mode (speed Y >= 0 = falling) */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GOOMBA);
    gb_write(&gb, wIsLinkInTheAir, 0x01);
    gb_write_hram(&gb, hIsSideScrolling, 0x01);
    gb_write_hram(&gb, hLinkSpeedY, 0x08); /* Falling */
    gb_write_hram(&gb, hLinkCountdown, 0x00);

    ApplyLinkCollisionWithEnemy(&gb, bc);

    assert(gb_read_hram(&gb, hLinkCountdown) == 0x02);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x02);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0xF0);

    /* Case 3: On ground (not airborne) -> Link takes damage */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GOOMBA);
    gb_write(&gb, wIsLinkInTheAir, 0x00);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00); /* 4 damage */

    ApplyLinkCollisionWithEnemy(&gb, bc);

    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);
    assert(gb_read(&gb, wInvincibilityCounter) == 0x50);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_LINK_HURT);

    /* Case 4: Airborne but rising in top-down mode (velocity Z positive = rising) -> takes damage */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GOOMBA);
    gb_write(&gb, wIsLinkInTheAir, 0x01);
    gb_write_hram(&gb, hIsSideScrolling, 0x00);
    gb_write_hram(&gb, hLinkVelocityZ, 0x18); /* Rising */
    gb_write_hram(&gb, hLinkCountdown, 0x00);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);

    ApplyLinkCollisionWithEnemy(&gb, bc);

    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);

    printf("[PASS] ApplyLinkCollisionWithEnemy (Goomba)\n");
}

/* Test ApplyLinkCollisionWithEnemy: Gel, Cue Ball, Rolling Bones Bar, Moblin King */
static void test_ApplyLinkCollision_SpecialEntities(void) {
    printf("[RUN ] ApplyLinkCollisionWithEnemy (Gel, Cue Ball, Moblin King)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x03;

    /* Case 1: Gel latch */
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GEL);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x80);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x04);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 2: Ignore collision countdown active blocks normal enemy damage */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wIgnoreLinkCollisionsCountdown, 0x0A);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 3: Cue Ball bypasses ignore collisions countdown */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_CUE_BALL);
    gb_write(&gb, wIgnoreLinkCollisionsCountdown, 0x0A);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);

    /* Case 4: Rolling Bones Bar bypasses ignore collisions countdown */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ROLLING_BONES_BAR);
    gb_write(&gb, wIgnoreLinkCollisionsCountdown, 0x0A);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);

    /* Case 5: Moblin King in state 4 */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN_KING);
    gb_write_hram(&gb, hActiveEntityState, 0x04);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x08);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_LINK_HURT);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    printf("[PASS] ApplyLinkCollisionWithEnemy (Gel, Cue Ball, Moblin King)\n");
}

/* Test ApplyLinkCollisionWithEnemy: Immunity and Damage Calculations */
static void test_ApplyLinkCollision_DamageCalculations(void) {
    printf("[RUN ] ApplyLinkCollisionWithEnemy (Immunity & Damage Math)\n");

    GBState gb;
    uint16_t bc = 0x02;

    /* Case 1: Invincibility counter active */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wInvincibilityCounter, 0x20);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x04); /* 0x18 = 24 */
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 2: Ocarina playing active */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wLinkPlayingOcarinaCountdown, 0x10);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 3: Got item dialog active */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wDialogGotItem, 0x01);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 4: Collision immunity flag active */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wIsLinkImmuneToCollisionDamage, 0x01);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 5: Standard green tunic, nominal damage (group 4 = 0x18 = 24 damage) */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x04);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 24);
    assert(gb_read(&gb, wInvincibilityCounter) == 0x50);
    assert(gb_read(&gb, wGuardianAcornCounter) == 0x00);

    /* Case 6: Blue Tunic halves damage (24 -> 12) */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wTunicType, TUNIC_BLUE);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x04);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 12);

    /* Case 7: Guardian Acorn: damage == 4 -> 0 damage */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wActivePowerUp, ACTIVE_POWER_UP_GUARDIAN_ACORN);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00); /* 4 damage */
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 8: Guardian Acorn: damage != 4 (e.g. 24) -> halved to 12 */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wActivePowerUp, ACTIVE_POWER_UP_GUARDIAN_ACORN);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x04); /* 24 damage */
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 12);

    /* Case 9: Power-up hits accumulation and loss after 3 hits */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wActivePowerUp, ACTIVE_POWER_UP_PIECE_OF_POWER);
    gb_write(&gb, wPowerUpHits, 0x01);
    gb_write_hram(&gb, hDefaultMusicTrack, MUSIC_OVERWORLD);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wPowerUpHits) == 0x02);
    assert(gb_read(&gb, wActivePowerUp) == ACTIVE_POWER_UP_PIECE_OF_POWER);

    /* 3rd hit drops power-up and restores music track */
    gb_write(&gb, wInvincibilityCounter, 0x00); /* Clear invincibility for next hit */
    gb_write(&gb, wIgnoreLinkCollisionsCountdown, 0x00);
    ApplyLinkCollisionWithEnemy(&gb, bc);
    assert(gb_read(&gb, wPowerUpHits) == 0x03);
    assert(gb_read(&gb, wActivePowerUp) == 0x00);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OVERWORLD);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_OVERWORLD);

    printf("[PASS] ApplyLinkCollisionWithEnemy (Immunity & Damage Math)\n");
}

/* Test DefaultEnemyDamageCollisionHandler alternating parity and func_003_6E2B */
static void test_DefaultEnemyDamageCollisionHandler_Parity(void) {
    printf("[RUN ] DefaultEnemyDamageCollisionHandler parity & dispatch\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    /* Setup entity hitbox at (0x40, 0x40) */
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    uint16_t hb = (uint16_t)(wEntitiesHitboxPositionTable + (bc * 4));
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x04);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x04);

    /* Link overlapping entity */
    gb_write_hram(&gb, hLinkPositionX, 0x38);
    gb_write_hram(&gb, hLinkPositionY, 0x38);

    /* Odd parity: hFrameCounter (0) ^ c (1) = 1 (bit 0 set) -> CheckLinkCollisionWithEnemy runs */
    gb_write_hram(&gb, hFrameCounter, 0x00);
    gb_write(&gb, wInvincibilityCounter, 0x00);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    DefaultEnemyDamageCollisionHandler(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);
    assert(gb_read(&gb, wInvincibilityCounter) == 0x50);

    /* Even parity: hFrameCounter (1) ^ c (1) = 0 (bit 0 clear) -> CheckLinkCollisionWithEnemy is skipped */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x04);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x04);
    gb_write_hram(&gb, hLinkPositionX, 0x38);
    gb_write_hram(&gb, hLinkPositionY, 0x38);
    gb_write_hram(&gb, hFrameCounter, 0x01);
    gb_write(&gb, wInvincibilityCounter, 0x00);
    DefaultEnemyDamageCollisionHandler(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x00); /* Skipped */

    printf("[PASS] DefaultEnemyDamageCollisionHandler parity & dispatch\n");
}

/* Test func_003_6E2B: Hitbox and Weapon Collision Branches */
static void test_func_003_6E2B_Branches(void) {
    printf("[RUN ] func_003_6E2B branches\n");

    GBState gb;
    uint16_t bc = 0x02;

    /* Case 1: Weapon inactive (wC140 == 0) -> early return */
    gb_init(&gb);
    gb_write(&gb, wC140, 0x00);
    func_003_6E2B(&gb, bc);
    assert(gb_read_hram(&gb, hJingle) == 0);

    /* Case 2: Flashing enemy (flash countdown < 0x18) -> early return */
    gb_init(&gb);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wEntitiesFlashCountdownTable + bc, 0x10);
    func_003_6E2B(&gb, bc);
    assert(gb_read_hram(&gb, hJingle) == 0);

    /* Case 3: Already processed hit this frame (wC1AC - 1 == bc) -> early return */
    gb_init(&gb);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wC1AC, bc + 1);
    func_003_6E2B(&gb, bc);
    assert(gb_read_hram(&gb, hJingle) == 0);

    /* Case 4: Ignoring hits countdown != 0 -> early return */
    gb_init(&gb);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x05);
    func_003_6E2B(&gb, bc);
    assert(gb_read_hram(&gb, hJingle) == 0);

    /* Setup overlapping weapon and entity hitboxes for entity tests */
    /* Weapon at X = 0x40, radius = 0x08, Y = 0x40, radius = 0x08 */
    /* Entity at X = 0x42, radius = 0x06, Y = 0x42, radius = 0x06 */
    uint16_t hb = (uint16_t)(wEntitiesHitboxPositionTable + (bc * 4));

    /* Case 5: Flame Shooter + Level 2 shield + direction UP -> blocks fire */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FLAME_SHOOTER);
    gb_write_hram(&gb, hActiveEntityPosX, 0x42);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x42);
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x06);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x06);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x40);
    gb_write(&gb, wC143, 0x08);
    gb_write(&gb, wShieldLevel, 0x02);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_UP);

    func_003_6E2B(&gb, bc);

    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x04);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x08);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x01);

    /* Case 6: Bouncing Bombite in state 2 -> reverses speed */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BOUNCING_BOMBITE);
    gb_write_hram(&gb, hActiveEntityState, 0x02);
    gb_write_hram(&gb, hActiveEntityPosX, 0x42);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x42);
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x06);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x06);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x40);
    gb_write(&gb, wC143, 0x08);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedYTable + bc, (uint8_t)-8);

    func_003_6E2B(&gb, bc);

    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == (uint8_t)-16);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x08);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x40);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + bc) == 0x08);

    /* Case 7: Knight with SWORD_CLINK_OFF -> clink spark */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_KNIGHT);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_SWORD_CLINK_OFF);
    gb_write_hram(&gb, hActiveEntityPosX, 0x42);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x42);
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x06);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x06);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x40);
    gb_write(&gb, wC143, 0x08);
    gb_write(&gb, wEntitiesPrivateState1Table + bc, 0x05);
    gb_write(&gb, wSwordCharge, 0x20);

    func_003_6E2B(&gb, bc);

    assert(gb_read(&gb, wEntitiesPrivateState1Table + bc) == (uint8_t)-5);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + bc) == 0x0C);
    assert(gb_read(&gb, wC160) == 0x01);
    assert(gb_read(&gb, wSwordCharge) == 0x00);
    assert(gb_read_hram(&gb, hMultiPurpose0) == 0x42);
    assert(gb_read_hram(&gb, hMultiPurpose1) == 0x42);

    /* Case 8: Spiked Beetle flipped on hit */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_SPIKED_BEETLE);
    gb_write_hram(&gb, hActiveEntityPosX, 0x42);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x42);
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x06);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x06);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x40);
    gb_write(&gb, wC143, 0x08);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);

    func_003_6E2B(&gb, bc);

    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x03);
    assert(gb_read(&gb, wEntitiesSpeedZTable + bc) == 0x20);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0xFF);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x10);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x00);

    /* Case 9: Pairodd projectile sets collisions table to 0xFF */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_PAIRODD_PROJECTILE);
    gb_write_hram(&gb, hActiveEntityPosX, 0x42);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x42);
    gb_write(&gb, hb + 0, 0x00);
    gb_write(&gb, hb + 1, 0x06);
    gb_write(&gb, hb + 2, 0x00);
    gb_write(&gb, hb + 3, 0x06);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x40);
    gb_write(&gb, wC143, 0x08);

    func_003_6E2B(&gb, bc);

    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0xFF);

    printf("[PASS] func_003_6E2B branches\n");
}


/* Test Data_003_6FE4, Data_003_73E7, Data_003_473C, Data_003_43EC table values */
static void test_DataTables_SwordDamage(void) {
    printf("[RUN ] Data_003_6FE4, Data_003_73E7, Data_003_473C, Data_003_43EC\n");

    /* Data_003_6FE4 size and values */
    assert(sizeof(Data_003_6FE4) == 4);
    assert(Data_003_6FE4[0] == 0x00);
    assert(Data_003_6FE4[1] == 0x01);
    assert(Data_003_6FE4[2] == 0x02);
    assert(Data_003_6FE4[3] == 0x03);

    /* Data_003_73E7 size and values */
    assert(sizeof(Data_003_73E7) == 4);
    assert(Data_003_73E7[0] == 0x2D);
    assert(Data_003_73E7[1] == 0x2E);
    assert(Data_003_73E7[2] == 0x38);
    assert(Data_003_73E7[3] == 0x37);

    /* Data_003_473C size and values */
    assert(sizeof(Data_003_473C) == 128);
    assert(Data_003_473C[0] == 0x00); /* sword basic, index 0 */
    assert(Data_003_473C[1] == 0x01); /* sword basic, index 1 */
    assert(Data_003_473C[2] == 0x02); /* sword basic, index 2 */
    assert(Data_003_473C[3] == 0x40); /* sword basic, index 3 */
    assert(Data_003_473C[6] == 0xFF); /* stun */
    assert(Data_003_473C[8 * 8 + 2] == 0x18); /* boomerang index 2 */
    assert(Data_003_473C[8 * 8 + 3] == 0xFE); /* boomerang burn */
    assert(Data_003_473C[8 * 8 + 5] == 0xFD); /* boomerang fairy */

    /* Data_003_43EC size and values */
    assert(sizeof(Data_003_43EC) == 848);
    assert(Data_003_43EC[0] == 0x01);
    assert(Data_003_43EC[1] == 0x01);
    assert(Data_003_43EC[52 * 16 + 0] == 0x01);
    assert(Data_003_43EC[52 * 16 + 9] == 0x06);

    printf("[PASS] Data_003_6FE4, Data_003_73E7, Data_003_473C, Data_003_43EC\n");
}

/* Test EnemyCollidedWithSword: Special Entities */
static void test_EnemyCollidedWithSword_SpecialEntities(void) {
    printf("[RUN ] EnemyCollidedWithSword (Special Entities)\n");

    GBState gb;
    uint16_t bc = 0x03;

    /* Case 1: Flame shooter ignores sword collision */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FLAME_SHOOTER);
    gb_write(&gb, wEntitiesStateTable + bc, 0x01);
    gb_write(&gb, wC160, 0x00);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x01);
    assert(gb_read(&gb, wC160) == 0x00);

    /* Case 2: Final Nightmare forms */
    /* Form 0: returns immediately */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FINAL_NIGHTMARE);
    gb_write(&gb, wFinalNightmareForm, 0x00);
    gb_write(&gb, wEntitiesStateTable + bc, 0x02);
    EnemyCollidedWithSword(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x02);

    /* Form 1: increments state and writes 0x06 */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FINAL_NIGHTMARE);
    gb_write(&gb, wFinalNightmareForm, 0x01);
    gb_write(&gb, wEntitiesStateTable + bc, 0x02);
    EnemyCollidedWithSword(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x06);

    /* Form 2: spin attack increments state */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FINAL_NIGHTMARE);
    gb_write(&gb, wFinalNightmareForm, 0x02);
    gb_write(&gb, wEntitiesStateTable + bc, 0x02);
    gb_write(&gb, wIsUsingSpinAttack, 0x01);
    EnemyCollidedWithSword(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x03);

    /* Form 2: wC16A < 4 increments state */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FINAL_NIGHTMARE);
    gb_write(&gb, wFinalNightmareForm, 0x02);
    gb_write(&gb, wEntitiesStateTable + bc, 0x02);
    gb_write(&gb, wIsUsingSpinAttack, 0x00);
    gb_write(&gb, wC16A, 0x03);
    EnemyCollidedWithSword(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x03);

    /* Form 2: wC16A >= 4 and no spin attack returns without increment */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_FINAL_NIGHTMARE);
    gb_write(&gb, wFinalNightmareForm, 0x02);
    gb_write(&gb, wEntitiesStateTable + bc, 0x02);
    gb_write(&gb, wIsUsingSpinAttack, 0x00);
    gb_write(&gb, wC16A, 0x04);
    EnemyCollidedWithSword(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x02);

    /* Case 3: Buzz Blob electrocution reaction */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BUZZ_BLOB);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesStateTable + bc, 0x00);
    gb_write(&gb, wSwordAnimationState, 0x03);
    gb_write(&gb, wC16A, 0x02);
    gb_write(&gb, wIsUsingSpinAttack, 0x01);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x01);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x40);
    assert(gb_read(&gb, wD464) == 0x40);
    assert(gb_read(&gb, wSwordAnimationState) == 0x00);
    assert(gb_read(&gb, wC16A) == 0x00);
    assert(gb_read(&gb, wIsUsingSpinAttack) == 0x00);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_BUZZ_BLOB_ELECTROCUTE);

    /* Case 4: Bouncing Bombite reaction */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BOUNCING_BOMBITE);
    gb_write_hram(&gb, hLinkPositionX, 0x40);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x20);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x02);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x40);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + bc) == 0x08);

    /* Case 5: Angler Fish countdown */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ANGLER_FISH);
    gb_write(&gb, wIgnoreLinkCollisionsCountdown, 0x00);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x08);

    /* Case 6: Slime Eye variants */
    /* hMultiPurposeG != 0 returns immediately */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_SLIME_EYE);
    gb_write_hram(&gb, hMultiPurposeG, 0x01);
    gb_write(&gb, wIgnoreLinkCollisionsCountdown, 0x00);
    EnemyCollidedWithSword(&gb, bc);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x10); /* from func_003_6DDF */

    /* privateState1 == 4 with pegasus boots */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_SLIME_EYE);
    gb_write_hram(&gb, hMultiPurposeG, 0x00);
    gb_write(&gb, wEntitiesPrivateState1Table + bc, 0x04);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    EnemyCollidedWithSword(&gb, bc);
    assert(gb_read(&gb, wEntitiesPrivateCountdown2Table + bc) == 0x0C);

    /* Case 7: Knight with SWORD_CLINK_OFF */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_KNIGHT);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_SWORD_CLINK_OFF);
    gb_write(&gb, wEntitiesPrivateState1Table + bc, 0x04);
    gb_write(&gb, wSwordCharge, 0x10);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wEntitiesPrivateState1Table + bc) == (uint8_t)-4);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + bc) == 0x0C);
    assert(gb_read(&gb, wC160) == 0x01);
    assert(gb_read(&gb, wSwordCharge) == 0x00);

    /* Case 8: Genie jar with SWORD_CLINK_OFF */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GENIE);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_SWORD_CLINK_OFF);
    gb_write(&gb, wEntitiesFlashCountdownTable + bc, 0x10);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wEntitiesFlashCountdownTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x10);

    /* Case 9: Cue Ball resets pegasus boots */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_CUE_BALL);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wIsRunningWithPegasusBoots) == 0x00);

    /* Case 10: Iron Mask: frontal vs rear collision */
    /* Frontal: Link facing RIGHT (0), Iron Mask facing LEFT (1) -> deflects */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_IRON_MASK);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    gb_write(&gb, wEntitiesPrivateState2Table + bc, 0x00);
    gb_write(&gb, wSwordLevel, 0x01);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x10);
    assert(gb_read(&gb, wC1AC) == 0x00); /* Did not call ApplySwordDamagesToEnemy */

    /* Rear: Link facing RIGHT (0), Iron Mask facing RIGHT (0) -> damage dealt */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_IRON_MASK);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesPrivateState2Table + bc, 0x00);
    gb_write(&gb, wSwordLevel, 0x01);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wC1AC) == (uint8_t)(bc + 1)); /* Proceeded to ApplySwordDamagesToEnemy */

    /* Case 11: Anti-Fairy immune to sword */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ANTI_FAIRY);
    gb_write(&gb, wC160, 0x00);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wC160) == 0x00); /* Returns without applying sword reaction */

    printf("[PASS] EnemyCollidedWithSword (Special Entities)\n");
}

/* Test EnemyCollidedWithSword: Default & Power Recoil */
static void test_EnemyCollidedWithSword_DefaultAndPowerRecoil(void) {
    printf("[RUN ] EnemyCollidedWithSword (Default & Power Recoil)\n");

    GBState gb;
    uint16_t bc = 0x02;

    /* Case 1: Standard sword collision without power boost */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, 0x00);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wSwordCharge, 0x20);
    gb_write(&gb, wC16A, 0x05);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wC160) == 0x01);
    assert(gb_read(&gb, wC16D) == 0x0C);
    assert(gb_read(&gb, wSwordCharge) == 0x00);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_ENEMY_HIT);
    assert(gb_read(&gb, wEntitiesPowerRecoilingTable + bc) == 0x00);

    /* Case 1b: Damage == 0 retains JINGLE_BUMP set during recoil */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, 0x00);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x04); /* damage 0 for sword level 1 */

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read_hram(&gb, hJingle) == JINGLE_BUMP);

    /* Case 2: Power recoil with Red Tunic */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, 0x00);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wTunicType, TUNIC_RED);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x10);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x20);
    assert(gb_read(&gb, wEntitiesPowerRecoilingTable + bc) == 0x01);
    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_POWER_HIT);

    /* Case 3: Power recoil with Piece of Power when entity dies */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write_hram(&gb, hActiveEntityType, 0x00);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wActivePowerUp, ACTIVE_POWER_UP_PIECE_OF_POWER);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x01); /* Will die from sword hit */
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);

    EnemyCollidedWithSword(&gb, bc);

    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DYING);
    assert(gb_read(&gb, wEntitiesPrivateCountdown3Table + bc) == 0x40);

    printf("[PASS] EnemyCollidedWithSword (Default & Power Recoil)\n");
}

/* Test ApplySwordDamagesToEnemy: Damage Types & SFX */
static void test_ApplySwordDamagesToEnemy_DamageTypes(void) {
    printf("[RUN ] ApplySwordDamagesToEnemy (Damage Types & SFX)\n");

    GBState gb;
    uint16_t bc = 0x01;

    /* Case 1: Basic sword level 1 sets attack damage type 0 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x10);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wC1AC) == (uint8_t)(bc + 1));
    assert(gb_read(&gb, wAttackDamageType) == 0x00);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_ENEMY_HIT);
    /* Health group 0 with damage type 0: entry 1 -> damage 1 */
    assert(gb_read(&gb, wEntitiesHealthTable + bc) == 0x0F);

    /* Case 2: Sword level 1 + spin attack boosts damage type to 1 (sword +1) */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wIsUsingSpinAttack, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x10);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wAttackDamageType) == 0x01);
    /* Health group 0 with damage type 1: entry 1 -> damage 2 */
    assert(gb_read(&gb, wEntitiesHealthTable + bc) == 0x0E);

    /* Case 3: Boss entity hurt sfx */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_IS_BOSS);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x20);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_BOSS_HURT);

    /* Case 4: Cucco hurt sfx */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_CUCCO);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x20);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read_hram(&gb, hWaveSfx) == WAVE_SFX_CUCCO_HURT);

    /* Case 5: Zero damage entity (health group 9 with sword basic deals 0) */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x09);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x10);
    gb_write_hram(&gb, hJingle, 0x00);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read_hram(&gb, hJingle) == 0x00); /* No hit jingle */
    assert(gb_read(&gb, wEntitiesHealthTable + bc) == 0x10); /* No damage taken */

    printf("[PASS] ApplySwordDamagesToEnemy (Damage Types & SFX)\n");
}

/* Test ApplySwordDamagesToEnemy: Burn, Stun, Morph */
static void test_ApplySwordDamagesToEnemy_SpecialDamages(void) {
    printf("[RUN ] ApplySwordDamagesToEnemy (Burn, Stun, Morph)\n");

    GBState gb;
    uint16_t bc = 0x02;

    /* Case 1: Burn effect (damage code 0xFE, e.g. magic powder on health group 0) */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x0A); /* damage_type = 9 (magic powder) */
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesPhysicsFlagsTable + bc, 0x04);
    gb_write(&gb, wEntitiesOptions1Table + bc, 0xFF);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_BURSTING_FLAME);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_BURNING);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x60);
    assert(gb_read(&gb, wEntitiesPhysicsFlagsTable + bc) == 0x06); /* +2 */
    assert(gb_read(&gb, wEntitiesOptions1Table + bc) == (ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL | ENTITY_OPT1_SWORD_CLINK_OFF | ENTITY_OPT1_IS_BOSS));

    /* Case 2: Stun effect (damage code 0xFF, e.g. hookshot type 6 on health group 8) */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x07); /* damage_type = 6 (hookshot) */
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x08);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_STUNNED);
    assert(gb_read(&gb, wEntitiesPrivateCountdown2Table + bc) == 0xFF);
    assert(gb_read(&gb, wEntitiesSpeedZTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x0A);

    /* Case 3: Morph / Fairy effect (damage code 0xFD, e.g. magic powder on health group 43) */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x0A); /* damage_type = 9 (magic powder) */
    gb_write(&gb, wEntitiesHealthGroup + bc, 43);
    gb_write(&gb, wEntitiesTypeTable + bc, 0x50);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x05);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wEntitiesTypeTable + bc) == 0x2F); /* Transformed into fairy */
    assert(gb_read(&gb, wEntitiesSlowTransitionCountdownTable + bc) == 0x80);
    assert(gb_read_hram(&gb, hMultiPurpose0) == 0x30);
    assert(gb_read_hram(&gb, hMultiPurpose1) == 0x3B); /* 0x40 - 0x05 */

    printf("[PASS] ApplySwordDamagesToEnemy (Burn, Stun, Morph)\n");
}

/* Test ApplySwordDamagesToEnemy: Dying & Defeat */
static void test_ApplySwordDamagesToEnemy_DyingAndDefeat(void) {
    printf("[RUN ] ApplySwordDamagesToEnemy (Dying & Boss Defeat)\n");

    GBState gb;
    uint16_t bc = 0x01;

    /* Case 1: Standard enemy defeat */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00); /* damage = 1 */
    gb_write(&gb, wEntitiesHealthTable + bc, 0x01); /* health 1 <= 1 -> dead */
    gb_write(&gb, wEntitiesStateTable + bc, 0x02);
    gb_write(&gb, wEntitiesPhysicsFlagsTable + bc, 0x50);
    gb_write(&gb, wEntitiesOptions1Table + bc, 0x00);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wEntitiesHealthTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DYING);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesPrivateCountdown3Table + bc) == 0x2F);
    assert(gb_read(&gb, wEntitiesFlashCountdownTable + bc) == 0x18);
    assert(gb_read(&gb, wEntitiesPhysicsFlagsTable + bc) == 0x54); /* (0x50 & 0xF0) | 0x04 */

    /* Case 2: Facade boss defeat */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x01);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_FACADE);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_IS_BOSS);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wBossAgonySFXCountdown) == 0x03);
    assert(gb_read(&gb, wEntitiesPrivateState2Table + bc) == 0x00);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_BOSS_DEFEAT);

    /* Case 3: Evil Eagle boss defeat dialog */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x01);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_EVIL_EAGLE);
    gb_write(&gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_IS_BOSS);
    gb_write_hram(&gb, hLinkPositionY, 0x50);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read_hram(&gb, hLinkPositionY) == 0x50); /* Restored after dialog */
    assert(gb_read(&gb, wBossAgonySFXCountdown) == 0x03);

    /* Case 4: Ghini defeat kills companion Hiding / Giant Ghinis */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x01);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_GHINI);

    /* Companion in slot 2 */
    gb_write(&gb, wEntitiesTypeTable + 2, ENTITY_HIDING_GHINI);
    gb_write(&gb, wEntitiesStateTable + 2, 0x00);
    gb_write(&gb, wEntitiesStatusTable + 2, ENTITY_STATUS_ACTIVE);

    /* Companion in slot 3 */
    gb_write(&gb, wEntitiesTypeTable + 3, ENTITY_GIANT_GHINI);
    gb_write(&gb, wEntitiesStateTable + 3, 0x00);
    gb_write(&gb, wEntitiesStatusTable + 3, ENTITY_STATUS_ACTIVE);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wEntitiesDroppedItemTable + bc) == ENTITY_DROPPABLE_RUPEE);
    assert(gb_read(&gb, wEntitiesStatusTable + 2) == ENTITY_STATUS_DYING);
    assert(gb_read(&gb, wEntitiesPrivateCountdown3Table + 2) == 0x1F);
    assert(gb_read(&gb, wEntitiesStatusTable + 3) == ENTITY_STATUS_DYING);
    assert(gb_read(&gb, wEntitiesPrivateCountdown3Table + 3) == 0x1F);

    /* Case 5: Moldorm survived hit gets longer flash/countdown */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wEntitiesHealthTable + bc, 0x10);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOLDORM);

    ApplySwordDamagesToEnemy(&gb, bc);

    assert(gb_read(&gb, wEntitiesFlashCountdownTable + bc) == 0x28);
    assert(gb_read(&gb, wEntitiesPrivateCountdown2Table + bc) == 0xC8);

    printf("[PASS] ApplySwordDamagesToEnemy (Dying & Boss Defeat)\n");
}

/* Test Data_003_74E4 and Data_003_74E8 ROM tables */
static void test_DataTables_SwordEnemyCollision(void) {
    printf("[RUN ] Data_003_74E4 and Data_003_74E8\n");

    assert(sizeof(Data_003_74E4) == 4);
    assert(Data_003_74E4[0] == 0x00);
    assert(Data_003_74E4[1] == 0xF0);
    assert(Data_003_74E4[2] == 0xF8);
    assert(Data_003_74E4[3] == 0xFC);

    assert(sizeof(Data_003_74E8) == 4);
    assert(Data_003_74E8[0] == 0xFC);
    assert(Data_003_74E8[1] == 0xFC);
    assert(Data_003_74E8[2] == 0xF0);
    assert(Data_003_74E8[3] == 0x00);

    printf("[PASS] Data_003_74E4 and Data_003_74E8\n");
}

/* Test func_003_73EB early branches to label_003_74EC */
static void test_func_003_73EB_EarlyBranches(void) {
    printf("[RUN ] func_003_73EB (early branches to label_003_74EC)\n");

    GBState gb;
    uint16_t bc = 0x01;

    /* Case 1: wIgnoreLinkCollisionsCountdown != 0 */
    gb_init(&gb);
    gb_write(&gb, wIgnoreLinkCollisionsCountdown, 0x05);
    gb_write(&gb, wC140, 0x40);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x40);
    gb_write(&gb, wC143, 0x08);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    func_003_73EB(&gb, bc);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x00);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x05);

    /* Case 2: wC1AC != 0 */
    gb_init(&gb);
    gb_write(&gb, wC1AC, 0x02);
    gb_write(&gb, wC140, 0x40);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    func_003_73EB(&gb, bc);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x00);

    /* Case 3: hLinkPunchedAwayCountdown != 0 */
    gb_init(&gb);
    gb_write_hram(&gb, hLinkPunchedAwayCountdown, 0x04);
    gb_write(&gb, wC140, 0x40);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    func_003_73EB(&gb, bc);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x00);

    /* Case 4: wIsUsingSpinAttack != 0 */
    gb_init(&gb);
    gb_write(&gb, wIsUsingSpinAttack, 0x01);
    gb_write(&gb, wC140, 0x40);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    func_003_73EB(&gb, bc);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x00);

    /* Case 5: wC140 == 0 */
    gb_init(&gb);
    gb_write(&gb, wC140, 0x00);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    func_003_73EB(&gb, bc);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x00);

    /* Case 6: Same direction (Link facing RIGHT, Entity facing RIGHT) */
    gb_init(&gb);
    gb_write(&gb, wC140, 0x40);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    func_003_73EB(&gb, bc);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x00);

    /* Case 7: Out of X range */
    gb_init(&gb);
    gb_write(&gb, wC140, 0x20); /* Sword X = 0x20 */
    gb_write(&gb, wC141, 0x04); /* Sword half-width = 4 */
    gb_write(&gb, wD5C0, 0x00);
    gb_write(&gb, wD5C1, 0x04); /* Entity half-width = 4 */
    gb_write_hram(&gb, hActiveEntityPosX, 0x40); /* Entity X = 0x40. Diff = 0x20 >= 8 */
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    func_003_73EB(&gb, bc);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x00);

    printf("[PASS] func_003_73EB (early branches to label_003_74EC)\n");
}

/* Test func_003_73EB sword collision with non-Blaino enemy */
static void test_func_003_73EB_SwordCollision_NonBlaino(void) {
    printf("[RUN ] func_003_73EB (sword collision with non-Blaino enemy)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;

    /* Entity at (0x50, 0x50), weapon at (0x50, 0x50) */
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN_SWORD);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x50);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x50);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x00);
    gb_write(&gb, wD5C0, 0x00);
    gb_write(&gb, wD5C1, 0x08);
    gb_write(&gb, wD5C2, 0x00);
    gb_write(&gb, wD5C3, 0x08);

    /* Sword active */
    gb_write(&gb, wC140, 0x50);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x50);
    gb_write(&gb, wC143, 0x08);

    /* Link facing UP (2), entity facing DOWN (3) */
    gb_write_hram(&gb, hLinkDirection, DIRECTION_UP);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_DOWN);

    /* Link position below entity */
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x70);

    /* Pegasus boots active, sword charge active */
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, wSwordCharge, 0x20);

    /* Spin attack timer */
    gb_write(&gb, wIsUsingSpinAttack, 0x00);
    gb_write(&gb, wC16A, 0x05);

    func_003_73EB(&gb, bc);

    /* Assertions */
    assert(gb_read(&gb, wIsRunningWithPegasusBoots) == 0x00);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x08);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0x08);
    assert(gb_read(&gb, wSwordCharge) == 0x00);
    assert(gb_read(&gb, wC16D) == 0x0C);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x0C);

    /* Spark coordinates in hMultiPurpose0/1 computed from Data_003_74E4/E8 with DIRECTION_UP (2) */
    assert(gb_read_hram(&gb, hMultiPurpose0) == (uint8_t)(0x50 + 0xF8));
    assert(gb_read_hram(&gb, hMultiPurpose1) == (uint8_t)(0x50 + 0xF0));

    printf("[PASS] func_003_73EB (sword collision with non-Blaino enemy)\n");
}

/* Test func_003_73EB sword collision with Blaino */
static void test_func_003_73EB_SwordCollision_Blaino(void) {
    printf("[RUN ] func_003_73EB (sword collision with Blaino)\n");

    GBState gb;
    uint16_t bc = 0x03;

    /* Case A: wD205 == 0 -> Jingle bump, link punched away countdown 0x0C */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BLAINO);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wC140, 0x50);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x50);
    gb_write(&gb, wC143, 0x08);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    gb_write(&gb, wD205, 0x00);

    func_003_73EB(&gb, bc);

    assert(gb_read_hram(&gb, hJingle) == JINGLE_BUMP);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x0C);

    /* Case B: wD205 == 1 -> countdown 0x10, punched away countdown 0x0C */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BLAINO);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wC140, 0x50);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x50);
    gb_write(&gb, wC143, 0x08);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    gb_write(&gb, wD205, 0x01);

    func_003_73EB(&gb, bc);

    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x10);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x0C);

    /* Case C: wD205 == 3 with high inertia -> knockout punch */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BLAINO);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wC140, 0x50);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x50);
    gb_write(&gb, wC143, 0x08);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    gb_write(&gb, wD205, 0x03);
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x30);

    func_003_73EB(&gb, bc);

    assert(gb_read(&gb, wLinkMotionState) == LINK_MOTION_UNKNOWN_0A);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0xD0);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);
    assert(gb_read_hram(&gb, hLinkVelocityZ) == 0x30);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_STRONG_BUMP);

    /* Case D: wD205 == 2 -> countdown 0x20 */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BLAINO);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wC140, 0x50);
    gb_write(&gb, wC141, 0x08);
    gb_write(&gb, wC142, 0x50);
    gb_write(&gb, wC143, 0x08);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    gb_write(&gb, wD205, 0x02);

    func_003_73EB(&gb, bc);

    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x20);
    assert(gb_read_hram(&gb, hLinkPunchedAwayCountdown) == 0x0C);

    printf("[PASS] func_003_73EB (sword collision with Blaino)\n");
}

/* Test label_003_74EC body collision and Blaino responses */
static void test_label_003_74EC_Behavior(void) {
    printf("[RUN ] label_003_74EC (body collision & Blaino responses)\n");

    GBState gb;
    uint16_t bc = 0x02;

    /* Case 1: Even frame parity -> skipped */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02); /* (2 ^ 2) & 1 == 0 */
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write_hram(&gb, hActiveEntityPosX, 0x58);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x58);
    label_003_74EC(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x00);

    /* Case 2: Odd parity, but outside hitbox bounds */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x03); /* (3 ^ 2) & 1 == 1 */
    gb_write_hram(&gb, hLinkPositionX, 0x10);
    gb_write_hram(&gb, hLinkPositionY, 0x10);
    gb_write_hram(&gb, hActiveEntityPosX, 0x60);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x60);
    label_003_74EC(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x00);

    /* Case 3: Overlapping hit with regular enemy while invincible -> no damage */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x03);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write_hram(&gb, hActiveEntityPosX, 0x58);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x58);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wInvincibilityCounter, 0x20);
    label_003_74EC(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x00);

    /* Case 4: Overlapping hit with regular enemy -> takes damage */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x03);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write_hram(&gb, hActiveEntityPosX, 0x58);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x58);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    label_003_74EC(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) > 0);

    /* Case 5: Overlapping hit with Blaino in state 2 */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x03);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write_hram(&gb, hActiveEntityPosX, 0x58);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x58);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BLAINO);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wD205, 0x02);
    label_003_74EC(&gb, bc);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + bc) == 0xA0);
    assert(gb_read(&gb, wIgnoreLinkCollisionsCountdown) == 0x20);

    /* Case 6: Overlapping hit with Blaino knockout punch (wD205 = 3, inertia >= 0x22) */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x03);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write_hram(&gb, hActiveEntityPosX, 0x58);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x58);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BLAINO);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wD205, 0x03);
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x28);
    gb_write(&gb, wEntitiesDirectionTable + bc, 0x00);
    label_003_74EC(&gb, bc);
    assert(gb_read(&gb, wLinkMotionState) == LINK_MOTION_UNKNOWN_0A);
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x30);
    assert(gb_read_hram(&gb, hLinkVelocityZ) == 0x30);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_STRONG_BUMP);

    printf("[PASS] label_003_74EC (body collision & Blaino responses)\n");
}

/* Test func_003_75A2 filtering and parity */
static void test_func_003_75A2_FilteringAndParity(void) {
    printf("[RUN ] func_003_75A2 (filtering and frame parity)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01; /* Active projectile entity */

    /* Case 1: Target entity is same as active entity (e == bc) */
    gb_write_hram(&gb, hFrameCounter, 0x01); /* Parity matches e=1 */
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);
    func_003_75A2(&gb, bc);
    /* No collision with self */
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* Case 2: Frame parity mismatch ((hFrameCounter ^ e) & 1 != 0) */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x00); /* Even frame counter */
    /* Target entity index 1 has odd index -> (0 ^ 1) & 1 == 1 -> skipped */
    uint16_t de = 0x01;
    bc = 0x00;
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + de, 0x40);
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);
    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* Case 3: Target entity inactive (< ENTITY_STATUS_ACTIVE) */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02); /* Even frame counter */
    de = 0x02; /* Even index matches */
    bc = 0x00;
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_INIT); /* < 5 */
    gb_write(&gb, wEntitiesPosXTable + de, 0x40);
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);
    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* Case 4: Target entity has ENTITY_PHYSICS_PROJECTILE_NOCLIP */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    de = 0x02;
    bc = 0x00;
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPhysicsFlagsTable + de, ENTITY_PHYSICS_PROJECTILE_NOCLIP);
    gb_write(&gb, wEntitiesPosXTable + de, 0x40);
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);
    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* Case 5: Target entity X distance out of bounds (dx >= 0x18) */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    de = 0x02;
    bc = 0x00;
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + de, 0x40 + 0x18); /* Too far */
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);
    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* Case 6: Target entity sprite variant is $FF */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    de = 0x02;
    bc = 0x00;
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + de, 0x40);
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);
    gb_write(&gb, wEntitiesSpriteVariantTable + de, 0xFF);
    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    printf("[PASS] func_003_75A2 (filtering and frame parity)\n");
}

/* Test func_003_75A2 Bouncing Bombite and Grabbable entities */
static void test_func_003_75A2_BouncingBombiteAndGrabbable(void) {
    printf("[RUN ] func_003_75A2 (Bouncing Bombite and Grabbable)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x00;
    uint16_t de = 0x02;

    /* Active entity is Bouncing Bombite, colliding with target Bombite */
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BOUNCING_BOMBITE);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x15);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x20);

    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_BOUNCING_BOMBITE);
    gb_write(&gb, wEntitiesPosXTable + de, 0x50);
    gb_write(&gb, wEntitiesPosYTable + de, 0x50);

    func_003_75A2(&gb, bc);

    /* Active bombite countdown cleared */
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x00);
    /* Target bombite received speeds and state */
    assert(gb_read(&gb, wEntitiesSpeedXTable + de) == 0x10);
    assert(gb_read(&gb, wEntitiesSpeedYTable + de) == 0x20);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + de) == 0x40);
    assert(gb_read(&gb, wEntitiesStateTable + de) == 0x02);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + de) == 0x08);

    /* Grabbable entity hit by Boomerang */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_BOOMERANG);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x30);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x20);

    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPhysicsFlagsTable + de, ENTITY_PHYSICS_GRABBABLE);
    gb_write(&gb, wEntitiesPosXTable + de, 0x30);
    gb_write(&gb, wEntitiesPosYTable + de, 0x30);

    func_003_75A2(&gb, bc);

    /* Boomerang transition countdown reset */
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x00);
    /* Target entity grabbed (wEntitiesPrivateState5Table set to bc + 1 = 1) */
    assert(gb_read(&gb, wEntitiesPrivateState5Table + de) == 0x01);

    printf("[PASS] func_003_75A2 (Bouncing Bombite and Grabbable)\n");
}

/* Test func_003_75A2 Magic Powder and Iron Mask */
static void test_func_003_75A2_MagicPowderAndIronMask(void) {
    printf("[RUN ] func_003_75A2 (Magic Powder and Iron Mask)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x00;
    uint16_t de = 0x02;

    /* Magic Powder hits Mad Batter */
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MAGIC_POWDER_SPRINKLE);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);

    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_MAD_BATTER);
    gb_write(&gb, wEntitiesHitboxFlagsTable + de, HITFLAGS_IGNORE_HITS);
    gb_write(&gb, wEntitiesStateTable + de, 0x00);
    gb_write(&gb, wEntitiesPosXTable + de, 0x40);
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);

    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + de) == 0x01);

    /* Magic Powder hits Tarin outdoors */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MAGIC_POWDER_SPRINKLE);
    gb_write_hram(&gb, hActiveEntityPosX, 0x60);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x60);
    gb_write(&gb, wIsIndoor, 0x00);

    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_TARIN);
    gb_write(&gb, wEntitiesHitboxFlagsTable + de, HITFLAGS_IGNORE_HITS);
    gb_write(&gb, wEntitiesStateTable + de, 0x00);
    gb_write(&gb, wEntitiesPosXTable + de, 0x60);
    gb_write(&gb, wEntitiesPosYTable + de, 0x60);

    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + de) == 0x01);
    assert(gb_read(&gb, wEntitiesSlowTransitionCountdownTable + de) == 0x7F);
    assert(gb_read(&gb, wEntitiesFlashCountdownTable + de) == 0x10);

    /* Iron Mask head-on collision with Arrow -> deflects (wEntitiesCollisionsTable = 1) */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT); /* Facing right (0) */

    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_IRON_MASK);
    gb_write(&gb, wEntitiesDirectionTable + de, DIRECTION_LEFT);  /* Facing left (1) -> 1 ^ 1 = 0 == 0 */
    gb_write(&gb, wEntitiesPrivateState2Table + de, 0x00);        /* Mask on */
    gb_write(&gb, wEntitiesPosXTable + de, 0x50);
    gb_write(&gb, wEntitiesPosYTable + de, 0x50);

    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x01); /* Deflected! */

    /* Iron Mask head-on collision with Hookshot Chain -> strips mask! */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_HOOKSHOT_CHAIN);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x50);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x50);

    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_IRON_MASK);
    gb_write(&gb, wEntitiesDirectionTable + de, DIRECTION_LEFT);
    gb_write(&gb, wEntitiesPrivateState2Table + de, 0x00);
    gb_write(&gb, wEntitiesPosXTable + de, 0x50);
    gb_write(&gb, wEntitiesPosYTable + de, 0x50);

    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesPrivateState2Table + de) == 0x01); /* Mask removed */
    /* Check spawned mask entity in slot 15 */
    assert(gb_read(&gb, wEntitiesStatusTable + 15) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesTypeTable + 15) == ENTITY_IRON_MASKS_MASK);
    assert(gb_read(&gb, wEntitiesPrivateState5Table + 15) == 0x01); /* c + 1 */

    printf("[PASS] func_003_75A2 (Magic Powder and Iron Mask)\n");
}

/* Test func_003_75A2 Projectile reactions, thrown bounce, and arrow handling */
static void test_func_003_75A2_ProjectileReactions(void) {
    printf("[RUN ] func_003_75A2 (projectile reactions and thrown bounce)\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x00;
    uint16_t de = 0x02;

    /* Thrown entity bounce: status 8 */
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, 0x77); /* generic entity */
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_THROWN);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10); /* 16 >> 2 = 4 -> ~4 = 0xFB */
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0xF0); /* -16 >> 2 = -4 -> ~(-4) = 3 */
    gb_write(&gb, wEntitiesPrivateCountdown3Table + bc, 0x00);

    /* Target enemy */
    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesPosXTable + de, 0x40);
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);

    func_003_75A2(&gb, bc);
    assert(gb_read(&gb, wEntitiesPrivateCountdown3Table + bc) == 0x0C);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0xFB);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x03);
    assert(gb_read(&gb, wEntitiesThrownDirectionTable + bc) == 0xFF);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x00);

    /* Active Arrow state 1: sets countdown 3 instead of unloading */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityState, 0x01); /* state 1 (bomb arrow / active) */
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);

    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesPosXTable + de, 0x40);
    gb_write(&gb, wEntitiesPosYTable + de, 0x40);

    func_003_75A2(&gb, bc);
    /* Countdown set to 3 by func_003_77A7 */
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x03);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE); /* Not unloaded */

    printf("[PASS] func_003_75A2 (projectile reactions and thrown bounce)\n");
}

/* Test CheckExplosionInteractionWithEntities and GetVectorTowardsOtherEntity */
static void test_CheckExplosionInteractionWithEntities(void) {
    printf("[RUN ] CheckExplosionInteractionWithEntities & GetVectorTowardsOtherEntity\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x00; /* Explosion entity */
    uint16_t de = 0x03; /* Target enemy within explosion radius */

    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);

    /* Set up target enemy: Active, within 0x30 distance */
    gb_write(&gb, wEntitiesStatusTable + de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + de, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesHealthGroup + de, 0x00);
    gb_write(&gb, wEntitiesHealthTable + de, 0x08);
    gb_write(&gb, wEntitiesPosXTable + de, 0x48);
    gb_write(&gb, wEntitiesPosYTable + de, 0x48);
    gb_write(&gb, wEntitiesPosZTable + de, 0x00);

    /* Set up Link position to verify it gets preserved */
    gb_write_hram(&gb, hLinkPositionX, 0x10);
    gb_write_hram(&gb, hLinkPositionY, 0x20);

    CheckExplosionInteractionWithEntities(&gb, bc);

    /* Verify Link position was preserved */
    assert(gb_read_hram(&gb, hLinkPositionX) == 0x10);
    assert(gb_read_hram(&gb, hLinkPositionY) == 0x20);

    /* Verify damage type was set to bomb */
    assert(gb_read(&gb, wAttackDamageType) == DAMAGE_TYPE_BOMB);

    /* Verify recoil velocities were set for target entity */
    assert(gb_read(&gb, wEntitiesRecoilVelocityX + de) != 0 ||
           gb_read(&gb, wEntitiesRecoilVelocityY + de) != 0);

    /* Verify an out-of-range entity is untouched */
    uint16_t far_de = 0x05;
    gb_write(&gb, wEntitiesStatusTable + far_de, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + far_de, ENTITY_OCTOROK);
    gb_write(&gb, wEntitiesHealthTable + far_de, 0x08);
    gb_write(&gb, wEntitiesPosXTable + far_de, 0xA0); /* 0xA0 - 0x40 = 0x60 >= 0x30 */
    gb_write(&gb, wEntitiesPosYTable + far_de, 0xA0);
    gb_write(&gb, wEntitiesRecoilVelocityX + far_de, 0x00);
    gb_write(&gb, wEntitiesRecoilVelocityY + far_de, 0x00);

    CheckExplosionInteractionWithEntities(&gb, bc);
    assert(gb_read(&gb, wEntitiesHealthTable + far_de) == 0x08);
    assert(gb_read(&gb, wEntitiesRecoilVelocityX + far_de) == 0x00);
    assert(gb_read(&gb, wEntitiesRecoilVelocityY + far_de) == 0x00);

    /* Direct test of GetVectorTowardsOtherEntity */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x88);
    gb_write_hram(&gb, hLinkPositionY, 0x99);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x60);
    gb_write(&gb, wEntitiesPosYTable + 0x02, 0x40);

    GetVectorTowardsOtherEntity(&gb, 0x20, 0x02);
    /* Link position restored */
    assert(gb_read_hram(&gb, hLinkPositionX) == 0x88);
    assert(gb_read_hram(&gb, hLinkPositionY) == 0x99);
    /* Vector components computed */
    assert(gb_read_hram(&gb, hMultiPurpose1) != 0);

    printf("[PASS] CheckExplosionInteractionWithEntities & GetVectorTowardsOtherEntity\n");
}

/* Test Projectile Collision Tables (03:6BD6, 03:6BDA, 03:6A1E) */
static void test_DataTables_ProjectileCollision(void) {
    printf("[RUN ] ReversedDirectionsTable, Data_003_6BDA, OctorokRockSpriteVariants\n");

    assert(sizeof(ReversedDirectionsTable) == 4);
    assert(ReversedDirectionsTable[DIRECTION_RIGHT] == DIRECTION_LEFT);
    assert(ReversedDirectionsTable[DIRECTION_LEFT] == DIRECTION_RIGHT);
    assert(ReversedDirectionsTable[DIRECTION_UP] == DIRECTION_DOWN);
    assert(ReversedDirectionsTable[DIRECTION_DOWN] == DIRECTION_UP);

    assert(sizeof(Data_003_6BDA) == 4);
    assert(Data_003_6BDA[0] == 0x02);
    assert(Data_003_6BDA[1] == 0x0A);
    assert(Data_003_6BDA[2] == 0x0E);
    assert(Data_003_6BDA[3] == 0x06);

    assert(sizeof(OctorokRockSpriteVariants) == 8);
    assert(OctorokRockSpriteVariants[0] == 0x6C && OctorokRockSpriteVariants[1] == 0x01);
    assert(OctorokRockSpriteVariants[2] == 0x6C && OctorokRockSpriteVariants[3] == 0x21);
    assert(OctorokRockSpriteVariants[4] == 0x5C && OctorokRockSpriteVariants[5] == 0x01);
    assert(OctorokRockSpriteVariants[6] == 0x5C && OctorokRockSpriteVariants[7] == 0x21);

    printf("[PASS] ReversedDirectionsTable, Data_003_6BDA, OctorokRockSpriteVariants\n");
}

/* Test CheckLinkCollisionWithProjectile (03:6BDE) */
static void test_CheckLinkCollisionWithProjectile(void) {
    printf("[RUN ] CheckLinkCollisionWithProjectile\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00); /* 4 nominal damage */

    /* 1. Link is non-interactive: returns false */
    gb_write(&gb, wLinkMotionState, LINK_MOTION_TYPE_NON_INTERACTIVE);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == false);

    /* 2. Link is in the air: returns false */
    gb_write(&gb, wLinkMotionState, 0);
    gb_write_hram(&gb, hLinkPositionZ, 4);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == false);

    /* 3. Link is out of range horizontally */
    gb_write_hram(&gb, hLinkPositionZ, 0);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x50); /* diff_x = 0x20 + 6 = 38 >= 12 */
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == false);

    /* 4. Link is out of range vertically */
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x60); /* diff_y = 0x20 + 6 = 38 >= 12 */
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == false);

    /* 5. In range, unshielded (wIsUsingShield = 0): damages Link, sets collisions table = 0xFF */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x32);
    gb_write_hram(&gb, hLinkPositionY, 0x42);
    gb_write(&gb, wIsUsingShield, 0);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);
    assert(gb_read(&gb, wInvincibilityCounter) == 0x50);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0xFF);

    /* 6. Harmless entity: func_003_6CC0 skips damage */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesPhysicsFlagsTable + bc, ENTITY_PHYSICS_HARMLESS);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x32);
    gb_write_hram(&gb, hLinkPositionY, 0x42);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0); /* skipped */
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0xFF);

    /* 7. Falling Link animation (0x4E): func_003_6CC0 skips damage */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write_hram(&gb, hLinkAnimationState, 0x4E);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x32);
    gb_write_hram(&gb, hLinkPositionY, 0x42);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* 8. Unshielded Moblin arrow: damages Link and unloads */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN_ARROW);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* 9. Shield block with opposite direction: plays JINGLE_SHIELD_TING, no damage */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wIsUsingShield, 1);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_LEFT); /* opposite */
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_SHIELD_TING);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0xFF);

    /* 10. Shield with non-opposite direction: does NOT block */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wIsUsingShield, 1);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_UP); /* not opposite */
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);

    /* 11. Laser beam against lower-level shield (wShieldLevel = 1): hits and unloads */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_LASER_BEAM);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wIsUsingShield, 1);
    gb_write(&gb, wShieldLevel, 1);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* 12. Laser beam reflected by Mirror Shield (wShieldLevel = 2, matching angle):
       Link direction RIGHT (0) -> e = 0x02. Entity direction 0x04 -> (4 - 2) & 0x0F = 2 < 5 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_LASER_BEAM);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wIsUsingShield, 1);
    gb_write(&gb, wShieldLevel, 2);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, 0x04);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x02);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_SWORD_POKING);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* 13. Laser beam with Mirror Shield at non-matching angle: (7 - 2) & 0x0F = 5 >= 5 -> unloads */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_LASER_BEAM);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityPosY, 0x40);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wIsUsingShield, 1);
    gb_write(&gb, wShieldLevel, 2);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesDirectionTable + bc, 0x07);
    assert(CheckLinkCollisionWithProjectile(&gb, bc) == true);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    printf("[PASS] CheckLinkCollisionWithProjectile\n");
}

/* Test OctorokRockEntityHandler (03:6A26) */
static void test_OctorokRockEntityHandler(void) {
    printf("[RUN ] OctorokRockEntityHandler\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;
    gb_write(&gb, wActiveEntityIndex, bc);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesHealthGroup + bc, 0x00);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wTransitionSequenceCounter, 4);

    /* Case 1: Transition countdown != 0: bypasses collision check */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x05);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    OctorokRockEntityHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);

    /* Case 2: Transition countdown == 0: executes collision check */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    OctorokRockEntityHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0xFF);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0x04);

    printf("[PASS] OctorokRockEntityHandler\n");
}

void test_bank3_entities_collision(void) {
    test_EntityDamagesForGroup();
    test_ApplyLinkCollision_CheepCheep();
    test_ApplyLinkCollision_Goomba();
    test_ApplyLinkCollision_SpecialEntities();
    test_ApplyLinkCollision_DamageCalculations();
    test_DefaultEnemyDamageCollisionHandler_Parity();
    test_func_003_6E2B_Branches();
    test_DataTables_SwordDamage();
    test_EnemyCollidedWithSword_SpecialEntities();
    test_EnemyCollidedWithSword_DefaultAndPowerRecoil();
    test_ApplySwordDamagesToEnemy_DamageTypes();
    test_ApplySwordDamagesToEnemy_SpecialDamages();
    test_ApplySwordDamagesToEnemy_DyingAndDefeat();
    test_DataTables_SwordEnemyCollision();
    test_func_003_73EB_EarlyBranches();
    test_func_003_73EB_SwordCollision_NonBlaino();
    test_func_003_73EB_SwordCollision_Blaino();
    test_label_003_74EC_Behavior();
    test_func_003_75A2_FilteringAndParity();
    test_func_003_75A2_BouncingBombiteAndGrabbable();
    test_func_003_75A2_MagicPowderAndIronMask();
    test_func_003_75A2_ProjectileReactions();
    test_CheckExplosionInteractionWithEntities();
    test_DataTables_ProjectileCollision();
    test_CheckLinkCollisionWithProjectile();
    test_OctorokRockEntityHandler();
}
