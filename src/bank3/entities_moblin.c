#include "bank3/entities_moblin.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_droppable.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/gameplay.h"
#include "constants/directions.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include "home/entities.h"
#include "home/room.h"
#include "home/bank.h"
#include "home/audio.h"
#include "home/gameplay.h"
#include "constants/audio.h"

/* ===== Data Tables (03:57FB-03:59D6) ===== */

/* OctorokSpriteVariants (03:57FB) - 8 variants * 4 bytes each = 32 bytes */
const uint8_t OctorokSpriteVariants[32] = {
    /* variant 0: down 0 */
    0x30, OAM_GBC_PAL_2 | OAMF_PAL0,
    0x30, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
    /* variant 1: down 1 */
    0x32, OAM_GBC_PAL_2 | OAMF_PAL0,
    0x32, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
    /* variant 2: up 0 */
    0x30, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP,
    0x30, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP | OAMF_XFLIP,
    /* variant 3: up 1 */
    0x32, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP,
    0x32, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP | OAMF_XFLIP,
    /* variant 4: left 0 */
    0x34, OAM_GBC_PAL_2 | OAMF_PAL0,
    0x36, OAM_GBC_PAL_2 | OAMF_PAL0,
    /* variant 5: left 1 */
    0x38, OAM_GBC_PAL_2 | OAMF_PAL0,
    0x3A, OAM_GBC_PAL_2 | OAMF_PAL0,
    /* variant 6: right 0 */
    0x36, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
    0x34, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
    /* variant 7: right 1 */
    0x3A, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
    0x38, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP
};

/* RoamingEnemySpeedXPerDirection (03:581B) */
const int8_t RoamingEnemySpeedXPerDirection[4] = { 8, -8, 0, 0 };

/* RoamingEnemySpeedYPerDirection (03:581F) */
const int8_t RoamingEnemySpeedYPerDirection[4] = { 0, 0, -8, 8 };

/* EntityVariantForDirection_03 (03:5823) */
const uint8_t EntityVariantForDirection_03[4] = { 6, 4, 2, 0 };

/* MoblinSpriteVariants (03:5917) - 8 variants * 4 bytes each = 32 bytes */
const uint8_t MoblinSpriteVariants[32] = {
    /* variant 0: down 0 */
    0x60, OAM_GBC_PAL_3,
    0x62, OAM_GBC_PAL_3,
    /* variant 1: down 1 */
    0x62, OAM_GBC_PAL_3 | OAMF_XFLIP,
    0x60, OAM_GBC_PAL_3 | OAMF_XFLIP,
    /* variant 2: up 0 */
    0x64, OAM_GBC_PAL_3,
    0x66, OAM_GBC_PAL_3,
    /* variant 3: up 1 */
    0x66, OAM_GBC_PAL_3 | OAMF_XFLIP,
    0x64, OAM_GBC_PAL_3 | OAMF_XFLIP,
    /* variant 4: left 0 */
    0x68, OAM_GBC_PAL_3,
    0x6A, OAM_GBC_PAL_3,
    /* variant 5: left 1 */
    0x6C, OAM_GBC_PAL_3,
    0x6E, OAM_GBC_PAL_3,
    /* variant 6: right 0 */
    0x6A, OAM_GBC_PAL_3 | OAMF_XFLIP,
    0x68, OAM_GBC_PAL_3 | OAMF_XFLIP,
    /* variant 7: right 1 */
    0x6E, OAM_GBC_PAL_3 | OAMF_XFLIP,
    0x6C, OAM_GBC_PAL_3 | OAMF_XFLIP
};

