#include "bank3/entities_arrow.h"
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

/* ===== Data Tables (03:6AC6-03:6B52) ===== */

/* EntityArrowSpriteVariants (03:6AC6 / 03:6BC6) - 4 variants * 4 bytes each = 16 bytes */
const uint8_t EntityArrowSpriteVariants[16] = {
    /* variant 0 (right): tile $2E/$2C, attrs with XFLIP */
    0x2E, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
    0x2C, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
    /* variant 1 (left): tile $2C/$2E, attrs without XFLIP */
    0x2C, OAM_GBC_PAL_1 | OAMF_PAL0,
    0x2E, OAM_GBC_PAL_1 | OAMF_PAL0,
    /* variant 2 (up): tile $2A, attrs with YFLIP */
    0x2A, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_YFLIP,
    0x2A, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_YFLIP | OAMF_XFLIP,
    /* variant 3 (down): tile $2A, attrs without YFLIP */
    0x2A, OAM_GBC_PAL_1 | OAMF_PAL0,
    0x2A, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP
};

/* Bomb Arrow bomb sprite - from 03:6A66 */
const uint8_t BombArrowBombSprite[2] = {
    0x80, OAM_GBC_PAL_5 | OAMF_PAL1  /* tile $80, palette 5 | OAMF_PAL1 */
};

/* Bomb Arrow offsets - from 03:6A68/03:6A6C */
const int8_t BombArrowBombXOffsetPerDirection[4] = { +4, -4, 0, 0 };
const int8_t BombArrowBombYOffsetPerDirection[4] = { -2, -2, -6, +4 };

/* Arrow spinning sprite variant frames - from 03:6B48 */
const uint8_t ArrowSpinningSpriteVariantFrames[4] = {
    DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT, DIRECTION_UP
};

/* Octorok Rock sprite variants - from 03:6A1E (OctorokRockSpriteVariants) */
const uint8_t OctorokRockSpriteVariants[8] = {
    /* variant 0: tile $6C, attrs */
    0x6C, 0x01,
    0x6C, 0x21,
    /* variant 1: tile $5C, attrs */
    0x5C, 0x01,
    0x5C, 0x21
};

/* ===== ArrowEntityHandler (03:6A34) ===== */
void ArrowEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* Increment the active projectiles count */
    /* ld hl, wActiveProjectileCount; inc [hl] */
    gb_write(gb, wActiveProjectileCount, gb_read(gb, wActiveProjectileCount) + 1);

    /* If hActiveEntityState == 1... */
    /* ldh a, [hActiveEntityState]; and a; jr nz, BombArrowHandler */
    if (gb_read_hram(gb, hActiveEntityState) != 0) {
        BombArrowHandler(gb, bc);
        return;
    }

    /* If GetEntityTransitionCountdown != 0... */
    /* call GetEntityTransitionCountdown; jp nz, ArrowRenderAndMove */
    if (GetEntityTransitionCountdown(gb, bc) != 0) {
        ArrowRenderAndMove(gb, bc);
        return;
    }

    /* hActiveEntityState == 0 and GetEntityTransitionCountdown == 0 */
    /* ld a, DAMAGE_TYPE_ARROW; ld [wAttackDamageType], a; call func_003_75A2 */
    gb_write(gb, wAttackDamageType, DAMAGE_TYPE_ARROW);
    func_003_75A2(gb, bc);

    /* call ArrowRenderAndMove */
    ArrowRenderAndMove(gb, bc);

    /* Allow shooting the Dungeon 8 statue in the eye */
    /* ldh a, [hActiveEntitySpriteVariant]; cp DIRECTION_UP; ret nz */
    if (gb_read_hram(gb, hActiveEntitySpriteVariant) != DIRECTION_UP) {
        return;
    }
    /* and the event trigger is TRIGGER_SHOOT_STATUE_EYE... */
    /* ld a, [wRoomEvent]; and EVENT_TRIGGER_MASK; cp TRIGGER_SHOOT_STATUE_EYE; ret nz */
    if ((gb_read(gb, wRoomEvent) & EVENT_TRIGGER_MASK) != TRIGGER_SHOOT_STATUE_EYE) {
        return;
    }
    /* and hObjectUnderEntity == OBJECT_ONE_EYED_STATUE... */
    /* ldh a, [hObjectUnderEntity]; cp OBJECT_ONE_EYED_STATUE; ret nz */
    if (gb_read_hram(gb, hObjectUnderEntity) != OBJECT_ONE_EYED_STATUE) {
        return;
    }
    /* call MarkTriggerAsResolved, and clear entity */
    /* call MarkTriggerAsResolved; jp UnloadEntityAndReturn */
    MarkTriggerAsResolved(gb);
    UnloadEntityAndReturn(gb, bc);
}

