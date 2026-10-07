#include "bank3/entities_physics.h"
#include "bank3/entities_collision.h"
#include "bank3/entities_pushed_block.h"
#include "constants/entities.h"
#include "constants/physics.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/gameplay.h"
#include "constants/directions.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include "constants/vfx.h"
#include "constants/audio.h"
#include "home/entities.h"
#include "home/vfx.h"
#include "home/room.h"
#include "home/bank.h"
#include "home/audio.h"
#include "home/gameplay.h"

/* ===== BouncingEntityPhysics (03:60B3) ===== */
void BouncingEntityPhysics(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);
    /* call func_003_6B7B */
    func_003_6B7B(gb, bc);
    /* call ApplyEntityInteractionWithBackground */
    ApplyEntityInteractionWithBackground(gb, bc);

    /* ldh a, [hIsSideScrolling]; and a; jr z, .sidescrollingEnd */
    if (gb_read_hram(gb, hIsSideScrolling) == 0) {
        goto sidescrollingEnd;
    }

    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and $08; jp z, .return */
    if ((gb_read(gb, wEntitiesCollisionsTable + bc) & 0x08) == 0) {
        return;
    }

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld a, [hl]; and $F0; add $05; ld [hl], a */
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);
    pos_y = (pos_y & 0xF0) + 0x05;
    gb_write(gb, wEntitiesPosYTable + bc, pos_y);

    /* ld hl, wEntitiesSpeedYTable; add hl, bc; ld a, [hl]; cpl; sra a; cp $F8; jr c, .makeBouncingNoise */
    int8_t speed_y = (int8_t)gb_read(gb, wEntitiesSpeedYTable + bc);
    speed_y = (int8_t)(~speed_y);  /* cpl */
    speed_y >>= 1;  /* sra */
    if ((uint8_t)speed_y < 0xF8) {
        goto makeBouncingNoise;
    }

    goto shallowWaterEnd;

sidescrollingEnd:
    /* ld hl, wEntitiesPosZTable; add hl, bc; ld a, [hl]; and $80; jr z, .return */
    if ((gb_read(gb, wEntitiesPosZTable + bc) & 0x80) == 0) {
        return;
    }

    /* xor a; ld [hl], a */
    gb_write(gb, wEntitiesPosZTable + bc, 0x00);

    /* ld hl, wEntitiesGroundStatusTable; add hl, bc; ld a, [hl] */
    uint8_t ground_status = gb_read(gb, wEntitiesGroundStatusTable + bc);

    /* ld hl, wEntitiesSpeedZTable; add hl, bc */
    /* cp ENTITY_GROUND_STATUS_SHALLOW_WATER; jr z, .shallowWaterEnd */
    if (ground_status == ENTITY_GROUND_STATUS_SHALLOW_WATER) {
        goto shallowWaterEnd;
    }

    /* ld a, [hl]; sra a; cpl; cp $07; jr nc, .makeBouncingNoise */
    int8_t speed_z = (int8_t)gb_read(gb, wEntitiesSpeedZTable + bc);
    speed_z >>= 1;
    speed_z = (int8_t)(~speed_z);
    if (speed_z >= 7) {
        goto makeBouncingNoise;
    }

shallowWaterEnd:
    /* xor a; push hl; ld hl, wEntitiesSpeedXTable; add hl, bc; ld [hl], a; ld hl, wEntitiesSpeedYTable; add hl, bc; ld [hl], a; pop hl; jr .makeBouncingNoiseEnd */
    gb_write(gb, wEntitiesSpeedXTable + bc, 0x00);
    gb_write(gb, wEntitiesSpeedYTable + bc, 0x00);
    goto makeBouncingNoiseEnd;

makeBouncingNoise:
    /* push af; push hl */
    /* ldh a, [hActiveEntityType]; cp ENTITY_KEY_DROP_POINT; jr nz, .keyEnd */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_KEY_DROP_POINT) {
        /* ld a, NOISE_SFX_CLINK; ldh [hNoiseSfx], a; jr .bombEnd */
        gb_write_hram(gb, hNoiseSfx, NOISE_SFX_CLINK);
        goto bombEnd;
    }

    /* cp ENTITY_BOMB; jr nz, .bombEnd */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BOMB) {
        /* ld hl, wEntitiesStatusTable; add hl, bc; ld a, [hl]; and a; jr z, .bombEnd */
        if (gb_read(gb, wEntitiesStatusTable + bc) == 0x00) {
            goto bombEnd;
        }
        /* cp ENTITY_STATUS_FALLING; jr z, .bombEnd */
        if (gb_read(gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_FALLING) {
            goto bombEnd;
        }
        /* ld a, JINGLE_BUMP; ldh [hJingle], a */
        gb_write_hram(gb, hJingle, JINGLE_BUMP);
    }

bombEnd: ;
    /* pop hl; pop af */
    /* fallthrough to makeBouncingNoiseEnd */

makeBouncingNoiseEnd: ;
    /* ld [hl], a; ld hl, wEntitiesSpeedXTable; add hl, bc; ld a, [hl]; sra a; cp $FF; jr nz, .clearXSpeedEnd; xor a; .clearXSpeedEnd: ld [hl], a */
    int8_t speed_x = (int8_t)gb_read(gb, wEntitiesSpeedXTable + bc);
    speed_x >>= 1;  /* sra */
    if (speed_x == -1) {  /* $FF */
        speed_x = 0;
    }
    gb_write(gb, wEntitiesSpeedXTable + bc, (uint8_t)speed_x);

    /* ldh a, [hIsSideScrolling]; and a; jr nz, .return */
    if (gb_read_hram(gb, hIsSideScrolling) != 0) {
        return;
    }

    /* ld hl, wEntitiesSpeedYTable; add hl, bc; ld a, [hl]; sra a; cp $FF; jr nz, .clearYSpeedEnd; xor a; .clearYSpeedEnd: ld [hl], a; .return: ret */
    int8_t speed_y2 = (int8_t)gb_read(gb, wEntitiesSpeedYTable + bc);
    speed_y2 >>= 1;  /* sra */
    if (speed_y2 == -1) {  /* $FF */
        speed_y2 = 0;
    }
    gb_write(gb, wEntitiesSpeedYTable + bc, (uint8_t)speed_y2);
}

/* ===== func_003_6B7B (03:6B7B) ===== */
void func_003_6B7B(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hIsSideScrolling]; and a; jr nz, .sideScrolling */
    if (gb_read_hram(gb, hIsSideScrolling) != 0) {
        goto sideScrolling;
    }

    /* call AddEntityZSpeedToPos_03 */
    AddEntityZSpeedToPos_03(gb, bc);

    /* ld hl, wEntitiesSpeedZTable; add hl, bc; ld a, [hl]; sub $02; ld [hl], a */
    uint8_t speed_z = gb_read(gb, wEntitiesSpeedZTable + bc);
    speed_z = (uint8_t)(speed_z - 0x02);
    gb_write(gb, wEntitiesSpeedZTable + bc, speed_z);
    return;

sideScrolling: ;
    /* ld hl, wEntitiesGroundStatusTable; add hl, bc; ld a, [hl]; ld e, a; ld d, b; and a; jr z, .updateXSpeedEnd */
    uint8_t ground_status = gb_read(gb, wEntitiesGroundStatusTable + bc);
    if (ground_status == 0) {
        goto updateXSpeedEnd;
    }

    /* ldh a, [hFrameCounter]; and $07; jr nz, .updateXSpeedEnd */
    if ((gb_read_hram(gb, hFrameCounter) & 0x07) != 0) {
        goto updateXSpeedEnd;
    }

    /* ld hl, wEntitiesSpeedXTable; add hl, bc; ld a, [hl]; and a; jr z, .updateXSpeedEnd */
    uint8_t speed_x = gb_read(gb, wEntitiesSpeedXTable + bc);
    if (speed_x == 0) {
        goto updateXSpeedEnd;
    }

    /* and $80; jr z, .positiveDifferenceX */
    if ((speed_x & 0x80) == 0) {
        goto positiveDifferenceX;
    }

    /* get here every 8 frames, if underwater and X speed is not 0 */
    /* inc [hl]; inc [hl] */
    speed_x += 2;
    gb_write(gb, wEntitiesSpeedXTable + bc, speed_x);
    goto updateXSpeedEnd;

positiveDifferenceX: ;
    /* dec [hl] */
    speed_x -= 1;
    gb_write(gb, wEntitiesSpeedXTable + bc, speed_x);

