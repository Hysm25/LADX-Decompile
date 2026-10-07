#include "bank2/items.h"
#include "constants/dialog.h"
#include "constants/entities.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/link.h"
#include "constants/maps.h"
#include "constants/memory.h"
#include "constants/physics.h"
#include "constants/rooms.h"
#include "constants/sfx.h"
#include "constants/tilesets.h"
#include "home/animated_tiles.h"
#include "home/check_items_to_use.h"
#include "home/copy_data.h"
#include "home/entities.h"
#include "home/link.h"
#include "home/room.h"

const int8_t HookshotChainSpeedX[4] = {
    0x30,  /* DIRECTION_RIGHT:  HOOKSHOT_CHAIN_SPEED ($30) */
    -0x30, /* DIRECTION_LEFT:  -HOOKSHOT_CHAIN_SPEED (-$30) */
    0x00,  /* DIRECTION_UP:     0 */
    0x00   /* DIRECTION_DOWN:   0 */
};

const int8_t HookshotChainSpeedY[4] = {
    0x00,  /* DIRECTION_RIGHT:  0 */
    0x00,  /* DIRECTION_LEFT:   0 */
    -0x30, /* DIRECTION_UP:    -HOOKSHOT_CHAIN_SPEED (-$30) */
    0x30   /* DIRECTION_DOWN:   HOOKSHOT_CHAIN_SPEED ($30) */
};

bool FireHookshot(GBState *gb) {
    if (!gb) return false;

    /* ld a, [wIsLinkInTheAir]; and a; ret nz */
    if (gb_read(gb, wIsLinkInTheAir) != 0) {
        return false;
    }

    /* Spawn hookshot chain projectile */
    uint8_t slot = 0;
    if (!SpawnPlayerProjectile(gb, ENTITY_HOOKSHOT_CHAIN, &slot)) {
        return false;
    }

    /* Transition countdown = $2A; variant = 0 */
    gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + slot), 0x2A);
    gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + slot), 0);

    /* Assign directional velocity vectors */
    uint8_t dir = gb_read_hram(gb, hLinkDirection) & 0x03;
    gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + slot), (uint8_t)HookshotChainSpeedX[dir]);
    gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + slot), (uint8_t)HookshotChainSpeedY[dir]);

    return true;
}

uint8_t func_002_523A(GBState *gb, uint16_t bc, uint8_t e) {
    if (gb) gb_write(gb, bc, e);
    return 0x14;
}

uint8_t func_002_523F(GBState *gb, uint16_t bc, uint8_t e) {
    if (gb) gb_write(gb, bc, e);
    return 0x14;
}

uint8_t func_002_524A(GBState *gb, uint16_t bc, uint8_t e) {
    if (gb) gb_write(gb, bc, e);
    return 0x10;
}

void HandleGotItemB(GBState *gb,
                    void (*apply_motion_state)(GBState *),
                    void (*func_020_4ab3)(GBState *)) {
    if (!gb) return;

    ResetSpinAttack(gb);
    gb_write(gb, wC16A, 0x00);
    gb_write(gb, wSwordAnimationState, 0x00);
    gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x00);

    if (apply_motion_state) {
        apply_motion_state(gb);
    } else {
        ApplyLinkMotionState(gb, NULL, NULL, NULL);
    }

    func_21E1(gb);

    int8_t vz = (int8_t)(gb_read_hram(gb, hLinkVelocityZ) - 2);
    gb_write_hram(gb, hLinkVelocityZ, (uint8_t)vz);

    uint8_t pz = gb_read_hram(gb, hLinkPositionZ);
    if ((pz & 0x80) != 0) {
        gb_write_hram(gb, hLinkPositionZ, 0x00);
        gb_write(gb, wC149, 0x00);
        gb_write_hram(gb, hLinkVelocityZ, 0x00);
    }

    gb_write_hram(gb, hLinkAnimationState, LINK_ANIMATION_STATE_UNKNOWN_6B);

    uint16_t bc = (uint16_t)(wLinkOAMBuffer + 0x10);
    uint8_t py = gb_read_hram(gb, hLinkPositionY);
    pz = gb_read_hram(gb, hLinkPositionZ);
    uint8_t c13b = gb_read(gb, wC13B);
    uint8_t multi0 = (uint8_t)(py - pz + c13b - 0x10);
    gb_write_hram(gb, hMultiPurpose0, multi0);

    uint8_t got_item = gb_read(gb, wDialogGotItem);
    if (got_item == DIALOG_GOT_PIECE_OF_POWER) {
        gb_write_hram(gb, hMultiPurpose1, (uint8_t)(gb_read_hram(gb, hLinkPositionX) - 0x08));
        uint8_t fc = gb_read_hram(gb, hFrameCounter);
        gb_write_hram(gb, hMultiPurpose3, (uint8_t)((fc << 2) & 0x10));
        gb_write_hram(gb, hMultiPurpose2, 0x06);
        func_1819(gb, func_020_4ab3);
        return;
    }

    gb_write(gb, bc++, (uint8_t)(multi0 + 0x02));
    gb_write(gb, bc++, gb_read_hram(gb, hLinkPositionX));

    uint8_t tile;
    uint8_t attr;
    if (got_item == DIALOG_GOT_GUARDIAN_ACORN) {
        tile = 0xAE;
        attr = func_002_523A(gb, bc, tile);
    } else if (got_item == DIALOG_GOT_MAGIC_POWDER) {
        tile = 0x8E;
        gb_write(gb, bc, tile);
        attr = 0x16;
    } else if (got_item == DIALOG_GOT_ROD) {
        tile = 0x8C;
        attr = func_002_524A(gb, bc, tile);
    } else {
        tile = 0x8E;
        attr = func_002_523F(gb, bc, tile);
    }

    bc++;
    gb_write(gb, bc, attr);
}

