#include "bank3/entities_handlers.h"
#include "bank3/entities_init_core.h"
#include "bank3/entities_bomb.h"
#include "bank3/entities_droppable.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_collision.h"
#include "bank3/entities_moblin.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/rooms.h"
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

/* ===== Data Tables (03:4C44-03:5721) ===== */

/* FireSpriteVariants (03:4C44) */
const uint8_t FireSpriteVariants[8] = {
    0x34, (OAM_GBC_PAL_2 | OAMF_PAL0),               /* variant0: tile $34, palette 2 */
    0x34, (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP),  /* variant0 flipped: tile $34, palette 2 | xflip */
    0x34, (OAM_GBC_PAL_4 | OAMF_PAL1),               /* variant1: tile $34, palette 4 | pal1 */
    0x34, (OAM_GBC_PAL_4 | OAMF_PAL1 | OAMF_XFLIP)   /* variant1 flipped: tile $34, palette 4 | pal1 | xflip */
};

/* Data_003_4CA4 (03:4CA4) */
const uint8_t Data_003_4CA4[4] = { 0x00, 0x00, 0x04, 0x00 };

/* Data_003_4CA8 (03:4CA8) */
const uint8_t Data_003_4CA8[4] = { 0x00, 0x01, 0x03, 0x06 };

/* Data_003_4CAC (03:4CAC) */
const uint8_t Data_003_4CAC[6] = { 0x24, 0x01, 0x24, 0x01, 0x3E, 0x01 };

/* Unknown020SpriteVariants (03:4CB2) */
const uint8_t Unknown020SpriteVariants[4] = {
    0x1E, 0x01, 0x1E, 0x61
};

/* Data_003_4E05 (03:4E05) */
const uint8_t Data_003_4E05[2] = { 0x10, 0xF0 };

/* Data_003_5488 (03:5488): Normal enemy death explosion display list (4 frames * 16 bytes) */
const uint8_t Data_003_5488[64] = {
    0x00, 0x00, 0x3C, 0x01, 0x00, 0x08, 0x3C, 0x21, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x3A, 0x01, 0x00, 0x08, 0x3A, 0x21, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFA, 0xFA, 0x3A, 0x01, 0xFA, 0x02, 0x3A, 0x21, 0x06, 0x06, 0x3A, 0x01, 0x06, 0x0E, 0x3A, 0x21,
    0x04, 0xFC, 0x30, 0x01, 0x04, 0x04, 0x30, 0x21, 0xFC, 0x04, 0x30, 0x01, 0xFC, 0x0C, 0x30, 0x21
};

/* Data_003_54C8 (03:54C8): Power recoil enemy death explosion display list (5 frames * 16 bytes) */
const uint8_t Data_003_54C8[80] = {
    0x00, 0x00, 0x3A, 0x01, 0x00, 0x08, 0x3A, 0x21, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF8, 0xF8, 0x3A, 0x01, 0xF8, 0x00, 0x3A, 0x21, 0x08, 0x08, 0x3A, 0x01, 0x08, 0x10, 0x3A, 0x21,
    0x08, 0xF8, 0x3A, 0x01, 0x08, 0x00, 0x3A, 0x21, 0xF8, 0x08, 0x3A, 0x01, 0xF8, 0x10, 0x3A, 0x21,
    0xF8, 0xF8, 0x10, 0x02, 0xF8, 0x00, 0x12, 0x02, 0xF8, 0x08, 0x12, 0x22, 0xF8, 0x10, 0x10, 0x22,
    0x08, 0xF8, 0x10, 0x42, 0x08, 0x00, 0x12, 0x42, 0x08, 0x08, 0x12, 0x62, 0x08, 0x10, 0x10, 0x62
};

/* DropTableByIndex (03:559D): Item dropped per health group offset */
const uint8_t DropTableByIndex[14] = {
    ENTITY_DROPPABLE_RUPEE,
    ENTITY_DROPPABLE_RUPEE,
    ENTITY_DROPPABLE_HEART,
    ENTITY_DROPPABLE_HEART,
    ENTITY_DROPPABLE_ARROWS,
    ENTITY_DROPPABLE_HEART,
    ENTITY_NONE,
    ENTITY_NONE,
    ENTITY_DROPPABLE_FAIRY,
    ENTITY_DROPPABLE_ARROWS,
    ENTITY_DROPPABLE_BOMBS,
    ENTITY_DROPPABLE_RUPEE,
    ENTITY_DROPPABLE_FAIRY,
    ENTITY_DROPPABLE_FAIRY
};

/* RandomDropChanceTable (03:55AB): Drop chance mask per health group offset */
const uint8_t RandomDropChanceTable[14] = {
    DROP_CHANCE_25_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_25_PERCENT,
    DROP_CHANCE_25_PERCENT,
    DROP_CHANCE_25_PERCENT,
    DROP_CHANCE_25_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_25_PERCENT,
    DROP_CHANCE_0_PERCENT
};

/* RandomDropChanceTableLowHealth (03:55B9): Drop chance mask per health group offset when on low health */
const uint8_t RandomDropChanceTableLowHealth[14] = {
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_0_PERCENT,
    DROP_CHANCE_50_PERCENT,
    DROP_CHANCE_0_PERCENT
};

/* DropTableRandom (03:55C7): Fallback random drop table (8 entries) */
const uint8_t DropTableRandom[8] = {
    ENTITY_DROPPABLE_RUPEE,
    ENTITY_DROPPABLE_HEART,
    ENTITY_DROPPABLE_BOMBS,
    ENTITY_DROPPABLE_FAIRY,
    ENTITY_DROPPABLE_RUPEE,
    ENTITY_DROPPABLE_HEART,
    ENTITY_DROPPABLE_BOMBS,
    ENTITY_DROPPABLE_ARROWS
};

/* DestroyedEntityHealthGroupOffsetTable (03:4826): Health group to drop table index mapping (53 entries) */
const uint8_t DestroyedEntityHealthGroupOffsetTable[53] = {
    0x02, 0x06, 0x01, 0x03, 0x03, 0x03, 0x0D, 0x08, 0x0A, 0x02, 0x07, 0x0B, 0x00, 0x04, 0x00, 0x08,
    0x04, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x0D, 0x0E, 0x00, 0x09, 0x03,
    0x06, 0x00, 0x02, 0x0E, 0x0E
};