/* ===== BombArrowHandler (03:6A70) ===== */
void BombArrowHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; jr z, .beforeExploding */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0) {
        goto beforeExploding;
    }

    /* ld a, ENTITY_BOMB; call SpawnNewEntity; jr c, .unloadAndReturn */
    uint16_t de = SpawnNewEntityInRange_impl(gb, ENTITY_BOMB, bc, MAX_ENTITIES - 1);
    if (de != 0xFFFF) {
        /* ldh a, [hMultiPurpose0]; ld hl, wEntitiesPosXTable; add hl, de; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesPosXTable + de), gb_read_hram(gb, hMultiPurpose0));
        /* ldh a, [hMultiPurpose1]; ld hl, wEntitiesPosYTable; add hl, de; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesPosYTable + de), gb_read_hram(gb, hMultiPurpose1));
        /* ld hl, wEntitiesTransitionCountdownTable; add hl, de; ld [hl], $17 */
        gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + de), 0x17);
        /* call PlayBombExplosionSfx */
        PlayBombExplosionSfx(gb);
    }

    /* jp UnloadEntityAndReturn */
    UnloadEntityAndReturn(gb, bc);
    return;

beforeExploding: ;
    /* Render the bomb arrow's bomb */
    /* ldh a, [hActiveEntitySpriteVariant]; push af; ld e, a; ld d, b; xor a; ldh [hActiveEntitySpriteVariant], a */
    uint8_t sprite_variant = gb_read_hram(gb, hActiveEntitySpriteVariant);
    gb_write_hram(gb, hActiveEntitySpriteVariant, 0x00);

    /* ld hl, BombArrowBombXOffsetPerDirection; add hl, de; ldh a, [hActiveEntityPosX]; add [hl]; ldh [hActiveEntityPosX], a */
    int8_t x_offset = BombArrowBombXOffsetPerDirection[sprite_variant & 0x03];
    uint8_t pos_x = gb_read_hram(gb, hActiveEntityPosX);
    gb_write_hram(gb, hActiveEntityPosX, (uint8_t)(pos_x + x_offset));

    /* ld hl, BombArrowBombYOffsetPerDirection; add hl, de; ldh a, [hActiveEntityVisualPosY]; add [hl]; ldh [hActiveEntityVisualPosY], a */
    int8_t y_offset = BombArrowBombYOffsetPerDirection[sprite_variant & 0x03];
    uint8_t visual_pos_y = gb_read_hram(gb, hActiveEntityVisualPosY);
    gb_write_hram(gb, hActiveEntityVisualPosY, (uint8_t)(visual_pos_y + y_offset));

    /* ld de, BombArrowBombSprite; call RenderActiveEntitySprite */
    RenderActiveEntitySprite(gb, BombArrowBombSprite, NULL);

    /* call CopyEntityPositionToActivePosition */
    CopyEntityPositionToActivePosition(gb, bc);

    /* pop af; ldh [hActiveEntitySpriteVariant], a */
    gb_write_hram(gb, hActiveEntitySpriteVariant, sprite_variant);

    /* Render the arrow itself */
    /* ld de, EntityArrowSpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, EntityArrowSpriteVariants, NULL);

    /* Deal (no) damage to other entities before exploding */
    /* ld a, DAMAGE_TYPE_BOMB_ARROW; ld [wAttackDamageType], a; call func_003_75A2 */
    gb_write(gb, wAttackDamageType, DAMAGE_TYPE_BOMB_ARROW);
    func_003_75A2(gb, bc);
    /* jr ArrowRenderAndMove.skipRendering */
    /* fallthrough to ArrowRenderAndMove skipRendering */
    ArrowRenderAndMove_skipRendering(gb, bc);
}

