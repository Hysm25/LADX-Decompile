#include "bank3/entities_bomb.h"
#include "bank3/entities_physics.h"
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

/* ===== Data Tables (03:6530-03:65CF) ===== */

/* BombSprite (03:6530) */
static const uint8_t BombSprite[2] = { 0x80, OAM_GBC_PAL_5 | OAMF_PAL1 };

/* ExplosionSpriteRect (03:6532) - 4 variants * 32 bytes each = 128 bytes */
static const int8_t ExplosionSpriteRect[128] = {
    /* variant 0 */
    -8, -8, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0, -8,  0, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
    -8,  8, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0, -8, 16, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
     8, -8, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0,  8,  0, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
     8,  8, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0,  8, 16, 0x32, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
    /* variant 1 */
    -8, -8, 0x10, OAM_GBC_PAL_2 | OAMF_PAL0,                          -8,  0, 0x12, OAM_GBC_PAL_2 | OAMF_PAL0,
    -8,  8, 0x12, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,             -8, 16, 0x10, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
     8, -8, 0x10, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP,              8,  0, 0x12, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP,
     8,  8, 0x12, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP | OAMF_YFLIP, 8, 16, 0x10, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP | OAMF_YFLIP,
    /* variant 2 */
    -4, -4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1, -4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1 | OAMF_XFLIP,
    -4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1, -4, 12, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1 | OAMF_XFLIP,
     4, -4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1,  4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1 | OAMF_XFLIP,
     4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1,  4, 12, 0x30, OAM_GBC_PAL_1 | OAMF_PAL1 | OAMF_XFLIP,
    /* variant 3 */
    -4, -4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0, -4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
    -4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0, -4, 12, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
     4, -4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0,  4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP,
     4,  4, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0,  4, 12, 0x30, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP
};

/* ExplosionSpriteVariantFrames (03:65B2) - 24 frames */
static const uint8_t ExplosionSpriteVariantFrames[24] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03
};

/* ===== RenderBombExplosion (03:65B0) ===== */
void RenderBombExplosion(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesSpriteVariantTable; add hl, bc; ld a, [hl]; sla x5; ld e, a; ld d, b */
    uint8_t variant = gb_read(gb, wEntitiesSpriteVariantTable + bc);
    variant = (variant << 5); /* sla x5 */

    /* ld hl, ExplosionSpriteRect; add hl, de; ld c, $08; jp RenderActiveEntitySpritesRect */
    const int8_t *sprite_rect = ExplosionSpriteRect + variant;
    RenderActiveEntitySpritesRect(gb, (const uint8_t *)sprite_rect, 8, NULL);
}