/* Data_003_56EA (03:56EA): Normal lift animation countdown delays */
const uint8_t Data_003_56EA[4] = { 0x01, 0x08, 0x08, 0x10 };

/* Data_003_56EE (03:56EE): Fast lift animation countdown delays (bomb, bracelet L2, red tunic, piece of power) */
const uint8_t Data_003_56EE[3] = { 0x01, 0x04, 0x04 };

/* Data_003_56F1 (03:56F1): wIsCarryingLiftedObject per direction and lift phase */
const uint8_t Data_003_56F1[17] = {
    0x0A, 0x37, 0x37, 0x37, /* right */
    0x01, 0x39, 0x39, 0x39, /* left */
    0x01, 0x3B, 0x3B, 0x3B, /* up */
    0x01, 0x3D, 0x3D, 0x3D, /* down */
    0x01                    /* index 16 (reads Data_003_5701[0]) */
};

/* Data_003_5701 (03:5701): X offset per direction and lift phase */
const uint8_t Data_003_5701[17] = {
    0x01, 0x10, 0x10, 0x08, /* right */
    0x00, 0xF0, 0xF0, 0xF8, /* left */
    0x00, 0x00, 0x00, 0x00, /* up */
    0x00, 0xFF, 0xFF, 0xFF, /* down */
    0xFF                    /* index 16 (reads Data_003_5711[0]) */
};

/* Data_003_5711 (03:5711): Y offset per direction and lift phase */
const uint8_t Data_003_5711[17] = {
    0xFF, 0x00, 0x00, 0x00, /* right */
    0x00, 0x00, 0x00, 0x00, /* left */
    0x00, 0x00, 0x00, 0x00, /* up */
    0x00, 0x00, 0x00, 0x08, /* down */
    0x00                    /* index 16 (reads Data_003_5721[0]) */
};

/* Data_003_5721 (03:5721): Z offset (or Y subtraction in sidescroll) per direction and lift phase */
const uint8_t Data_003_5721[17] = {
    0x00, 0x00, 0x00, 0x08,
    0x0E, 0x00, 0x00, 0x08,
    0x0E, 0x00, 0x00, 0x08,
    0x0E, 0x00, 0x00, 0x00,
    0x0E                    /* index 16 */
};

/* ===== Entity 25/26 Stubs (03:4C44) ===== */
void EntityInitEntity25(GBState *gb, uint16_t bc) {
    EntityBurningHandler(gb, bc);
}

void EntityInitEntity26(GBState *gb, uint16_t bc) {
    EntityBurningHandler(gb, bc);
}

void Entity25Handler(GBState *gb, uint16_t bc) {
    EntityBurningHandler(gb, bc);
}

void Entity26Handler(GBState *gb, uint16_t bc) {
    EntityBurningHandler(gb, bc);
}

/* ===== EntityBurningHandler (03:4C4C) ===== */
void EntityBurningHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; jr z, .burningEnd */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0) {
        goto burningEnd;
    }

    /* Animate the entity burning with fire */
    /* ldh a, [hFrameCounter]; rra; rra; rra; and $01; ldh [hActiveEntitySpriteVariant], a */
    uint8_t frame = gb_read_hram(gb, hFrameCounter);
    uint8_t variant = (frame >> 3) & 0x01;
    gb_write_hram(gb, hActiveEntitySpriteVariant, variant);

    /* ld de, FireSpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, FireSpriteVariants, NULL);

    /* ld hl, wEntitiesSpriteVariantTable; add hl, bc; ld a, [hl]; ldh [hActiveEntitySpriteVariant], a */
    uint8_t sprite_variant = gb_read(gb, wEntitiesSpriteVariantTable + bc);
    gb_write_hram(gb, hActiveEntitySpriteVariant, sprite_variant);

    /* call ExecuteActiveEntityHandler_trampoline */
    ExecuteActiveEntityHandler_trampoline(gb, NULL);

    /* call ReturnIfNonInteractive_03.allowInactiveEntity */
    if (ReturnIfNonInteractive_03(gb, true)) {
        return;
    }

    /* call ApplyRecoilIfNeeded_03 */
    ApplyRecoilIfNeeded_03(gb, bc);

    /* call BouncingEntityPhysics */
    BouncingEntityPhysics(gb, bc);

    /* call ClearEntitySpeed */
    ClearEntitySpeed(gb, bc);
    return;

burningEnd:
    /* If burning a Gibdo... */
    /* ldh a, [hActiveEntityType]; cp ENTITY_GIBDO; jr nz, gibdoEnd */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_GIBDO) {
        /* ... replace it by a Stalfos. */
        /* ld hl, wEntitiesTypeTable; add hl, bc; ld [hl], ENTITY_STALFOS_EVASIVE */
        gb_write(gb, wEntitiesTypeTable + bc, ENTITY_STALFOS_EVASIVE);
        /* ld hl, wEntitiesStatusTable; add hl, bc; ld [hl], ENTITY_STATUS_ACTIVE */
        gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
        /* jp ConfigureNewEntity.attributes */
        ConfigureNewEntity_attributes(gb, bc);
        return;
    }

    /* gibdoEnd: */
    /* ld hl, wEntitiesPrivateCountdown3Table; add hl, bc; ld [hl], $1F */
    gb_write(gb, wEntitiesPrivateCountdown3Table + bc, 0x1F);

    /* ld hl, wEntitiesStatusTable; add hl, bc; ld [hl], ENTITY_STATUS_DYING */
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_DYING);

    /* ld hl, wEntitiesPhysicsFlagsTable; add hl, bc; ld [hl], 4 */
    gb_write(gb, wEntitiesPhysicsFlagsTable + bc, 4);

    /* ld hl, hNoiseSfx; ld [hl], NOISE_SFX_ENEMY_DESTROYED */
    gb_write_hram(gb, hNoiseSfx, NOISE_SFX_ENEMY_DESTROYED);
}

