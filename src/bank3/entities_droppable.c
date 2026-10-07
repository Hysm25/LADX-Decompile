#include "bank3/entities_droppable.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_collision.h"
#include "bank3/entities_pushed_block.h"
#include "bank3/entities_init_core.h"
#include "bank1/room_transition.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/maps.h"
#include "constants/gameplay.h"
#include "constants/directions.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/vfx.h"
#include "constants/gfx.h"
#include "constants/dialog.h"
#include "constants/audio.h"
#include "home/entities.h"
#include "home/room.h"
#include "home/bank.h"
#include "home/audio.h"
#include "home/dialog.h"
#include "home/gameplay.h"
#include "home/link.h"
#include "home/vfx.h"

/* ===== DroppableMagicPowderEntityHandler (03:6057) ===== */
void DroppableMagicPowderEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wIsIndoor]; and a; jr z, .jr_6063 */
    if (gb_read(gb, wIsIndoor) == 0) {
        goto jr_6063;
    }

    /* ldh a, [hMapId]; cp MAP_COLOR_DUNGEON; jr z, jr_003_606A */
    if (gb_read_hram(gb, hMapId) == MAP_COLOR_DUNGEON) {
        goto jr_003_606A;
    }

jr_6063:
    /* ld a, [wHasToadstool]; and a; jp nz, UnloadEntityAndReturn */
    if (gb_read(gb, wHasToadstool) != 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

jr_003_606A:
    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    DroppableRevealOrReturnIfNeeded(gb, bc);
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableMagicPowderSprite; call RenderActiveEntitySprite; jp PickableHandler */
    static const uint8_t DroppableMagicPowderSprite[2] = { 0x55, 0x01 };
    RenderActiveEntitySprite(gb, DroppableMagicPowderSprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== DroppableArrowsEntityHandler (03:607D) ===== */
void DroppableArrowsEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    DroppableRevealOrReturnIfNeeded(gb, bc);
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableArrowSprite; call RenderActiveEntitySpritesPair; jp PickableHandler */
    static const uint8_t DroppableArrowSprite[4] = {
        0x2A, 0x41,  /* tile $2A, palette 1 | OAMF_PAL0 | OAMF_YFLIP */
        0x2A, 0x41 | OAMF_XFLIP
    };
    RenderActiveEntitySpritesPair(gb, DroppableArrowSprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== DroppableDisappearIfNeeded (03:608C) ===== */
void DroppableDisappearIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntitySlowTransitionCountdown; cp $1C; ret nc */
    uint8_t countdown = GetEntitySlowTransitionCountdown(gb, bc);
    if (countdown >= 0x1C) {
        return;
    }

    /* and a; jp z, UnloadEntityAndReturn */
    if (countdown == 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* and $01; dec a; jp SetEntitySpriteVariant */
    uint8_t variant = (countdown & 0x01) - 1;
    SetEntitySpriteVariant(gb, bc, variant);
}

/* ===== DroppableRupeeEntityHandler (03:609E) ===== */
void DroppableRupeeEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    DroppableRevealOrReturnIfNeeded(gb, bc);
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableRupeeSprite; call RenderActiveEntitySprite; fallthrough to PickableHandler */
    static const uint8_t DroppableRupeeSprite[2] = { 0xA6, 0xE1 };
    RenderActiveEntitySprite(gb, DroppableRupeeSprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== PickableHandler (03:60AA) ===== */
void PickableHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* call PickableHandleGrabbedByItemIfNeeded */
    uint8_t grabbed = gb_read(gb, wEntitiesPrivateState5Table + bc);
    PickableHandleGrabbedByItemIfNeeded(gb, bc);
    if (grabbed != 0) {
        return;
    }

    /* call PickableCollectIfNeeded */
    PickableCollectIfNeeded(gb, bc);
}

/* ===== DroppableRevealOrReturnIfNeeded (03:61DE) ===== */
void DroppableRevealOrReturnIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesPrivateState3Table; add hl, bc; ld a, [hl]; and a; jp z, .return */
    if (gb_read(gb, wEntitiesPrivateState3Table + bc) == 0) {
        return;
    }

    /* ld a, [wRoomTransitionState]; and a; jp nz, .remainInvisible */
    if (gb_read(gb, wRoomTransitionState) != 0) {
        goto remainInvisible;
    }

    /* ld a, [hl]; cp $02; jr nz, .checkPegasusBootsCollision */
    uint8_t private_state3 = gb_read(gb, wEntitiesPrivateState3Table + bc);
    if (private_state3 != 0x02) {
        goto checkPegasusBootsCollision;
    }

    /* Items buried, hidden in bushes, or indoors: */
    /* ldh a, [hActiveEntityType]; cp ENTITY_DROPPABLE_SECRET_SEASHELL; jr z, .skipNotActiveIfIndoors */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_DROPPABLE_SECRET_SEASHELL) {
        goto skipNotActiveIfIndoors;
    }

    /* If indoors and not a seashell, the item can't be dug up or dropped by bushes. */
    /* ld a, [wIsIndoor]; and a; jp nz, .remainInvisible */
    if (gb_read(gb, wIsIndoor) != 0) {
        goto remainInvisible;
    }

    /* Items knocked down with the Pegasus Boots: */
    /* ld a, [wScreenShakeCountdown]; and a; jr z, .remainInvisible */
    /* ld a, [wPegasusBootsCollisionCountdown]; and a; jr z, .remainInvisible */
    /* ldh a, [hActiveEntityPosX]; add $08; ld hl, wPegasusBootsCollisionPosX; sub [hl]; add $10; cp $20; jr nc, .remainInvisible */
    /* ldh a, [hActiveEntityPosY]; add $08; ld hl, wPegasusBootsCollisionPosY; sub [hl]; add $10; cp $20; jr nc, .remainInvisible */
checkPegasusBootsCollision:
    if (gb_read(gb, wScreenShakeCountdown) == 0) {
        goto remainInvisible;
    }
    if (gb_read(gb, wPegasusBootsCollisionCountdown) == 0) {
        goto remainInvisible;
    }
    int16_t diff_x = (int16_t)gb_read_hram(gb, hActiveEntityPosX) + 8;
    diff_x -= gb_read(gb, wPegasusBootsCollisionPosX);
    diff_x += 0x10;
    if (diff_x >= 0x20) {
        goto remainInvisible;
    }
    int16_t diff_y = (int16_t)gb_read_hram(gb, hActiveEntityPosY) + 8;
    diff_y -= gb_read(gb, wPegasusBootsCollisionPosY);
    diff_y += 0x10;
    if (diff_y >= 0x20) {
        goto remainInvisible;
    }

skipNotActiveIfIndoors:
    /* call func_003_7E0E */
    func_003_7E0E(gb, bc);

    /* ldh a, [hActiveEntityType]; cp ENTITY_DROPPABLE_HEART; jr z, .activeIfOnShortGrass */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_DROPPABLE_HEART) {
        goto activeIfOnShortGrass;
    }

    /* cp ENTITY_DROPPABLE_SECRET_SEASHELL; jr nz, .activeIfOnShortGrassEnd */
    if (gb_read_hram(gb, hActiveEntityType) != ENTITY_DROPPABLE_SECRET_SEASHELL) {
        goto activeIfOnShortGrassEnd;
    }

    /* Seashells buried under short grass (some of these don't exist) */
    /* ldh a, [hMapRoom]; cp UNKNOWN_ROOM_DA; jr z, .activeIfOnShortGrassEnd */
    /* cp UNKNOWN_ROOM_A5; jr z, .activeIfOnShortGrassEnd */
    /* cp UNKNOWN_ROOM_74; jr z, .activeIfOnShortGrassEnd */
    /* cp UNKNOWN_ROOM_3A; jr z, .activeIfOnShortGrassEnd */
    /* cp UNKNOWN_ROOM_A8; jr z, .activeIfOnShortGrassEnd */
    /* cp UNKNOWN_ROOM_B2; jr z, .activeIfOnShortGrassEnd */
    uint8_t map_room = gb_read_hram(gb, hMapRoom);
    if (map_room == UNKNOWN_ROOM_DA || map_room == UNKNOWN_ROOM_A5 ||
        map_room == UNKNOWN_ROOM_74 || map_room == UNKNOWN_ROOM_3A ||
        map_room == UNKNOWN_ROOM_A8 || map_room == UNKNOWN_ROOM_B2) {
        goto activeIfOnShortGrassEnd;
    }

activeIfOnShortGrass:
    /* ldh a, [hObjectUnderEntity]; cp OBJECT_SHORT_GRASS; jr z, .setOptionsAndReveal */
    if (gb_read_hram(gb, hObjectUnderEntity) == OBJECT_SHORT_GRASS) {
        goto setOptionsAndReveal;
    }

    /* jr .activeIfOnShovelHole */
    goto activeIfOnShovelHole;

activeIfOnShortGrassEnd:
    /* ld hl, wEntitiesPrivateState4Table; add hl, bc; ld [hl], $01 */
    gb_write(gb, wEntitiesPrivateState4Table + bc, 0x01);

activeIfOnShovelHole:
    /* ldh a, [hObjectUnderEntity]; cp OBJECT_SHOVEL_HOLE; jr nz, .remainInvisible */
    if (gb_read_hram(gb, hObjectUnderEntity) != OBJECT_SHOVEL_HOLE) {
        goto remainInvisible;
    }

setOptionsAndReveal:
    /* ld hl, wEntitiesOptions1Table; add hl, bc; ld [hl], ENTITY_OPT1_SPLASH_IN_WATER|ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL; jr .reveal */
    gb_write(gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_SPLASH_IN_WATER | ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL);
    /* fallthrough to .reveal */
    /* .reveal: (items revealed are thrown away from Link) */
    /* ld hl, wEntitiesPrivateState3Table; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesPrivateState3Table + bc, 0);
    /* ld hl, wEntitiesPrivateState4Table; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesPrivateState4Table + bc, 0);
    /* call GetEntityPrivateCountdown1; ld [hl], $18 */
    gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x18);
    /* ld a, $0C; call GetVectorTowardsLink */
    uint8_t vec_x, vec_y;
    GetVectorTowardsLink(gb, &vec_x, &vec_y);
    /* ldh a, [hMultiPurpose1]; cpl; inc a; ld hl, wEntitiesSpeedXTable; add hl, bc; ld [hl], a */
    /* ldh a, [hMultiPurpose0]; cpl; inc a; ld hl, wEntitiesSpeedYTable; add hl, bc; ld [hl], a */
    /* ld hl, wEntitiesSpeedZTable; add hl, bc; ld [hl], $20 */
    gb_write(gb, wEntitiesSpeedZTable + bc, 0x20);
    /* call GetEntitySlowTransitionCountdown; ld [hl], $80 */
    gb_write(gb, wEntitiesSlowTransitionCountdownTable + bc, 0x80);