/* MaskedIronMaskSpriteVariants (03:5048) - 8 variants * 4 bytes each = 32 bytes */
const uint8_t MaskedIronMaskSpriteVariants[32] = {
    /* variant 0: down 0 */
    0x58, OAM_GBC_PAL_3,
    0x5A, OAM_GBC_PAL_3,
    /* variant 1: down 1 */
    0x5A, OAM_GBC_PAL_3 | OAMF_XFLIP,
    0x58, OAM_GBC_PAL_3 | OAMF_XFLIP,
    /* variant 2: up 0 */
    0x58, OAM_GBC_PAL_3 | OAMF_YFLIP,
    0x5A, OAM_GBC_PAL_3 | OAMF_YFLIP,
    /* variant 3: up 1 */
    0x5A, OAM_GBC_PAL_3 | OAMF_YFLIP | OAMF_XFLIP,
    0x58, OAM_GBC_PAL_3 | OAMF_YFLIP | OAMF_XFLIP,
    /* variant 4: left 0 */
    0x5C, OAM_GBC_PAL_3,
    0x5E, OAM_GBC_PAL_3,
    /* variant 5: left 1 */
    0x78, OAM_GBC_PAL_3,
    0x7A, OAM_GBC_PAL_3,
    /* variant 6: right 0 */
    0x5E, OAM_GBC_PAL_3 | OAMF_XFLIP,
    0x5C, OAM_GBC_PAL_3 | OAMF_XFLIP,
    /* variant 7: right 1 */
    0x7A, OAM_GBC_PAL_3 | OAMF_XFLIP,
    0x78, OAM_GBC_PAL_3 | OAMF_XFLIP
};

/* Moblin Arrow data (03:5937-03:5946) */
const int8_t MoblinArrowOffsetXPerDirection[4] = { 8, -8, 4, -4 };
const int8_t MoblinArrowOffsetYPerDirection[4] = { -4, -4, -8, 0 };
const int8_t MoblinArrowSpeedXPerDirection[4] = { 32, -32, 0, 0 };
const int8_t MoblinArrowSpeedYPerDirection[4] = { 0, 0, -32, 32 };

/* Octorok Rock data (03:598C-03:5997) */
const int8_t OctorokRockOffsetXPerDirection[4] = { 8, -8, 0, 0 };
const int8_t OctorokRockOffsetYPerDirection[4] = { 0, 0, -8, 8 };
const int8_t OctorokRockSpeedXPerDirection[4] = { 32, -32, 0, 0 };
const int8_t OctorokRockSpeedYPerDirection[4] = { 0, 0, -32, 32 };

/* ===== OctorokEntityHandler (03:57E9) ===== */
void OctorokEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, OctorokSpriteVariants */
    /* ld a, [wGameplayType]; cp GAMEPLAY_CREDITS; jr z, .creditsEnd */
    /* ld a, $30; ldh [hActiveEntityTilesOffset], a */
    if (gb_read(gb, wGameplayType) != GAMEPLAY_CREDITS) {
        gb_write_hram(gb, hActiveEntityTilesOffset, 0x30);
    }

    /* .creditsEnd: call AnimateRoamingEnemy; ret */
    AnimateRoamingEnemy_with_sprites(gb, bc, OctorokSpriteVariants);
}

/* ===== MoblinEntityHandler (03:5827) ===== */
void MoblinEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hMapId]; cp MAP_BOWWOW_HIDEOUT; jr nz, .hideoutEnd */
    if (gb_read_hram(gb, hMapId) == MAP_BOWWOW_HIDEOUT) {
        /* ld a, [wIsBowWowFollowingLink]; cp $80; jp nz, UnloadEntityAndReturn */
        if (gb_read(gb, wIsBowWowFollowingLink) != 0x80) {
            UnloadEntityAndReturn(gb, bc);
            return;
        }
    }

    /* .hideoutEnd: ld a, c; ld [wD153], a */
    gb_write(gb, wD153, (uint8_t)(bc & 0xFF));

    /* ld de, MoblinSpriteVariants; fallthrough to AnimateRoamingEnemy */
    AnimateRoamingEnemy_with_sprites(gb, bc, MoblinSpriteVariants);
}