void HandleGotItemA(GBState *gb,
                    void (*apply_motion_state)(GBState *),
                    void (*func_020_4ab3)(GBState *)) {
    if (!gb) return;

    if (gb_read(gb, wDialogGotItemCountdown) == 0x2E) {
        gb_write_hram(gb, hJingle, JINGLE_GOT_POWER_UP);
    }

    HandleGotItemB(gb, apply_motion_state, func_020_4ab3);
}

/* Bank 2 Magic Rod Visuals & Attack Sprites (02:52E0-02:53AF) */

const int8_t LinkDirectionToMagicRodXOffset[8] = {
    0x0D, (int8_t)0xF3, 0x00, (int8_t)0xFF,
    0x08, (int8_t)0xF8, 0x0C, (int8_t)0xF5
};

const int8_t LinkDirectionToMagicRodYOffset[8] = {
    0x00, 0x00, (int8_t)0xF3, 0x0E,
    (int8_t)0xF3, (int8_t)0xF3, (int8_t)0xFC, 0x00
};

const uint8_t LinkDirectionToMagicRodTiles[16] = {
    0x06, 0x08, 0x08, 0x06, 0x04, 0xFF, 0xFF, 0x04,
    0x04, 0xFF, 0xFF, 0x04, 0x06, 0x08, 0x08, 0x06
};

const uint8_t LinkDirectionToMagicRodOAMAttributes[16] = {
    0x02, 0x02, 0x22, 0x22, 0x22, 0x02, 0x02, 0x42,
    0x22, 0x02, 0x02, 0x22, 0x02, 0x02, 0x22, 0x22
};

const int8_t LinkDirectionToEntitiesPositionX[4] = {
    4, -4, -4, 4
};

const int8_t LinkDirectionToEntitiesPositionY[4] = {
    4, 4, -4, 4
};

static const uint8_t data_13AD[8] = {
    0x30, 0xD0, 0x00, 0x00,
    0x40, 0xC0, 0x00, 0x00
};

static const uint8_t data_13B5[8] = {
    0x00, 0x00, 0xD0, 0x30,
    0x00, 0x00, 0xC0, 0x40
};

void label_002_5310(GBState *gb) {
    if (!gb) return;

    uint8_t countdown = gb_read(gb, wLinkAttackStepAnimationCountdown) & ATTACK_STEP_DURATION_MASK;
    uint8_t dir = gb_read_hram(gb, hLinkDirection);
    if (countdown >= 0x08) {
        dir += 0x04;
    }
    uint8_t offset = dir & 0x07;

    uint8_t y_offset = (uint8_t)LinkDirectionToMagicRodYOffset[offset];
    gb_write_hram(gb, hMultiPurpose0, y_offset);

    uint8_t x_offset = (uint8_t)LinkDirectionToMagicRodXOffset[offset];
    gb_write_hram(gb, hMultiPurpose1, x_offset);

    uint8_t tile0 = LinkDirectionToMagicRodTiles[offset * 2];
    gb_write_hram(gb, hMultiPurpose2, tile0);

    uint8_t tile1 = LinkDirectionToMagicRodTiles[offset * 2 + 1];
    gb_write_hram(gb, hMultiPurpose3, tile1);

    uint8_t attr0 = LinkDirectionToMagicRodOAMAttributes[offset * 2];
    gb_write_hram(gb, hMultiPurpose4, attr0);

    uint8_t attr1 = LinkDirectionToMagicRodOAMAttributes[offset * 2 + 1];
    gb_write_hram(gb, hMultiPurpose5, attr1);

    uint16_t de = wLinkOAMBuffer + 0x10;
    uint16_t bc = wLinkOAMBuffer + 0x14;

    uint8_t final_y = (uint8_t)(gb_read(gb, wC145) + gb_read(gb, wC13B) + y_offset);
    gb_write_hram(gb, hMultiPurpose0, final_y);

    if (tile0 != 0xFF) {
        gb_write(gb, de, final_y);
    }
    if (tile1 != 0xFF) {
        gb_write(gb, bc, final_y);
    }

    de++;
    bc++;

    uint8_t pos_x = (uint8_t)(x_offset + gb_read_hram(gb, hLinkPositionX));
    gb_write(gb, de, pos_x);
    gb_write(gb, bc, (uint8_t)(pos_x + 0x08));

    de++;
    bc++;

    gb_write(gb, de, tile0);
    gb_write(gb, bc, tile1);

    de++;
    bc++;

    gb_write(gb, de, attr0);
    gb_write(gb, bc, attr1);
}