updateXSpeedEnd: ;
    /* ld hl, Data_003_6B73; add hl, de; ld a, [hl] */
    /* ld hl, wEntitiesSpeedYTable; add hl, bc; add [hl]; ld [hl], a */
    static const uint8_t Data_003_6B73[4] = { 0x02, 0x01, 0x02, 0x02 };
    static const uint8_t Data_003_6B77[4] = { 0x40, 0x08, 0x40, 0x40 };
    
    uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);
    if (entity_type < 4) {
        uint8_t y_add = Data_003_6B73[entity_type];
        uint8_t speed_y = gb_read(gb, wEntitiesSpeedYTable + bc);
        speed_y += y_add;
        gb_write(gb, wEntitiesSpeedYTable + bc, speed_y);

        uint8_t y_sub = Data_003_6B77[entity_type];
        if (speed_y >= y_sub) {
            speed_y = y_sub;
            gb_write(gb, wEntitiesSpeedYTable + bc, speed_y);
        }
    }
    return;
}

/* Directional speed tables */
static const uint8_t Data_003_6E0C[2] = { 0x0C, 0xF4 };

/* ===== func_003_6C6B (03:6C6B) ===== */
bool func_003_6C6B(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    /* ldh a, [hFrameCounter]; xor c; rra; jp nc, jr_003_6CCB
       rra rotates bit 0 of (hFrameCounter ^ c) into CF.
       If CF == 0, jp nc jumps to jr_003_6CCB (and a; ret), returning false.
       If CF == 1, jp nc is not taken, returning true. */
    uint8_t frame = gb_read_hram(gb, hFrameCounter);
    uint8_t c = (uint8_t)(bc & 0xFF);
    if (((frame ^ c) & 0x01) == 0) {
        return false;
    }
    return true;
}

/* ===== func_003_6CC0 (03:6CC0) ===== */
bool func_003_6CC0(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    /* ld hl, wEntitiesPhysicsFlagsTable; add hl, bc; ld a, [hl]; and ENTITY_PHYSICS_HARMLESS; jr z, jr_003_6CCD
       jr_003_6CC9: scf; ret */
    if ((gb_read(gb, wEntitiesPhysicsFlagsTable + bc) & ENTITY_PHYSICS_HARMLESS) != 0) {
        return true;  /* Harmless: returns with carry set (skip hurting Link) */
    }

    /* jr_003_6CCD: ldh a, [hLinkAnimationState]; sub $4E; cp $02; jr c, jr_003_6CC9 */
    uint8_t anim = (uint8_t)(gb_read_hram(gb, hLinkAnimationState) - 0x4E);
    if (anim < 0x02) {
        return true;  /* Link animation 0x4E or 0x4F (falling into pit): carry set (skip hurting Link) */
    }
    return false;     /* Not harmless and not falling: proceed to hurt Link */
}

/* ===== label_003_6FA7 (03:6FA7) ===== */
void label_003_6FA7(GBState *gb, uint8_t magnitude) {
    if (!gb) return;

    /* push de; call GetEntityXDistanceToLink_03; ld a, e; and a; pop de; ld a, e; jr z, .jr_6FB3; cpl; inc a; .jr_6FB3: ldh [hLinkSpeedX], a; xor a; ldh [hLinkSpeedY], a; ret */
    uint8_t dir = 0;
    uint8_t dist = 0;
    GetEntityXDistanceToLink_03(gb, &dir, &dist);
    int8_t speed_x = (int8_t)magnitude;
    if (dir != 0) {
        speed_x = (int8_t)(-speed_x);
    }
    gb_write_hram(gb, hLinkSpeedX, (uint8_t)speed_x);
    gb_write_hram(gb, hLinkSpeedY, 0x00);
}

/* ===== func_003_7565 (03:7565) ===== */
void func_003_7565_with_length(GBState *gb, uint8_t length) {
    if (!gb) return;

    GetVectorTowardsLink_with_length(gb, length, NULL, NULL);
    gb_write_hram(gb, hLinkSpeedY, gb_read_hram(gb, hMultiPurpose0));
    gb_write_hram(gb, hLinkSpeedX, gb_read_hram(gb, hMultiPurpose1));
}

void func_003_7565(GBState *gb) {
    func_003_7565_with_length(gb, 0x12);
}

/* ===== func_003_6F93 (03:6F93) ===== */
void func_003_6F93(GBState *gb) {
    if (!gb) return;

    /* ld a, JINGLE_BUMP; ldh [hJingle], a */
    gb_write_hram(gb, hJingle, JINGLE_BUMP);

    /* call ResetPegasusBoots */
    ResetPegasusBoots(gb);

    /* ld a, $0C; ld [wIgnoreLinkCollisionsCountdown], a */
    gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x0C);

    /* ldh a, [hActiveEntityType]; cp ENTITY_ROLLING_BONES_BAR; jr nz, jr_003_6FB9 */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_ROLLING_BONES_BAR) {
        label_003_6FA7(gb, 0x10);
        return;
    }

    /* jr_003_6FB9: ld a, $12; call func_003_7565 */
    func_003_7565_with_length(gb, 0x12);

    /* ld hl, hIndexOfObjectBelowLink; ldh a, [hPressedButtonsMask]; and $0F; ld a, $08; or [hl]; jr z, ConfigureEntityRecoil; ld a, $20; ConfigureEntityRecoil */
    uint16_t bc = gb_read(gb, wActiveEntityIndex);
    uint8_t recoil_amount = 0x20;
    uint8_t obj_below = gb_read_hram(gb, hIndexOfObjectBelowLink);
    uint8_t check = (uint8_t)(0x08 | obj_below);
    if (check == 0) {
        recoil_amount = 0x08;
    }
    ConfigureEntityRecoil(gb, bc, recoil_amount);
}

/* ===== func_003_6F5C (03:6F5C) ===== */
void func_003_6F5C(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call func_003_6F93 */
    func_003_6F93(gb);

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x00);
}

/* ===== func_003_6DDF (03:6DDF) ===== */
void func_003_6DDF(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call ResetPegasusBoots */
    ResetPegasusBoots(gb);

    /* ld a, $10; ld [wIgnoreLinkCollisionsCountdown], a */
    gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x10);

    /* ldh a, [hActiveEntityType]; ld e, $18; cp ENTITY_ROLLING_BONES_BAR; jp z, label_003_6FA7 */
    uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);
    if (entity_type == ENTITY_ROLLING_BONES_BAR) {
        label_003_6FA7(gb, 0x18);
        return;
    }

    /* cp ENTITY_FACADE; jr nz, .facadeEnd */
    if (entity_type == ENTITY_FACADE) {
        /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld [hl], $01 */
        gb_write(gb, wEntitiesCollisionsTable + bc, 0x01);
    }

    /* cp ENTITY_MOLDORM; ld a, $14; jr nz, .moldormEnd; ld a, $18; .moldormEnd: call func_003_7565 */
    uint8_t length = (entity_type == ENTITY_MOLDORM) ? 0x18 : 0x14;
    func_003_7565_with_length(gb, length);

    /* ldh a, [hIsSideScrolling]; and a; jr nz, jr_003_6E0E */
    if (gb_read_hram(gb, hIsSideScrolling) == 0) {
        return;
    }

    /* jr_003_6E0E: ldh a, [hLinkPhysicsModifier]; cp $02; jr z, setCarryAndReturn */
    if (gb_read_hram(gb, hLinkPhysicsModifier) == 0x02) {
        return;
    }

    /* call GetEntityXDistanceToLink_03; ld d, b; ld hl, Data_003_6E0C; add hl, de; ld a, [hl]; ldh [hLinkSpeedX], a */
    uint8_t dir_x = 0;
    uint8_t dist_x = 0;
    GetEntityXDistanceToLink_03(gb, &dir_x, &dist_x);

    uint8_t speed_x = Data_003_6E0C[dir_x & 1];
    gb_write_hram(gb, hLinkSpeedX, speed_x);

    /* ld a, $F4; ldh [hLinkSpeedY], a */
    gb_write_hram(gb, hLinkSpeedY, 0xF4);

    /* xor a; ldh [hLinkPhysicsModifier], a; scf; ret */
    gb_write_hram(gb, hLinkPhysicsModifier, 0x00);
}

/* Added to an entity's x position when checking for collisions with walls.
 * Row: collision box type (0..3)
 * Column: movement direction (right=0, left=1, up=2, down=3) */
const int8_t EntityCollisionPointsX[16] = {
    13,   2,   8,   8,
    10,   6,   8,   8,
    16,  -1,   8,   8,
    13,   2,   8,   8
};

/* Added to an entity's y position when checking for collisions with walls. */
const int8_t EntityCollisionPointsY[16] = {
     8,   8,   2,  13,
     8,   8,   6,  10,
     8,   8,  -1,  16,
     8,   8,   2,  13
};