remainInvisible:
    return;
}

/* ===== func_003_61C0 (03:61C0) ===== */
void func_003_61C0(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hFrameCounter]; and $03; jr nz, ret_003_61DD */
    if ((gb_read_hram(gb, hFrameCounter) & 0x03) != 0) {
        return;
    }

    /* ld hl, wEntitiesPosZTable; add hl, bc; ld a, [hl]; cp $10; jr z, ret_003_61DD */
    uint8_t pos_z = gb_read(gb, wEntitiesPosZTable + bc);
    if (pos_z == 0x10) {
        return;
    }

    /* bit 7, a; jr z, .jr_61D6 */
    if ((pos_z & 0x80) == 0) {
        goto jr_61D6;
    }

    /* inc [hl]; jr ret_003_61DD */
    gb_write(gb, wEntitiesPosZTable + bc, pos_z + 1);
    return;

jr_61D6:
    /* cp $10; jr nc, .jr_61DC */
    if (pos_z >= 0x10) {
        goto jr_61DC;
    }

    /* inc [hl]; ret */
    gb_write(gb, wEntitiesPosZTable + bc, pos_z + 1);
    return;

jr_61DC:
    /* dec [hl]; ret */
    gb_write(gb, wEntitiesPosZTable + bc, pos_z - 1);
    return;
}

