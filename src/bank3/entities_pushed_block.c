#include "bank3/entities_pushed_block.h"
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
#include "home/link.h"
#include "constants/audio.h"

/* ===== Data Tables (03:515E-03:5245) ===== */

/* Unknown011SpriteVariants (03:5235) - 4 variants * 4 bytes = 16 bytes */
static const uint8_t Unknown011SpriteVariants[16] = {
    0x6E, 0xC7,  0x6E, 0xC7 | OAMF_XFLIP,  /* variant 0 */
    0xF8, 0xE7,  0xFA, 0xE7               /* variant 1 */
};

/* Unknown010SpriteVariants (03:5245) - 2 variants * 4 bytes = 8 bytes */
static const uint8_t Unknown010SpriteVariants[8] = {
    0x7E, 0xC7,  0x7E, 0xC7 | OAMF_XFLIP
};

/* Data_003_515E (03:515E) */
static const uint8_t Data_003_515E[4] = { 0xF8, 0xF9, 0xFA, 0xFB };

/* Data_003_5162 (03:5162) */
static const uint8_t Data_003_5162[4] = { 0x0E, 0x1E, 0x0F, 0x1F };

/* ===== PushedBlockEntityHandler (03:5249) ===== */
void PushedBlockEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wIsIndoor]; ldh [hActiveEntitySpriteVariant], a */
    uint8_t is_indoor = gb_read(gb, wIsIndoor);
    gb_write_hram(gb, hActiveEntitySpriteVariant, is_indoor);

    /* ld de, Unknown011SpriteVariants; and a; jr nz, .render */
    const uint8_t *de = Unknown011SpriteVariants;
    if (is_indoor != 0) {
        goto render;
    }

    /* ldh a, [hMapRoom]; cp ROOM_OW_COLOR_DUNGEON_ENTRANCE; jr nz, .render */
    if (gb_read_hram(gb, hMapRoom) == ROOM_OW_COLOR_DUNGEON_ENTRANCE) {
        de = Unknown010SpriteVariants;
    }

render:
    /* call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, de, NULL);

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* call UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);

    /* call func_003_52D4 */
    func_003_52D4(gb, bc);

    /* call CheckLinkCollisionWithEnemy.collisionEvenInTheAir; jr nc, .jr_5276 */
    if (CheckLinkCollisionWithEnemy(gb, bc)) {
        /* call CopyLinkFinalPositionToPosition */
        CopyLinkFinalPositionToPosition(gb);

        /* ld a, $03; ld [wIsLinkPushing], a */
        gb_write(gb, wIsLinkPushing, 0x03);
    }

    /* .jr_5276: ldh a, [hMapRoom]; cp UNKNOWN_ROOM_C7; jr z, .jr_5282 */
    uint8_t map_room = gb_read_hram(gb, hMapRoom);
    if (map_room == UNKNOWN_ROOM_C7) {
        goto jr_5282;
    }

    /* ld a, [wIsIndoor]; and a; jr nz, jr_003_5286 */
    if (gb_read(gb, wIsIndoor) != 0) {
        goto jr_003_5286;
    }

jr_5282: ;
    /* ld a, $02; ldh [hLinkInteractiveMotionBlocked], a */
    gb_write_hram(gb, hLinkInteractiveMotionBlocked, 0x02);