/* ===== MoblinArrowEntityHandler (03:6ACC) ===== */
void MoblinArrowEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; jr nz, ArrowRenderAndMove */
    if (GetEntityTransitionCountdown(gb, bc) != 0) {
        ArrowRenderAndMove(gb, bc);
        return;
    }

    /* call CheckLinkCollisionWithProjectile; fallthrough to ArrowRenderAndMove */
    CheckLinkCollisionWithProjectile(gb, bc);
    ArrowRenderAndMove(gb, bc);
}

/* ===== ArrowRenderAndMove (03:6AD4) ===== */
void ArrowRenderAndMove(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, EntityArrowSpriteVariants; call RenderActiveEntitySpritesPair */
    /* Select sprite variants based on entity type (matches OctorokRockEntityHandler behavior) */
    const uint8_t *sprite_variants = EntityArrowSpriteVariants;
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_OCTOROK_ROCK) {
        sprite_variants = OctorokRockSpriteVariants;
    }
    RenderActiveEntitySpritesPair(gb, sprite_variants, NULL);

    /* .skipRendering: fallthrough to ArrowRenderAndMove_skipRendering */
    ArrowRenderAndMove_skipRendering(gb, bc);
}

/* ===== ArrowRenderAndMove skipRendering entry point (03:6ADA) ===== */
void ArrowRenderAndMove_skipRendering(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call ReturnIfNonInteractive_03; call GetEntityTransitionCountdown; jr nz, ArrowRockAfterHittingWall */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    if (GetEntityTransitionCountdown(gb, bc) != 0) {
        ArrowRockAfterHittingWall(gb, bc);
        return;
    }

    /* call UpdateEntityPosWithSpeed_03; call ApplySwordIntersectionWithObjects */
    UpdateEntityPosWithSpeed_03(gb, bc);
    ApplySwordIntersectionWithObjects(gb, bc);

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and a; jr z, EntityBounceOffWallX.return */
    if (gb_read(gb, (uint16_t)(wEntitiesCollisionsTable + bc)) == 0) {
        return;
    }

    /* call GetEntityTransitionCountdown (result unused, reloaded later) */
    (void)GetEntityTransitionCountdown(gb, bc);

    /* ldh a, [hActiveEntityType]; cp ENTITY_MAGIC_ROD_FIREBALL; jr nz, .fireballEnd */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_MAGIC_ROD_FIREBALL) {
        /* call GetEntityPrivateCountdown1; ld [hl], $30; ret */
        GetEntityPrivateCountdown1(gb, bc);
        gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown1Table + bc), 0x30);
        return;
    }

    /* .fireballEnd: ld [hl], $18; ld hl, wEntitiesSpeedZTable; add hl, bc; ld [hl], $10 */
    gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0x18);
    gb_write(gb, (uint16_t)(wEntitiesSpeedZTable + bc), 0x10);

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; inc a; jr z, .skipSound */
    uint8_t collisions = gb_read(gb, (uint16_t)(wEntitiesCollisionsTable + bc));
    if ((uint8_t)(collisions + 1) == 0) {
        goto skipSound;
    }

    /* ld a, JINGLE_SWORD_POKING; ldh [hJingle], a */
    gb_write_hram(gb, hJingle, JINGLE_SWORD_POKING);