/* BCD addition helper: adds a value to a BCD byte, returns result */
static uint8_t bcd_add(uint8_t a, uint8_t b) {
    uint16_t result = a + b;
    if ((result & 0x0F) > 0x09) result += 0x06;
    if (result > 0x99) result += 0x60;
    return (uint8_t)result;
}

/* Forward declarations */
static void PickableCollect(GBState *gb, uint16_t bc);
static void ProcessPowerUp_internal(GBState *gb, uint16_t bc, uint8_t power_up, uint8_t dialog);

/* ===== PickableCanBeCollectedBySwordTable (03:629E) ===== */
static const uint8_t PickableCanBeCollectedBySwordTableData[17] = {
    1, /* ENTITY_DROPPABLE_HEART ($2D) */
    1, /* ENTITY_DROPPABLE_RUPEE ($2E) */
    0, /* ENTITY_DROPPABLE_FAIRY ($2F) */
    0, /* ENTITY_KEY_DROP_POINT ($30) */
    1, /* ENTITY_SWORD_SHIELD_PICKUP ($31) */
    0, /* ENTITY_IRON_MASKS_MASK ($32) */
    1, /* ENTITY_PIECE_OF_POWER ($33) */
    1, /* ENTITY_GUARDIAN_ACORN ($34) */
    0, /* ENTITY_HEART_PIECE ($35) */
    0, /* ENTITY_HEART_CONTAINER ($36) */
    1, /* ENTITY_DROPPABLE_ARROWS ($37) */
    1, /* ENTITY_DROPPABLE_BOMBS ($38) */
    0, /* ENTITY_INSTRUMENT_OF_THE_SIRENS ($39) */
    0, /* ENTITY_SLEEPY_TOADSTOOL ($3A) */
    1, /* ENTITY_DROPPABLE_MAGIC_POWDER ($3B) */
    0, /* ENTITY_HIDING_SLIME_KEY ($3C) */
    0  /* ENTITY_DROPPABLE_SECRET_SEASHELL ($3D) */
};