/* ===== EntityFallHandler (03:4CB6) ===== */
void EntityFallHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hMapId]; cp MAP_COLOR_DUNGEON; jr nz, colorShellEnd */
    if (gb_read_hram(gb, hMapId) == MAP_COLOR_DUNGEON) {
        /* ld hl, wEntitiesTypeTable; add hl, bc; ld a, [hl] */
        uint8_t entity_type = gb_read(gb, wEntitiesTypeTable + bc);

        /* cp ENTITY_COLOR_SHELL_RED; jr z, animateColorShell */
        /* cp ENTITY_COLOR_SHELL_GREEN; jr z, animateColorShell */
        /* cp ENTITY_COLOR_SHELL_BLUE; jr z, animateColorShell */
        if (entity_type == ENTITY_COLOR_SHELL_RED ||
            entity_type == ENTITY_COLOR_SHELL_GREEN ||
            entity_type == ENTITY_COLOR_SHELL_BLUE) {
            /* animateColorShell */
            /* ld hl, wEntitiesStatusTable; add hl, bc; ld a, ENTITY_STATUS_ACTIVE; ld [hl], a */
            gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
            /* ld hl, wEntitiesStateTable; add hl, bc; ld a, $06; ld [hl], a; ret */
            gb_write(gb, wEntitiesStateTable + bc, 0x06);
            return;
        }
    }

    /* call GetEntityTransitionCountdown; jr nz, jr_003_4D07 */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0) {
        /* ld hl, wEntitiesOptions1Table; add hl, bc; ld a, [hl]; and ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL; jr nz, jr_4CEF */
        uint8_t options1 = gb_read(gb, wEntitiesOptions1Table + bc);
        if ((options1 & ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL) == 0) {
            /* ld hl, wD460; ld [hl], $01 */
            gb_write(gb, wD460, 0x01);
        }

        /* ldh a, [hActiveEntityType]; cp ENTITY_WRECKING_BALL; jr nz, jr_4D04 */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_WRECKING_BALL) {
            /* ld a, $16; ld [wWreckingBallRoom], a */
            gb_write(gb, wWreckingBallRoom, 0x16);
            /* ld a, $50; ld [wWreckingBallPosX], a */
            gb_write(gb, wWreckingBallPosX, 0x50);
            /* ld a, $27; ld [wWreckingBallPosY], a */
            gb_write(gb, wWreckingBallPosY, 0x27);
        }

        /* jp UnloadEntityAndReturn */
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* jr_003_4D07: cp $40; jr c, jr_003_4D29 */
    if (countdown >= 0x40) {
        /* ldh a, [hActiveEntityType]; cp ENTITY_OCTOROK; jr z, jr_4D19 */
        /* cp ENTITY_MOBLIN; jr z, jr_4D19 */
        /* cp ENTITY_MOBLIN_SWORD; jr nz, jr_003_4D22 */
        uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);
        if (entity_type == ENTITY_OCTOROK ||
            entity_type == ENTITY_MOBLIN ||
            entity_type == ENTITY_MOBLIN_SWORD) {
            /* jr_4D19: call SetEntityVariantForDirection_03 x3 */
            SetEntityVariantForDirection_03(gb, bc);
            SetEntityVariantForDirection_03(gb, bc);
            SetEntityVariantForDirection_03(gb, bc);
        }

        /* call ExecuteActiveEntityHandler_trampoline */
        ExecuteActiveEntityHandler_trampoline(gb, NULL);

        /* call ReturnIfNonInteractive_03.allowInactiveEntity */
        ReturnIfNonInteractive_03(gb, true);
        return;
    }

    /* jr_003_4D29: rra x4; and $03; ld hl, wEntitiesSpriteVariantTable; add hl, bc; ld [hl], a; ldh [hActiveEntitySpriteVariant], a */
    uint8_t variant = (countdown >> 4) & 0x03;
    gb_write(gb, wEntitiesSpriteVariantTable + bc, variant);
    gb_write_hram(gb, hActiveEntitySpriteVariant, variant);

    /* ld e, a; ld d, b; ld hl, Data_003_4CA4; add hl, de; ldh a, [hActiveEntityVisualPosY]; add [hl]; ldh [hActiveEntityVisualPosY], a */
    uint8_t visual_pos_y = gb_read_hram(gb, hActiveEntityVisualPosY);
    uint8_t offset = Data_003_4CA4[variant];
    gb_write_hram(gb, hActiveEntityVisualPosY, (uint8_t)(visual_pos_y + offset));

    /* ld a, e; cp $03; jr nz, jr_4D51 */
    if (variant == 0x03) {
        /* xor a; ldh [hActiveEntitySpriteVariant], a; ld de, Unknown020SpriteVariants; call RenderActiveEntitySpritesPair */
        gb_write_hram(gb, hActiveEntitySpriteVariant, 0x00);
        RenderActiveEntitySpritesPair(gb, Unknown020SpriteVariants, NULL);
    } else {
        /* jr_4D51: ld de, Data_003_4CAC; call RenderActiveEntitySprite */
        RenderActiveEntitySprite(gb, Data_003_4CAC, NULL);
    }

    /* jr_003_4D57: call ReturnIfNonInteractive_03.allowInactiveEntity */
    if (ReturnIfNonInteractive_03(gb, true)) {
        return;
    }

    /* call GetEntityTransitionCountdown; cp $3F; jr nz, jr_4D66 */
    countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0x3F) {
        /* ld hl, hJingle; ld [hl], JINGLE_ITEM_FALLING */
        gb_write_hram(gb, hJingle, JINGLE_ITEM_FALLING);
    }

    /* jr_4D66: rra x4; and $03; ld e, a; ld d, b; ld hl, Data_003_4CA8; add hl, de; ld e, [hl] */
    variant = (countdown >> 4) & 0x03;
    uint8_t speed = Data_003_4CA8[variant];

    /* ldh a, [hLinkPositionX]; push af */
    uint8_t saved_link_x = gb_read_hram(gb, hLinkPositionX);
    /* ld hl, wEntitiesFallingTargetXTable; add hl, bc; ld a, [hl]; ldh [hLinkPositionX], a */
    gb_write_hram(gb, hLinkPositionX, gb_read(gb, wEntitiesFallingTargetXTable + bc));

    /* ldh a, [hLinkPositionY]; push af */
    uint8_t saved_link_y = gb_read_hram(gb, hLinkPositionY);
    /* ld hl, wEntitiesFallingTargetYTable; add hl, bc; ld a, [hl]; ldh [hLinkPositionY], a */
    gb_write_hram(gb, hLinkPositionY, gb_read(gb, wEntitiesFallingTargetYTable + bc));

    /* ld a, e; call ApplyVectorTowardsLink */
    ApplyVectorTowardsLink_with_length(gb, bc, speed);

    /* pop af; ldh [hLinkPositionY], a; pop af; ldh [hLinkPositionX], a */
    gb_write_hram(gb, hLinkPositionY, saved_link_y);
    gb_write_hram(gb, hLinkPositionX, saved_link_x);

    /* jp UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);
}

/* ===== EntityThrownHandler (03:4D94) ===== */
void EntityThrownHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call ExecuteActiveEntityHandler_trampoline */
    ExecuteActiveEntityHandler_trampoline(gb, NULL);

    /* call ReturnIfNonInteractive_03.allowInactiveEntity */
    if (ReturnIfNonInteractive_03(gb, true)) {
        return;
    }

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld [hl], $02 */
    gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x02);

    /* call BouncingEntityPhysics */
    BouncingEntityPhysics(gb, bc);

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0);

    /* call BombEntityHandler.BounceOffWalls */
    BombBounceOffWalls(gb, bc);

    /* call EntityCheckThrowAtTriggers */
    EntityCheckThrowAtTriggers(gb, bc);

    /* ldh a, [hActiveEntityType]; cp ENTITY_GENIE; jr nz, .genieEnd */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_GENIE) {
        /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and a; jr z, .genieEnd */
        uint8_t collisions = gb_read(gb, wEntitiesCollisionsTable + bc);
        if (collisions != 0) {
            /* ld hl, wEntitiesFlashCountdownTable; add hl, bc; ld [hl], $20 */
            gb_write(gb, wEntitiesFlashCountdownTable + bc, 0x20);
            /* ld hl, hWaveSfx; ld [hl], WAVE_SFX_BOSS_HURT */
            gb_write_hram(gb, hWaveSfx, WAVE_SFX_BOSS_HURT);
            /* ld hl, wEntitiesPrivateState4Table; add hl, bc; ld a, [hl]; inc a; ld [hl], a; cp $03; jr z, .genie2 */
            uint8_t private_state4 = gb_read(gb, wEntitiesPrivateState4Table + bc);
            private_state4++;
            gb_write(gb, wEntitiesPrivateState4Table + bc, private_state4);
            if (private_state4 == 0x03) {
                goto genie2;
            }
        }
    }

    /* ld a, DAMAGE_TYPE_THROW_AT; ld [wAttackDamageType], a */
    gb_write(gb, wAttackDamageType, DAMAGE_TYPE_THROW_AT);

    /* call func_003_75A2 */
    func_003_75A2(gb, bc);

    /* ld hl, wEntitiesSpeedXTable; add hl, bc; ld a, [hl] */
    /* ld hl, wEntitiesSpeedYTable; add hl, bc; or [hl]; jr nz, .return */
    uint8_t speed_x = gb_read(gb, wEntitiesSpeedXTable + bc);
    uint8_t speed_y = gb_read(gb, wEntitiesSpeedYTable + bc);
    if ((speed_x | speed_y) != 0) {
        return;
    }

    /* call EntityBecomeStunned */
    EntityBecomeStunned(gb, bc);

    /* ldh a, [hActiveEntityType]; cp ENTITY_GENIE; jr nz, .return */
    if (gb_read_hram(gb, hActiveEntityType) != ENTITY_GENIE) {
        return;
    }