/* ===== AnimateRoamingEnemy_with_sprites (03:583C) ===== */
void AnimateRoamingEnemy_with_sprites(GBState *gb, uint16_t bc, const uint8_t *sprites) {
    if (!gb) return;

    /* call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, sprites, NULL);

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld a, [hl]; and a; jr z, .recoilEnd */
    if (gb_read(gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + bc)) != 0) {
        /* ld hl, wEntitiesStateTable; add hl, bc; ld a, $01; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesStateTable + bc), 0x01);
        /* ldh [hActiveEntityState], a */
        gb_write_hram(gb, hActiveEntityState, 0x01);
        /* call GetEntityTransitionCountdown; ld [hl], $40 */
        GetEntityTransitionCountdown(gb, bc);
        gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0x40);
    }

    /* .recoilEnd: call ApplyRecoilIfNeeded_03 */
    ApplyRecoilIfNeeded_03(gb, bc);

    /* call DefaultEnemyDamageCollisionHandler */
    DefaultEnemyDamageCollisionHandler(gb, bc);

    /* ldh a, [hActiveEntityState]; and a; jr z, RoamingEnemyState0Handler */
    if (gb_read_hram(gb, hActiveEntityState) == 0) {
        RoamingEnemyState0Handler(gb, bc);
        return;
    }

    /* call GetEntityTransitionCountdown; jr z, jr_003_5896 */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0) {
        goto jr_003_5896;
    }

    /* cp $0A; jr nz, .projectileEnd */
    if (countdown != 0x0A) {
        goto projectileEnd;
    }

    /* call GetEntityPrivateCountdown1; jr nz, .projectileEnd */
    if (GetEntityPrivateCountdown1(gb, bc) != 0) {
        goto projectileEnd;
    }

    /* call GetEntityDirectionToLink_03 */
    uint8_t dir_to_link = GetEntityDirectionToLink_03(gb);
    /* ld hl, wEntitiesDirectionTable; add hl, bc; ld a, e; cp [hl]; jr nz, .projectileEnd */
    if (dir_to_link != gb_read(gb, (uint16_t)(wEntitiesDirectionTable + bc))) {
        goto projectileEnd;
    }

    /* ldh a, [hActiveEntityType]; cp ENTITY_IRON_MASK; jr z, .projectileEnd */
    uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);
    if (entity_type == ENTITY_IRON_MASK) {
        goto projectileEnd;
    }

    /* cp ENTITY_OCTOROK; jr z, jr_003_588D */
    if (entity_type == ENTITY_OCTOROK) {
        goto jr_003_588D;
    }

    /* call SpawnMoblinArrow */
    SpawnMoblinArrow(gb, bc);

projectileEnd:
    /* call ApplyEntityInteractionWithBackground; ret */
    ApplyEntityInteractionWithBackground(gb, bc);
    return;

jr_003_588D:
    /* ld a, [wGameplayType]; cp GAMEPLAY_CREDITS; ret z */
    if (gb_read(gb, wGameplayType) == GAMEPLAY_CREDITS) {
        return;
    }

    /* jp SpawnOctorokRock */
    SpawnOctorokRock(gb, bc);
    return;

jr_003_5896: ;
    /* call GetRandomByte; and $1F; or $20; ld [hl], a */
    uint8_t random = (uint8_t)((GetRandomByte(gb) & 0x1F) | 0x20);
    gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), random);

    /* ld hl, wEntitiesStateTable; add hl, bc; ld [hl], $00 */
    gb_write(gb, (uint16_t)(wEntitiesStateTable + bc), 0x00);

    /* ld hl, wEntitiesPrivateState1Table; add hl, bc; ld a, [hl]; inc a */
    uint8_t private_state1 = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesPrivateState1Table + bc)) + 1);
    /* and $03; ld [hl], a */
    private_state1 &= 0x03;
    gb_write(gb, (uint16_t)(wEntitiesPrivateState1Table + bc), private_state1);

    /* cp $00; jr nz, .jr_58B6 */
    uint8_t dir;
    if (private_state1 == 0) {
        /* call GetEntityDirectionToLink_03; jr jr_003_58B9 */
        dir = GetEntityDirectionToLink_03(gb);
    } else {
        /* .jr_58B6: call GetRandomByte */
        dir = GetRandomByte(gb);
    }

    /* jr_003_58B9: and $03; ld hl, wEntitiesDirectionTable; add hl, bc; ld [hl], a */
    dir &= 0x03;
    gb_write(gb, (uint16_t)(wEntitiesDirectionTable + bc), dir);

    /* ld hl, RoamingEnemySpeedXPerDirection; add hl, de; ld a, [hl]; ld hl, wEntitiesSpeedXTable; add hl, bc; ld [hl], a */
    int8_t speed_x = RoamingEnemySpeedXPerDirection[dir];
    gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + bc), (uint8_t)speed_x);

    /* ld hl, RoamingEnemySpeedYPerDirection; add hl, de; ld a, [hl]; ld hl, wEntitiesSpeedYTable; add hl, bc; ld [hl], a; ret */
    int8_t speed_y = RoamingEnemySpeedYPerDirection[dir];
    gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + bc), (uint8_t)speed_y);
}