void label_002_538B_entity(GBState *gb, uint8_t de) {
    if (!gb) return;

    uint8_t dir = gb_read_hram(gb, hLinkDirection) & 0x03;
    uint8_t pos_x = (uint8_t)(gb_read_hram(gb, hLinkPositionX) + (uint8_t)LinkDirectionToEntitiesPositionX[dir]);
    gb_write(gb, (uint16_t)(wEntitiesPosXTable + de), pos_x);

    uint8_t pos_y = (uint8_t)(gb_read_hram(gb, hLinkPositionY) + (uint8_t)LinkDirectionToEntitiesPositionY[dir]);
    gb_write(gb, (uint16_t)(wEntitiesPosYTable + de), pos_y);

    gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + de), 0x00);

    /* label_140F */
    uint8_t offset = dir;
    if (gb_read(gb, wActivePowerUp) == POWER_UP_PIECE_OF_POWER) {
        offset += 4;
    }
    gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + de), data_13AD[offset]);
    gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + de), data_13B5[offset]);
}

void label_002_538B(GBState *gb) {
    label_002_538B_entity(gb, 0);
}

/* ClampItemCount (02:60D8-02:60DF)
 * Clamps the item count at de to the maximum value at hl.
 * If [de] >= [hl], sets [de] = [hl]. Then increments hl and returns.
 * Inputs:
 *   hl - address of maximum item count
 *   de - address of current item count
 */
void ClampItemCount(GBState *gb, uint16_t hl, uint16_t de) {
    if (!gb) return;
    uint8_t max_val = gb_read(gb, hl);
    uint8_t cur_val = gb_read(gb, de);
    if (cur_val >= max_val) {
        gb_write(gb, de, max_val);
    }
    /* hl is incremented in the assembly (inc hl) but not used after.
     * The C signature does not return hl, matching the assembly's
     * register modification convention. */
}

/* BCD addition helper: adds a value to a BCD byte, returns result */
static uint8_t bcd_add(uint8_t a, uint8_t b) {
    uint16_t result = a + b;
    if ((result & 0x0F) > 0x09) result += 0x06;
    if (result > 0x99) result += 0x60;
    return (uint8_t)result;
}

/* BCD subtraction helper: subtracts a value from a BCD byte, returns result */
static uint8_t bcd_sub(uint8_t a, uint8_t b) {
    if (a < b) {
        /* Underflow - will be handled by caller */
        return (uint8_t)(a + 0x100 - b);
    }
    uint16_t result = a - b;
    if ((a & 0x0F) < (b & 0x0F)) result -= 0x06;
    return (uint8_t)result;
}

/* Data_002_66F9 - Tile mapping for revealed minimap rooms (02:66F9) */
static const uint8_t Data_002_66F9[16] = {
    0x00, 0x02, 0x03, 0x07, 0x05, 0x0A, 0x0B, 0x0F,
    0x04, 0x08, 0x09, 0x0E, 0x06, 0x0C, 0x0D, 0x01
};

/* LoadMinimap (02:6709-02:67E4)
 * Loads the dungeon minimap into wDungeonMinimap ($D480-$D4BF).
 * Handles special cases for Color Dungeon, Eagle's Tower (collapsed),
 * and Evil Eagle's boss room. Updates visited rooms based on dungeon
 * map/compass status and room status flags. On GBC, copies palette data.
 *
 * @param gb Pointer to Game Boy system state.
 */