genie2:
    /* ld hl, wEntitiesStatusTable; add hl, bc; ld [hl], $05 */
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    /* call IncrementEntityState; ld [hl], $01 */
    IncrementEntityState(gb, bc);
    gb_write(gb, wEntitiesStateTable + bc, 0x01);
    /* call GetEntityTransitionCountdown; ld [hl], $80 */
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x80);
    /* ld hl, wEntitiesPrivateState3Table; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesPrivateState3Table + bc, 0);
}

/* Helper for stun countdown and horizontal shaking (03:4E72) */
static void EntityStunnedCountdownHandler(GBState *gb, uint16_t bc) {
    /* ld hl, wEntitiesPrivateCountdown2Table; add hl, bc; ld a, [hl]; and a; jr nz, .jr_4E85 */
    uint8_t countdown2 = gb_read(gb, wEntitiesPrivateCountdown2Table + bc);
    if (countdown2 == 0) {
        /* ld hl, wEntitiesStatusTable; add hl, bc; ld [hl], $05 */
        gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
        /* ld hl, wEntitiesSpeedZTable; add hl, bc; ld [hl], b */
        gb_write(gb, wEntitiesSpeedZTable + bc, 0);
    }

    /* .jr_4E85: cp $38; ret nc */
    if (countdown2 >= 0x38) {
        return;
    }

    /* srl a; srl a; and $01; ld e, a; ld d, b; ld hl, Data_003_4E05; add hl, de; ld a, [hl] */
    uint8_t variant = (countdown2 >> 2) & 0x01;
    uint8_t speed = Data_003_4E05[variant];

    /* ld hl, wEntitiesSpeedXTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesSpeedXTable + bc, speed);

    /* call AddEntitySpeedToPos_03 */
    AddEntitySpeedToPos_03(gb, bc);

    /* jp ClearEntitySpeed */
    ClearEntitySpeed(gb, bc);
}