/* ===== AnimateRoamingEnemy (03:583C) ===== */
void AnimateRoamingEnemy(GBState *gb, uint16_t bc) {
    if (!gb) return;
    const uint8_t *sprites = MoblinSpriteVariants;
    uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);
    if (entity_type == ENTITY_OCTOROK) {
        sprites = OctorokSpriteVariants;
    } else if (entity_type == ENTITY_IRON_MASK) {
        sprites = MaskedIronMaskSpriteVariants;
    }
    AnimateRoamingEnemy_with_sprites(gb, bc, sprites);
}

/* ===== RoamingEnemyState0Handler (03:58D7) ===== */
void RoamingEnemyState0Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and $0F; jr nz, .collided */
    uint8_t collisions = gb_read(gb, (uint16_t)(wEntitiesCollisionsTable + bc));
    uint16_t target_table;
    if ((collisions & 0x0F) != 0) {
        target_table = wEntitiesCollisionsTable;
    } else {
        /* call GetEntityTransitionCountdown; jr nz, StopWalkingEnd */
        if (GetEntityTransitionCountdown(gb, bc) != 0) {
            goto StopWalkingEnd;
        }
        target_table = wEntitiesTransitionCountdownTable;
    }

    /* .collided: call GetRandomByte; and $0F; or $10; ld [hl], a */
    uint8_t random = (uint8_t)((GetRandomByte(gb) & 0x0F) | 0x10);
    gb_write(gb, (uint16_t)(target_table + bc), random);

    /* ld hl, wEntitiesStateTable; add hl, bc; ld [hl], $01 */
    gb_write(gb, (uint16_t)(wEntitiesStateTable + bc), 0x01);

    /* call ClearEntitySpeed */
    ClearEntitySpeed(gb, bc);

StopWalkingEnd:
    /* call UpdateEntityPosWithSpeed_03; call ApplyEntityInteractionWithBackground; ret */
    UpdateEntityPosWithSpeed_03(gb, bc);
    ApplyEntityInteractionWithBackground(gb, bc);
}

/* ===== SetEntityVariantForDirection_03 (03:58FC) ===== */
void SetEntityVariantForDirection_03(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesDirectionTable; add hl, bc; ld e, [hl]; ld d, b */
    uint8_t direction = gb_read(gb, (uint16_t)(wEntitiesDirectionTable + bc));

    /* ld hl, EntityVariantForDirection_03; add hl, de; push hl */
    uint8_t variant = EntityVariantForDirection_03[direction & 0x03];

    /* ld hl, wEntitiesInertiaTable; add hl, bc; inc [hl] */
    uint8_t inertia = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesInertiaTable + bc)) + 1);
    gb_write(gb, (uint16_t)(wEntitiesInertiaTable + bc), inertia);

    /* ld a, [hl]; rra x3; pop hl; and $01; or [hl] */
    uint8_t inertia_bit = (uint8_t)((inertia >> 3) & 0x01);
    variant |= inertia_bit;

    /* jp SetEntitySpriteVariant */
    SetEntitySpriteVariant(gb, bc, variant);
}