void LoadMinimap(GBState *gb) {
    if (!gb) return;

    /* Check if this is Evil Eagle's boss room (Indoor B room E8)
     * If so, don't load the minimap */
    uint8_t map_room = gb_read_hram(gb, hMapRoom);
    if (map_room == ROOM_INDOOR_B_EAGLES_TOWER_BOSS) {
        return;
    }

    /* Select minimap table based on map ID */
    uint16_t hl = MinimapsTable;
    uint8_t map_id = gb_read_hram(gb, hMapId);

    if (map_id == MAP_COLOR_DUNGEON) {
        hl = ColorDungeonMinimap;
    } else {
        /* Compute minimap address: map_id * 0x40 (swap + 2x sla/rl) */
        uint8_t a = map_id;
        a = (a << 4) | (a >> 4);  /* swap a */
        uint16_t de = ((uint16_t)a) * 4;  /* sla e; rl d; sla e; rl d */
        hl += de;
    }

    /* Special case: Eagle's Tower collapsed */
    if (map_id == MAP_EAGLES_TOWER) {
        uint8_t has_instrument7 = gb_read(gb, wHasInstrument7);
        if ((has_instrument7 & 0x04) != 0) {
            hl = EaglesTowerCollapsedMinimap;
        }
    }

    /* Copy minimap data (0x40 bytes) to wDungeonMinimap */
    uint16_t de = wDungeonMinimap;
    uint16_t bc = 0x0040;
    CopyData(gb, hl, de, bc);

    /* Process each of the 64 minimap rooms */
    de = 0;
    for (uint8_t e = 0; e < 0x40; e++) {
        uint8_t a = gb_read(gb, wDungeonMinimap + e);

        if (a == 0x7D) {  /* Blank - not shown on map */
            continue;
        }

        if (a == 0xED || a == 0xEE) {  /* Chest room or Nightmare marker */
            uint8_t has_compass = gb_read(gb, wHasDungeonCompass);
            if (has_compass == 0) {
                gb_write(gb, wDungeonMinimap + e, 0xEF);  /* Mark as regular room */
                continue;
            }
            /* Fall through to room status check */
        } else {
            uint8_t has_map = gb_read(gb, wHasDungeonMap);
            if (has_map == 0) {
                gb_write(gb, wDungeonMinimap + e, 0x7D);  /* Hide room */
                continue;
            }
        }

        /* Get room status for this map position */
        uint16_t de_temp = e;
        uint16_t room_status_addr = GetRoomStatusAddressForMapPosition(gb, de_temp);
        uint8_t status = gb_read(gb, room_status_addr);

        if ((status & 0x80) == 0) {  /* Room not visited */
            continue;
        }

        /* Room visited - update tile based on status */
        uint8_t c = status & 0x0F;
        uint8_t tile = Data_002_66F9[c];
        tile++;  /* inc a */
        tile += 0xCF;  /* add $CF */
        c = tile;

        uint8_t current = gb_read(gb, wDungeonMinimap + e);
        if (current == 0xEE || current == 0xED) {
            /* Chest or Nightmare room - check bit 4 or bit 5 of status */
            uint16_t de_temp2 = e;
            room_status_addr = GetRoomStatusAddressForMapPosition(gb, de_temp2);
            status = gb_read(gb, room_status_addr);
            uint8_t bit_mask = (current == 0xED) ? 0x10 : 0x20;
            if ((status & bit_mask) == 0) {
                continue;
            }
        }

        /* Update minimap tile */
        gb_write(gb, wDungeonMinimap + e, c);

        /* If no dungeon map, show as unexplored (0x7D) */
        if (gb_read(gb, wHasDungeonMap) == 0) {
            gb_write(gb, wDungeonMinimap + e, 0x7D);
        }
    }

    /* GBC palette handling */
    if (gb_read_hram(gb, hIsGBC) != 0) {
        for (uint8_t e = 0; e < 0x40; e++) {
            uint8_t d = 0x01;
            gb_write(gb, rSVBK, 0x00);
            uint8_t tile = gb_read(gb, wDungeonMinimap + e);
            if (tile == 0xED) {
                d = 0x06;
            }
            gb_write(gb, rSVBK, 0x02);
            gb_write(gb, wDungeonMinimap + e, d);
        }

        gb_write(gb, rSVBK, 0x00);
    }
}

/* func_002_753A (02:753A-02:754E)
 * Updates wC13B when Link is swimming, then checks hookshot state
 * and falls through to func_002_754F.
 *
 * @param gb Pointer to Game Boy system state.
 */
void func_002_753A(GBState *gb) {
    if (!gb) return;

    /* ld a, [wLinkMotionState]; cp LINK_MOTION_SWIMMING; jr nz, .jr_7549 */
    if (gb_read(gb, wLinkMotionState) == LINK_MOTION_SWIMMING) {
        /* ld a, [wC13B]; add $04; ld [wC13B], a */
        uint8_t c13b = gb_read(gb, wC13B);
        gb_write(gb, wC13B, (uint8_t)(c13b + 0x04));
    }

    /* ld a, [wIsUsingHookshot]; and a; jr z, jr_002_7587 */
    if (gb_read(gb, wIsUsingHookshot) != 0) {
        func_002_754F(gb);
    }
}

/* func_002_754F (02:754F-02:755A)
 * Checks if Link is in the air or using Pegasus boots, clears position
 * increment if not, then falls through to func_002_755B.
 *
 * @param gb Pointer to Game Boy system state.
 */
void func_002_754F(GBState *gb) {
    if (!gb) return;

    /* ld hl, wIsLinkInTheAir; ld a, [wIsRunningWithPegasusBoots]; or [hl]; jr nz, func_002_755B */
    if (gb_read(gb, wIsLinkInTheAir) != 0 || gb_read(gb, wIsRunningWithPegasusBoots) != 0) {
        func_002_755B(gb);
        return;
    }

    /* call ClearLinkPositionIncrement */
    ClearLinkPositionIncrement(gb);

    /* fallthrough to func_002_755B */
    func_002_755B(gb);
}

/* func_002_755B (02:755B-02:7586)
 * Gets the object under Link and determines a value for wC13B
 * based on the object type and Link's state. Called when getting an item
 * and for showing location on the minimap.
 *
 * @param gb Pointer to Game Boy system state.
 */
