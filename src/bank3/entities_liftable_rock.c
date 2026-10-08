#include "bank3/entities_liftable_rock.h"
#include "bank3/entities_droppable.h"
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

/* ===== Data Tables (03:5398-03:53A7) ===== */

/* LiftableRockOutdoorSpriteVariants (03:5398) - 4 variants * 4 bytes = 16 bytes */
static const uint8_t LiftableRockOutdoorSpriteVariants[16] = {
    0xF0, 0xE7, 0xF2, 0xE7,  /* variant 0: Rock */
    0xF4, 0xE6, 0xF6, 0xE6   /* variant 1: Bush */
};

/* LiftableRockIndoorSpriteVariants (03:53A0) - 4 variants * 4 bytes = 16 bytes */
static const uint8_t LiftableRockIndoorSpriteVariants[16] = {
    0xF0, 0xE6, 0xF2, 0xE6,  /* variant 0: Pot/Skull */
    0xF4, 0xE6, 0xF6, 0xE6   /* variant 1: Unused? */
};

/* ===== Entity4BHandler (03:5326) ===== */
void Entity4BHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld d, $03; fallthrough to LiftableRockEntityHandler */
    /* This is a special entry point that sets d=3 before calling LiftableRockEntityHandler */
    (void)gb;  /* d register is handled in LiftableRockEntityHandler */
    LiftableRockEntityHandler(gb, bc);
}

/* ===== LiftableRockEntityHandler (03:5328) ===== */
void LiftableRockEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, c; ld [wPickedUpRockIndex], a */
    uint8_t c = bc & 0xFF;
    gb_write(gb, wPickedUpRockIndex, c);

    /* call GetEntityPrivateCountdown1; ldh [hMultiPurpose0], a; jp z, LiftableRockIntactHandler */
    uint8_t countdown1 = GetEntityPrivateCountdown1(gb, bc);
    gb_write_hram(gb, hMultiPurpose0, countdown1);
    if (countdown1 == 0) {
        LiftableRockIntactHandler(gb, bc);
        return;
    }

    /* cp $01; jr nz, jr_003_5395 */
    if (countdown1 != 0x01) {
        goto jr_003_5395;
    }

    /* Last frame */
    /* ld hl, wEntitiesPrivateState5Table; add hl, bc; ld a, [hl]; and a; jr z, .spawnFairyEnd */
    uint8_t private_state5 = gb_read(gb, wEntitiesPrivateState5Table + bc);
    if (private_state5 != 0) {
        /* call GetRandomByte; and $03; jr nz, .spawnFairyEnd */
        uint8_t random = GetRandomByte(gb) & 0x03;
        if (random == 0) {
            /* ld a, ENTITY_DROPPABLE_FAIRY; call SpawnNewEntity; jr c, .spawnFairyEnd */
            uint16_t de = SpawnNewEntity_trampoline(gb, ENTITY_DROPPABLE_FAIRY, NULL);
            if (de != 0xFFFF) {
                /* ldh a, [hMultiPurpose0]; ld hl, wEntitiesPosXTable; add hl, de; ld [hl], a */
                gb_write(gb, wEntitiesPosXTable + de, gb_read_hram(gb, hMultiPurpose0));
                /* ldh a, [hMultiPurpose1]; ld hl, wEntitiesPosYTable; add hl, de; ld [hl], a */
                gb_write(gb, wEntitiesPosYTable + de, gb_read_hram(gb, hMultiPurpose1));
                /* ldh a, [hMultiPurpose3]; ld hl, wEntitiesPosZTable; add hl, de; ld [hl], a */
                gb_write(gb, wEntitiesPosZTable + de, gb_read_hram(gb, hMultiPurpose3));
                /* ld hl, wEntitiesSlowTransitionCountdownTable; add hl, de; ld [hl], $80 */
                gb_write(gb, wEntitiesSlowTransitionCountdownTable + de, 0x80);
            }
        }
    }

    /* .spawnFairyEnd: */
    /* ldh a, [hActiveEntitySpriteVariant]; and a; jr nz, .marinReactionEnd */
    if (gb_read_hram(gb, hActiveEntitySpriteVariant) != 0) {
        goto marinReactionEnd;
    }

    /* If inside a house... ldh a, [hMapId]; cp MAP_GHOST_HOUSE; jr z, .insideHouse */
    /* cp MAP_HOUSE; jr nz, .marinReactionEnd */
    uint8_t map_id = gb_read_hram(gb, hMapId);
    if (map_id == MAP_GHOST_HOUSE || map_id == MAP_HOUSE) {
        /* ... and Marin is following Link... ld a, [wIsMarinFollowingLink]; and a; jr z, .marinReactionEnd */
        if (gb_read(gb, wIsMarinFollowingLink) != 0) {
            /* draw a random number; and $3F; jr nz, .marinReaction2 */
            uint8_t random = GetRandomByte(gb) & 0x3F;
            if (random == 0) {
                /* Open Marin reaction 1 (Dialog028) */
                OpenDialogInTable0_trampoline(gb, 0x28);
                /* jp UnloadEntityAndReturn */
                UnloadEntityAndReturn(gb, bc);
                return;
            }
            /* .marinReaction2: Open Marin reaction 2 (Dialog199) */
            OpenDialogInTable0_trampoline(gb, 0xC7);  /* Dialog199 */
        }
    }

    /* fall through to marinReactionEnd */
    /* jp UnloadEntityAndReturn */
    UnloadEntityAndReturn(gb, bc);
    return;