/* ===== SpawnMoblinArrow (03:5947) ===== */
void SpawnMoblinArrow(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, ENTITY_MOBLIN_ARROW; call SpawnNewEntity; jr c, ret_003_598B */
    uint16_t de = SpawnNewEntityInRange_impl(gb, ENTITY_MOBLIN_ARROW, bc, MAX_ENTITIES - 1);
    if (de == 0xFFFF) {
        return;
    }

    /* push bc; ldh a, [hMultiPurpose2]; ld c, a */
    uint8_t multi_purpose2 = gb_read_hram(gb, hMultiPurpose2);
    uint8_t dir = (uint8_t)(multi_purpose2 & 0x03);

    /* ld hl, MoblinArrowOffsetXPerDirection; add hl, bc; ldh a, [hMultiPurpose0]; add [hl] */
    int8_t offset_x = MoblinArrowOffsetXPerDirection[dir];
    uint8_t pos_x = gb_read_hram(gb, hMultiPurpose0);
    gb_write(gb, (uint16_t)(wEntitiesPosXTable + de), (uint8_t)(pos_x + offset_x));

    /* ld hl, MoblinArrowOffsetYPerDirection; add hl, bc; ldh a, [hMultiPurpose1]; add [hl] */
    int8_t offset_y = MoblinArrowOffsetYPerDirection[dir];
    uint8_t pos_y = gb_read_hram(gb, hMultiPurpose1);
    gb_write(gb, (uint16_t)(wEntitiesPosYTable + de), (uint8_t)(pos_y + offset_y));

    /* ld hl, MoblinArrowSpeedXPerDirection; add hl, bc; ld a, [hl]; ld hl, wEntitiesSpeedXTable; add hl, de; ld [hl], a */
    int8_t speed_x = MoblinArrowSpeedXPerDirection[dir];
    gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + de), (uint8_t)(speed_x));

    /* ld hl, MoblinArrowSpeedYPerDirection; add hl, bc; ld a, [hl]; ld hl, wEntitiesSpeedYTable; add hl, de; ld [hl], a */
    int8_t speed_y = MoblinArrowSpeedYPerDirection[dir];
    gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + de), (uint8_t)(speed_y));

    /* ldh a, [hMultiPurpose2]; ld hl, wEntitiesSpriteVariantTable; add hl, de; ld [hl], a */
    gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + de), multi_purpose2);

    /* ld hl, wEntitiesDirectionTable; add hl, de; ld [hl], a */
    gb_write(gb, (uint16_t)(wEntitiesDirectionTable + de), multi_purpose2);

    /* pop bc; ret */
}

/* ===== SpawnOctorokRock (03:5998) ===== */
void SpawnOctorokRock(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, ENTITY_OCTOROK_ROCK; call SpawnNewEntity; jr c, .return */
    uint16_t de = SpawnNewEntityInRange_impl(gb, ENTITY_OCTOROK_ROCK, bc, MAX_ENTITIES - 1);
    if (de == 0xFFFF) {
        return;
    }

    /* push bc; ldh a, [hMultiPurpose2]; ld hl, wEntitiesDirectionTable; add hl, de; ld [hl], a */
    uint8_t multi_purpose2 = gb_read_hram(gb, hMultiPurpose2);
    gb_write(gb, (uint16_t)(wEntitiesDirectionTable + de), multi_purpose2);

    /* ld c, a; ld hl, OctorokRockOffsetXPerDirection; add hl, bc */
    uint8_t dir = (uint8_t)(multi_purpose2 & 0x03);
    int8_t offset_x = OctorokRockOffsetXPerDirection[dir];
    uint8_t pos_x = gb_read_hram(gb, hMultiPurpose0);
    gb_write(gb, (uint16_t)(wEntitiesPosXTable + de), (uint8_t)(pos_x + offset_x));

    /* ldh a, [hMultiPurpose1]; add [hl]; ld hl, wEntitiesPosYTable; add hl, de; ld [hl], a */
    int8_t offset_y = OctorokRockOffsetYPerDirection[dir];
    uint8_t pos_y = gb_read_hram(gb, hMultiPurpose1);
    gb_write(gb, (uint16_t)(wEntitiesPosYTable + de), (uint8_t)(pos_y + offset_y));

    /* ld hl, OctorokRockSpeedXPerDirection; add hl, bc; ld a, [hl]; ld hl, wEntitiesSpeedXTable; add hl, de; ld [hl], a */
    int8_t speed_x = OctorokRockSpeedXPerDirection[dir];
    gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + de), (uint8_t)(speed_x));

    /* ld hl, OctorokRockSpeedYPerDirection; add hl, bc; ld a, [hl]; ld hl, wEntitiesSpeedYTable; add hl, de; ld [hl], a */
    int8_t speed_y = OctorokRockSpeedYPerDirection[dir];
    gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + de), (uint8_t)(speed_y));

    /* pop bc; and a; .return: ret */
}