const uint8_t CollisionsTableFlagPerDirection[4] = {
    0x01, 0x02, 0x04, 0x08
};

/* Indexed by: object id - OBJ_PHYSICS_CONVEYOR */
const int8_t EntityOnConveyorMovementX[8] = {
    0, 0, -1, 1, 1, -1, 1, -1
};

const int8_t EntityOnConveyorMovementY[8] = {
    1, -1, 0, 0, 1, 1, -1, -1
};

const uint8_t FineCollisionShapes[72] = {
    /* 0x7C: Open door */
    1, 0, 1, 0,
    /* 0x7D */
    0, 1, 0, 1,
    /* 0x7E */
    1, 1, 0, 0,
    /* 0x7F */
    0, 0, 1, 1,
    /* 0x80: Fine collision */
    1, 0, 1, 0,
    /* 0x81 */
    0, 1, 0, 1,
    /* 0x82 */
    1, 1, 0, 0,
    /* 0x83 */
    0, 0, 1, 1,
    /* 0x84 */
    0, 1, 1, 1,
    /* 0x85 */
    1, 0, 1, 1,
    /* 0x86 */
    1, 1, 0, 1,
    /* 0x87 */
    1, 1, 1, 0,
    /* 0x88 */
    1, 0, 0, 0,
    /* 0x89 */
    0, 1, 0, 0,
    /* 0x8A */
    0, 0, 1, 0,
    /* 0x8B */
    0, 0, 0, 1,
    /* 0x8C */
    0, 1, 1, 0,
    /* 0x8D */
    1, 0, 0, 1
};

const uint8_t SwitchBlockLoweredStatePerObject[2] = {
    0x00, 0x02
};

/* ===== func_003_7E0E (03:7E0E) ===== */
void func_003_7E0E(GBState *gb, uint16_t bc) {
    if (!gb) return;

    uint8_t pos_x = gb_read(gb, wEntitiesPosXTable + bc);
    uint8_t sub_x = (uint8_t)(pos_x - 1);
    gb_write_hram(gb, hMultiPurpose4, sub_x);
    uint8_t left = (uint8_t)(sub_x & 0xF0);
    gb_write_hram(gb, hIntersectedObjectLeft, left);

    uint8_t swap_x = (uint8_t)((left >> 4) & 0x0F);

    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);
    uint8_t sub_y = (uint8_t)(pos_y - 7);
    gb_write_hram(gb, hMultiPurpose5, sub_y);
    uint8_t top = (uint8_t)(sub_y & 0xF0);
    gb_write_hram(gb, hIntersectedObjectTop, top);

    uint8_t tile_idx = (uint8_t)(top | swap_x);
    uint8_t obj = gb_read(gb, wRoomObjects + tile_idx);
    gb_write_hram(gb, hObjectUnderEntity, obj);

    uint16_t de = (uint16_t)(((uint16_t)gb_read(gb, wIsIndoor) << 8) | obj);
    uint8_t physics = GetObjectPhysicsFlagsAndRestoreBank3(gb, de);
    gb_write_hram(gb, hMultiPurpose3, physics);
}

/* ===== ApplyEntityCollisionWithObject (03:7ACD) ===== */
bool ApplyEntityCollisionWithObject(GBState *gb, uint16_t bc, uint16_t de) {
    if (!gb) return true;

    uint8_t pos_x = gb_read(gb, wEntitiesPosXTable + bc);
    uint8_t sub_x = (uint8_t)(pos_x - 8);
    uint8_t h0 = gb_read_hram(gb, hMultiPurpose0);
    uint8_t table_offset = (uint8_t)(h0 + (uint8_t)de);
    uint8_t x_point = (uint8_t)(sub_x + EntityCollisionPointsX[table_offset & 0x0F]);
    gb_write_hram(gb, hMultiPurpose4, x_point);
    uint8_t h1 = (uint8_t)((x_point >> 4) & 0x0F);
    gb_write_hram(gb, hMultiPurpose1, h1);

    uint8_t y_base;
    if ((uint8_t)de == 3) {
        uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);
        if (entity_type == ENTITY_WRECKING_BALL || entity_type == ENTITY_LIFTABLE_ROCK) {
            uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);
            uint8_t pos_z = gb_read(gb, wEntitiesPosZTable + bc);
            uint8_t z_val = ((pos_z & 0x80) != 0) ? 0 : pos_z;
            y_base = (uint8_t)(pos_y - (z_val >> 1));
        } else {
            y_base = gb_read(gb, wEntitiesPosYTable + bc);
        }
    } else {
        y_base = gb_read(gb, wEntitiesPosYTable + bc);
    }

    uint8_t sub_y = (uint8_t)(y_base - 0x10);
    uint8_t y_point = (uint8_t)(sub_y + EntityCollisionPointsY[table_offset & 0x0F]);
    gb_write_hram(gb, hMultiPurpose5, y_point);

    uint8_t tile_idx = (uint8_t)((y_point & 0xF0) | h1);
    uint8_t obj = gb_read(gb, wRoomObjects + tile_idx);
    gb_write_hram(gb, hObjectUnderEntity, obj);

    if (obj == OBJECT_LIFTABLE_ROCK) {
        goto isHookshottable;
    }

    uint16_t de_flags = (uint16_t)(((uint16_t)gb_read(gb, wIsIndoor) << 8) | obj);
    uint8_t physics = GetObjectPhysicsFlagsAndRestoreBank3(gb, de_flags);
    gb_write_hram(gb, hMultiPurpose3, physics);

    uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);
    if (entity_type == ENTITY_FISH || entity_type == ENTITY_WATER_TEKTITE) {
        if (physics == OBJ_PHYSICS_SHALLOW_WATER || physics == OBJ_PHYSICS_DEEP_WATER) {
            return true;
        }
        goto doesCollide;
    }

    if (physics == OBJ_PHYSICS_NONE) {
        return true;
    }

    if (physics == OBJ_PHYSICS_LAVA || physics == OBJ_PHYSICS_PIT || physics == OBJ_PHYSICS_PIT_WARP) {
        if (gb_read(gb, wEntitiesPosZTable + bc) != 0) {
            return true;
        }
        if (gb_read(gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0) {
            goto doesCollide;
        }
        if (entity_type == ENTITY_MOLDORM) {
            goto doesCollide;
        }
        return true;
    }

    if (physics >= 0x7C && physics < 0x90) {
        if (physics < 0x80) {
            if (entity_type == ENTITY_SPARK_COUNTER_CLOCKWISE ||
                entity_type == ENTITY_SPARK_CLOCKWISE ||
                (gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_IS_BOSS) != 0) {
                goto setCollisionsTableFlagsAndReturn;
            }
        } else {
            if (entity_type == ENTITY_WRECKING_BALL || entity_type == ENTITY_BOMB) {
                return true;
            }
        }

        uint8_t shape_index = (uint8_t)((physics - 0x7C) << 2);
        uint8_t p4 = gb_read_hram(gb, hMultiPurpose4);
        uint8_t col = (uint8_t)((p4 >> 3) & 0x01);
        uint8_t p5 = gb_read_hram(gb, hMultiPurpose5);
        uint8_t row = (uint8_t)((p5 >> 3) & 0x02);
        uint8_t shape_byte = FineCollisionShapes[shape_index + row + col];
        if (shape_byte == 0) {
            return true;
        }
    }

    if (physics >= OBJ_PHYSICS_LEDGE && physics < (OBJ_PHYSICS_LEDGE + 4)) {
        uint8_t ledge_dir = (uint8_t)(physics - OBJ_PHYSICS_LEDGE);
        if (ledge_dir == gb_read(gb, wEntitiesThrownDirectionTable + bc)) {
            if (gb_read(gb, wEntitiesPosZTable + bc) == 0) {
                goto doesCollide;
            }
            uint8_t unkJ = gb_read(gb, wEntitiesUnknowTableJ + bc);
            gb_write(gb, wEntitiesUnknowTableJ + bc, (uint8_t)(unkJ + 1));
            return true;
        }

        if (entity_type == ENTITY_WRECKING_BALL) {
            goto doesCollide;
        }

        uint8_t unkJ = gb_read(gb, wEntitiesUnknowTableJ + bc);
        if (unkJ == 0) {
            goto doesCollide;
        }

        uint8_t frame = gb_read_hram(gb, hFrameCounter);
        if ((frame & 0x03) == 0) {
            return true;
        }
        if (gb_read(gb, wIsIndoor) != 0) {
            gb_write(gb, wEntitiesUnknowTableJ + bc, (uint8_t)(unkJ - 1));
            return true;
        }
        if ((frame & 0x01) == 0) {
            return true;
        }
        gb_write(gb, wEntitiesUnknowTableJ + bc, (uint8_t)(unkJ - 1));
        return true;
    }

    if (physics == OBJ_PHYSICS_TRACTOR_DEVICE) {
        goto setCollisionsTableFlagsAndReturn;
    }

    if (physics >= 0xA0) {
        return true;
    }

    if (physics >= OBJ_PHYSICS_LEDGE_OVERWORLD) {
        goto doesCollide;
    }

    if (physics == OBJ_PHYSICS_SOLID || physics == OBJ_PHYSICS_DOOR) {
        goto hookshotEnd;
    }

    if (physics != OBJ_PHYSICS_OCEAN_SWITCH_BLOCK) {
        return true;
    }

    if (entity_type == ENTITY_WRECKING_BALL || entity_type == ENTITY_BOMB) {
        return true;
    }

    if (entity_type == ENTITY_HOOKSHOT_CHAIN && gb_read(gb, wLinkStandingOnSwitchBlock) != 0) {
        return true;
    }

    if (obj < OBJECT_LOWERED_BLOCK || obj > OBJECT_RAISED_BLOCK) {
        goto setCollisionsTableFlagsAndReturn;
    }

    uint8_t lowered_state = SwitchBlockLoweredStatePerObject[obj - OBJECT_LOWERED_BLOCK];
    if ((gb_read(gb, wSwitchBlocksState) ^ lowered_state) == 0) {
        return true;
    }

doesCollide:
    if (gb_read_hram(gb, hMultiPurpose3) == OBJ_PHYSICS_HOOKSHOTABLE) {
isHookshottable:
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_HOOKSHOT_CHAIN) {
            if (GetEntityTransitionCountdown(gb, bc) >= 0x26) {
                UnloadEntity(gb, bc);
            }
            gb_write(gb, wEntitiesStateTable + bc, 0x01);
        }
    }