void PickableCanBeCollectedBySwordTable(GBState *gb) {
    (void)gb;
}

/* ===== PickableHandleGrabbedByItemIfNeeded (03:62AF) ===== */
void PickableHandleGrabbedByItemIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return;

    uint8_t grabbed = gb_read(gb, wEntitiesPrivateState5Table + bc);
    if (grabbed == 0) {
        return;
    }

    uint8_t grabber = (uint8_t)(grabbed - 1);
    uint8_t status = gb_read(gb, wEntitiesStatusTable + grabber);
    if (status == 0) {
        PickableCollect(gb, bc);
        return;
    }

    uint8_t type = gb_read(gb, wEntitiesTypeTable + grabber);
    if (type != ENTITY_BOOMERANG && type != ENTITY_HOOKSHOT_CHAIN) {
        PickableCollect(gb, bc);
        return;
    }

    /* Snap to boomerang or hookshot */
    uint8_t pos_x = gb_read(gb, wEntitiesPosXTable + grabber);
    gb_write(gb, wEntitiesPosXTable + bc, pos_x);
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + grabber);
    gb_write(gb, wEntitiesPosYTable + bc, pos_y);
    gb_write(gb, wEntitiesPosZTable + bc, 0);
}

/* ===== PickableCollectIfNeeded (03:62EB) ===== */
void PickableCollectIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return;

    if (GetEntityPrivateCountdown1(gb, bc) != 0) {
        return;
    }

    uint8_t type = gb_read_hram(gb, hActiveEntityType);
    uint8_t table_idx = (uint8_t)(type - 0x2D);
    if (table_idx < sizeof(PickableCanBeCollectedBySwordTableData)) {
        if (PickableCanBeCollectedBySwordTableData[table_idx]) {
            uint8_t ignore_hits = gb_read(gb, wEntitiesIgnoreHitsCountdownTable + bc);
            gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0);
            func_003_6E2B(gb, bc);
            gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, ignore_hits);
        }
    }

    if (!func_003_6C6B(gb, bc)) {
        return;
    }

    PickableCollect(gb, bc);
}

