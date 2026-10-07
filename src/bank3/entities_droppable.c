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
    PickableHandleGrabbedByItemIfNeeded(gb, bc);

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

/* ===== PickableCanBeCollectedBySwordTable (03:62FA) ===== */
void PickableCanBeCollectedBySwordTable(GBState *gb) {
    if (!gb) return;
    /* Table of TRUE/FALSE values indexed by entity type */
    /* TRUE for ENTITY_DROPPABLE_HEART, ENTITY_DROPPABLE_RUPEE, ENTITY_SWORD_SHIELD_PICKUP */
    /* FALSE for ENTITY_DROPPABLE_FAIRY, ENTITY_KEY_DROP_POINT */
    /* This is a data table - no code needed */
}

/* ===== PickableHandleGrabbedByItemIfNeeded (03:62AF) ===== */
void PickableHandleGrabbedByItemIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles when item is grabbed by sword/bomb */
    (void)bc;
}

/* ===== PickableCollectIfNeeded (03:62EB) ===== */
void PickableCollectIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles item collection */
    (void)bc;
}

/* ===== Pick Droppable Magic Powder (03:6316) ===== */
void PickDroppableMagicPowder(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles magic powder pickup */
    (void)bc;
}

/* ===== Pick Secret Seashell (03:631E) ===== */
void PickSecretSeashell(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles secret seashell pickup */
    (void)bc;
}

/* ===== Increase Value At HL Clamp At 99 (03:6363) ===== */
void IncreaseValueAtHLClampAt99(GBState *gb) {
    if (!gb) return;
    /* Placeholder - increases value at HL, clamping at 99 */
}

/* ===== Pick Droppable Arrows (03:6378) ===== */
void PickDroppableArrows(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles arrow pickup */
    (void)bc;
}

/* ===== Pick Droppable Bombs (03:6385) ===== */
void PickDroppableBombs(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles bomb pickup */
    (void)bc;
}

/* ===== Pick Sirens Instrument (03:638F) ===== */
void PickSirensInstrument(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles instrument pickup */
    (void)bc;
}

/* ===== Hold Pickup In The Air (03:6396) ===== */
void HoldPickupInTheAir(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - holds pickup in the air animation */
    (void)bc;
}

/* ===== Pick Heart Container (03:63A9) ===== */
void PickHeartContainer(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles heart container pickup */
    (void)bc;
}

/* ===== Pick Toadstool Or Dungeon Key (03:63B6) ===== */
void PickToadstoolOrDungeonKey(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles toadstool or dungeon key pickup */
    (void)bc;
}

/* ===== Pick Heart Piece (03:63C3) ===== */
void PickHeartPiece(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles heart piece pickup */
    (void)bc;
}

/* ===== Pick Guardian Acorn (03:63D0) ===== */
void PickGuardianAcorn(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles guardian acorn pickup */
    (void)bc;
}

/* ===== Pick Piece Of Power (03:63DD) ===== */
void PickPieceOfPower(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles piece of power pickup */
    (void)bc;
}

/* ===== Process Power Up (03:63E4) ===== */
void ProcessPowerUp(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - processes power up */
    (void)bc;
}

/* ===== Move Pickup In The Air (03:63EA) ===== */
void MovePickupInTheAir(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - moves pickup in the air */
    (void)bc;
}

/* ===== Pick Sword (03:63F4) ===== */
void PickSword(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles sword pickup */
    (void)bc;
}

/* ===== Give Inventory Item (03:640C) ===== */
void GiveInventoryItem(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - gives inventory item */
    (void)bc;
}

/* ===== Pick Droppable Key (03:643E) ===== */
void PickDroppableKey(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles key pickup */
    (void)bc;
}

/* ===== Pick Droppable Heart (03:645E) ===== */
void PickDroppableHeart(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles heart pickup */
    (void)bc;
}

/* ===== Pick Droppable Rupee (03:646B) ===== */
void PickDroppableRupee(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles rupee pickup */
    (void)bc;
}

/* ===== Pick Droppable Fairy (03:6478) ===== */
void PickDroppableFairy(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - handles fairy pickup */
    (void)bc;
}

/* ===== Spawn New Entity (03:4F68) ===== */
void SpawnNewEntity(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - spawns new entity */
    (void)bc;
}

/* ===== Spawn New Entity In Range (03:4F7E) ===== */
void SpawnNewEntityInRange(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - spawns new entity in range */
    (void)bc;
}

/* ===== Configure New Entity Helper (03:4FD0) ===== */
void ConfigureNewEntity_helper(GBState *gb, uint16_t bc) {
    if (!gb) return;
    /* Placeholder - helper for configuring new entity */
    (void)bc;
}