#include "bank3/entities_magic_rod.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_arrow.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/gameplay.h"
#include "constants/directions.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include "constants/vfx.h"
#include "home/entities.h"
#include "home/vfx.h"
#include "home/room.h"
#include "home/bank.h"
#include "home/audio.h"
#include "home/gameplay.h"
#include "constants/audio.h"

/* HRAM address for object below Link (alias for hMultiPurposeH at 0xFFE9) */
#ifndef hIndexOfObjectBelowLink
#define hIndexOfObjectBelowLink hMultiPurposeH
#endif

/* ===== Data Tables (03:69AA) ===== */

/* MagicRodFireballSpriteVariants (03:69AA) */
static const uint8_t MagicRodFireballSpriteVariants[8] = {
    /* variant 0 */
    0x36, OAM_GBC_PAL_2 | OAMF_PAL0,
    0x36, OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP,
    /* variant 1 */
    0x36, OAM_GBC_PAL_2 | OAMF_PAL1,
    0x36, OAM_GBC_PAL_2 | OAMF_PAL1 | OAMF_XFLIP
};

/* FireSpriteVariants (03:4C44) - referenced by MagicRodFireballEntityHandler */
static const uint8_t FireSpriteVariants[8] = {
    0x34, 0x02, 0x34, 0x42,
    0x34, 0x04, 0x34, 0x44
};

/* ===== MagicRodFireballEntityHandler (03:69B2) ===== */
void MagicRodFireballEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wActiveProjectileCount; inc [hl] */
    gb_write(gb, wActiveProjectileCount, gb_read(gb, wActiveProjectileCount) + 1);

    /* ld a, DAMAGE_TYPE_MAGIC_ROD; ld [wAttackDamageType], a; call func_003_75A2 */
    gb_write(gb, wAttackDamageType, DAMAGE_TYPE_MAGIC_ROD);
    func_003_75A2(gb, bc);

    /* ldh a, [hFrameCounter]; rra x3; and $01; ld hl, wEntitiesSpriteVariantTable; add hl, bc; ld [hl], a */
    uint8_t frame = gb_read_hram(gb, hFrameCounter);
    uint8_t variant = (frame >> 3) & 0x01;
    gb_write(gb, wEntitiesSpriteVariantTable + bc, variant);

    /* call GetEntityPrivateCountdown1; jr z, .beforeHittingWall */
    if (GetEntityPrivateCountdown1(gb, bc) == 0) {
        goto beforeHittingWall;
    }

    /* If the fireball has hit a wall, just draw fire. */
    /* dec a; jp z, UnloadEntity */
    if (GetEntityPrivateCountdown1(gb, bc) == 1) {
        UnloadEntity(gb, bc);
        return;
    }

    /* ld de, FireSpriteVariants; jp RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, FireSpriteVariants, NULL);
    return;

beforeHittingWall:
    /* ld de, MagicRodFireballSpriteVariants; call ArrowRenderAndMove.skipLoadingSprites */
    RenderActiveEntitySpritesPair(gb, MagicRodFireballSpriteVariants, NULL);

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* call GetEntityTransitionCountdown; jr nz, ArrowRockAfterHittingWall */
    if (GetEntityTransitionCountdown(gb, bc) != 0) {
        ArrowRockAfterHittingWall(gb, bc);
        return;
    }

    /* call UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);

    /* call ApplySwordIntersectionWithObjects */
    ApplySwordIntersectionWithObjects(gb, bc);

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and a; jr z, EntityBounceOffWallX.return */
    if (gb_read(gb, wEntitiesCollisionsTable + bc) != 0) {
        /* call GetEntityTransitionCountdown; ldh a, [hActiveEntityType]; cp ENTITY_MAGIC_ROD_FIREBALL; jr nz, .fireballEnd */
        /* call GetEntityPrivateCountdown1; ld [hl], $30; ret */
        GetEntityPrivateCountdown1(gb, bc);
        gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x30);
        return;
    }

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* ld a, [wIsIndoor]; and a; ldh a, [hObjectUnderEntity]; jr z, .outdoors */
    if (gb_read(gb, wIsIndoor) != 0) {
        /* Indoors: check for frozen block */
        /* cp OBJECT_FROZEN_BLOCK; jr z, .burnObject; jr .return */
        if (gb_read_hram(gb, hObjectUnderEntity) == OBJECT_FROZEN_BLOCK) {
            goto burnObject;
        }
        return;
    }

    /* .outdoors: */
    /* Outdoors: check for bush */
    /* cp OBJECT_BUSH_GROUND_STAIRS; jr z, .burnObject */
    /* cp OBJECT_BUSH; jr nz, .return */
    uint8_t obj = gb_read_hram(gb, hObjectUnderEntity);
    if (obj != OBJECT_BUSH_GROUND_STAIRS && obj != OBJECT_BUSH) {
        return;
    }

burnObject:
    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesCollisionsTable + bc, 0);
    /* call GetEntityPrivateCountdown1; ld [hl], b */
    gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0);
    /* ldh a, [hIndexOfObjectBelowLink]; ld e, a; ld d, b; call RevealObjectUnderObject_trampoline */
    (void)gb_read_hram(gb, hIndexOfObjectBelowLink);
    RevealObjectUnderObject_trampoline(gb, NULL);
    
    /* ldh a, [hIntersectedObjectLeft]; add $08; ldh [hMultiPurpose0], a */
    gb_write_hram(gb, hMultiPurpose0, gb_read_hram(gb, hIntersectedObjectLeft) + 0x08);
    /* ldh a, [hIntersectedObjectTop]; add $10; ldh [hMultiPurpose1], a */
    gb_write_hram(gb, hMultiPurpose1, gb_read_hram(gb, hIntersectedObjectTop) + 0x10);
    /* ld a, TRANSCIENT_VFX_SMOKE; call AddTranscientVfx */
    AddTranscientVfx(gb, TRANSCIENT_VFX_SMOKE);
    /* ld a, NOISE_SFX_ENEMY_DESTROYED; ldh [hNoiseSfx], a; ret */
    gb_write_hram(gb, hNoiseSfx, NOISE_SFX_ENEMY_DESTROYED);
    return;
}