skipSound:
    /* call AlertSwordMoblins */
    AlertSwordMoblins(gb);

    /* ldh a, [hActiveEntityType]; cp ENTITY_ARROW; jr nz, .enemyProjectileBounce */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_ARROW) {
        /* Player arrows bounce more off walls than Moblin arrows or Octorok rocks */
        /* .playerArrowBounceY: SpeedY = (-(int8_t)SpeedY) >> 2 */
        /* .playerArrowBounce:  SpeedX = (-(int8_t)SpeedX) >> 2 */
        uint8_t raw_y = gb_read(gb, (uint16_t)(wEntitiesSpeedYTable + bc));
        int8_t speed_y = (int8_t)(uint8_t)(~raw_y + 1);
        speed_y >>= 2;
        gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + bc), (uint8_t)speed_y);

        uint8_t raw_x = gb_read(gb, (uint16_t)(wEntitiesSpeedXTable + bc));
        int8_t speed_x = (int8_t)(uint8_t)(~raw_x + 1);
        speed_x >>= 2;
        gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + bc), (uint8_t)speed_x);
        return;
    }

    /* .enemyProjectileBounce: call EntityBounceOffWallY; fallthrough to EntityBounceOffWallX */
    EntityBounceOffWallY(gb, bc);
    EntityBounceOffWallX(gb, bc);
}

/* ===== EntityBounceOffWallX (03:6B34) ===== */
void EntityBounceOffWallX(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesSpeedXTable; add hl, bc; ld a, [hl]; cpl; inc a; sra a; sra a; sra a; ld [hl], a; ret */
    uint8_t raw_x = gb_read(gb, (uint16_t)(wEntitiesSpeedXTable + bc));
    int8_t speed_x = (int8_t)(uint8_t)(~raw_x + 1);
    speed_x >>= 3;
    gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + bc), (uint8_t)speed_x);
}

/* ===== EntityBounceOffWallY (03:6B43) ===== */
void EntityBounceOffWallY(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesSpeedYTable; add hl, bc; ld a, [hl]; cpl; inc a; sra a; sra a; sra a; ld [hl], a; ret */
    uint8_t raw_y = gb_read(gb, (uint16_t)(wEntitiesSpeedYTable + bc));
    int8_t speed_y = (int8_t)(uint8_t)(~raw_y + 1);
    speed_y >>= 3;
    gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + bc), (uint8_t)speed_y);
}

/* ===== ArrowRockAfterHittingWall (03:6B4C) ===== */
void ArrowRockAfterHittingWall(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* cp $01; jp z, UnloadEntityAndReturn / jr nz, .unloadEnd */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0x01) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* Octorok rocks don't spin after hitting a wall, only arrows do */
    /* ldh a, [hActiveEntityType]; cp ENTITY_OCTOROK_ROCK; jr z, .spinningEnd */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_OCTOROK_ROCK) {
        goto spinningEnd;
    }

    /* call GetEntityTransitionCountdown; srl a x3; and $03; ld e, a; ld d, b; ld hl, ArrowSpinningSpriteVariantFrames; add hl, de; ld a, [hl]; call SetEntitySpriteVariant */
    uint8_t frame = GetEntityTransitionCountdown(gb, bc);
    frame >>= 3;  /* srl x3 */
    frame &= 0x03;
    uint8_t variant = ArrowSpinningSpriteVariantFrames[frame];
    SetEntitySpriteVariant(gb, bc, variant);

spinningEnd:
    /* call UpdateEntityPosWithSpeed_03; jr func_003_6B7B */
    UpdateEntityPosWithSpeed_03(gb, bc);
    func_003_6B7B(gb, bc);
}

/* ===== OctorokRockEntityHandler (03:6A26) ===== */
void OctorokRockEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; jr nz, .jr_6A2E */
    if (GetEntityTransitionCountdown(gb, bc) == 0) {
        /* call CheckLinkCollisionWithProjectile */
        CheckLinkCollisionWithProjectile(gb, bc);
    }

    /* .jr_6A2E: ld de, OctorokRockSpriteVariants; jp ArrowRenderAndMove.skipLoadingSprites */
    ArrowRenderAndMove(gb, bc);
}