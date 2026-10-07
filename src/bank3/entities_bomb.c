#include "bank3/entities_bomb.h"
#include "bank3/entities_arrow.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_droppable.h"
#include "bank3/entities_pushed_block.h"
#include "bank3/entities_liftable_rock.h"
#include "bank3/entities_collision.h"
#include "bank3/entities_handlers.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/physics.h"
#include "constants/gameplay.h"
#include "constants/directions.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include "constants/audio.h"
#include "home/entities.h"
#include "home/room.h"
#include "home/bank.h"
#include "home/audio.h"
#include "home/gameplay.h"
#include "home/link.h"

/* ===== Data Tables (03:6530-03:69A1) ===== */

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

/* ExplosionSpriteVariantFrames (03:65CA) - 24 frames */
static const uint8_t ExplosionSpriteVariantFrames[24] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03
};

/* BombRightBeforeExplodingSprite (03:5484) */
static const uint8_t BombRightBeforeExplodingSprite[4] = {
    0x30, OAM_GBC_PAL_1 | OAMF_PAL0,
    0x30, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP | OAMF_YFLIP
};

/* BombObjectPuzzleDestroyingX (03:671F) */
static const int8_t BombObjectPuzzleDestroyingX[9] = {
    (int8_t)0xF8, 0x08, 0x18,
    (int8_t)0xF8, 0x08, 0x18,
    (int8_t)0xF8, 0x08, 0x18
};

/* BombObjectPuzzleDestroyingY (03:6728) */
static const int8_t BombObjectPuzzleDestroyingY[9] = {
    (int8_t)0xF8, (int8_t)0xF8, (int8_t)0xF8,
    0x08, 0x08, 0x08,
    0x18, 0x18, 0x18
};

/* BombedWallObjects (03:6739) */
static const uint8_t BombedWallObjects[4] = {
    OBJECT_BOMBED_PASSAGE_VERTICAL,
    OBJECT_BOMBED_PASSAGE_VERTICAL,
    OBJECT_BOMBED_PASSAGE_HORIZONTAL,
    OBJECT_BOMBED_PASSAGE_HORIZONTAL
};

/* BombedWallTilesIndexes (03:673D) */
static const uint8_t BombedWallTilesIndexes[16] = {
    0x72, 0x72, 0x73, 0x73, 0x04, 0x04, 0x04, 0x04,
    0x69, 0x79, 0x69, 0x79, 0x04, 0x04, 0x04, 0x04
};

/* BombedCaveDoorTilesIndexesDMG (03:674D) */
static const uint8_t BombedCaveDoorTilesIndexesDMG[4] = {
    0x64, 0x66, 0x65, 0x67
};

/* BombedCaveDoorTilesIndexesGBC (03:6751) */
static const uint8_t BombedCaveDoorTilesIndexesGBC[4] = {
    0x64, 0x66, 0x64, 0x66
};

/* BombedWallCurrentRoomStatus (03:6755) */
static const uint8_t BombedWallCurrentRoomStatus[4] = {
    ROOM_STATUS_DOOR_OPEN_UP,
    ROOM_STATUS_DOOR_OPEN_DOWN,
    ROOM_STATUS_DOOR_OPEN_LEFT,
    ROOM_STATUS_DOOR_OPEN_RIGHT
};

/* BombedWallAdjacentRoomStatus (03:6759) */
static const uint8_t BombedWallAdjacentRoomStatus[4] = {
    ROOM_STATUS_DOOR_OPEN_DOWN,
    ROOM_STATUS_DOOR_OPEN_UP,
    ROOM_STATUS_DOOR_OPEN_RIGHT,
    ROOM_STATUS_DOOR_OPEN_LEFT
};

/* BombedWallAdjacentRoomMapPosDiff (03:675D) */
static const int8_t BombedWallAdjacentRoomMapPosDiff[4] = {
    -8, 8, -1, 1
};

/* BombedGiantSkullTilesIndexes (03:6761) */
static const uint8_t BombedGiantSkullTilesIndexes[4] = {
    0x72, 0x73, 0x73, 0x72
};

/* GiantSkullDiffFromPrevPositionX (03:6769) */
static const int8_t GiantSkullDiffFromPrevPositionX[4] = {
    0x00, 0x10, (int8_t)0xF0, 0x10
};