static void PickableCollect(GBState *gb, uint16_t bc) {
    uint8_t load_order = gb_read(gb, wEntitiesLoadOrderTable + bc);
    DidKillEnemy_label_3F78(gb, bc, load_order);

    uint8_t type = gb_read_hram(gb, hActiveEntityType);
    uint8_t table_idx = (uint8_t)(type - 0x2D);

    if (table_idx < (ENTITY_DROPPABLE_FAIRY - 0x2D)) {
        gb_write_hram(gb, hJingle, JINGLE_GOT_HEART);
    } else {
        gb_write_hram(gb, hWaveSfx, WAVE_SFX_SEASHELL);
    }

    switch (table_idx) {
        case 0x00: PickDroppableHeart(gb, bc); break;
        case 0x01: PickDroppableRupee(gb, bc); break;
        case 0x02: PickDroppableFairy(gb, bc); break;
        case 0x03: PickDroppableKey(gb, bc); break;
        case 0x04: PickSword(gb, bc); break;
        case 0x05: break; /* MovePickupInTheAir.return */
        case 0x06: PickPieceOfPower(gb, bc); break;
        case 0x07: PickGuardianAcorn(gb, bc); break;
        case 0x08: PickHeartPiece(gb, bc); break;
        case 0x09: PickHeartContainer(gb, bc); break;
        case 0x0A: PickDroppableArrows(gb, bc); break;
        case 0x0B: PickDroppableBombs(gb, bc); break;
        case 0x0C: PickSirensInstrument(gb, bc); break;
        case 0x0D: PickToadstoolOrDungeonKey(gb, bc); break;
        case 0x0E: PickDroppableMagicPowder(gb, bc); break;
        case 0x0F: PickToadstoolOrDungeonKey(gb, bc); break;
        case 0x10: PickSecretSeashell(gb, bc); break;
        default: break;
    }
}

/* ===== GiveInventoryItem (03:6472) ===== */
void GiveInventoryItem(GBState *gb, uint16_t item) {
    if (!gb) return;
    uint8_t item_id = (uint8_t)item;

    for (uint16_t i = 0; i < 12; i++) {
        if (gb_read(gb, wInventoryBButtonSlot + i) == item_id) {
            return;
        }
    }

    for (uint16_t i = 0; i < 12; i++) {
        if (gb_read(gb, wInventoryBButtonSlot + i) == 0) {
            gb_write(gb, wInventoryBButtonSlot + i, item_id);
            return;
        }
    }
}

/* ===== IncreaseValueAtHLClampAt99 (03:6373) ===== */
void IncreaseValueAtHLClampAt99_addr(GBState *gb, uint16_t addr) {
    if (!gb) return;
    uint8_t count = gb_read(gb, addr);
    if (count == 0x99) return;
    gb_write(gb, addr, bcd_add(count, 1));
}

void IncreaseValueAtHLClampAt99(GBState *gb) {
    IncreaseValueAtHLClampAt99_addr(gb, wSeashellsCount);
}

/* ===== PickDroppableMagicPowder (03:6350) ===== */
void PickDroppableMagicPowder(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    gb_write_hram(gb, hReplaceTiles, REPLACE_TILES_MAGIC_POWDER);
    GiveInventoryItem(gb, 0x0C);
    uint8_t count = gb_read(gb, wMagicPowderCount);
    uint8_t max = gb_read(gb, wMaxMagicPowder);
    if (count < max) {
        gb_write(gb, wMagicPowderCount, bcd_add(count, 1));
    }
}

/* ===== PickSecretSeashell (03:6368) ===== */
void PickSecretSeashell(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    OpenDialogInTable0(gb, Dialog0EF);
    MarkRoomCompleted(gb);
    IncreaseValueAtHLClampAt99(gb);
}

/* ===== PickDroppableArrows (03:637D) ===== */
void PickDroppableArrows(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    uint8_t count = gb_read(gb, wArrowCount);
    uint8_t max = gb_read(gb, wMaxArrows);
    if (count < max) {
        gb_write(gb, wArrowCount, bcd_add(count, 1));
    }
}

