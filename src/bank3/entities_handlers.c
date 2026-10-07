#include "bank3/entities_handlers.h"
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

/* ===== Data Tables (03:4C44-03:4CAC) ===== */

/* FireSpriteVariants (03:4C44) */
static const uint8_t FireSpriteVariants[8] = {
    0x34, 0x02,  /* variant0: tile $34, palette 2 */
    0x34, 0x42,  /* variant0 flipped: tile $34, palette 2 | xflip */
    0x34, 0x04,  /* variant1: tile $34, palette 4 */
    0x34, 0x44   /* variant1 flipped: tile $34, palette 4 | xflip */
};

/* Unknown020SpriteVariants (03:4CB2) */
static const uint8_t Unknown020SpriteVariants[4] = {
    0x1E, 0x01, 0x1E, 0x61
};

/* Data_003_4CA4 (03:4CA4) */
static const uint8_t Data_003_4CA4[4] = { 0x00, 0x00, 0x04, 0x00 };

/* Data_003_4CAC (03:4CAC) */
static const uint8_t Data_003_4CAC[6] = { 0x24, 0x01, 0x24, 0x01, 0x3E, 0x01 };

/* Data_003_4CA8 (03:4CA8) */
static const uint8_t Data_003_4CA8[4] = { 0x00, 0x01, 0x03, 0x06 };

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
        /* Note: ConfigureNewEntity.attributes is called via ConfigureEntityHitbox */
        ConfigureEntityHitbox(gb, bc);
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
    if (gb_read_hram(gb, hMapId) != MAP_COLOR_DUNGEON) {
        goto colorShellEnd;
    }

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

colorShellEnd: ;
    /* call GetEntityTransitionCountdown; jr nz, jr_003_4D07 */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown != 0) {
        goto jr_003_4D07;
    }

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

jr_003_4D07:
    /* cp $40; jr c, jr_003_4D29 */
    if (countdown < 0x40) {
        goto jr_003_4D29;
    }

    /* ldh a, [hActiveEntityType]; cp ENTITY_OCTOROK; jr z, jr_4D19 */
    /* cp ENTITY_MOBLIN; jr z, jr_4D19 */
    /* cp ENTITY_MOBLIN_SWORD; jr nz, jr_003_4D22 */
    entity_type = gb_read_hram(gb, hActiveEntityType);
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
    if (ReturnIfNonInteractive_03(gb, true)) {
        return;
    }
    return;

jr_003_4D29: ;
    /* rra x4; and $03; ld hl, wEntitiesSpriteVariantTable; add hl, bc; ld [hl], a; ldh [hActiveEntitySpriteVariant], a */
    uint8_t variant = (countdown >> 4) & 0x03;
    gb_write(gb, wEntitiesSpriteVariantTable + bc, variant);
    gb_write_hram(gb, hActiveEntitySpriteVariant, variant);

    /* ld e, a; ld d, b; ld hl, Data_003_4CA4; add hl, de; ldh a, [hActiveEntityVisualPosY]; add [hl]; ldh [hActiveEntityVisualPosY], a */
    uint8_t visual_pos_y = gb_read_hram(gb, hActiveEntityVisualPosY);
    uint8_t offset = Data_003_4CA4[variant];
    gb_write_hram(gb, hActiveEntityVisualPosY, (uint8_t)(visual_pos_y + offset));

    /* ld a, e; cp $03; jr nz, jr_4D51 */
    if (variant != 0x03) {
        /* jr_4D51: ld de, Data_003_4CAC; call RenderActiveEntitySprite */
        RenderActiveEntitySprite(gb, Data_003_4CAC, NULL);
    } else {
        /* xor a; ldh [hActiveEntitySpriteVariant], a; ld de, Unknown020SpriteVariants; call RenderActiveEntitySpritesPair */
        gb_write_hram(gb, hActiveEntitySpriteVariant, 0x00);
        RenderActiveEntitySpritesPair(gb, Unknown020SpriteVariants, NULL);
    }

    /* jr jr_003_4D57 */
    /* jr_003_4D57: call ReturnIfNonInteractive_03.allowInactiveEntity */
    ;
    if (ReturnIfNonInteractive_03(gb, true)) {
        return;
    }

    /* call GetEntityTransitionCountdown; cp $3F; jr nz, jr_4D66 */
    countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0x3F) {
        /* ld hl, hJingle; ld [hl], JINGLE_ITEM_FALLING */
        gb_write_hram(gb, hJingle, JINGLE_ITEM_FALLING);
    }

    /* rra x4; and $03; ld e, a; ld d, b; ld hl, Data_003_4CA8; add hl, de */
    variant = (countdown >> 4) & 0x03;
    (void)Data_003_4CA8; /* Suppress unused warning - assembly continues with more sprite rendering */
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
                /* .genie2: fall through to set stunned */
                goto genie2;
            }
        }
        /* .genieEnd: */
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
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_GENIE) {
    genie2:
        /* ld hl, wEntitiesStatusTable; add hl, bc; ld [hl], $05 */
        gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
        /* call IncrementEntityState */
        IncrementEntityState(gb, bc);
        /* ld [hl], $01 */
        gb_write(gb, wEntitiesStateTable + bc, 0x01);
        /* call GetEntityTransitionCountdown; ld [hl], $80 */
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x80);
        /* ld hl, wEntitiesPrivateState3Table; add hl, bc; ld [hl], b */
        gb_write(gb, wEntitiesPrivateState3Table + bc, 0);
    }

    /* .return: ret */
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
        uint8_t joypad = gb_read_hram(gb, hJoypadState);
        if (joypad & J_B) {
            EntityGetLiftedUp(gb, bc);
            return;
        }
        goto jr_003_4E72;
    }

    /* .noBraceletB */
    /* ld a, [wInventoryItems.AButtonSlot]; cp INVENTORY_POWER_BRACELET; jr nz, jr_003_4E72 */
    uint8_t a_button = gb_read(gb, wInventoryItems_AButtonSlot);
    if (a_button == INVENTORY_POWER_BRACELET) {
        /* ldh a, [hJoypadState]; and J_A; jr z, jr_003_4E72 */
        uint8_t joypad = gb_read_hram(gb, hJoypadState);
        if (joypad & J_A) {
            /* fallthrough to EntityGetLiftedUp */
            EntityGetLiftedUp(gb, bc);
            return;
        }
    }

jr_003_4E72: ;
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
    static const uint8_t Data_003_4E05[2] = { 0x10, 0xF0 };
    int8_t speed = (int8_t)Data_003_4E05[variant];

    /* ld hl, wEntitiesSpeedXTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesSpeedXTable + bc, (uint8_t)speed);

    /* call AddEntitySpeedToPos_03 */
    AddEntitySpeedToPos_03(gb, bc);

    /* jp ClearEntitySpeed */
    ClearEntitySpeed(gb, bc);
}

/* ===== EntityGetLiftedUp (03:4E35) ===== */
void EntityGetLiftedUp(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wC3CF]; and a; jr nz, jr_003_4E72 */
    if (gb_read(gb, wC3CF) != 0) {
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
    /* Implementation based on bank3.asm:03:5732 */
    /* This is a stub - the full implementation is in bank3.asm at 03:5732 */
    (void)bc;
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

    /* ret */
}