/* GiantSkullDiffFromPrevPositionY (03:676D) */
static const int8_t GiantSkullDiffFromPrevPositionY[4] = {
    0x00, 0x00, 0x10, 0x00
};

/* BombObjectBasicDestroyingX (03:68E6) */
static const int8_t BombObjectBasicDestroyingX[9] = {
    (int8_t)0xF8, 0x08, 0x18,
    (int8_t)0xF8, 0x08, 0x18,
    (int8_t)0xF8, 0x08, 0x18
};

/* BombObjectBasicDestroyingY (03:68EF) */
static const int8_t BombObjectBasicDestroyingY[9] = {
    (int8_t)0xF8, (int8_t)0xF8, (int8_t)0xF8,
    0x08, 0x08, 0x08,
    0x18, 0x18, 0x18
};

/* ===== RenderBombExplosion (03:65B0) ===== */
void RenderBombExplosion(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesSpriteVariantTable; add hl, bc; ld a, [hl]; sla x5; ld e, a; ld d, b */
    uint8_t variant = gb_read(gb, (uint16_t)(wEntitiesSpriteVariantTable + bc));
    uint8_t offset = (uint8_t)(variant << 5); /* sla x5 */

    /* ld hl, ExplosionSpriteRect; add hl, de; ld c, $08; jp RenderActiveEntitySpritesRect */
    const int8_t *sprite_rect = ExplosionSpriteRect + offset;
    RenderActiveEntitySpritesRect(gb, (const uint8_t *)sprite_rect, 8, NULL);
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
    uint8_t physics = gb_read(gb, (uint16_t)(wEntitiesPhysicsFlagsTable + bc));
    physics = (uint8_t)((physics & ENTITY_PHYSICS_MASK) | 0x08);
    gb_write(gb, (uint16_t)(wEntitiesPhysicsFlagsTable + bc), physics);

    /* call RenderBombExplosion */
    RenderBombExplosion(gb, bc);

    /* ld a, [wIsIndoor]; and a; jr z, .ret */
    if (gb_read(gb, wIsIndoor) == 0) {
        return;
    }

    /* ld a, [wTransitionSequenceCounter]; cp $04; ret nz */
    if (gb_read(gb, wTransitionSequenceCounter) != 0x04) {
        return;
    }

    /* On DMG, the background flashes between two palettes when a bomb explodes. */
    uint8_t e = 0xE4; /* %11_10_01_00 */
    uint8_t room_trans = gb_read(gb, wRoomTransitionState);
    if (room_trans == 0) {
        if ((GetEntityTransitionCountdown(gb, bc) & 0x04) != 0) {
            e = 0x84; /* %10_00_01_00 */
        }
    }

    /* .bgFlashing: ld hl, wBGPalette; ld [hl], e */
    gb_write(gb, wBGPalette, e);
}

/* ===== BombExplosionVisuals.smallExplosion (03:668C) ===== */
static void BombExplosionVisuals_smallExplosion(GBState *gb) {
    RenderActiveEntitySpritesPair(gb, BombRightBeforeExplodingSprite, NULL);
    ReturnIfNonInteractive_03(gb, false);
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
    uint8_t private_state4 = gb_read(gb, (uint16_t)(wEntitiesPrivateState4Table + bc));
    if (private_state4 == 0x4C) {
        return;
    }

    /* cp $0E; jr c, .checkForDestroyableObjectEnd; cp $17; jr nc, .checkForDestroyableObjectEnd */
    if (countdown >= 0x0E && countdown < 0x17) {
        uint16_t de = (uint16_t)(countdown - 0x0E);
        CheckForBombDestroyableObjectBasic(gb, bc, de);
        CheckForBombDestroyableObjectPuzzle(gb, bc, de);
    }

    /* .checkForDestroyableObjectEnd: cp $12; jr nz, .ret */
    if (countdown != 0x12) {
        return;
    }

    /* ld hl, wEntitiesPrivateState4Table; add hl, bc; ld a, [hl]; and a; jr nz, .enemyBomb */
    private_state4 = gb_read(gb, (uint16_t)(wEntitiesPrivateState4Table + bc));
    if (private_state4 == 0) {
        /* Link bomb: call CheckExplosionInteractionWithEntities; jr .enemyBombEnd */
        CheckExplosionInteractionWithEntities(gb, bc);
        goto enemyBombEnd;
    }

    /* .enemyBomb: Enemy bomb - check if Link is in explosion radius */
    {
        uint8_t diff_x = (uint8_t)(gb_read_hram(gb, hActiveEntityPosX) - gb_read_hram(gb, hLinkPositionX) + 0x18);
        if (diff_x >= 0x30) {
            goto enemyBombEnd;
        }

        uint8_t diff_y = (uint8_t)(gb_read_hram(gb, hActiveEntityPosY) - gb_read_hram(gb, hLinkPositionY) + 0x18);
        if (diff_y >= 0x30) {
            goto enemyBombEnd;
        }

        ApplyLinkCollisionWithEnemy(gb, bc);
        uint8_t link_speed_x = (uint8_t)(gb_read_hram(gb, hLinkSpeedX) << 1);
        uint8_t link_speed_y = (uint8_t)(gb_read_hram(gb, hLinkSpeedY) << 1);
        gb_write_hram(gb, hLinkSpeedX, link_speed_x);
        gb_write_hram(gb, hLinkSpeedY, link_speed_y);
    }

enemyBombEnd:
    /* ld a, $04; ld [wSwordMoblinAlertingSoundCounter], a */
    gb_write(gb, wSwordMoblinAlertingSoundCounter, 0x04);
}