hookshotEnd:
    if ((gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_NO_WALL_COLLISION) != 0) {
        return true;
    }

setCollisionsTableFlagsAndReturn:
    {
        uint8_t dir_flag = CollisionsTableFlagPerDirection[de & 0x03];
        uint8_t prev = gb_read(gb, wEntitiesCollisionsTable + bc);
        gb_write(gb, wEntitiesCollisionsTable + bc, (uint8_t)(prev | dir_flag));
        return false;
    }
}

/* ===== ApplyEntityInteractionWithBackground (03:7893) ===== */
void ApplyEntityInteractionWithBackground(GBState *gb, uint16_t bc) {
    if (!gb) return;

    uint8_t prev_ground = gb_read(gb, wEntitiesGroundStatusTable + bc);
    gb_write_hram(gb, hMultiPurpose0, prev_ground);

    gb_write(gb, wEntitiesGroundStatusTable + bc, 0);
    gb_write_hram(gb, hMultiPurpose1, 0);
    gb_write(gb, wEntityHorizontallyCollidedObject, 0);
    gb_write(gb, wEntityVerticallyCollidedObject, 0);

    /* If entity z position is positive (and not zero), skip handling water and tall grass */
    uint8_t pos_z = gb_read(gb, wEntitiesPosZTable + bc);
    if ((pos_z & 0x80) == 0 && pos_z != 0) {
        goto interactWithGroundEnd;
    }

    /* .negativeZ */
    gb_write(gb, wEntitiesUnknowTableJ + bc, 0);
    if ((gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_NO_GROUND_INTERACTION) != 0) {
        goto interactWithGroundEnd;
    }

    func_003_7E0E(gb, bc);

    uint8_t ground_status = ENTITY_GROUND_STATUS_DEEP_WATER;
    uint8_t obj_under = gb_read_hram(gb, hObjectUnderEntity);
    if (obj_under == OBJECT_WATER_LADDER_SIDESCROLL) {
        goto setGroundStatus;
    }

    uint8_t mp3 = gb_read_hram(gb, hMultiPurpose3);
    if (mp3 == 0) {
        goto interactWithGroundEnd;
    }

    if (mp3 == OBJ_PHYSICS_LAVA || mp3 == OBJ_PHYSICS_DEEP_WATER) {
        ground_status = 0x02;
        uint8_t active_type = gb_read_hram(gb, hActiveEntityType);
        if (active_type == ENTITY_FISH ||
            active_type == ENTITY_PEAHAT ||
            active_type == ENTITY_ROOSTER ||
            active_type == ENTITY_BOW_WOW ||
            active_type == ENTITY_MARIN_AT_THE_SHORE) {
            goto setGroundStatus;
        }

        UnloadEntity(gb, bc);
        goto createWaterSplash;
    }

    if (mp3 == OBJ_PHYSICS_WATER_SIDESCROLL) {
        goto setGroundStatus;
    }

    ground_status++; /* becomes 2: SHALLOW_WATER */
    if (mp3 == OBJ_PHYSICS_SHALLOW_WATER) {
        goto setGroundStatus;
    }

    if (mp3 != OBJ_PHYSICS_GRASS) {
        goto setGroundStatusEnd;
    }

    ground_status++; /* becomes 3: TALL_GRASS */

setGroundStatus:
    gb_write(gb, wEntitiesGroundStatusTable + bc, ground_status);

setGroundStatusEnd:
    if ((gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_SPLASH_IN_WATER) != 0) {
        uint8_t cur_ground = gb_read(gb, wEntitiesGroundStatusTable + bc);
        uint8_t old_ground = gb_read_hram(gb, hMultiPurpose0);
        if (old_ground != cur_ground &&
            cur_ground != ENTITY_GROUND_STATUS_TALL_GRASS &&
            old_ground != ENTITY_GROUND_STATUS_TALL_GRASS) {
            if (gb_read_hram(gb, hIsSideScrolling) != 0) {
                if (gb_read_hram(gb, hActiveEntityType) != ENTITY_CHEEP_CHEEP_JUMPING) {
                    uint8_t speed_y = gb_read(gb, wEntitiesSpeedYTable + bc);
                    if ((speed_y & 0x80) == 0) {
                        gb_write(gb, wEntitiesSpeedYTable + bc, 0);
                        int8_t speed_x = (int8_t)gb_read(gb, wEntitiesSpeedXTable + bc);
                        gb_write(gb, wEntitiesSpeedXTable + bc, (uint8_t)(speed_x >> 1));
                    }
                }
            } else {
                uint8_t speed_z = gb_read(gb, wEntitiesSpeedZTable + bc);
                if ((speed_z & 0x80) != 0 && speed_z < 0xE7) {
                    goto checkSplashCountdown;
                }
                goto createWaterSplashEnd;
            }

checkSplashCountdown:
            if (gb_read(gb, wEntitiesPrivateCountdown3Table + bc) == 0) {
createWaterSplash:
                gb_write_hram(gb, hMultiPurpose0, gb_read(gb, wEntitiesPosXTable + bc));
                gb_write_hram(gb, hMultiPurpose1, gb_read(gb, wEntitiesPosYTable + bc));
                gb_write_hram(gb, hJingle, JINGLE_WATER_SPLASH);
                AddTranscientVfx(gb, TRANSCIENT_VFX_WATER_SPLASH);
            }
        }
    }

createWaterSplashEnd:
    {
        uint8_t mp3_c = gb_read_hram(gb, hMultiPurpose3);
        if ((uint8_t)(mp3_c + 1) >= (uint8_t)(OBJ_PHYSICS_CONVEYOR + 1)) {
            uint8_t conv_idx = (uint8_t)(mp3_c - OBJ_PHYSICS_CONVEYOR);
            if ((gb_read_hram(gb, hFrameCounter) & 0x03) == 0) {
                uint8_t px = gb_read(gb, wEntitiesPosXTable + bc);
                gb_write(gb, wEntitiesPosXTable + bc, (uint8_t)(px + EntityOnConveyorMovementX[conv_idx & 0x07]));
                uint8_t py = gb_read(gb, wEntitiesPosYTable + bc);
                gb_write(gb, wEntitiesPosYTable + bc, (uint8_t)(py + EntityOnConveyorMovementY[conv_idx & 0x07]));
            }
            goto interactWithGroundEnd;
        }
    }

    {
        uint8_t obj_under_ent = gb_read_hram(gb, hObjectUnderEntity);
        uint8_t mp3_val = gb_read_hram(gb, hMultiPurpose3);
        bool on_pit = (obj_under_ent == OBJECT_WELL) ||
                      (mp3_val == OBJ_PHYSICS_PIT) ||
                      (mp3_val == OBJ_PHYSICS_PIT_WARP);
        if (on_pit) {
            uint8_t active_type = gb_read_hram(gb, hActiveEntityType);
            if (active_type == ENTITY_BOW_WOW ||
                active_type == ENTITY_ROOSTER ||
                active_type == ENTITY_HEART_CONTAINER) {
                goto interactWithGroundEnd;
            }

            if (active_type == ENTITY_MARIN_AT_THE_SHORE) {
                if (gb_read(gb, wLinkMotionState) != LINK_MOTION_FALLING_DOWN ||
                    gb_read_hram(gb, hObjectUnderEntity) != OBJECT_WELL) {
                    goto interactWithGroundEnd;
                }
            }

            uint8_t ignore_hits = gb_read(gb, wEntitiesIgnoreHitsCountdownTable + bc);
            if (ignore_hits == 0) {
                goto interactWithGroundEnd;
            }

            gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, (uint8_t)(ignore_hits - 1));
            gb_write(gb, wEntitiesFlashCountdownTable + bc, 0);
            gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_FALLING);

            uint8_t left = gb_read_hram(gb, hIntersectedObjectLeft);
            gb_write(gb, wEntitiesFallingTargetXTable + bc, (uint8_t)(left + 8));

            uint8_t top = gb_read_hram(gb, hIntersectedObjectTop);
            gb_write(gb, wEntitiesFallingTargetYTable + bc, (uint8_t)(top + 16));

            gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x6F);

            if (active_type == ENTITY_MOBLIN_SWORD ||
                active_type == ENTITY_MOBLIN ||
                active_type == ENTITY_OCTOROK) {
                goto interactWithGroundEnd;
            }

            gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x48);
            if (gb_read(gb, wEntitiesIgnoreHitsCountdownTable + bc) == 0) {
                gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x2F);
                gb_write_hram(gb, hJingle, JINGLE_ITEM_FALLING);
            }
        }
    }