/* ===== EntityStunnedHandler (03:4E07) ===== */
void EntityStunnedHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call ExecuteActiveEntityHandler_trampoline */
    ExecuteActiveEntityHandler_trampoline(gb, NULL);

    /* call ReturnIfNonInteractive_03.allowInactiveEntity */
    if (ReturnIfNonInteractive_03(gb, true)) {
        return;
    }

    /* call ApplyRecoilIfNeeded_03 */
    ApplyRecoilIfNeeded_03(gb, bc);

    /* call BouncingEntityPhysics */
    BouncingEntityPhysics(gb, bc);

    /* call ClearEntitySpeed */
    ClearEntitySpeed(gb, bc);

    /* call func_003_6E2B */
    func_003_6E2B(gb, bc);

    /* ld a, [wInventoryItems.BButtonSlot]; cp INVENTORY_POWER_BRACELET; jr nz, .noBraceletB */
    uint8_t b_button = gb_read(gb, wInventoryItems_BButtonSlot);
    if (b_button == INVENTORY_POWER_BRACELET) {
        /* ldh a, [hJoypadState]; and J_B; jr nz, EntityGetLiftedUp */
        if (gb_read_hram(gb, hJoypadState) & J_B) {
            EntityGetLiftedUp(gb, bc);
            return;
        }
        EntityStunnedCountdownHandler(gb, bc);
        return;
    }

    /* .noBraceletB */
    /* ld a, [wInventoryItems.AButtonSlot]; cp INVENTORY_POWER_BRACELET; jr nz, jr_003_4E72 */
    uint8_t a_button = gb_read(gb, wInventoryItems_AButtonSlot);
    if (a_button == INVENTORY_POWER_BRACELET) {
        /* ldh a, [hJoypadState]; and J_A; jr z, jr_003_4E72 */
        if (gb_read_hram(gb, hJoypadState) & J_A) {
            EntityGetLiftedUp(gb, bc);
            return;
        }
    }

    EntityStunnedCountdownHandler(gb, bc);
}

/* ===== EntityGetLiftedUp (03:4E35) ===== */
void EntityGetLiftedUp(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wC3CF]; and a; jr nz, jr_003_4E72 */
    if (gb_read(gb, wC3CF) != 0) {
        EntityStunnedCountdownHandler(gb, bc);
        return;
    }

    /* ld hl, wEntitiesPhysicsFlagsTable; add hl, bc; ld a, [hl]; push hl; push af */
    uint8_t physics = gb_read(gb, wEntitiesPhysicsFlagsTable + bc);

    /* or ENTITY_PHYSICS_HARMLESS; ld [hl], a */
    gb_write(gb, wEntitiesPhysicsFlagsTable + bc, physics | ENTITY_PHYSICS_HARMLESS);

    /* call CheckLinkCollisionWithEnemy */
    bool collision = CheckLinkCollisionWithEnemy(gb, bc);

    /* rl e; pop af; pop hl; ld [hl], a; rr e; jr nc, jr_003_4E72 */
    gb_write(gb, wEntitiesPhysicsFlagsTable + bc, physics);
    if (!collision) {
        EntityStunnedCountdownHandler(gb, bc);
        return;
    }

    /* ld a, $01; ld [wC3CF], a */
    gb_write(gb, wC3CF, 0x01);

    /* ld hl, wEntitiesStatusTable; add hl, bc; ld [hl], $07 */
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_LIFTED);

    /* ld a, WAVE_SFX_LIFT_UP; ldh [hWaveSfx], a */
    gb_write_hram(gb, hWaveSfx, WAVE_SFX_LIFT_UP);

    /* ld hl, wEntitiesLiftedTable; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesLiftedTable + bc, 0);

    /* call GetEntityTransitionCountdown; ld [hl], $02 */
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x02);

    /* ldh a, [hLinkDirection]; ld [wC15D], a */
    uint8_t link_dir = gb_read_hram(gb, hLinkDirection);
    gb_write(gb, wC15D, link_dir);

    /* jp EntityLiftedHandler */
    EntityLiftedHandler(gb, bc);
}

/* ===== EntityLiftedHandler (03:5732) ===== */
void EntityLiftedHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hActiveEntityType]; ld [wLiftedEntityType], a */
    uint8_t active_type = gb_read_hram(gb, hActiveEntityType);
    gb_write(gb, wLiftedEntityType, active_type);

    /* cp ENTITY_BOMB; jr nz, .jr_5745 */
    if (active_type == ENTITY_BOMB) {
        /* ld hl, wEntitiesFlashCountdownTable; add hl, bc; ld [hl], b */
        gb_write(gb, wEntitiesFlashCountdownTable + bc, 0);
        /* call RenderBomb; jr jr_003_5748 */
        RenderBomb(gb, bc);
    } else {
        /* .jr_5745: call ExecuteActiveEntityHandler_trampoline */
        ExecuteActiveEntityHandler_trampoline(gb, NULL);
    }

    /* jr_003_5748: ld hl, wEntitiesLiftedTable; add hl, bc; ld a, [hl]; ld e, a; ld d, b */
    uint8_t lifted_val = gb_read(gb, wEntitiesLiftedTable + bc);
    uint8_t e = lifted_val;

    /* cp $04; jr z, jr_003_5789 */
    if (lifted_val != 0x04) {
        /* ld a, [wC15D]; ldh [hLinkDirection], a */
        gb_write_hram(gb, hLinkDirection, gb_read(gb, wC15D));

        /* push hl; call GetEntityTransitionCountdown; pop hl; and a; jr nz, jr_003_5789 */
        uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
        if (countdown == 0) {
            /* inc [hl] */
            lifted_val++;
            gb_write(gb, wEntitiesLiftedTable + bc, lifted_val);

            /* ld hl, Data_003_56EA */
            const uint8_t *table = Data_003_56EA;

            /* ldh a, [hActiveEntityType]; cp ENTITY_BOMB; jr z, .jr_577F */
            /* ld a, [wPowerBraceletLevel]; cp $02; jr nc, .jr_577F */
            /* ld a, [wTunicType]; and TUNIC_RED; jr nz, .jr_577F */
            /* ld a, [wActivePowerUp]; cp ACTIVE_POWER_UP_PIECE_OF_POWER; jr nz, jr_003_5782 */
            if (active_type == ENTITY_BOMB ||
                gb_read(gb, wPowerBraceletLevel) >= 0x02 ||
                (gb_read(gb, wTunicType) & TUNIC_RED) != 0 ||
                gb_read(gb, wActivePowerUp) == ACTIVE_POWER_UP_PIECE_OF_POWER) {
                /* .jr_577F: ld hl, Data_003_56EE */
                table = Data_003_56EE;
            }

            /* jr_003_5782: add hl, de; ld a, [hl]; ld hl, wEntitiesTransitionCountdownTable; add hl, bc; ld [hl], a */
            gb_write(gb, wEntitiesTransitionCountdownTable + bc, table[e]);
        }
    }

    /* jr_003_5789: ld a, e; cp $00; jr nz, .jr_578F; inc e */
    if (e == 0) {
        e++;
    }

    /* .jr_578F: call func_003_5795 */
    func_003_5795(gb, bc, e);

    /* jp label_003_57E6 -> jp label_397B */
    label_397B(gb, NULL);
}