/* ===== BombBounceOffWalls (03:66FA) ===== */
void BombBounceOffWalls(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and $03; jr z, .noCollisionX */
    uint8_t collisions = gb_read(gb, (uint16_t)(wEntitiesCollisionsTable + bc));
    if ((collisions & 0x03) != 0) {
        EntityBounceOffWallX(gb, bc);
    }

    /* ldh a, [hIsSideScrolling]; and a; ret nz */
    if (gb_read_hram(gb, hIsSideScrolling) != 0) {
        return;
    }

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and $0C; ret z */
    collisions = gb_read(gb, (uint16_t)(wEntitiesCollisionsTable + bc));
    if ((collisions & 0x0C) != 0) {
        EntityBounceOffWallY(gb, bc);
    }
}

/* ===== RenderBomb (03:6711) ===== */
void RenderBomb(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, hActiveEntityVisualPosY; inc [hl]; inc [hl] */
    uint8_t visual_y = (uint8_t)(gb_read_hram(gb, hActiveEntityVisualPosY) + 2);
    gb_write_hram(gb, hActiveEntityVisualPosY, visual_y);

    /* ld de, BombSprite; call RenderActiveEntitySprite */
    RenderActiveEntitySprite(gb, BombSprite, NULL);

    /* jp CopyEntityPositionToActivePosition */
    CopyEntityPositionToActivePosition(gb, bc);
}

/* ===== BombEntityHandler (03:6696) ===== */
void BombEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* If bomb is outside of the screen, clear it */
    uint8_t visual_y = gb_read_hram(gb, hActiveEntityVisualPosY);
    if ((uint8_t)(visual_y + 0x10) >= (SCRN_Y + 0x10)) {
        UnloadEntity(gb, bc);
        return;
    }

    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown < 0x18) {
        BombExplosionHandler(gb, bc);
        return;
    }

    if (countdown == 0x18) {
        countdown--;
        gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), countdown);
        PlayBombExplosionSfx(gb);
    }

    /* .notExploding: inc [hl] (wHasPlacedBomb) */
    uint8_t placed = gb_read(gb, wHasPlacedBomb);
    gb_write(gb, wHasPlacedBomb, (uint8_t)(placed + 1));

    if (countdown < 0x22) {
        BombExplosionVisuals_smallExplosion(gb);
        return;
    }

    if (countdown == 0x48) {
        gb_write(gb, (uint16_t)(wEntitiesFlashCountdownTable + bc), 0x30);
    }

    /* .skipStartFlashing */
    RenderBomb(gb, bc);
    CheckForEntityFallingDownQuicksandHole(gb, bc);
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    BouncingEntityPhysics(gb, bc);
    gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown2Table + bc), 0xFF);

    uint8_t cd1 = GetEntityPrivateCountdown1(gb, bc);
    uint8_t priv4 = gb_read(gb, (uint16_t)(wEntitiesPrivateState4Table + bc));
    if ((cd1 | priv4) == 0) {
        uint8_t b_slot = gb_read(gb, wInventoryBButtonSlot);
        uint8_t joypad = gb_read_hram(gb, hJoypadState);

        if (b_slot == INVENTORY_BOMBS) {
            if ((joypad & J_B) != 0) {
                EntityGetLiftedUp(gb, bc);
            }
        } else {
            uint8_t a_slot = gb_read(gb, wInventoryAButtonSlot);
            if (a_slot == INVENTORY_BOMBS && (joypad & J_A) != 0) {
                EntityGetLiftedUp(gb, bc);
            }
        }
    }

    BombBounceOffWalls(gb, bc);
}