void func_002_755B(GBState *gb) {
    if (!gb) return;

    /* Get object under Link */
    uint8_t obj_id = GetObjectUnderLink(gb);

    /* Default value for wC13B */
    uint8_t c = 0x04;

    /* Check wD463 */
    if (gb_read(gb, wD463) == 0x01) {
        goto write_wC13B;
    }

    /* Check if standing on switch block */
    c = 0xFC;  /* -4 */
    if (gb_read(gb, wLinkStandingOnSwitchBlock) != 0) {
        goto write_wC13B;
    }

    /* Get object physics flags */
    uint8_t physics = GetObjectPhysicsFlags_trampoline(gb, obj_id);
    c = 0x02;

    if (physics == OBJ_PHYSICS_SHALLOW_WATER) {  /* $05 */
        goto write_wC13B;
    }
    if (physics == OBJ_PHYSICS_RAISED) {  /* $09 */
        goto write_wC13B;
    }
    if (physics == OBJ_PHYSICS_LOWERED) {  /* $08 */
        c = 0xFD;  /* -3 */
        goto write_wC13B;
    }

    /* Default case - return without writing wC13B */
    return;

write_wC13B:
    gb_write(gb, wC13B, c);
}

/* func_002_61BA (02:61BA)
 * Helper called during subscreen scrolling.
 */
void func_002_61BA(GBState *gb) {
    if (!gb) return;
    func_002_755B(gb);
    ApplyLinkMotionState(gb, NULL, NULL, NULL);
    DrawLinkSpriteAndReturn(gb);
    AnimateEntitiesAndRestoreBank02(gb, NULL);
}

/* LoadRupeesDigits (02:62CE-02:6413)
 * Loads the rupees digit tiles for the current rupees count.
 */
void LoadRupeesDigits(GBState *gb) {
    if (!gb) return;

    uint8_t a = gb_read(gb, wDrawCommandsSize);
    uint8_t e = a;
    a = (uint8_t)(a + 0x06);
    gb_write(gb, wDrawCommandsSize, a);

    uint16_t hl = (uint16_t)(wDrawCommand + e);
    gb_write(gb, hl++, 0x9C);
    gb_write(gb, hl++, 0x2A);
    gb_write(gb, hl++, 0x02);

    uint8_t rupee_high = gb_read(gb, wRupeeCountHigh);
    gb_write(gb, hl++, (uint8_t)((rupee_high & 0x0F) + 0xB0));

    uint8_t rupee_low = gb_read(gb, wRupeeCountLow);
    gb_write(gb, hl++, (uint8_t)(((rupee_low >> 4) & 0x0F) + 0xB0));
    gb_write(gb, hl++, (uint8_t)((rupee_low & 0x0F) + 0xB0));
}

/* UpdateRupeesCount (02:6209-02:62CB)
 * Updates the rupee count display and plays sound effects.
 */