/* ===== PickDroppableBombs (03:6385) ===== */
void PickDroppableBombs(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    GiveInventoryItem(gb, 0x02);
    uint8_t count = gb_read(gb, wBombCount);
    uint8_t max = gb_read(gb, wMaxBombs);
    if (count < max) {
        gb_write(gb, wBombCount, bcd_add(count, 1));
    }
}

/* ===== MovePickupInTheAir (03:641E) ===== */
void MovePickupInTheAir(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    static const uint8_t offset_x[4] = { 0xE4, 0x14, 0xE4, 0x14 };
    static const uint8_t offset_y[4] = { 0xD4, 0xD4, 0x04, 0x04 };

    for (int8_t e = 3; e >= 0; e--) {
        uint8_t link_x = gb_read_hram(gb, hLinkPositionX);
        uint8_t link_y = gb_read_hram(gb, hLinkPositionY);
        gb_write_hram(gb, hMultiPurpose0, (uint8_t)(link_x + offset_x[e]));
        gb_write_hram(gb, hMultiPurpose1, (uint8_t)(link_y + offset_y[e]));
        AddTranscientVfx(gb, TRANSCIENT_VFX_MOVING_SPARKLE);
        gb_write(gb, (uint16_t)(wTranscientVfxCountdownTable + e), 0x22);
        gb_write(gb, (uint16_t)(wC590 + e), (uint8_t)e);
    }
}

/* ===== HoldPickupInTheAir (03:63A1) ===== */
void HoldPickupInTheAir(GBState *gb, uint16_t bc) {
    if (!gb) return;
    uint8_t link_x = gb_read_hram(gb, hLinkPositionX);
    gb_write_hram(gb, hLinkPositionX, (uint8_t)(link_x + 4));
    MovePickupInTheAir(gb, bc);
    gb_write_hram(gb, hLinkPositionX, link_x);
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x68);
    gb_write(gb, wC111, 0x68);
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    ResetSpinAttack(gb);
}

/* ===== PickSirensInstrument (03:6392) ===== */
void PickSirensInstrument(GBState *gb, uint16_t bc) {
    if (!gb) return;
    gb_write(gb, wBossDefeated, 0);
    gb_write(gb, wObjectAffectingBGPalette, 0);
    gb_write(gb, wMusicTrackToPlay, MUSIC_OBTAIN_INSTRUMENT);
    gb_write(gb, wC167, MUSIC_OBTAIN_INSTRUMENT);
    HoldPickupInTheAir(gb, bc);
}

/* ===== PickHeartContainer (03:63B0) ===== */
void PickHeartContainer(GBState *gb, uint16_t bc) {
    if (!gb) return;
    gb_write(gb, wActivePowerUp, 0);
    gb_write(gb, wMusicTrackToPlay, MUSIC_HEART_CONTAINER);
    gb_write(gb, wBossDefeated, MUSIC_HEART_CONTAINER);
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x70);
    gb_write(gb, wC111, 0x70);
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    ResetSpinAttack(gb);
}

/* ===== PickToadstoolOrDungeonKey (03:63C7) ===== */
void PickToadstoolOrDungeonKey(GBState *gb, uint16_t bc) {
    if (!gb) return;
    gb_write(gb, wMusicTrackToPlay, MUSIC_OBTAIN_ITEM);
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x68);
    gb_write(gb, wC111, 0x68);
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    ResetSpinAttack(gb);
}

/* ===== PickHeartPiece (03:63E4) ===== */
void PickHeartPiece(GBState *gb, uint16_t bc) {
    if (!gb) return;
    gb_write(gb, wMusicTrackToPlay, MUSIC_OBTAIN_ITEM);
    IncrementEntityState(gb, bc);
    gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    ResetSpinAttack(gb);
}