marinReactionEnd:
    /* jp UnloadEntityAndReturn */
    UnloadEntityAndReturn(gb, bc);
    return;

jr_003_5395:
    /* jp label_3935 */
    label_3935(gb, NULL);
}

/* ===== LiftableRockIntactHandler (03:53A8) ===== */
void LiftableRockIntactHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* Select sprite variants based on indoor/outdoor */
    const uint8_t *de = LiftableRockOutdoorSpriteVariants;
    if (gb_read(gb, wIsIndoor) != 0) {
        de = LiftableRockIndoorSpriteVariants;
    }

    /* .render: call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, de, NULL);

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* ld a, DAMAGE_TYPE_THROW_AT; ld [wAttackDamageType], a; call func_003_75A2 */
    gb_write(gb, wAttackDamageType, DAMAGE_TYPE_THROW_AT);
    func_003_75A2(gb, bc);

    /* call BouncingEntityPhysics */
    BouncingEntityPhysics(gb, bc);

    /* ld hl, wEntitiesStatusTable; add hl, bc; ld a, [hl]; cp ENTITY_STATUS_FALLING; jp z, ret_003_5406 */
    uint8_t status = gb_read(gb, wEntitiesStatusTable + bc);
    if (status == ENTITY_STATUS_FALLING) {
        return;
    }

    /* ld hl, wEntitiesPosZTable; add hl, bc; ld a, [hl]; and a; jr z, LiftableRockStartSmashingAnimation */
    uint8_t pos_z = gb_read(gb, wEntitiesPosZTable + bc);
    if (pos_z == 0) {
        LiftableRockStartSmashingAnimation(gb, bc);
        return;
    }

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and a; jr z, ret_003_5406 */
    uint8_t collisions = gb_read(gb, wEntitiesCollisionsTable + bc);
    if (collisions == 0) {
        return;
    }

    /* call EntityCheckThrowAtTriggers; fallthrough to LiftableRockStartSmashingAnimation */
    EntityCheckThrowAtTriggers(gb, bc);
    /* fallthrough */
    LiftableRockStartSmashingAnimation(gb, bc);
}

/* ===== LiftableRockStartSmashingAnimation (03:53E4) ===== */
void LiftableRockStartSmashingAnimation(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, hNoiseSfx; ld [hl], NOISE_SFX_CUT_GRASS */
    gb_write_hram(gb, hNoiseSfx, NOISE_SFX_CUT_GRASS);

    /* ld e, $1F; ldh a, [hActiveEntitySpriteVariant]; cp $FF; jr z, .grassSound */
    /* cp $01; jr z, .grassSound */
    /* ld [hl], NOISE_SFX_POT_SMASHED; ld e, $0F */
    uint8_t e = 0x1F;
    uint8_t variant = gb_read_hram(gb, hActiveEntitySpriteVariant);
    if (variant != 0xFF && variant != 0x01) {
        gb_write_hram(gb, hNoiseSfx, NOISE_SFX_POT_SMASHED);
        e = 0x0F;
    }

    /* ld hl, wEntitiesPrivateCountdown1Table; add hl, bc; ld [hl], e */
    gb_write(gb, wEntitiesPrivateCountdown1Table + bc, e);

    /* ld hl, wEntitiesPhysicsFlagsTable; add hl, bc; inc [hl]; inc [hl] */
    uint8_t physics = gb_read(gb, wEntitiesPhysicsFlagsTable + bc);
    physics += 2;
    gb_write(gb, wEntitiesPhysicsFlagsTable + bc, physics);

    return;
}

/* ===== SmashRock (03:5407) ===== */
void SmashRock(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, ENTITY_LIFTABLE_ROCK; call SpawnNewEntity; ret c */
    uint16_t de = SpawnNewEntity_slot(gb, ENTITY_LIFTABLE_ROCK);
    if (de == 0xFFFF) {
        return;
    }

    /* ldh a, [hMultiPurpose0]; ld hl, wEntitiesPosXTable; add hl, de; ld [hl], a */
    gb_write(gb, (uint16_t)(wEntitiesPosXTable + de), gb_read_hram(gb, hMultiPurpose0));

    /* ldh a, [hMultiPurpose1]; ld hl, hMultiPurpose3; sub [hl]; ld hl, wEntitiesPosYTable; add hl, de; ld [hl], a */
    uint8_t y = (uint8_t)(gb_read_hram(gb, hMultiPurpose1) - gb_read_hram(gb, hMultiPurpose3));
    gb_write(gb, (uint16_t)(wEntitiesPosYTable + de), y);

    /* ld hl, wEntitiesSpriteVariantTable; add hl, de; ld [hl], $00 */
    gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + de), 0x00);

    /* ld hl, wEntitiesPrivateCountdown1Table; add hl, de; ld [hl], $0F */
    gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown1Table + de), 0x0F);

    /* ld hl, wEntitiesPhysicsFlagsTable; add hl, de; ld [hl], 4 | ENTITY_PHYSICS_HARMLESS | ENTITY_PHYSICS_PROJECTILE_NOCLIP */
    gb_write(gb, (uint16_t)(wEntitiesPhysicsFlagsTable + de),
             (uint8_t)(4 | ENTITY_PHYSICS_HARMLESS | ENTITY_PHYSICS_PROJECTILE_NOCLIP));

    /* ld a, NOISE_SFX_POT_SMASHED; ldh [hNoiseSfx], a */
    gb_write_hram(gb, hNoiseSfx, NOISE_SFX_POT_SMASHED);

    /* jp UnloadEntityAndReturn */
    UnloadEntityAndReturn(gb, bc);
}