interactWithGroundEnd:
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BOW_WOW) {
        return;
    }

    gb_write(gb, wEntityHorizontallyCollidedObject, 0);
    uint8_t hitbox_flags = gb_read(gb, wEntitiesHitboxFlagsTable + bc);
    uint8_t mp0 = (uint8_t)((hitbox_flags & 0x03) << 2);
    gb_write_hram(gb, hMultiPurpose0, mp0);
    gb_write(gb, wEntitiesCollisionsTable + bc, 0);

    uint8_t speed_x = gb_read(gb, wEntitiesSpeedXTable + bc);
    if (speed_x != 0) {
        uint16_t de_x = ((speed_x & 0x80) != 0) ? 1 : 0;
        bool no_collision_x = ApplyEntityCollisionWithObject(gb, bc, de_x);
        if (!no_collision_x) {
            uint8_t obj_col = gb_read_hram(gb, hObjectUnderEntity);
            gb_write(gb, wEntityHorizontallyCollidedObject, obj_col);
            if (gb_read_hram(gb, hActiveEntityNoBGCollision) == 0) {
                gb_write(gb, wEntitiesPosXTable + bc, gb_read_hram(gb, hActiveEntityPosX));
            }
        }
    }

    uint8_t speed_y = gb_read(gb, wEntitiesSpeedYTable + bc);
    if (speed_y != 0) {
        uint16_t de_y = ((speed_y & 0x80) != 0) ? 2 : 3;
        bool no_collision_y = ApplyEntityCollisionWithObject(gb, bc, de_y);
        if (!no_collision_y) {
            uint8_t obj_col = gb_read_hram(gb, hObjectUnderEntity);
            gb_write(gb, wEntityVerticallyCollidedObject, obj_col);
            if (gb_read_hram(gb, hActiveEntityNoBGCollision) == 0) {
                gb_write(gb, wEntitiesPosYTable + bc, gb_read_hram(gb, hActiveEntityPosY));
            }
        }
    }
}

/* Data_003_69A2 (03:69A2) - Lit torch replacement tiles for 2x2 draw command */
const uint8_t Data_003_69A2[8] = {
    0x6C, 0x74, 0x6D, 0x75, 0x00, 0x00, 0x00, 0x00
};