static void ProcessPowerUp_internal(GBState *gb, uint16_t bc, uint8_t power_up, uint8_t dialog) {
    gb_write(gb, wActivePowerUp, power_up);
    gb_write(gb, wDialogGotItem, dialog);
    gb_write(gb, wDialogGotItemCountdown, 0x30);
    gb_write(gb, wC111, 0x30);
    gb_write(gb, wPowerUpHits, 0);
    gb_write(gb, wMusicTrackToPlay, MUSIC_OBTAIN_POWERUP);
    gb_write_hram(gb, hDefaultMusicTrackAlt, MUSIC_ACTIVE_POWER_UP);
    gb_write_hram(gb, hNextDefaultMusicTrack, MUSIC_ACTIVE_POWER_UP);
    MovePickupInTheAir(gb, bc);
}

/* ===== PickGuardianAcorn (03:63F6) ===== */
void PickGuardianAcorn(GBState *gb, uint16_t bc) {
    if (!gb) return;
    ProcessPowerUp_internal(gb, bc, ACTIVE_POWER_UP_GUARDIAN_ACORN, DIALOG_GOT_GUARDIAN_ACORN);
}

/* ===== PickPieceOfPower (03:63FC) ===== */
void PickPieceOfPower(GBState *gb, uint16_t bc) {
    if (!gb) return;
    ProcessPowerUp_internal(gb, bc, ACTIVE_POWER_UP_PIECE_OF_POWER, DIALOG_GOT_PIECE_OF_POWER);
}

/* ===== ProcessPowerUp (03:6400) ===== */
void ProcessPowerUp(GBState *gb, uint16_t bc) {
    if (!gb) return;
    ProcessPowerUp_internal(gb, bc, ACTIVE_POWER_UP_PIECE_OF_POWER, DIALOG_GOT_PIECE_OF_POWER);
}

/* ===== PickSword (03:644D) ===== */
void PickSword(GBState *gb, uint16_t bc) {
    if (!gb) return;
    if (gb_read(gb, wSwordLevel) == 0) {
        gb_write(gb, wMusicTrackToPlay, MUSIC_OBTAIN_SWORD);
        gb_write(gb, wC167, MUSIC_OBTAIN_SWORD);
        HoldPickupInTheAir(gb, bc);
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0xA0);
        gb_write_hram(gb, hNextDefaultMusicTrack, MUSIC_SILENCE);
    } else {
        uint8_t shield_level = gb_read(gb, wEntitiesPrivateState1Table + bc);
        gb_write(gb, wShieldLevel, shield_level);
        GiveInventoryItem(gb, INVENTORY_SHIELD);
    }
}

/* ===== PickDroppableKey (03:648F) ===== */
void PickDroppableKey(GBState *gb, uint16_t bc) {
    if (!gb) return;
    uint8_t room = gb_read_hram(gb, hMapRoom);
    if (room == ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_4) {
        gb_write(gb, wMusicTrackToPlay, MUSIC_OBTAIN_ITEM);
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x68);
        gb_write(gb, wC111, 0x68);
        gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
        ResetSpinAttack(gb);
        return;
    }

    if (room == ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_FALL) {
        uint8_t st = gb_read(gb, wIndoorARoomStatus + 0x69);
        gb_write(gb, wIndoorARoomStatus + 0x69, (uint8_t)(st | 0x10));
    }

    if (gb_read_hram(gb, hActiveEntitySpriteVariant) != 0) {
        gb_write(gb, wMusicTrackToPlay, MUSIC_OBTAIN_ITEM);
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x68);
        gb_write(gb, wC111, 0x68);
        gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
        ResetSpinAttack(gb);
        return;
    }

    MarkRoomCompleted(gb);
    uint8_t keys = gb_read(gb, wSmallKeysCount);
    gb_write(gb, wSmallKeysCount, (uint8_t)(keys + 1));
    SynchronizeDungeonsItemFlags_trampoline(gb, SynchronizeDungeonsItemFlags);
}

/* ===== PickDroppableHeart (03:64B7) ===== */
void PickDroppableHeart(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    uint8_t health = gb_read(gb, wAddHealthBuffer);
    gb_write(gb, wAddHealthBuffer, (uint8_t)(health + 8));
}