/* ===== func_003_5795 (03:5795) ===== */
void func_003_5795(GBState *gb, uint16_t bc, uint8_t e) {
    if (!gb) return;

    /* ldh a, [hLinkDirection]; sla a; sla a; add e; ld e, a; ld d, $00 */
    uint8_t dir = gb_read_hram(gb, hLinkDirection);
    uint8_t index = (uint8_t)((dir << 2) + e);

    /* ld hl, Data_003_56F1; add hl, de; ld a, [hl]; ld [wIsCarryingLiftedObject], a */
    gb_write(gb, wIsCarryingLiftedObject, Data_003_56F1[index]);

    /* ld hl, Data_003_5701; add hl, de; ld a, [hl]; ld hl, hLinkPositionX; add [hl]; ld hl, wEntitiesPosXTable; add hl, bc; ld [hl], a */
    uint8_t link_x = gb_read_hram(gb, hLinkPositionX);
    gb_write(gb, wEntitiesPosXTable + bc, (uint8_t)(link_x + Data_003_5701[index]));

    /* ld hl, Data_003_5711; add hl, de; ld a, [hl]; ld hl, hLinkPositionY; add [hl]; ld hl, wC13B; add [hl]; ld hl, wEntitiesPosYTable; add hl, bc; ld [hl], a */
    uint8_t link_y = gb_read_hram(gb, hLinkPositionY);
    uint8_t c13b = gb_read(gb, wC13B);
    uint8_t entity_y = (uint8_t)(link_y + Data_003_5711[index] + c13b);
    gb_write(gb, wEntitiesPosYTable + bc, entity_y);

    /* ldh a, [hIsSideScrolling]; and a; jr z, .jr_57D7 */
    if (gb_read_hram(gb, hIsSideScrolling) != 0) {
        /* push hl; ld hl, Data_003_5721; add hl, de; ld e, [hl]; pop hl; ld a, [hl]; sub e; ld [hl], a; ret */
        uint8_t z_offset = Data_003_5721[index];
        gb_write(gb, wEntitiesPosYTable + bc, (uint8_t)(entity_y - z_offset));
    } else {
        /* .jr_57D7: ld hl, Data_003_5721; add hl, de; ld a, [hl]; ld hl, hLinkPositionZ; add [hl]; ld hl, wEntitiesPosZTable; add hl, bc; ld [hl], a; ret */
        uint8_t link_z = gb_read_hram(gb, hLinkPositionZ);
        uint8_t z_offset = Data_003_5721[index];
        gb_write(gb, wEntitiesPosZTable + bc, (uint8_t)(link_z + z_offset));
    }
}

/* ===== EntityBecomeStunned (03:7267) ===== */
void EntityBecomeStunned(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesStatusTable; add hl, bc; ld [hl], ENTITY_STATUS_STUNNED */
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_STUNNED);

    /* ld hl, wEntitiesPrivateCountdown2Table; add hl, bc; ld [hl], $FF */
    gb_write(gb, wEntitiesPrivateCountdown2Table + bc, 0xFF);

    /* ld hl, wEntitiesSpeedZTable; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesSpeedZTable + bc, 0);
}

/* ===== EntityDeathHandler (03:5518) ===== */
void EntityDeathHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesOptions1Table; add hl, bc; ld a, [hl]; and ENTITY_OPT1_IS_BOSS */
    /* jr z, .dying; jp ExecuteActiveEntityHandler */
    uint8_t options1 = gb_read(gb, (uint16_t)(wEntitiesOptions1Table + bc));
    if ((options1 & ENTITY_OPT1_IS_BOSS) != 0) {
        ExecuteActiveEntityHandler(gb, NULL);
        return;
    }

    /* .dying: ld hl, wEntitiesPrivateCountdown3Table; add hl, bc; ld a, [hl]; and a */
    /* jp z, DidKillEnemy */
    uint8_t countdown3 = gb_read(gb, (uint16_t)(wEntitiesPrivateCountdown3Table + bc));
    if (countdown3 == 0) {
        DidKillEnemy(gb, bc, SpawnEnemyDrop);
        return;
    }

    /* hl = (wEntitiesPowerRecoilingTable[bc] != 0 ? Data_003_54C8 : Data_003_5488) */
    uint8_t power_recoiling = gb_read(gb, (uint16_t)(wEntitiesPowerRecoilingTable + bc));
    const uint8_t *data_table = (power_recoiling != 0) ? Data_003_54C8 : Data_003_5488;

    /* cp $20; jr nc, jr_003_556F */
    if (countdown3 < 0x20) {
        /* rla; and $30; ld e, a */
        uint8_t e = (uint8_t)((countdown3 << 1) & 0x30);
        const uint8_t *sprite_data = data_table + e;

        /* cp $30; jr nz, .jr_003_5555 */
        if (e == 0x30 && power_recoiling != 0) {
            /* .powerRecoil: ld c, $08; call RenderActiveEntitySpritesRect; ld a, $04; call func_015_7964_trampoline */
            RenderActiveEntitySpritesRect(gb, sprite_data, 8, NULL);
            func_015_7964_trampoline(gb, NULL);
        } else {
            /* .jr_003_5555: ld c, $04; call RenderActiveEntitySpritesRect */
            RenderActiveEntitySpritesRect(gb, sprite_data, 4, NULL);
        }

        /* .renderEnd: call ReturnIfNonInteractive_03; call ApplyRecoilIfNeeded_03; ret */
        if (ReturnIfNonInteractive_03(gb, false)) {
            return;
        }
        ApplyRecoilIfNeeded_03(gb, bc);
        return;
    }

    /* jr_003_556F: */
    /* call ExecuteActiveEntityHandler_trampoline */
    ExecuteActiveEntityHandler_trampoline(gb, NULL);

    /* call ReturnIfNonInteractive_03.allowInactiveEntity */
    if (ReturnIfNonInteractive_03(gb, true)) {
        return;
    }

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld a, [hl]; and a; jr nz, jr_003_5599 */
    if (gb_read(gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + bc)) == 0) {
        /* ld hl, wEntitiesPrivateCountdown3Table; add hl, bc; ld [hl], $1F */
        gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown3Table + bc), 0x1F);

        /* ld a, [wTunicType]; and a; jr nz, .jr_5594 */
        /* ld a, [wActivePowerUp]; cp ACTIVE_POWER_UP_PIECE_OF_POWER; jr nz, .jr_5594 */
        /* ld a, WAVE_SFX_UNKNOWN_12; ldh [hWaveSfx], a */
        if (gb_read(gb, wTunicType) == 0 && gb_read(gb, wActivePowerUp) == ACTIVE_POWER_UP_PIECE_OF_POWER) {
            gb_write_hram(gb, hWaveSfx, WAVE_SFX_UNKNOWN_12);
        }

        /* .jr_5594: ld hl, hNoiseSfx; ld [hl], NOISE_SFX_ENEMY_DESTROYED */
        gb_write_hram(gb, hNoiseSfx, NOISE_SFX_ENEMY_DESTROYED);
    }

    /* jr_003_5599: call ApplyRecoilIfNeeded_03; ret */
    ApplyRecoilIfNeeded_03(gb, bc);
}