/* ===== Iron Mask Data Tables (03:4FEB-03:4FFB) ===== */

/* UnmaskedIronMaskSpriteVariants (03:4FEB) - 2 variants * 2 sprites * 2 bytes = 8 bytes */
const uint8_t UnmaskedIronMaskSpriteVariants[8] = {
    0x70, OAM_GBC_PAL_2 | OAMF_PAL0,
    0x72, OAM_GBC_PAL_2 | OAMF_PAL0,
    0x72, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
    0x70, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP
};

/* IronMaskSpeedXValues (03:4FF3) */
const int8_t IronMaskSpeedXValues[4] = {
    0x0C, (int8_t)0xF4, 0x00, 0x00
};

/* IronMaskSpeedYValues (03:4FF7) */
const int8_t IronMaskSpeedYValues[4] = {
    0x00, 0x00, (int8_t)0xF4, 0x0C
};

/* ===== IronMaskEntityHandler (03:4FFB) ===== */
void IronMaskEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesPrivateState2Table; add hl, bc; ld a, [hl]; and a; jr z, .masked */
    uint8_t unmasked = gb_read(gb, (uint16_t)(wEntitiesPrivateState2Table + bc));
    if (unmasked == 0) {
        /* .masked: ld de, MaskedIronMaskSpriteVariants; call AnimateRoamingEnemy; ret */
        AnimateRoamingEnemy(gb, bc);
        return;
    }

    /* ld de, UnmaskedIronMaskSpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, UnmaskedIronMaskSpriteVariants, NULL);

    /* call ReturnIfNonInteractive_03 */
    ReturnIfNonInteractive_03(gb, false);

    /* call ApplyRecoilIfNeeded_03 */
    ApplyRecoilIfNeeded_03(gb, bc);

    /* call DefaultEnemyDamageCollisionHandler */
    DefaultEnemyDamageCollisionHandler(gb, bc);

    /* call UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);

    /* call ApplyEntityInteractionWithBackground */
    ApplyEntityInteractionWithBackground(gb, bc);

    /* call GetEntityTransitionCountdown; jr nz, .changeDirectionEnd */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0) {
        /* call GetRandomByte; and $1F; add $20; ld [hl], a */
        uint8_t new_timer = (uint8_t)((GetRandomByte(gb) & 0x1F) + 0x20);
        gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), new_timer);

        /* and $03; ld e, a; ld d, b; ld hl, IronMaskSpeedXValues; add hl, de; ld a, [hl]... */
        uint8_t dir = (uint8_t)(new_timer & 0x03);
        gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + bc), (uint8_t)IronMaskSpeedXValues[dir]);
        gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + bc), (uint8_t)IronMaskSpeedYValues[dir]);
    }

    /* .changeDirectionEnd: ldh a, [hFrameCounter]; rra 4x; and $01; jp SetEntitySpriteVariant */
    uint8_t frame = gb_read_hram(gb, hFrameCounter);
    uint8_t variant = (uint8_t)((frame >> 4) & 0x01);
    SetEntitySpriteVariant(gb, bc, variant);
}