jr_003_5286: ;
    /* ld hl, wEntitiesInertiaTable; add hl, bc; ld a, [hl]; inc a; ld [hl], a; cp $21; ret nz */
    uint8_t inertia = gb_read(gb, wEntitiesInertiaTable + bc);
    inertia++;
    gb_write(gb, wEntitiesInertiaTable + bc, inertia);
    if (inertia != 0x21) {
        return;
    }

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, inertia);

    /* call ApplyEntityInteractionWithBackground */
    ApplyEntityInteractionWithBackground(gb, bc);

    /* ld hl, wEntitiesStatusTable; add hl, bc; ld a, [hl]; and a; ret z */
    uint8_t status = gb_read(gb, wEntitiesStatusTable + bc);
    if (status == 0) {
        return;
    }

    /* cp $02; ret z */
    if (status == 0x02) {
        return;
    }

    /* call UnloadEntity */
    UnloadEntity(gb, bc);

    /* ld de, Data_003_5162; ld b, $C4 */
    /* ld a, [wIsIndoor]; and a; jr z, .jr_52B5 */
    /* ld de, Data_003_515E; ld b, $A6 */
    const uint8_t *data_ptr = Data_003_5162;
    uint8_t b_val = 0xC4;
    if (gb_read(gb, wIsIndoor) != 0) {
        data_ptr = Data_003_515E;
        b_val = 0xA6;
    }

    /* call func_003_51C9 */
    func_003_51C9(gb, bc, data_ptr, b_val);

    /* ld a, [wRoomEvent]; and EVENT_TRIGGER_MASK; cp TRIGGER_PUSH_SINGLE_BLOCK; jr z, .jr_52D1 */
    uint8_t room_event = gb_read(gb, wRoomEvent) & EVENT_TRIGGER_MASK;
    if (room_event == TRIGGER_PUSH_SINGLE_BLOCK) {
        goto jr_52D1;
    }

    /* cp TRIGGER_PUSH_BLOCKS; ret nz */
    if (room_event != TRIGGER_PUSH_BLOCKS) {
        return;
    }

    /* call ApplyEntityInteractionWithBackground */
    ApplyEntityInteractionWithBackground(gb, bc);

    /* ld a, [wEntityHorizontallyCollidedObject]; cp $A7; jr z, .jr_52D1 */
    uint8_t collided_obj = gb_read(gb, wEntityHorizontallyCollidedObject);
    if (collided_obj == 0xA7) {
        goto jr_52D1;
    }

    /* cp $A6; ret nz */
    if (collided_obj != 0xA6) {
        return;
    }

jr_52D1:
    /* jp MarkTriggerAsResolved */
    MarkTriggerAsResolved(gb);
}

/* ===== func_003_52D4 (03:52D4) ===== */
void func_003_52D4(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld e, $0F; ld d, b; label_003_52D7: */
    uint8_t e = 0x0F;
    uint8_t d = (bc >> 8) & 0xFF;

    while (true) {
        /* ld hl, wEntitiesStatusTable; add hl, de; ld a, [hl]; cp $05; jr c, .jr_531E */
        uint8_t status = gb_read(gb, wEntitiesStatusTable + e);
        if (status < 0x05) {
            goto jr_531E;
        }

        /* ld hl, wEntitiesPhysicsFlagsTable; add hl, de; ld a, [hl]; and ENTITY_PHYSICS_PROJECTILE_NOCLIP; jr nz, .jr_531E */
        uint8_t physics = gb_read(gb, wEntitiesPhysicsFlagsTable + e);
        if (physics & ENTITY_PHYSICS_PROJECTILE_NOCLIP) {
            goto jr_531E;
        }

        /* ld hl, wEntitiesPosXTable; add hl, de; ldh a, [hActiveEntityPosX]; sub [hl]; add $0C; cp $18; jr nc, .jr_531E */
        uint8_t entity_pos_x = gb_read(gb, wEntitiesPosXTable + e);
        uint8_t active_pos_x = gb_read_hram(gb, hActiveEntityPosX);
        int16_t diff_x = (int16_t)active_pos_x - entity_pos_x + 0x0C;
        if ((uint8_t)diff_x >= 0x18) {
            goto jr_531E;
        }

        /* ld hl, wEntitiesPosYTable; add hl, de; ld a, [hl]; ld hl, wEntitiesPosZTable; add hl, de; sub [hl]; ld hl, hActiveEntityVisualPosY; sub [hl]; add $0C; cp $18; jr nc, .jr_531E */
        uint8_t entity_pos_y = gb_read(gb, wEntitiesPosYTable + e);
        uint8_t entity_pos_z = gb_read(gb, wEntitiesPosZTable + e);
        uint8_t visual_pos_y = gb_read_hram(gb, hActiveEntityVisualPosY);
        int16_t diff_y = (int16_t)entity_pos_y - entity_pos_z - visual_pos_y + 0x0C;
        if ((uint8_t)diff_y >= 0x18) {
            goto jr_531E;
        }

        /* ld hl, wEntitiesPhysicsFlagsTable; add hl, de; ld a, [hl]; and ENTITY_PHYSICS_GRABBABLE; jr nz, .jr_531E */
        if (physics & ENTITY_PHYSICS_GRABBABLE) {
            goto jr_531E;
        }

        /* push bc; ld c, e; ld b, d; push de; ld a, $08; call ConfigureEntityRecoil; pop de; pop bc */
        uint16_t other_bc = ((uint16_t)d << 8) | e;
        ConfigureEntityRecoil(gb, other_bc, 0x08);

jr_531E:
        /* dec e; ld a, e; cp $FF; jp nz, label_003_52D7 */
        if (e == 0) {
            break;
        }
        e--;
    }
}