/* ===== ApplySwordIntersectionWithObjects (03:7CAB) ===== */
bool ApplySwordIntersectionWithObjects(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    /* ld de, $00; push bc */
    /* ld hl, wEntitiesPosXTable; add hl, bc; ld a, [hl] */
    /* ldh [hMultiPurpose4], a; and $F0; ldh [hIntersectedObjectLeft], a; swap a */
    uint8_t pos_x = gb_read(gb, wEntitiesPosXTable + bc);
    gb_write_hram(gb, hMultiPurpose4, pos_x);
    uint8_t left = (uint8_t)(pos_x & 0xF0);
    gb_write_hram(gb, hIntersectedObjectLeft, left);
    uint8_t c = (uint8_t)(left >> 4);

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld c, a */
    /* ld a, [hl]; sub $08; ldh [hMultiPurpose5], a; and $F0; ldh [hIntersectedObjectTop], a */
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);
    uint8_t multi5 = (uint8_t)(pos_y - 0x08);
    gb_write_hram(gb, hMultiPurpose5, multi5);
    uint8_t top = (uint8_t)(multi5 & 0xF0);
    gb_write_hram(gb, hIntersectedObjectTop, top);

    /* or c; ld c, a; ldh [hIndexOfObjectBelowLink], a */
    uint8_t tile_idx = (uint8_t)(top | c);
    gb_write_hram(gb, hIndexOfObjectBelowLink, tile_idx);

    /* ld b, $00; ld hl, wRoomObjects; ld a, h; add hl, bc; ld h, a; pop bc */
    uint16_t room_obj_addr = (uint16_t)(wRoomObjects + tile_idx);
    uint8_t obj = gb_read(gb, room_obj_addr);
    gb_write_hram(gb, hObjectUnderEntity, obj);

    /* cp OBJECT_TORCH_LIT; jp z, jr_003_7E03 */
    if (obj == OBJECT_TORCH_LIT) {
        return false;
    }

    /* cp OBJECT_TORCH_UNLIT; jp nz, jr_003_7D6B */
    if (obj == OBJECT_TORCH_UNLIT) {
        /* ldh a, [hIsGBC]; and a; jr z, .jr_7CFD */
        if (gb_read_hram(gb, hIsGBC) != 0) {
            /* ld a, [wLinkMotionState]; cp LINK_MOTION_REVOLVING_DOOR; ret z */
            if (gb_read(gb, wLinkMotionState) == LINK_MOTION_REVOLVING_DOOR) {
                return false;
            }
            /* ld a, [wBGPaletteTransitionEffect]; and a; jp nz, jr_003_7E03 */
            if (gb_read(gb, wBGPaletteTransitionEffect) != 0) {
                return false;
            }
            /* ld a, [wRoomTransitionState]; and a; jp nz, jr_003_7E03 */
            if (gb_read(gb, wRoomTransitionState) != 0) {
                return false;
            }
        }

        /* .jr_7CFD: ldh a, [hActiveEntityType]; cp ENTITY_MAGIC_ROD_FIREBALL; jr nz, jr_003_7D6B */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_MAGIC_ROD_FIREBALL && gb_read(gb, wIsIndoor) != 0) {
            /* push hl; ld a, NOISE_SFX_BURSTING_FLAME; ldh [hNoiseSfx], a */
            gb_write_hram(gb, hNoiseSfx, NOISE_SFX_BURSTING_FLAME);

            /* ld a, ENTITY_MAGIC_POWDER_SPRINKLE; call SpawnNewEntity; jr c, jr_003_7D6A */
            uint16_t new_slot = SpawnNewEntity_trampoline(gb, ENTITY_MAGIC_POWDER_SPRINKLE, NULL);
            if (new_slot != 0xFFFF) {
                /* pop hl; ld a, $AC; ld [hl], a; ld [wDDD8], a */
                gb_write(gb, room_obj_addr, OBJECT_TORCH_LIT);
                gb_write(gb, wDDD8, OBJECT_TORCH_LIT);

                /* push bc; ld c, e; ld b, d; ld d, h; ld e, l */
                /* ld hl, wEntitiesPrivateState1Table; add hl, bc; ld [hl], d */
                /* ld hl, wEntitiesPrivateState2Table; add hl, bc; ld [hl], e */
                gb_write(gb, wEntitiesPrivateState1Table + new_slot, (uint8_t)(room_obj_addr >> 8));
                gb_write(gb, wEntitiesPrivateState2Table + new_slot, (uint8_t)(room_obj_addr & 0xFF));

                /* ldh a, [hIntersectedObjectLeft]; ld hl, wEntitiesPosXTable; add hl, bc; ld [hl], a */
                gb_write(gb, wEntitiesPosXTable + new_slot, left);
                /* ldh a, [hIntersectedObjectTop]; ld hl, wEntitiesPosYTable; add hl, bc; ld [hl], a */
                gb_write(gb, wEntitiesPosYTable + new_slot, top);

                /* ld hl, wEntitiesStateTable; add hl, bc; ld [hl], $01 */
                gb_write(gb, wEntitiesStateTable + new_slot, 0x01);

                /* call GetEntitySlowTransitionCountdown; ld [hl], $80 */
                gb_write(gb, wEntitiesSlowTransitionCountdownTable + new_slot, 0x80);

                /* pop bc; ld hl, wC1A2; inc [hl] */
                gb_write(gb, wC1A2, (uint8_t)(gb_read(gb, wC1A2) + 1));

                /* ld a, [wC3CD]; and a; jr z, .jr_7D63 */
                uint8_t c3cd = gb_read(gb, wC3CD);
                if (c3cd != 0) {
                    /* sub $04; ld [wC3CD], a */
                    gb_write(gb, wC3CD, (uint8_t)(c3cd - 4));

                    /* ldh a, [hIsGBC]; and a; jr z, .jr_7D63 */
                    if (gb_read_hram(gb, hIsGBC) != 0) {
                        /* ld a, $40; ld [wBGPaletteTransitionEffect], a; ld a, $0B; ld [wDDD7], a */
                        gb_write(gb, wBGPaletteTransitionEffect, 0x40);
                        gb_write(gb, wDDD7, 0x0B);
                    }
                }

                /* .jr_7D63: ld de, Data_003_69A2; push de; jp label_003_51F5 */
                label_003_51F5(gb, Data_003_69A2);
                return false;
            }
            /* jr_003_7D6A: pop hl, fall through to jr_003_7D6B */
        }
    }

    /* jr_003_7D6B: */
    uint8_t is_indoor = gb_read(gb, wIsIndoor);
    uint16_t de_phys = (uint16_t)(((uint16_t)is_indoor << 8) | obj);
    uint8_t physics = GetObjectPhysicsFlagsAndRestoreBank3(gb, de_phys);
    gb_write_hram(gb, hMultiPurpose1, physics);

    /* and a; jp z, jr_003_7E03 */
    if (physics == 0) {
        return false;
    }

    /* ldh [hMultiPurpose3], a */
    gb_write_hram(gb, hMultiPurpose3, physics);

    /* cp $FF; jp z, label_003_7E05 */
    if (physics == 0xFF) {
        /* label_003_7E05: ldh a, [hActiveEntityType]; cp ENTITY_BOOMERANG; jr z, jr_003_7DE3 */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BOOMERANG) {
            goto collision;
        }
        /* jp UnloadEntityAndReturn */
        UnloadEntity(gb, bc);
        return false;
    }

    /* cp $D0; jr c, jr_003_7DC0; cp $D4; jr nc, jr_003_7DC0 */
    if (physics >= 0xD0 && physics < 0xD4) {
        /* sub $D0; ld hl, wEntitiesThrownDirectionTable; add hl, bc; cp [hl] */
        uint8_t ledge_dir = (uint8_t)(physics - 0xD0);
        uint8_t thrown_dir = gb_read(gb, wEntitiesThrownDirectionTable + bc);

        if (ledge_dir == thrown_dir) {
            /* jr_003_7DB0: ld hl, wEntitiesPosZTable; add hl, bc; ld a, [hl]; and a; jr z, jr_003_7DE3 */
            if (gb_read(gb, wEntitiesPosZTable + bc) == 0) {
                goto collision;
            }
            /* ld hl, wEntitiesUnknowTableJ; add hl, bc; inc [hl]; jp jr_003_7E03 */
            gb_write(gb, wEntitiesUnknowTableJ + bc, (uint8_t)(gb_read(gb, wEntitiesUnknowTableJ + bc) + 1));
            return false;
        }

        /* ld hl, wEntitiesUnknowTableJ; add hl, bc; ld a, [hl]; and a; jr z, jr_003_7DE3 */
        uint8_t table_j = gb_read(gb, wEntitiesUnknowTableJ + bc);
        if (table_j == 0) {
            goto collision;
        }

        /* ldh a, [hFrameCounter]; and $03; jr z, jr_003_7E03 */
        uint8_t frame = gb_read_hram(gb, hFrameCounter);
        if ((frame & 0x03) == 0) {
            return false;
        }

        /* ld a, [wIsIndoor]; and a; jr nz, .jr_7DAC */
        /* ldh a, [hFrameCounter]; and $01; jr z, jr_003_7E03 */
        if (is_indoor == 0 && (frame & 0x01) == 0) {
            return false;
        }

        /* .jr_7DAC: dec [hl]; jp jr_003_7E03 */
        gb_write(gb, wEntitiesUnknowTableJ + bc, (uint8_t)(table_j - 1));
        return false;
    }

    /* jr_003_7DC0: cp $7C; jp c, label_003_7DCD; cp $90; jp nc, label_003_7DCD; jp jr_003_7E03 */
    if (physics >= 0x7C && physics < 0x90) {
        return false;
    }

    /* label_003_7DCD: ldh a, [hMultiPurpose3] */
    /* cp $A0; jr nc, jr_003_7E03 */
    if (physics >= 0xA0) {
        return false;
    }

    /* cp $50; jr z, jr_003_7E03; cp $51; jr z, jr_003_7E03 */
    if (physics == 0x50 || physics == 0x51) {
        return false;
    }

    /* cp $10; jr nc, jr_003_7DE3; cp $01; jr nz, jr_003_7E03 */
    if (physics < 0x10 && physics != 0x01) {
        return false;
    }

collision:
    /* jr_003_7DE3: */
    /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld [hl], $01 */
    gb_write(gb, wEntitiesCollisionsTable + bc, 0x01);

    /* ldh a, [hActiveEntityType]; cp ENTITY_BOOMERANG; jr nz, .jr_7DF3 */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BOOMERANG) {
        /* call GetEntityTransitionCountdown; ret z */
        if (GetEntityTransitionCountdown(gb, bc) == 0) {
            return false;
        }
    }

    /* .jr_7DF3: ld hl, wEntitiesPosXTable; add hl, bc; ldh a, [hActiveEntityPosX]; ld [hl], a */
    gb_write(gb, wEntitiesPosXTable + bc, gb_read_hram(gb, hActiveEntityPosX));

    /* ld hl, wEntitiesPosYTable; add hl, bc; ldh a, [hActiveEntityPosY]; ld [hl], a */
    gb_write(gb, wEntitiesPosYTable + bc, gb_read_hram(gb, hActiveEntityPosY));

    /* scf; ret */
    return true;
}

/* ===== GetEntityXDistanceToLink_03 (03:7ED9) ===== */
void GetEntityXDistanceToLink_03_idx(GBState *gb, uint16_t bc, uint8_t *e, uint8_t *d) {
    if (!gb) return;
    uint8_t dir = DIRECTION_RIGHT;
    uint8_t link_x = gb_read_hram(gb, hLinkPositionX);
    uint8_t ent_x = gb_read(gb, wEntitiesPosXTable + bc);
    uint8_t diff = (uint8_t)(link_x - ent_x);
    if ((diff & 0x80) != 0) {
        dir = (uint8_t)(dir + 1); /* DIRECTION_LEFT */
    }
    if (e) *e = dir;
    if (d) *d = diff;
}

void GetEntityXDistanceToLink_03(GBState *gb, uint8_t *e, uint8_t *d) {
    if (!gb) return;
    uint16_t bc = gb_read(gb, wActiveEntityIndex);
    GetEntityXDistanceToLink_03_idx(gb, bc, e, d);
}

/* ===== GetEntityYDistanceToLink_03 (03:7EE9) ===== */
void GetEntityYDistanceToLink_03_idx(GBState *gb, uint16_t bc, uint8_t *e, uint8_t *d) {
    if (!gb) return;
    uint8_t dir = DIRECTION_UP;
    uint8_t link_y = gb_read_hram(gb, hLinkPositionY);
    uint8_t ent_y = gb_read(gb, wEntitiesPosYTable + bc);
    uint8_t diff = (uint8_t)(link_y - ent_y);
    uint8_t ent_z = gb_read(gb, wEntitiesPosZTable + bc);
    diff = (uint8_t)(diff + ent_z);
    if ((diff & 0x80) == 0) {
        dir = (uint8_t)(dir + 1); /* DIRECTION_DOWN */
    }
    if (e) *e = dir;
    if (d) *d = diff;
}

void GetEntityYDistanceToLink_03(GBState *gb, uint8_t *e, uint8_t *d) {
    if (!gb) return;
    uint16_t bc = gb_read(gb, wActiveEntityIndex);
    GetEntityYDistanceToLink_03_idx(gb, bc, e, d);
}