void UpdateRupeesCount(GBState *gb) {
    if (!gb) return;

    /* Check if draw commands queue has space (every other frame) */
    if ((gb_read(gb, wDrawCommandsSize) | (gb_read_hram(gb, hFrameCounter) & 0x01)) != 0) {
        return;
    }

    /* Check wC3CE */
    if (gb_read(gb, wC3CE) != 0) {
        gb_write(gb, wC3CE, gb_read(gb, wC3CE) - 1);
        return;
    }

    /* Check add rupee buffer */
    if ((gb_read(gb, wAddRupeeBufferLow) | gb_read(gb, wAddRupeeBufferHigh)) == 0) {
        return;
    }

    /* Play rupee sound */
    gb_write_hram(gb, hWaveSfx, WAVE_SFX_RUPEE);

    /* Calculate rupees to add (buffer * 8 + 1, capped at 9) */
    uint8_t e = gb_read(gb, wAddRupeeBufferLow);
    uint8_t a = gb_read(gb, wAddRupeeBufferHigh);
    e = (uint8_t)(e << 3);
    a = (uint8_t)(a << 3);
    a = (uint8_t)(a + 1);
    if (a >= 0x0A) {
        a = 0x09;
    }

    /* Subtract from buffer */
    e = a;
    a = gb_read(gb, wAddRupeeBufferLow) - e;
    gb_write(gb, wAddRupeeBufferLow, a);
    a = gb_read(gb, wAddRupeeBufferHigh) - 0;
    gb_write(gb, wAddRupeeBufferHigh, a);

    /* Add to rupee count (BCD) */
    a = bcd_add(gb_read(gb, wRupeeCountLow), e);
    gb_write(gb, wRupeeCountLow, a);

    a = bcd_add(gb_read(gb, wRupeeCountHigh), 0);
    gb_write(gb, wRupeeCountHigh, a);

    /* Cap at 999 rupees (0x09 0x99) */
    if (a >= 0x10) {
        gb_write(gb, wRupeeCountHigh, 0x09);
        gb_write(gb, wRupeeCountLow, 0x99);
        gb_write(gb, wAddRupeeBufferHigh, 0x00);
        gb_write(gb, wAddRupeeBufferLow, 0x00);
    }

    LoadRupeesDigits(gb);

    /* Check subtract rupee buffer */
    if ((gb_read(gb, wSubstractRupeeBufferLow) | gb_read(gb, wSubstractRupeeBufferHigh)) == 0) {
        return;
    }

    /* Play cut grass sound for rupee deduction */
    gb_write_hram(gb, hWaveSfx, NOISE_SFX_CUT_GRASS);

    /* Calculate rupees to subtract */
    e = gb_read(gb, wSubstractRupeeBufferLow);
    a = gb_read(gb, wSubstractRupeeBufferHigh);
    e = (uint8_t)(e << 3);
    a = (uint8_t)(a << 3);
    a = (uint8_t)(a + 1);
    if (a >= 0x0A) {
        a = 0x09;
    }

    /* Subtract from buffer */
    uint8_t sub_e = a;
    a = gb_read(gb, wSubstractRupeeBufferLow) - sub_e;
    gb_write(gb, wSubstractRupeeBufferLow, a);
    a = gb_read(gb, wSubstractRupeeBufferHigh) - 0;
    gb_write(gb, wSubstractRupeeBufferHigh, a);

    /* Check if rupees are already zero */
    if ((gb_read(gb, wRupeeCountLow) | gb_read(gb, wRupeeCountHigh)) == 0) {
        return;
    }

    /* Subtract from rupee count (BCD) */
    a = bcd_sub(gb_read(gb, wRupeeCountLow), sub_e);
    gb_write(gb, wRupeeCountLow, a);

    a = bcd_sub(gb_read(gb, wRupeeCountHigh), 0);
    gb_write(gb, wRupeeCountHigh, a);

    /* Check for underflow */
    if (a >= 0x10) {  /* carry occurred */
        /* Reset to zero */
        gb_write(gb, wRupeeCountHigh, 0x00);
        gb_write(gb, wRupeeCountLow, 0x00);
        gb_write(gb, wSubstractRupeeBufferHigh, 0x00);
        gb_write(gb, wSubstractRupeeBufferLow, 0x00);
    }

    LoadRupeesDigits(gb);
}

/* ThresholdLowHealthTable (02:6308) */
const uint8_t ThresholdLowHealthTable[16] = {
    0x00, /*  0 max hearts */
    0x22, /*  1 max hearts */
    0xC9, /*  2 max hearts */
    0x05, /*  3 max hearts */
    0x05, /*  4 max hearts */
    0x05, /*  5 max hearts */
    0x09, /*  6 max hearts */
    0x09, /*  7 max hearts */
    0x09, /*  8 max hearts */
    0x11, /*  9 max hearts */
    0x11, /* 10 max hearts */
    0x11, /* 11 max hearts */
    0x19, /* 12 max hearts */
    0x19, /* 13 max hearts */
    0x19, /* 14 max hearts */
    0x19  /* 15 max hearts */
};

/* UpdateHealth (02:6317-02:63D8)
 * Updates health display and handles low health warnings.
 */
void UpdateHealth(GBState *gb) {
    if (!gb) return;

    /* Reset low health flag */
    gb_write(gb, wIsOnLowHeath, 0x00);

    uint8_t max_hearts = gb_read(gb, wMaxHearts);

    /* Check if health is below low health threshold (02:6308) */
    if (gb_read(gb, wHealth) < ThresholdLowHealthTable[max_hearts & 0x0F]) {
        /* Low health */
        gb_write(gb, wIsOnLowHeath, 0x01);

        /* Check and update low health SFX timer */
        uint8_t timer = gb_read(gb, wTimeToNextLowHealthSFX);
        if (timer == 0) {
            /* Play low hearts sound */
            gb_write_hram(gb, hWaveSfx, WAVE_SFX_LOW_HEARTS);
            timer = LOW_HEALTH_SFX_PAUSE;
        } else if (timer == 0xFF) {
            timer = LOW_HEALTH_SFX_PAUSE;
        } else {
            timer = timer - 1;
        }
        gb_write(gb, wTimeToNextLowHealthSFX, timer);
    }

    /* Return if even frame */
    if ((gb_read_hram(gb, hFrameCounter) & 0x01) == 0) {
        return;
    }

    /* Return if background copy not executed */
    if (gb_read(gb, wDrawCommandsSize) != 0) {
        return;
    }

    /* Check if health needs to be added */
    if (gb_read(gb, wAddHealthBuffer) == 0) {
        goto reduce_health;
    }

    /* Decrease add health buffer */
    gb_write(gb, wAddHealthBuffer, gb_read(gb, wAddHealthBuffer) - 1);

    /* Cap max hearts at 14 (0x0E) */
    if (max_hearts >= 0x0F) {
        max_hearts = 0x0E;
    }

    /* Calculate max health (max_hearts * 8) */
    uint8_t max_health = max_hearts * 8;

    /* If health not at max, increment */
    if (gb_read(gb, wHealth) != max_health) {
        gb_write(gb, wHealth, gb_read(gb, wHealth) + 1);
        /* Check if one heart is filled up (health % 8 == 7) */
        if ((gb_read(gb, wHealth) & (ONE_HEART - 1)) == (ONE_HEART - 2)) {
            gb_write_hram(gb, hWaveSfx, WAVE_SFX_HEART_PICKED_UP);
        }
    } else {
        /* Health at max, reset add buffer */
        gb_write(gb, wAddHealthBuffer, 0x00);
        goto reduce_health;
    }
    return;

reduce_health:
    /* Check if health needs to be reduced */
    if (gb_read(gb, wSubtractHealthBuffer) == 0) {
        return;
    }

    /* Decrease sub health buffer */
    gb_write(gb, wSubtractHealthBuffer, gb_read(gb, wSubtractHealthBuffer) - 1);

    /* Decrease health */
    if (gb_read(gb, wHealth) > 0) {
        gb_write(gb, wHealth, gb_read(gb, wHealth) - 1);
    }
    LoadHeartsCount(gb);
}