/* ===== SpawnEnemyDrop (03:55CF) ===== */
void SpawnEnemyDrop(GBState *gb, uint16_t bc) {
    if (!gb) return;

    uint8_t drop_entity = 0;

    /* ldh a, [hActiveEntityType]; cp ENTITY_LIKE_LIKE; jr nz, .likeLikeEnd */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_LIKE_LIKE) {
        /* ld hl, wEntitiesPrivateState1Table; add hl, bc; ld a, [hl]; and a; jr z, .likeLikeEnd */
        if (gb_read(gb, (uint16_t)(wEntitiesPrivateState1Table + bc)) != 0) {
            /* ld a, ENTITY_SWORD_SHIELD_PICKUP; jp .dropEntity */
            drop_entity = ENTITY_SWORD_SHIELD_PICKUP;
            goto dropEntity;
        }
    }

    /* .likeLikeEnd: */
    /* ld hl, wEntitiesDroppedItemTable; add hl, bc; ld a, [hl]; cp ENTITY_NONE; ret z */
    uint8_t dropped_item = gb_read(gb, (uint16_t)(wEntitiesDroppedItemTable + bc));
    if (dropped_item == ENTITY_NONE) {
        return;
    }
    /* and a; jp nz, .dropEntity */
    if (dropped_item != DROP_RANDOM) {
        drop_entity = dropped_item;
        goto dropEntity;
    }

    /* check if wGuardianAcornCounter reached limit */
    /* ld a, [wGuardianAcornCounter]; inc a; ld [wGuardianAcornCounter], a */
    uint8_t acorn_counter = (uint8_t)(gb_read(gb, wGuardianAcornCounter) + 1);
    gb_write(gb, wGuardianAcornCounter, acorn_counter);
    /* cp GUARDIAN_ACORN_COUNTER_MAX; jr c, .noGuardianAcornDrop */
    if (acorn_counter >= GUARDIAN_ACORN_COUNTER_MAX) {
        /* xor a; ld [wGuardianAcornCounter], a */
        gb_write(gb, wGuardianAcornCounter, 0);

        /* ld a, [wInBossBattle]; ld hl, wActivePowerUp; or [hl]; ld hl, hIsSideScrolling; or [hl]; jr nz, .noGuardianAcornDrop */
        uint8_t blocked = (uint8_t)(gb_read(gb, wInBossBattle) |
                                    gb_read(gb, wActivePowerUp) |
                                    gb_read_hram(gb, hIsSideScrolling));
        if (blocked == 0) {
            /* ld a, ENTITY_GUARDIAN_ACORN; jp .dropEntity */
            drop_entity = ENTITY_GUARDIAN_ACORN;
            goto dropEntity;
        }
    }

    /* .noGuardianAcornDrop: */
    /* ld hl, wEntitiesHealthGroup; add hl, bc; ld e, [hl]; ld d, b */
    /* ld hl, DestroyedEntityHealthGroupOffsetTable; add hl, de; ld a, [hl]; and a; ret z */
    uint8_t health_group = gb_read(gb, (uint16_t)(wEntitiesHealthGroup + bc));
    if (health_group >= sizeof(DestroyedEntityHealthGroupOffsetTable)) {
        return;
    }
    uint8_t group_offset = DestroyedEntityHealthGroupOffsetTable[health_group];
    if (group_offset == 0) {
        return;
    }

    /* ld e, a */
    /* How many enemies to kill before a Piece of Power drops? */
    uint8_t pop_threshold = PIECE_OF_POWER_COUNTER_MAX_LOW_MAX_HEALTH;
    uint8_t max_hearts = gb_read(gb, wMaxHearts);
    if (max_hearts >= LOW_MAX_HEALTH) {
        if (max_hearts < MEDIUM_MAX_HEALTH) {
            pop_threshold = PIECE_OF_POWER_COUNTER_MAX_MEDIUM_MAX_HEALTH;
        } else {
            pop_threshold = PIECE_OF_POWER_COUNTER_MAX_HIGH_MAX_HEALTH;
        }
    }

    /* .pieceOfPowerDrop: */
    /* ld hl, wPieceOfPowerKillCount; inc [hl]; ld a, [hl]; cp d; jr c, .noPieceOfPowerDrop */
    uint8_t pop_count = (uint8_t)(gb_read(gb, wPieceOfPowerKillCount) + 1);
    gb_write(gb, wPieceOfPowerKillCount, pop_count);
    if (pop_count >= pop_threshold) {
        /* ld [hl], b */
        gb_write(gb, wPieceOfPowerKillCount, 0);

        /* ld a, [wInBossBattle]; ld hl, hIsSideScrolling; or [hl]; ld hl, wActivePowerUp; or [hl]; jr nz, .noPieceOfPowerDrop */
        uint8_t blocked = (uint8_t)(gb_read(gb, wInBossBattle) |
                                    gb_read_hram(gb, hIsSideScrolling) |
                                    gb_read(gb, wActivePowerUp));
        if (blocked == 0) {
            /* ld a, ENTITY_PIECE_OF_POWER; jr .dropEntity */
            drop_entity = ENTITY_PIECE_OF_POWER;
            goto dropEntity;
        }
    }

    /* .noPieceOfPowerDrop: */
    /* ld hl, (RandomDropChanceTable - 1); ld a, [wIsOnLowHeath]; and a; jr z, .dropRandomEntity */
    /* ld hl, (RandomDropChanceTableLowHealth - 1) */
    const uint8_t *chance_table = (gb_read(gb, wIsOnLowHeath) != 0) ?
                                  RandomDropChanceTableLowHealth :
                                  RandomDropChanceTable;

    /* .dropRandomEntity: add hl, de */
    /* call GetRandomByte; and [hl]; ret nz */
    uint8_t chance_mask = chance_table[group_offset - 1];
    if ((GetRandomByte(gb) & chance_mask) != 0) {
        return;
    }

    /* ld hl, (DropTableByIndex - 1); add hl, de; ld a, [hl] */
    drop_entity = DropTableByIndex[group_offset - 1];
    /* cp ENTITY_NONE; jr nz, .dropEntity */
    if (drop_entity == ENTITY_NONE) {
        /* call GetRandomByte; and %00000111; ld e, a; ld hl, DropTableRandom; add hl, de; ld a, [hl] */
        uint8_t rand_idx = (uint8_t)(GetRandomByte(gb) & 0x07);
        drop_entity = DropTableRandom[rand_idx];
    }