/* ===== PickDroppableRupee (03:64BF) ===== */
void PickDroppableRupee(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    uint8_t rupee = gb_read(gb, wAddRupeeBufferLow);
    gb_write(gb, wAddRupeeBufferLow, (uint8_t)(rupee + 1));
}

/* ===== PickDroppableFairy (03:64C6) ===== */
void PickDroppableFairy(GBState *gb, uint16_t bc) {
    if (!gb) return;
    (void)bc;
    uint8_t health = gb_read(gb, wAddHealthBuffer);
    gb_write(gb, wAddHealthBuffer, (uint8_t)(health + 0x30));
}

/* ===== ConfigureNewEntity_helper (03:6524) ===== */
void ConfigureNewEntity_helper(GBState *gb, uint16_t bc) {
    if (!gb) return;
    uint8_t old_active = gb_read(gb, wActiveEntityIndex);
    gb_write(gb, wActiveEntityIndex, (uint8_t)bc);
    ConfigureNewEntity(gb);
    gb_write(gb, wActiveEntityIndex, old_active);
}

/* ===== SpawnNewEntityInRange (03:64CC) ===== */
uint16_t SpawnNewEntityInRange_impl(GBState *gb, uint8_t entity_type, uint16_t bc, uint8_t start_e) {
    if (!gb) return 0xFFFF;
    for (int8_t e = (int8_t)start_e; e >= 0; e--) {
        if (gb_read(gb, (uint16_t)(wEntitiesStatusTable + e)) == 0) {
            gb_write(gb, (uint16_t)(wEntitiesStatusTable + e), ENTITY_STATUS_ACTIVE);
            gb_write(gb, (uint16_t)(wEntitiesTypeTable + e), entity_type);

            gb_write_hram(gb, hMultiPurpose0, gb_read(gb, (uint16_t)(wEntitiesPosXTable + bc)));
            gb_write_hram(gb, hMultiPurpose1, gb_read(gb, (uint16_t)(wEntitiesPosYTable + bc)));
            gb_write_hram(gb, hMultiPurpose2, gb_read(gb, (uint16_t)(wEntitiesDirectionTable + bc)));
            gb_write_hram(gb, hMultiPurpose3, gb_read(gb, (uint16_t)(wEntitiesPosZTable + bc)));

            ConfigureNewEntity_helper(gb, (uint16_t)e);

            gb_write(gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + e), 0x01);
            gb_write(gb, (uint16_t)(wEntitiesPosXSignTable + e), gb_read(gb, (uint16_t)(wEntitiesPosXSignTable + bc)));
            gb_write(gb, (uint16_t)(wEntitiesPosYSignTable + e), gb_read(gb, (uint16_t)(wEntitiesPosYSignTable + bc)));

            return (uint16_t)e;
        }
    }
    return 0xFFFF;
}

uint16_t SpawnNewEntity_slot(GBState *gb, uint8_t entity_type) {
    if (!gb) return 0xFFFF;
    uint8_t bc = gb_read(gb, wActiveEntityIndex);
    return SpawnNewEntityInRange_impl(gb, entity_type, bc, MAX_ENTITIES - 1);
}

uint16_t SpawnNewEntityInRange_slot(GBState *gb, uint8_t entity_type, uint8_t start_slot) {
    if (!gb) return 0xFFFF;
    uint8_t bc = gb_read(gb, wActiveEntityIndex);
    return SpawnNewEntityInRange_impl(gb, entity_type, bc, start_slot);
}

/* ===== SpawnNewEntity (03:64CA) ===== */
void SpawnNewEntity(GBState *gb, uint16_t bc) {
    if (!gb) return;
    uint8_t type = gb_read_hram(gb, hActiveEntityType);
    SpawnNewEntityInRange_impl(gb, type, bc, MAX_ENTITIES - 1);
}

void SpawnNewEntityInRange(GBState *gb, uint16_t bc) {
    if (!gb) return;
    uint8_t type = gb_read_hram(gb, hActiveEntityType);
    uint8_t start_e = gb_read_hram(gb, hMultiPurpose0);
    SpawnNewEntityInRange_impl(gb, type, bc, start_e);
}