/* LoadHeartsCount (02:6414-02:64FF)
 * Loads the hearts count display.
 */
void LoadHeartsCount(GBState *gb) {
    if (!gb) return;

    uint8_t a = gb_read(gb, wDrawCommandsSize);
    uint8_t e = a;
    a = (uint8_t)(a + 0x14);
    gb_write(gb, wDrawCommandsSize, a);

    uint16_t hl = (uint16_t)(wDrawCommand + e);

    /* 21 bytes copied from Data_002_63FF (02:63FF) */
    static const uint8_t Data_002_63FF[21] = {
        0x9C, 0x0D, 0x06, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
        0x9C, 0x2D, 0x06, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
        0x00
    };
    for (uint8_t i = 0; i < 21; i++) {
        gb_write(gb, hl++, Data_002_63FF[i]);
    }

    /* Draw heart tiles starting at wDrawCommand.data + de = wDrawCommand + e + 3 */
    hl = (uint16_t)(wDrawCommand + e + 3);
    uint8_t c = 0;
    uint8_t health = gb_read(gb, wHealth);
    if (health == 0) {
        goto draw_empty_hearts;
    }
    gb_write_hram(gb, hMultiPurpose0, health);

draw_full_hearts:
    health = gb_read_hram(gb, hMultiPurpose0);
    if (health < ONE_HEART) {
        /* Partial heart */
        if (health != 0) {
            gb_write(gb, hl++, 0xCE); /* half heart tile */
            c++;
            if (c == 7) {
                hl += 3; /* skip second row command header (0x9C, 0x2D, 0x06) */
            }
        }
        goto draw_empty_hearts;
    }
    health -= ONE_HEART;
    gb_write_hram(gb, hMultiPurpose0, health);
    gb_write(gb, hl++, 0xA9); /* full heart tile */
    c++;
    if (c == 7) {
        hl += 3; /* skip second row command header (0x9C, 0x2D, 0x06) */
    }
    goto draw_full_hearts;

draw_empty_hearts:
    if (c >= gb_read(gb, wMaxHearts)) {
        return;
    }
    gb_write(gb, hl++, 0xCD); /* empty heart tile */
    c++;
    if (c == 7) {
        hl += 3; /* skip second row command header (0x9C, 0x2D, 0x06) */
    }
    goto draw_empty_hearts;
}

/* func_002_60E0 (02:60E0-02:6206)
 * Inventory and subscreen handler.
 * Clamps item counts, checks interactive state, handles subscreen transitions,
 * inventory scrolling, and updates rupees/health display.
 */