/* ===== GetEntityDirectionToLink_03 (03:7EFE) ===== */
uint8_t GetEntityDirectionToLink_03(GBState *gb) {
    if (!gb) return 0;
    uint8_t dir_x = 0;
    uint8_t dist_x = 0;
    GetEntityXDistanceToLink_03(gb, &dir_x, &dist_x);
    gb_write_hram(gb, hMultiPurpose0, dir_x);

    uint8_t abs_x = dist_x;
    if ((abs_x & 0x80) != 0) {
        abs_x = (uint8_t)(~abs_x + 1);
    }

    uint8_t dir_y = 0;
    uint8_t dist_y = 0;
    GetEntityYDistanceToLink_03(gb, &dir_y, &dist_y);
    gb_write_hram(gb, hMultiPurpose1, dir_y);

    uint8_t abs_y = dist_y;
    if ((abs_y & 0x80) != 0) {
        abs_y = (uint8_t)(~abs_y + 1);
    }

    uint8_t result_dir;
    if (abs_y >= abs_x) {
        result_dir = gb_read_hram(gb, hMultiPurpose1);
    } else {
        result_dir = gb_read_hram(gb, hMultiPurpose0);
    }
    return result_dir;
}

/* ===== GetVectorTowardsLink (03:7E45) ===== */
void GetVectorTowardsLink_with_length(GBState *gb, uint8_t length, uint8_t *val0, uint8_t *val1) {
    if (!gb) return;

    gb_write_hram(gb, hMultiPurpose1, length);
    if (length == 0) {
        gb_write_hram(gb, hMultiPurpose0, 0x00);
        if (val0) *val0 = 0x00;
        if (val1) *val1 = 0x00;
        return;
    }

    uint8_t dir_y = 0;
    uint8_t dist_y = 0;
    GetEntityYDistanceToLink_03(gb, &dir_y, &dist_y);
    /* dec e; dec e; ld a, e; ldh [hMultiPurpose2], a */
    uint8_t dy_flag = (uint8_t)(dir_y - 2); /* 0 if UP (dy < 0), 1 if DOWN (dy >= 0) */
    gb_write_hram(gb, hMultiPurpose2, dy_flag);

    uint8_t abs_y = dist_y;
    if ((abs_y & 0x80) != 0) {
        abs_y = (uint8_t)(~abs_y + 1);
    }
    gb_write_hram(gb, hMultiPurposeC, abs_y);

    uint8_t dir_x = 0;
    uint8_t dist_x = 0;
    GetEntityXDistanceToLink_03(gb, &dir_x, &dist_x);
    /* ldh [hMultiPurpose3], a (where a is e: 1 if LEFT, 0 if RIGHT) */
    gb_write_hram(gb, hMultiPurpose3, dir_x);

    uint8_t abs_x = dist_x;
    if ((abs_x & 0x80) != 0) {
        abs_x = (uint8_t)(~abs_x + 1);
    }
    gb_write_hram(gb, hMultiPurposeD, abs_x);

    uint8_t swapped = 0;
    /* cp [hl] where a = [hMultiPurposeD] (abs_x) and [hl] = [hMultiPurposeC] (abs_y) */
    /* if abs_x < abs_y, swap them and swapped = 1 */
    if (gb_read_hram(gb, hMultiPurposeD) < gb_read_hram(gb, hMultiPurposeC)) {
        swapped = 1;
        uint8_t tmp_c = gb_read_hram(gb, hMultiPurposeC);
        uint8_t tmp_d = gb_read_hram(gb, hMultiPurposeD);
        gb_write_hram(gb, hMultiPurposeD, tmp_c);
        gb_write_hram(gb, hMultiPurposeC, tmp_d);
    }

    uint8_t acc = 0;
    uint8_t res0 = 0;
    uint8_t counter = gb_read_hram(gb, hMultiPurpose1);
    uint8_t small = gb_read_hram(gb, hMultiPurposeC);
    uint8_t large = gb_read_hram(gb, hMultiPurposeD);

    while (counter > 0) {
        uint16_t sum = (uint16_t)acc + small;
        if (sum > 0xFF) {
            acc = (uint8_t)(sum - large);
            res0++;
        } else {
            uint8_t s = (uint8_t)sum;
            if (s >= large) {
                s = (uint8_t)(s - large);
                res0++;
            }
            acc = s;
        }
        counter--;
    }
    gb_write_hram(gb, hMultiPurposeB, acc);
    gb_write_hram(gb, hMultiPurpose0, res0);

    /* If X and Y were swapped before, swap back */
    if (swapped != 0) {
        uint8_t val_0 = gb_read_hram(gb, hMultiPurpose0);
        uint8_t val_1 = gb_read_hram(gb, hMultiPurpose1);
        gb_write_hram(gb, hMultiPurpose0, val_1);
        gb_write_hram(gb, hMultiPurpose1, val_0);
    }

    /* If dy < 0 (hMultiPurpose2 == 0), negate Y */
    if (gb_read_hram(gb, hMultiPurpose2) == 0) {
        uint8_t y_val = gb_read_hram(gb, hMultiPurpose0);
        gb_write_hram(gb, hMultiPurpose0, (uint8_t)(~y_val + 1));
    }

    /* If dx < 0 (hMultiPurpose3 != 0), negate X */
    if (gb_read_hram(gb, hMultiPurpose3) != 0) {
        uint8_t x_val = gb_read_hram(gb, hMultiPurpose1);
        gb_write_hram(gb, hMultiPurpose1, (uint8_t)(~x_val + 1));
    }

    if (val0) *val0 = gb_read_hram(gb, hMultiPurpose0);
    if (val1) *val1 = gb_read_hram(gb, hMultiPurpose1);
}

void GetVectorTowardsLink(GBState *gb, uint8_t *x, uint8_t *y) {
    if (!gb) return;
    uint8_t len = gb_read_hram(gb, hMultiPurpose1);
    if (len == 0) {
        len = 0x12; /* Default length */
    }
    GetVectorTowardsLink_with_length(gb, len, x, y);
}

/* ===== ApplyVectorTowardsLink (03:7EC7) ===== */
void ApplyVectorTowardsLink(GBState *gb, uint16_t bc) {
    if (!gb) return;
    GetVectorTowardsLink(gb, NULL, NULL);
    gb_write(gb, wEntitiesSpeedYTable + bc, gb_read_hram(gb, hMultiPurpose0));
    gb_write(gb, wEntitiesSpeedXTable + bc, gb_read_hram(gb, hMultiPurpose1));
}

/* ===== AddEntitySpeedToPos_03 (03:7F32) ===== */
void AddEntitySpeedToPos_03(GBState *gb, uint16_t bc) {
    if (!gb) return;
    uint8_t speed = gb_read(gb, wEntitiesSpeedXTable + bc);
    if (speed == 0) return;

    /* swap a; and $F0 */
    uint8_t speed_frac = (uint8_t)(((speed << 4) | (speed >> 4)) & 0xF0);
    uint8_t acc = gb_read(gb, wEntitiesSpeedXAccTable + bc);
    uint16_t sum = (uint16_t)acc + speed_frac;
    gb_write(gb, wEntitiesSpeedXAccTable + bc, (uint8_t)sum);
    uint8_t carry = (sum > 0xFF) ? 1 : 0;

    /* Sign extension for high nibble */
    uint8_t e = (speed & 0x80) ? 0xF0 : 0x00;
    uint8_t int_part = (uint8_t)((((speed << 4) | (speed >> 4)) & 0x0F) | e);

    uint8_t pos = gb_read(gb, wEntitiesPosXTable + bc);
    pos = (uint8_t)(pos + int_part + carry);
    gb_write(gb, wEntitiesPosXTable + bc, pos);
}

/* ===== AddEntityZSpeedToPos_03 (03:8790) ===== */
void AddEntityZSpeedToPos_03(GBState *gb, uint16_t bc) {
    if (!gb) return;
    uint8_t speed = gb_read(gb, wEntitiesSpeedZTable + bc);
    if (speed == 0) return;

    uint8_t speed_frac = (uint8_t)(((speed << 4) | (speed >> 4)) & 0xF0);
    uint8_t acc = gb_read(gb, wEntitiesSpeedZAccTable + bc);
    uint16_t sum = (uint16_t)acc + speed_frac;
    gb_write(gb, wEntitiesSpeedZAccTable + bc, (uint8_t)sum);
    uint8_t carry = (sum > 0xFF) ? 1 : 0;

    uint8_t e = (speed & 0x80) ? 0xF0 : 0x00;
    uint8_t int_part = (uint8_t)((((speed << 4) | (speed >> 4)) & 0x0F) | e);

    uint8_t pos = gb_read(gb, wEntitiesPosZTable + bc);
    pos = (uint8_t)(pos + int_part + carry);
    gb_write(gb, wEntitiesPosZTable + bc, pos);
}