/* ===== CheckForBombDestroyableObjectPuzzle (03:6771) ===== */
void CheckForBombDestroyableObjectPuzzle(GBState *gb, uint16_t bc, uint16_t de) {
    if (!gb) return;

    /* ldh a, [hIsSideScrolling]; and a; jp nz, .return */
    if (gb_read_hram(gb, hIsSideScrolling) != 0) {
        return;
    }

    if (de >= 9) {
        return;
    }

    /* ld hl, wEntitiesPosXTable; add hl, bc; ld a, [hl]; sub $08; add [hl]; and $F0 */
    uint8_t pos_x = gb_read(gb, (uint16_t)(wEntitiesPosXTable + bc));
    uint8_t obj_left = (uint8_t)((pos_x - 0x08 + (int8_t)BombObjectPuzzleDestroyingX[de]) & 0xF0);
    gb_write_hram(gb, hIntersectedObjectLeft, obj_left);

    /* swap a */
    uint8_t c = (uint8_t)(obj_left >> 4);

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld a, [hl]; sub $10; add [hl]; and $F0 */
    uint8_t pos_y = gb_read(gb, (uint16_t)(wEntitiesPosYTable + bc));
    uint8_t obj_top = (uint8_t)((pos_y - 0x10 + (int8_t)BombObjectPuzzleDestroyingY[de]) & 0xF0);
    gb_write_hram(gb, hIntersectedObjectTop, obj_top);

    /* or c; ld c, a; ld b, $00 */
    c = (uint8_t)(c | obj_top);

    /* ld hl, wRoomObjects; ld a, h; add hl, bc; ld h, a */
    uint16_t obj_addr = (uint16_t)(wRoomObjects + c);

    /* ld a, c; ldh [hIndexOfObjectBelowLink], a */
    gb_write_hram(gb, hIndexOfObjectBelowLink, c);

    /* ld a, [hl]; ldh [hObjectUnderEntity], a; ld e, a */
    uint8_t obj = gb_read(gb, obj_addr);
    gb_write_hram(gb, hObjectUnderEntity, obj);

    /* cp OBJECT_GIANT_SKULL_TL; jr c, .giantSkullEnd; cp OBJECT_GIANT_SKULL_BR + 1; jr nc, .giantSkullEnd */
    if (obj >= OBJECT_GIANT_SKULL_TL && obj <= OBJECT_GIANT_SKULL_BR && gb_read(gb, wIsIndoor) == 0) {
        /* Destroy the giant skull */
        gb_write_hram(gb, hJingle, JINGLE_PUZZLE_SOLVED);

        gb_write_hram(gb, hIntersectedObjectTop, (uint8_t)(gb_read_hram(gb, hIntersectedObjectTop) & 0xE0));
        gb_write_hram(gb, hIntersectedObjectLeft, (uint8_t)(gb_read_hram(gb, hIntersectedObjectLeft) & 0xE0));

        Spawn2x2RubbleEntities_trampoline(gb, 0x03, NULL);

        c = (uint8_t)(c & 0xEE);
        gb_write(gb, (uint16_t)(wRoomObjects + c), OBJECT_ROCKY_GROUND);
        BackupObjectInRAM2(gb, (uint16_t)(wRoomObjects + c), 0x83);

        gb_write(gb, (uint16_t)(wRoomObjects + c + 1), OBJECT_ROCKY_GROUND);
        BackupObjectInRAM2(gb, (uint16_t)(wRoomObjects + c + 1), 0x83);

        gb_write(gb, (uint16_t)(wRoomObjects + c + 0x10), OBJECT_ROCKY_GROUND);
        BackupObjectInRAM2(gb, (uint16_t)(wRoomObjects + c + 0x10), 0x83);

        gb_write(gb, (uint16_t)(wRoomObjects + c + 0x11), OBJECT_ROCKY_GROUND);
        gb_write(gb, wDDD8, OBJECT_ROCKY_GROUND);
        BackupObjectInRAM2(gb, (uint16_t)(wRoomObjects + c + 0x11), 0x83);

        for (int8_t loop_c = 3; loop_c >= 0; loop_c--) {
            uint8_t room = gb_read_hram(gb, hMapRoom);
            uint8_t status = (uint8_t)(gb_read(gb, (uint16_t)(wOverworldRoomStatus + room)) | OW_ROOM_STATUS_OPENED);
            gb_write(gb, (uint16_t)(wOverworldRoomStatus + room), status);
            gb_write_hram(gb, hRoomStatus, status);

            label_003_51F5(gb, BombedGiantSkullTilesIndexes);

            uint8_t cur_left = gb_read_hram(gb, hIntersectedObjectLeft);
            gb_write_hram(gb, hIntersectedObjectLeft, (uint8_t)(cur_left + GiantSkullDiffFromPrevPositionX[loop_c]));

            uint8_t cur_top = gb_read_hram(gb, hIntersectedObjectTop);
            gb_write_hram(gb, hIntersectedObjectTop, (uint8_t)(cur_top + GiantSkullDiffFromPrevPositionY[loop_c]));
        }
        return;
    }

    /* .giantSkullEnd: ld a, [wIsIndoor]; ld d, a; call GetObjectPhysicsFlags_trampoline */
    uint8_t is_indoor = gb_read(gb, wIsIndoor);
    uint8_t flags = GetObjectPhysicsFlags_trampoline(gb, (uint16_t)((is_indoor << 8) | obj));
    if (flags < (OBJ_PHYSICS_DOOR_CLOSED | 0x09)) {
        return;
    }

    uint8_t door_idx = (uint8_t)(flags - (OBJ_PHYSICS_DOOR_CLOSED | 0x09));
    if (door_idx >= 4) {
        return;
    }

    /* Destroy bombable doors/walls */
    gb_write_hram(gb, hJingle, JINGLE_PUZZLE_SOLVED);

    if (is_indoor == 0) {
        /* Outdoors (cave entrance) */
        uint8_t idx = gb_read_hram(gb, hIndexOfObjectBelowLink);
        gb_write(gb, (uint16_t)(wRoomObjects + idx), OBJECT_ROCKY_CAVE_DOOR);
        gb_write(gb, wDDD8, OBJECT_ROCKY_CAVE_DOOR);
        BackupObjectInRAM2(gb, (uint16_t)(wRoomObjects + idx), 0x83);

        const uint8_t *tiles = (gb_read_hram(gb, hIsGBC) != 0) ? BombedCaveDoorTilesIndexesGBC : BombedCaveDoorTilesIndexesDMG;
        uint8_t room = gb_read_hram(gb, hMapRoom);
        uint8_t status = (uint8_t)(gb_read(gb, (uint16_t)(wOverworldRoomStatus + room)) | OW_ROOM_STATUS_OPENED);
        gb_write(gb, (uint16_t)(wOverworldRoomStatus + room), status);
        gb_write_hram(gb, hRoomStatus, status);

        label_003_51F5(gb, tiles);
        return;
    }

    /* Indoors bombable door */
    uint16_t status_addr = GetRoomStatusAddressInHL(gb);
    uint8_t cur_status = (uint8_t)(gb_read(gb, status_addr) | BombedWallCurrentRoomStatus[door_idx]);
    gb_write(gb, status_addr, cur_status);
    gb_write_hram(gb, hRoomStatus, cur_status);

    /* Set room status for adjacent room */
    uint8_t indoor_room = gb_read(gb, wIndoorRoom);
    uint8_t adj_map_pos = (uint8_t)(indoor_room + (int8_t)BombedWallAdjacentRoomMapPosDiff[door_idx]);
    uint16_t adj_addr = GetRoomStatusAddressForMapPosition(gb, (uint16_t)adj_map_pos);
    if (adj_addr != 0) {
        uint8_t adj_status = (uint8_t)(gb_read(gb, adj_addr) | BombedWallAdjacentRoomStatus[door_idx]);
        gb_write(gb, adj_addr, adj_status);
    }

    /* Replace object in wRoomObjects */
    uint8_t tile_left = (uint8_t)((gb_read_hram(gb, hIntersectedObjectLeft) >> 4) & 0x0F);
    uint8_t tile_top = (uint8_t)(gb_read_hram(gb, hIntersectedObjectTop) & 0xF0);
    uint8_t tile_idx = (uint8_t)(tile_top | tile_left);
    uint8_t wall_obj = BombedWallObjects[door_idx];
    gb_write(gb, (uint16_t)(wRoomObjects + tile_idx), wall_obj);
    gb_write(gb, wDDD8, wall_obj);

    uint8_t tile_table_offset = (uint8_t)(((door_idx & 0x02) != 0) ? 8 : 0);
    label_003_51F5(gb, BombedWallTilesIndexes + tile_table_offset);
}