void func_002_60E0(GBState *gb) {
    if (!gb) return;

    /* Clamp item counts */
    ClampItemCount(gb, wMaxMagicPowder, wMagicPowderCount);
    ClampItemCount(gb, wMaxMagicPowder, wBombCount);
    ClampItemCount(gb, wMaxMagicPowder, wArrowCount);

    /* If Link is not interactive, return */
    if (gb_read(gb, wLinkMotionState) >= LINK_MOTION_TYPE_NON_INTERACTIVE) {
        return;
    }

    /* Check dialog state */
    if (gb_read(gb, wDialogState) != 0) {
        goto inventory_fully_closed2;
    }

    /* Check room transition state */
    if (gb_read(gb, wRoomTransitionState) != 0) {
        return;
    }

    /* Handle subscreen transition state */
    if (gb_read(gb, wInventoryAppearing) != 0) {
        /* Scroll subscreen */
        if (gb_read(gb, wInventoryShouldScroll) != 0) {
            /* Apply scroll increment to wWindowY */
            uint8_t inc = gb_read(gb, wSubscreenScrollIncrement);
            uint8_t new_window_y = gb_read(gb, wWindowY) + inc;
            gb_write(gb, wWindowY, new_window_y);

            /* Check if window reached bottom (0x80) */
            if (new_window_y == 0x80) {
                /* Subscreen fully open */
                gb_write_hram(gb, hVolumeRight, 0x03);
                gb_write_hram(gb, hVolumeLeft, 0x30);
                gb_write(gb, wInventoryAppearing, 0x00);
            } else if (new_window_y != 0x00) {
                /* Subscreen scrolling */
                func_002_61BA(gb);
                return;
            } else {
                /* Subscreen fully open at top */
                gb_write_hram(gb, hVolumeRight, 0x03);
                gb_write_hram(gb, hVolumeLeft, 0x30);
                gb_write(gb, wInventoryAppearing, 0x00);
            }
        } else {
            if (gb_read(gb, wDrawCommand) == 0) {
                /* Increment wInventoryShouldScroll */
                gb_write(gb, wInventoryShouldScroll, gb_read(gb, wInventoryShouldScroll) + 1);
            }
            func_002_61BA(gb);
        }
        return;
    }

    /* Check if map should be opened (SELECT) */
    if ((gb_read_hram(gb, hPressedButtonsMask) & J_SELECT) != 0) {
        goto inventory_fully_closed2;
    }

    /* Check if subscreen should be opened (START) */
    if ((gb_read_hram(gb, hJoypadState) & J_START) == 0) {
        goto inventory_fully_closed2;
    }

    /* Check wWindowY */
    uint8_t window_y = gb_read(gb, wWindowY);
    if (window_y != 0) {
        /* Additional checks when window is not at top */
        if (gb_read(gb, wD464) != 0) {
            goto inventory_fully_closed2;
        }
        if (gb_read(gb, wC167) != 0) {
            goto inventory_fully_closed2;
        }
        if (gb_read_hram(gb, hLinkInteractiveMotionBlocked) == 0x02) {
            goto inventory_fully_closed2;
        }
        if ((uint8_t)(gb_read_hram(gb, hLinkAnimationState) + 1) == 0) {
            goto inventory_fully_closed2;
        }
    }

    /* Check ocarina menu state */
    uint8_t ocarina_open = gb_read(gb, wOcarinaMenuOpen);
    uint8_t ocarina_opening = gb_read(gb, wOcarinaMenuOpening);
    uint8_t ocarina_closing = gb_read(gb, wOcarinaMenuClosing);
    if ((ocarina_open | ocarina_opening | ocarina_closing) != 0) {
        goto inventory_fully_closed2;
    }

    /* Open subscreen */
    gb_write(gb, wInventoryAppearing, 0x01);
    gb_write(gb, wInventoryShouldScroll, 0x01);
    gb_write_hram(gb, hJingle, JINGLE_CLOSE_INVENTORY);

    /* Flip wSubscreenScrollIncrement (two's complement) */
    uint8_t scroll_inc = gb_read(gb, wSubscreenScrollIncrement);
    scroll_inc = (uint8_t)(~scroll_inc + 1);
    gb_write(gb, wSubscreenScrollIncrement, scroll_inc);

    /* Check if subscreen should close (scroll_inc positive) */
    if ((scroll_inc & 0x80) == 0) {
        /* Subscreen closing - never executed on DX */
        gb_write_hram(gb, hVolumeRight, 0x07);
        gb_write_hram(gb, hVolumeLeft, 0x70);
        return;
    }

    /* Subscreen opening */
    gb_write(gb, wTransitionSequenceCounter, 0x00);
    gb_write(gb, wC16C, 0x00);
    gb_write_hram(gb, hPressedButtonsMask, 0x00);
    gb_write_hram(gb, hJoypadState, 0x00);
    gb_write(gb, wGameplaySubtype, GAMEPLAY_INVENTORY_INITIAL);
    gb_write(gb, wGameplayType, GAMEPLAY_INVENTORY);
    gb_write_hram(gb, hJingle, JINGLE_OPEN_INVENTORY);
    gb_write(gb, wInventoryShouldScroll, 0x00);
    gb_write(gb, wC154, 0x0B);

    /* Determine tileset to load */
    uint8_t is_indoor = gb_read(gb, wIsIndoor);
    uint8_t tileset = TILESET_LOAD_INVENTORY;

    if (is_indoor != 0) {
        uint8_t map_id = gb_read_hram(gb, hMapId);
        if (map_id == MAP_COLOR_DUNGEON || map_id < MAP_WINDFISHS_EGG) {
            tileset = TILESET_LOAD_DUNGEON_MINIMAP;
        } else {
            tileset = TILESET_LOAD_INVENTORY;
        }
    }

    if (tileset == TILESET_LOAD_DUNGEON_MINIMAP) {
        LoadMinimap(gb);
    }
    gb_write_hram(gb, hNeedsUpdatingBGTiles, tileset);
    return;

inventory_fully_closed2:
    /* Check dialog state */
    {
        uint8_t dialog_state = gb_read(gb, wDialogState) & ~DIALOG_BOX_BOTTOM_FLAG;
        if (dialog_state == 0 || dialog_state == DIALOG_END) {
            UpdateRupeesCount(gb);
            UpdateHealth(gb);
            return;
        }
        if (dialog_state == DIALOG_CHOICE) {
            return;
        }
        return;
    }
}