/* ===== BombExplosionHandler (03:65E2) ===== */
void BombExplosionHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call BombExplosionVisuals */
    BombExplosionVisuals(gb, bc);

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* call GetEntityTransitionCountdown; and a; jp z, UnloadEntityAndReturn */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ld e, a; ld hl, wEntitiesPrivateState4Table; add hl, bc; ld a, [hl]; cp $4C; ld a, e; jp z, .ret */
    uint8_t private_state4 = gb_read(gb, wEntitiesPrivateState4Table + bc);
    if (private_state4 == 0x4C) {
        return;
    }

    /* cp $0E; jr c, .checkForDestroyableObjectEnd */
    if (countdown >= 0x0E && countdown < 0x17) {
        /* push af; sub $0E; ld e, a; ld d, b; push de */
        (void)(countdown - 0x0E);
        
        /* call CheckForBombDestroyableObjectBasic */
        CheckForBombDestroyableObjectBasic(gb, bc);
        
        /* call CheckForBombDestroyableObjectPuzzle */
        CheckForBombDestroyableObjectPuzzle(gb, bc);
        /* pop af */
    }

    /* .checkForDestroyableObjectEnd: cp $12; jr nz, .ret */
    if (countdown != 0x12) {
        return;
    }

    /* ld hl, wEntitiesPrivateState4Table; add hl, bc; ld a, [hl]; and a; jr nz, .enemyBomb */
    private_state4 = gb_read(gb, wEntitiesPrivateState4Table + bc);
    if (private_state4 == 0) {
        /* Link bomb */
        /* call CheckExplosionInteractionWithEntities; jr .enemyBombEnd */
        CheckExplosionInteractionWithEntities(gb, bc);
        goto enemyBombEnd;
    }

    /* .enemyBomb: Enemy bomb - check if Link is in explosion radius */
    /* ldh a, [hActiveEntityPosX]; ld hl, hLinkPositionX; sub [hl]; add $18; cp $30; jr nc, .enemyBombEnd */
    int16_t diff_x = (int16_t)gb_read_hram(gb, hActiveEntityPosX) - gb_read_hram(gb, hLinkPositionX) + 0x18;
    if (diff_x < 0) diff_x = -diff_x;
    if (diff_x >= 0x30) {
        goto enemyBombEnd;
    }

    /* ldh a, [hActiveEntityPosY]; ld hl, hLinkPositionY; sub [hl]; add $18; cp $30; jr nc, .enemyBombEnd */
    int16_t diff_y = (int16_t)gb_read_hram(gb, hActiveEntityPosY) - gb_read_hram(gb, hLinkPositionY) + 0x18;
    if (diff_y < 0) diff_y = -diff_y;
    if (diff_y >= 0x30) {
        goto enemyBombEnd;
    }

    /* call ApplyLinkCollisionWithEnemy */
    ApplyLinkCollisionWithEnemy(gb, bc);
    /* ld hl, hLinkSpeedX; sla [hl]; ld hl, hLinkSpeedY; sla [hl] */
    uint8_t link_speed_x = gb_read_hram(gb, hLinkSpeedX) << 1;
    uint8_t link_speed_y = gb_read_hram(gb, hLinkSpeedY) << 1;
    gb_write_hram(gb, hLinkSpeedX, link_speed_x);
    gb_write_hram(gb, hLinkSpeedY, link_speed_y);

enemyBombEnd:
    /* ld a, $04; ld [wSwordMoblinAlertingSoundCounter], a; ret */
    gb_write(gb, wSwordMoblinAlertingSoundCounter, 0x04);
}

/* ===== BombExplosionVisuals (03:6650) ===== */
void BombExplosionVisuals(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; ld e, a; ld d, b; ld hl, ExplosionSpriteVariantFrames; add hl, de; ld a, [hl]; call SetEntitySpriteVariant */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown < 24) {
        uint8_t variant = ExplosionSpriteVariantFrames[countdown];
        SetEntitySpriteVariant(gb, bc, variant);
    }

    /* ld hl, wEntitiesPhysicsFlagsTable; add hl, bc; ld a, [hl]; and ENTITY_PHYSICS_MASK; or $08; ld [hl], a */
    uint8_t physics = gb_read(gb, wEntitiesPhysicsFlagsTable + bc);
    physics = (physics & 0xF0) | 0x08;
    gb_write(gb, wEntitiesPhysicsFlagsTable + bc, physics);

    /* call RenderBombExplosion */
    RenderBombExplosion(gb, bc);

    /* ld a, [wIsIndoor]; and a; jr z, .ret */
    if (gb_read(gb, wIsIndoor) == 0) {
        return;
    }

    /* ld a, [wTransitionSequenceCounter]; cp $04 */
    if (gb_read(gb, wTransitionSequenceCounter) >= 0x04) {
        return;
    }
    /* The assembly continues but we'll stop here for now */
}

/* ===== BombEntityHandler (03:6677) ===== */
void BombEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* This is a complex handler - stub for now */
    /* The full implementation handles bomb states, timer, explosion, etc. */
    (void)bc;
}

/* ===== RenderBomb (03:678B) - also called from lifted item handler ===== */
void RenderBomb(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;

    /* call RenderActiveEntitySprite with BombSprite */
    RenderActiveEntitySprite(gb, BombSprite, NULL);
}

/* ===== CheckForBombDestroyableObjectPuzzle (03:6878) ===== */
void CheckForBombDestroyableObjectPuzzle(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - checks if bomb can destroy puzzle objects */
    (void)bc;
}

/* ===== CheckForBombDestroyableObjectBasic (03:68C0) ===== */
void CheckForBombDestroyableObjectBasic(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - checks if bomb can destroy basic objects */
    (void)bc;
}