#include "bank3/entities_handlers.h"
#include "bank3/entities_init_core.h"
#include "bank3/entities_bomb.h"
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