/* ===== CheckForBombDestroyableObjectBasic (03:68F8) ===== */
void CheckForBombDestroyableObjectBasic(GBState *gb, uint16_t bc, uint16_t de) {
    if (!gb) return;
    (void)bc;

    if (de >= 9) {
        return;
    }

    /* ld hl, BombObjectBasicDestroyingX; add hl, de; ldh a, [hActiveEntityPosX]; add [hl]; sub $08; and $F0 */
    uint8_t left = (uint8_t)((gb_read_hram(gb, hActiveEntityPosX) + (int8_t)BombObjectBasicDestroyingX[de] - 0x08) & 0xF0);
    gb_write_hram(gb, hIntersectedObjectLeft, left);

    /* swap a; ld c, a */
    uint8_t c = (uint8_t)(left >> 4);

    /* ld hl, BombObjectBasicDestroyingY; add hl, de; ldh a, [hActiveEntityVisualPosY]; add [hl]; sub $10; and $F0 */
    uint8_t top = (uint8_t)((gb_read_hram(gb, hActiveEntityVisualPosY) + (int8_t)BombObjectBasicDestroyingY[de] - 0x10) & 0xF0);
    gb_write_hram(gb, hIntersectedObjectTop, top);

    /* or c; ld e, a */
    uint8_t e = (uint8_t)(top | c);

    /* ld hl, wRoomObjects; add hl, de; ld a, h; cp HIGH(wRoomObjectsArea); jp nz, .return */
    uint16_t obj_addr = (uint16_t)(wRoomObjects + e);
    if ((obj_addr >> 8) != 0xD7) {
        return;
    }

    uint8_t is_indoor = gb_read(gb, wIsIndoor);
    uint8_t obj = gb_read(gb, obj_addr);
    gb_write_hram(gb, hObjectUnderEntity, obj);

    if (is_indoor == 0) {
        gb_write_hram(gb, hMultiPurposeH, obj);
        if (obj != OBJECT_TALL_GRASS && obj != OBJECT_BUSH_GROUND_STAIRS && obj != OBJECT_BUSH) {
            return;
        }
    } else {
        if (obj != OBJECT_BOMBABLE_BLOCK) {
            return;
        }
        uint16_t status_addr = GetRoomStatusAddressInHL(gb);
        uint8_t status = (uint8_t)(gb_read(gb, status_addr) | ROOM_STATUS_EVENT_3);
        gb_write(gb, status_addr, status);
        gb_write_hram(gb, hRoomStatus, status);
    }

    /* .bombedGrassOrBush: */
    RevealObjectUnderObject_trampoline(gb, NULL);

    /* Spawn bush entity (ENTITY_LIFTABLE_ROCK) for visual effect */
    uint16_t rock_slot = SpawnNewEntity_slot(gb, ENTITY_LIFTABLE_ROCK);
    if (rock_slot == 0xFFFF) {
        return;
    }

    gb_write(gb, wLinkAttackStepAnimationCountdown, 0);

    gb_write(gb, (uint16_t)(wEntitiesPosXTable + rock_slot), (uint8_t)(gb_read_hram(gb, hIntersectedObjectLeft) + 0x08));
    gb_write(gb, (uint16_t)(wEntitiesPosYTable + rock_slot), (uint8_t)(gb_read_hram(gb, hIntersectedObjectTop) + 0x10));

    uint8_t variant = (uint8_t)(gb_read(gb, wIsIndoor) ^ 0x01);
    gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + rock_slot), variant);
    gb_write_hram(gb, hActiveEntitySpriteVariant, variant);

    if (gb_read_hram(gb, hMultiPurposeH) == OBJECT_TALL_GRASS) {
        gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + rock_slot), 0xFF);
        gb_write_hram(gb, hActiveEntitySpriteVariant, 0xFF);
    }

    LiftableRockStartSmashingAnimation(gb, rock_slot);
}