/* ===== UpdateEntityPosWithSpeed_03 (03:8729) ===== */
void UpdateEntityPosWithSpeed_03(GBState *gb, uint16_t bc) {
    if (!gb) return;
    AddEntitySpeedToPos_03(gb, bc);
    AddEntitySpeedToPos_03(gb, (uint16_t)(bc + 0x10));
}

/* ===== ConfigureEntityRecoil (03:6FCC) ===== */
void ConfigureEntityRecoil(GBState *gb, uint16_t bc, uint8_t recoil_amount) {
    if (!gb) return;

    GetVectorTowardsLink_with_length(gb, recoil_amount, NULL, NULL);

    /* ldh a, [hMultiPurpose0]; cpl; inc a; ld hl, wEntitiesRecoilVelocityY; add hl, bc; ld [hl], a */
    uint8_t vec_y = gb_read_hram(gb, hMultiPurpose0);
    uint8_t recoil_y = (uint8_t)(~vec_y + 1);
    gb_write(gb, wEntitiesRecoilVelocityY + bc, recoil_y);

    /* ldh a, [hMultiPurpose1]; cpl; inc a; ld hl, wEntitiesRecoilVelocityX; add hl, bc; ld [hl], a */
    uint8_t vec_x = gb_read_hram(gb, hMultiPurpose1);
    uint8_t recoil_x = (uint8_t)(~vec_x + 1);
    gb_write(gb, wEntitiesRecoilVelocityX + bc, recoil_x);

    /* jp StartIgnoringHitsForEntity */
    StartIgnoringHitsForEntity_idx(gb, bc);
}

/* ===== ReturnIfNonInteractive_03 (03:7F78) ===== */
bool ReturnIfNonInteractive_03(GBState *gb, bool allow_inactive_entity) {
    if (!gb) return true;

    /* ldh a, [hActiveEntityStatus]; cp ENTITY_STATUS_ACTIVE; jr nz, .skip */
    if (!allow_inactive_entity) {
        uint8_t status = gb_read_hram(gb, hActiveEntityStatus);
        if (status != ENTITY_STATUS_ACTIVE) {
            return true;
        }
    }

    /* .allowInactiveEntity: ld a, [wGameplayType] */
    uint8_t gameplay_type = gb_read(gb, wGameplayType);

    /* cp GAMEPLAY_WORLD_MAP; jr z, .skip */
    if (gameplay_type == GAMEPLAY_WORLD_MAP) {
        return true;
    }

    /* cp GAMEPLAY_CREDITS; jr z, .creditsEnd */
    if (gameplay_type != GAMEPLAY_CREDITS) {
        /* cp GAMEPLAY_WORLD; jr nz, .skip */
        if (gameplay_type != GAMEPLAY_WORLD) {
            return true;
        }

        /* ld a, [wTransitionSequenceCounter]; cp $04; jr nz, .skip */
        if (gb_read(gb, wTransitionSequenceCounter) != 0x04) {
            return true;
        }
    }

    /* .creditsEnd: ld a, [wDialogState]; ld hl, wC1A8; or [hl]; ld hl, wInventoryAppearing; or [hl]; jr nz, .skip */
    uint8_t dialog_state = gb_read(gb, wDialogState);
    uint8_t c1a8 = gb_read(gb, wC1A8);
    uint8_t inventory_appearing = gb_read(gb, wInventoryAppearing);
    if ((dialog_state | c1a8 | inventory_appearing) != 0) {
        return true;
    }

    /* ld a, [wRoomTransitionState]; and a; jr z, .return */
    uint8_t room_transition = gb_read(gb, wRoomTransitionState);
    if (room_transition != 0) {
        return true;
    }

    /* .return */
    return false;
}

/* ===== ApplyRecoilIfNeeded_03 (03:7FA9) ===== */
void ApplyRecoilIfNeeded_03(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld a, [hl]; and a; ret z */
    uint8_t countdown = gb_read(gb, wEntitiesIgnoreHitsCountdownTable + bc);
    if (countdown == 0) {
        return;
    }

    /* dec a; ld [hl], a */
    countdown--;
    gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, countdown);

    /* call label_3E8E */
    label_3E8E(gb, bc);

    /* Temporarily replace entity speed by recoil speed */
    uint8_t orig_speed_x = gb_read(gb, wEntitiesSpeedXTable + bc);
    uint8_t orig_speed_y = gb_read(gb, wEntitiesSpeedYTable + bc);

    uint8_t recoil_x = gb_read(gb, wEntitiesRecoilVelocityX + bc);
    uint8_t recoil_y = gb_read(gb, wEntitiesRecoilVelocityY + bc);
    gb_write(gb, wEntitiesSpeedXTable + bc, recoil_x);
    gb_write(gb, wEntitiesSpeedYTable + bc, recoil_y);

    /* call UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);

    /* ld hl, wEntitiesOptions1Table; add hl, bc; ld a, [hl]; and ENTITY_OPT1_ALLOW_OUT_OF_BOUNDS; jr nz, .restoreOriginalSpeed */
    uint8_t options1 = gb_read(gb, wEntitiesOptions1Table + bc);
    if ((options1 & ENTITY_OPT1_ALLOW_OUT_OF_BOUNDS) == 0) {
        /* call ApplyEntityInteractionWithBackground */
        ApplyEntityInteractionWithBackground(gb, bc);
    }

    /* .restoreOriginalSpeed: restore original speeds */
    gb_write(gb, wEntitiesSpeedYTable + bc, orig_speed_y);
    gb_write(gb, wEntitiesSpeedXTable + bc, orig_speed_x);

    /* call StopEntityRecoilOnCollision */
    StopEntityRecoilOnCollision(gb, bc);
}

/* ===== CheckForEntityFallingDownQuicksandHole (03:5CEA) ===== */
bool CheckForEntityFallingDownQuicksandHole(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    /* ld a, [wIsIndoor]; and a; jr nz, .jr_5D34 */
    if (gb_read(gb, wIsIndoor) != 0) {
        return false;
    }

    /* ldh a, [hMapRoom]; cp ROOM_OW_YARNA_LANMOLA; jr nz, .jr_5D34 */
    if (gb_read_hram(gb, hMapRoom) != ROOM_OW_YARNA_LANMOLA) {
        return false;
    }

    /* ldh a, [hActiveEntityPosY]; sub $48; add $03; cp $06; jr nc, .jr_5D34 */
    uint8_t diff_y = (uint8_t)(gb_read_hram(gb, hActiveEntityPosY) - 0x48 + 0x03);
    if (diff_y >= 0x06) {
        return false;
    }

    /* ldh a, [hActiveEntityPosX]; sub $50; add $03; cp $06; jr nc, .jr_5D34 */
    uint8_t diff_x = (uint8_t)(gb_read_hram(gb, hActiveEntityPosX) - 0x50 + 0x03);
    if (diff_x >= 0x06) {
        return false;
    }

    /* ld hl, wEntitiesPosZTable; add hl, bc; ld a, [hl]; and a; jr nz, .jr_5D34 */
    if (gb_read(gb, (uint16_t)(wEntitiesPosZTable + bc)) != 0) {
        return false;
    }

    /* ld hl, wEntitiesStatusTable; add hl, bc; ld a, [hl]; cp $05; jr nz, .jr_5D34 */
    if (gb_read(gb, (uint16_t)(wEntitiesStatusTable + bc)) != ENTITY_STATUS_ACTIVE) {
        return false;
    }

    /* ld [hl], $02 */
    gb_write(gb, (uint16_t)(wEntitiesStatusTable + bc), ENTITY_STATUS_FALLING);

    /* ld hl, wEntitiesFallingTargetXTable; add hl, bc; ld [hl], $50 */
    gb_write(gb, (uint16_t)(wEntitiesFallingTargetXTable + bc), 0x50);

    /* ld hl, wEntitiesFallingTargetYTable; add hl, bc; ld [hl], $48 */
    gb_write(gb, (uint16_t)(wEntitiesFallingTargetYTable + bc), 0x48);

    /* call GetEntityTransitionCountdown; ld [hl], $2F */
    GetEntityTransitionCountdown(gb, bc);
    gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0x2F);

    /* ld a, JINGLE_ITEM_FALLING; ldh [hJingle], a; scf; ret */
    gb_write_hram(gb, hJingle, JINGLE_ITEM_FALLING);
    return true;
}