dropEntity:
    /* .dropEntity: call SpawnNewEntity; ret c */
    {
        uint16_t de = SpawnNewEntity_slot(gb, drop_entity);
        if (de == 0xFFFF) {
            return;
        }

        /* ld hl, wEntitiesPrivateState1Table; add hl, bc; ld a, [hl] */
        /* ld hl, wEntitiesPrivateState1Table; add hl, de; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesPrivateState1Table + de),
                 gb_read(gb, (uint16_t)(wEntitiesPrivateState1Table + bc)));

        /* ldh a, [hMultiPurpose0]; ld hl, wEntitiesPosXTable; add hl, de; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesPosXTable + de), gb_read_hram(gb, hMultiPurpose0));

        /* ldh a, [hMultiPurpose1]; ld hl, wEntitiesPosYTable; add hl, de; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesPosYTable + de), gb_read_hram(gb, hMultiPurpose1));

        /* ld hl, wEntitiesSlowTransitionCountdownTable; add hl, de; ld [hl], DROP_DESPAWN_TIME */
        gb_write(gb, (uint16_t)(wEntitiesSlowTransitionCountdownTable + de), DROP_DESPAWN_TIME);

        /* ld hl, wEntitiesPrivateCountdown1Table; add hl, de; ld [hl], DROP_COUNTDOWN_TIME */
        gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown1Table + de), DROP_COUNTDOWN_TIME);

        /* ld hl, wEntitiesPrivateCountdown3Table; add hl, de; ld [hl], $03 */
        gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown3Table + de), 0x03);

        /* ldh a, [hIsSideScrolling]; and a; jr nz, .isSideScrolling */
        if (gb_read_hram(gb, hIsSideScrolling) != 0) {
            /* .isSideScrolling: ld hl, wEntitiesSpeedYTable; add hl, de; ld [hl], $EC */
            gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + de), 0xEC);
        } else {
            /* ld hl, wEntitiesTypeTable; add hl, de; ld a, [hl] */
            uint8_t spawned_type = gb_read(gb, (uint16_t)(wEntitiesTypeTable + de));
            if (spawned_type == ENTITY_KEY_DROP_POINT) {
                /* ldh a, [hActiveEntityType]; cp ENTITY_ARMOS_KNIGHT; jr nz, .noSpriteUpdate */
                if (gb_read_hram(gb, hActiveEntityType) == ENTITY_ARMOS_KNIGHT) {
                    /* ld hl, wEntitiesSpriteVariantTable; add hl, de; ld [hl], $03 */
                    gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + de), 0x03);
                }
            } else if (spawned_type == ENTITY_HIDING_SLIME_KEY) {
                /* .noSpriteUpdate: cp ENTITY_HIDING_SLIME_KEY; jr nz, .slimeKeyEnd */
                /* ldh a, [hMapRoom]; cp ROOM_OW_KANALET_CASTLE_CROW; jr z, .moveKeyTowardsLink */
                /* cp ROOM_OW_KANALET_CASTLE_FIVE_PITS; jr nz, .slimeKeyEnd */
                uint8_t map_room = gb_read_hram(gb, hMapRoom);
                if (map_room == ROOM_OW_KANALET_CASTLE_CROW || map_room == ROOM_OW_KANALET_CASTLE_FIVE_PITS) {
                    /* .moveKeyTowardsLink: push bc; push de; ld c, e; ld b, d; ld a, $10; call ApplyVectorTowardsLink; pop de; pop bc */
                    ApplyVectorTowardsLink_with_length(gb, de, 0x10);
                }
            }

            /* .slimeKeyEnd: ld hl, wEntitiesSpeedZTable; add hl, de; ld [hl], $18 */
            gb_write(gb, (uint16_t)(wEntitiesSpeedZTable + de), 0x18);
        }

        /* .applyDefaultPosZ: */
        /* ld hl, wEntitiesPosZTable; add hl, bc; ld a, [hl] */
        /* ld hl, wEntitiesPosZTable; add hl, de; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesPosZTable + de),
                 gb_read(gb, (uint16_t)(wEntitiesPosZTable + bc)));
    }
}