/* ===== func_003_51C9 (03:51C9) ===== */
void func_003_51C9(GBState *gb, uint16_t bc, const uint8_t *data_ptr, uint8_t b_val) {
    if (!gb || !data_ptr) return;
    (void)bc;

    /* ldh a, [hActiveEntityPosY]; sub $0F; ldh [hIntersectedObjectTop], a */
    uint8_t top = (uint8_t)(gb_read_hram(gb, hActiveEntityPosY) - 0x0F);
    gb_write_hram(gb, hIntersectedObjectTop, top);

    /* ldh a, [hActiveEntityPosX]; sub $07; ldh [hIntersectedObjectLeft], a */
    uint8_t left = (uint8_t)(gb_read_hram(gb, hActiveEntityPosX) - 0x07);
    gb_write_hram(gb, hIntersectedObjectLeft, left);

    /* swap a; and $0F; ld e, a; ldh a, [hIntersectedObjectTop]; and $F0; or e; ld e, a; ld d, $00 */
    uint8_t de_offset = (uint8_t)((top & 0xF0) | ((left >> 4) & 0x0F));
    uint16_t room_obj_addr = (uint16_t)(wRoomObjects + de_offset);

    /* ld hl, wRoomObjects; add hl, de; pop af; ld [hl], a; ld [wDDD8], a; ld a, $03; call BackupObjectInRAM2 */
    gb_write(gb, room_obj_addr, b_val);
    gb_write(gb, wDDD8, b_val);
    BackupObjectInRAM2(gb, room_obj_addr, 0x03);

    /* label_003_51F5: call label_2887 */
    GetIntersectedObjectBGAddress(gb);

    /* ld a, [wDrawCommandsSize]; ld e, a; ld d, $00; ld hl, wDrawCommand; add hl, de; add $0A; ld [wDrawCommandsSize], a */
    uint8_t size = gb_read(gb, wDrawCommandsSize);
    uint16_t hl = (uint16_t)(wDrawCommand + size);
    gb_write(gb, wDrawCommandsSize, (uint8_t)(size + 10));

    /* ldh a, [hIntersectedObjectBGAddressHigh]; ld [hl+], a */
    /* ldh a, [hIntersectedObjectBGAddressLow]; ld [hl+], a */
    /* ld a, $81; ld [hl+], a */
    /* ld a, [de]; inc de; ld [hl+], a */
    /* ld a, [de]; inc de; ld [hl+], a */
    uint8_t bg_high = gb_read_hram(gb, hIntersectedObjectBGAddressHigh);
    uint8_t bg_low = gb_read_hram(gb, hIntersectedObjectBGAddressLow);

    gb_write(gb, hl++, bg_high);
    gb_write(gb, hl++, bg_low);
    gb_write(gb, hl++, 0x81);
    gb_write(gb, hl++, data_ptr[0]);
    gb_write(gb, hl++, data_ptr[1]);

    /* ldh a, [hIntersectedObjectBGAddressHigh]; ld [hl+], a */
    /* ldh a, [hIntersectedObjectBGAddressLow]; inc a; ld [hl+], a */
    /* ld a, $81; ld [hl+], a */
    /* ld a, [de]; inc de; ld [hl+], a */
    /* ld a, [de]; inc de; ld [hl+], a */
    /* xor a; ld [hl], a */
    gb_write(gb, hl++, bg_high);
    gb_write(gb, hl++, (uint8_t)(bg_low + 1));
    gb_write(gb, hl++, 0x81);
    gb_write(gb, hl++, data_ptr[2]);
    gb_write(gb, hl++, data_ptr[3]);
    gb_write(gb, hl, 0x00);

    /* ldh a, [hIsGBC]; and a; jr z, .ret_5234; push bc; ld a, $03; call func_91D; pop bc */
    if (gb_read_hram(gb, hIsGBC) != 0) {
        func_91D(gb, 0x03, NULL);
    }
}