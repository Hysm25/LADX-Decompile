#include "bank3/entities_init_basic.h"
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

/* ===== EntityInitHorsePiece (03:4926) ===== */
void EntityInitHorsePiece(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wEntitiesLoadOrderTable; add hl, bc; ld e, [hl]; ld d, b */
    uint8_t load_order = gb_read(gb, wEntitiesLoadOrderTable + bc);
    (void)bc; /* d = (bc >> 8) & 0xFF; unused in C */

    /* ld hl, Data_003_4924; add hl, de; ld a, [hl] */
    /* jp SetEntitySpriteVariant */
    static const uint8_t Data_003_4924[2] = { 0x01, 0x04 };
    uint8_t variant = Data_003_4924[load_order & 1];
    SetEntitySpriteVariant(gb, bc, variant);
}

/* ===== EntityInitMarinAtTalTalHeights (03:4934) ===== */
void EntityInitMarinAtTalTalHeights(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld a, [hl]; sub $03; ld [hl], a; ret */
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);
    gb_write(gb, wEntitiesPosYTable + bc, (uint8_t)(pos_y - 0x03));
}

/* ===== EntityInitSnake (03:493D) ===== */
void EntityInitSnake(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* call GetEntityPrivateCountdown1 */
    /* ld hl, wEntitiesPrivateCountdown1Table; add hl, bc; ld a, [hl]; and a; ret z */
    GetEntityPrivateCountdown1(gb, bc);

    /* ld [hl], $30 */
    gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x30);
}

/* ===== EntityInitSideViewPlatformVertical (03:4943) ===== */
void EntityInitSideViewPlatformVertical(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ldh a, [hMapRoom]; cp UNKNOWN_ROOM_65; ret nz */
    uint8_t map_room = gb_read_hram(gb, hMapRoom);
    if (map_room != UNKNOWN_ROOM_65) {
        return;
    }

    /* ldh a, [hActiveEntityVisualPosY]; cp $50; ret c */
    uint8_t visual_pos_y = gb_read_hram(gb, hActiveEntityVisualPosY);
    if (visual_pos_y < 0x50) {
        return;
    }

    /* ld hl, wEntitiesPrivateState1Table; add hl, bc; inc [hl]; ret */
    uint8_t state = gb_read(gb, (uint16_t)(wEntitiesPrivateState1Table + bc));
    gb_write(gb, (uint16_t)(wEntitiesPrivateState1Table + bc), (uint8_t)(state + 1));
}

/* ===== EntityInitZol (03:4953) ===== */
void EntityInitZol(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wEntitiesHealthTable; add hl, bc; ld [hl], $02; ret */
    gb_write(gb, wEntitiesHealthTable + bc, 0x02);
}

/* ===== EntityInitMarinAtTheShore (03:495A) ===== */
void EntityInitMarinAtTheShore(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wIsMarinInAnimalVillage; ld a, [wIsMarinFollowingLink]; or [hl]; jp nz, UnloadEntityAndReturn */
    uint8_t marin_in_village = gb_read(gb, wIsMarinInAnimalVillage);
    uint8_t marin_following = gb_read(gb, wIsMarinFollowingLink);
    if ((marin_in_village | marin_following) != 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ret */
}

/* ===== EntityInitBomber (03:4965) ===== */
void EntityInitBomber(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wEntitiesPosZTable; add hl, bc; ld [hl], $10 */
    gb_write(gb, wEntitiesPosZTable + bc, 0x10);

    /* call GetRandomByte */
    uint8_t random = GetRandomByte(gb);

    /* ld hl, wEntitiesInertiaTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesInertiaTable + bc, random);
}

/* ===== EntityInitBushCrawler (03:4973) ===== */
void EntityInitBushCrawler(GBState *gb) {
    if (!gb) return;
    /* ret */
    (void)gb;
}

/* ===== EntityInitTarinBeekeeper (03:4974) ===== */
void EntityInitTarinBeekeeper(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* call EntityShiftPosition */
    EntityShiftPosition(gb, bc);

    /* ld a, $02; jp SetEntitySpriteVariant */
    SetEntitySpriteVariant(gb, bc, 0x02);
}

/* ===== EntityInitTelephone (03:497C) ===== */
void EntityInitTelephone(GBState *gb) {
    if (!gb) return;

    /* ld a, MUSIC_ULRIRA; jr SetMusicTrackIfHasSword */
    SetMusicTrackIfHasSword(gb, MUSIC_ULRIRA);
}

/* ===== EntityInitRichard (03:4980) ===== */
void EntityInitRichard(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld a, [wGoldenLeavesCount]; cp SLIME_KEY; jr c, .jr_003_4993 */
    uint8_t golden_leaves = gb_read(gb, wGoldenLeavesCount);
    if (golden_leaves < SLIME_KEY) {
        /* ld a, MUSIC_RICHARD_HOUSE */
        /* fallthrough to SetMusicTrackIfHasSword */
        SetMusicTrackIfHasSword(gb, MUSIC_RICHARD_HOUSE);
        return;
    }

    /* ld hl, wEntitiesPosXTable; add hl, bc; ld [hl], $58 */
    gb_write(gb, wEntitiesPosXTable + bc, 0x58);

    /* ld hl, wEntitiesDirectionTable; add hl, bc; ld [hl], DIRECTION_DOWN */
    gb_write(gb, wEntitiesDirectionTable + bc, DIRECTION_DOWN);

    /* .jr_003_4993: ld a, MUSIC_RICHARD_HOUSE */
    /* fallthrough to SetMusicTrackIfHasSword */
    SetMusicTrackIfHasSword(gb, MUSIC_RICHARD_HOUSE);
}

/* ===== SetMusicTrackIfHasSword (03:4995) ===== */
void SetMusicTrackIfHasSword(GBState *gb, uint8_t music_track) {
    if (!gb) return;

    /* ld e, a; ld a, [wSwordLevel]; and a; ret z */
    uint8_t sword_level = gb_read(gb, wSwordLevel);
    if (sword_level == 0) {
        return;
    }

    /* ld a, e; fallthrough to SetMusicTrack */
    SetMusicTrack(gb, music_track);
}

/* ===== SetMusicTrack (03:499C) ===== */
void SetMusicTrack(GBState *gb, uint8_t music_track) {
    if (!gb) return;

    /* ld [wMusicTrackToPlay], a */
    gb_write(gb, wMusicTrackToPlay, music_track);

    /* ldh [hDefaultMusicTrack], a */
    gb_write_hram(gb, hDefaultMusicTrack, music_track);

    /* ldh [hDefaultMusicTrackAlt], a */
    gb_write_hram(gb, hDefaultMusicTrackAlt, music_track);

    /* ldh [hNextDefaultMusicTrack], a */
    gb_write_hram(gb, hNextDefaultMusicTrack, music_track);
}

/* ===== EntityInitFinalNightmare (03:49A6) ===== */
void EntityInitFinalNightmare(GBState *gb) {
    if (!gb) return;

    /* xor a; ld [wFinalNightmareForm], a; jp label_27F2 */
    gb_write(gb, wFinalNightmareForm, 0x00);
    label_27F2(gb);
}

/* ===== EntityInitDreamShrineBed (03:49AD) ===== */
void EntityInitDreamShrineBed(GBState *gb) {
    if (!gb) return;

    /* ld a, MUSIC_DREAM_SHRINE_BED; jr SetMusicTrack */
    SetMusicTrack(gb, MUSIC_DREAM_SHRINE_BED);
}

/* ===== EntityInitFishermanUnderBridge (03:49B1) ===== */
void EntityInitFishermanUnderBridge(GBState *gb) {
    if (!gb) return;

    /* ld a, MUSIC_FISHERMAN_UNDER_BRIDGE; jr SetMusicTrack */
    SetMusicTrack(gb, MUSIC_FISHERMAN_UNDER_BRIDGE);
}

/* ===== EntityInitKikiTheMonkey (03:49B5) ===== */
void EntityInitKikiTheMonkey(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* xor a; ld [wC168], a */
    gb_write(gb, wC168, 0x00);

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld a, [hl]; sub $04; ld [hl], a */
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);
    gb_write(gb, wEntitiesPosYTable + bc, (uint8_t)(pos_y - 0x04));
}

/* ===== EntityInitFireballShooter (03:49C2) ===== */
void EntityInitFireballShooter(GBState *gb) {
    if (!gb) return;

    /* call GetRandomByte; jp SetEntitySpriteVariant */
    uint8_t random = GetRandomByte(gb);
    uint16_t bc = gb_read(gb, wActiveEntityIndex);
    SetEntitySpriteVariant(gb, bc, random);
}

/* ===== EntityInitAntiKirby (03:49C8) ===== */
void EntityInitAntiKirby(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* call GetEntitySlowTransitionCountdown */
    GetEntitySlowTransitionCountdown(gb, bc);

    /* call GetRandomByte */
    uint8_t random = GetRandomByte(gb);

    /* and $3F; add $10; ld [hl], a */
    random = (random & 0x3F) + 0x10;
    gb_write(gb, wEntitiesSlowTransitionCountdownTable + bc, random);
}

/* ===== EntityInitMovingBlockMover (03:49D4) ===== */
void EntityInitMovingBlockMover(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld a, [hl]; add $0A; ld [hl], a */
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);
    gb_write(gb, wEntitiesPosYTable + bc, (uint8_t)(pos_y + 0x0A));

    /* ld hl, wEntitiesPrivateState2Table; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesPrivateState2Table + bc, gb_read(gb, wEntitiesPosYTable + bc));
}

/* ===== EntityInitDesertLanmola (03:49E2) ===== */
void EntityInitDesertLanmola(GBState *gb) {
    if (!gb) return;

    /* xor a; ldh [hDefaultMusicTrack], a; ret */
    gb_write_hram(gb, hDefaultMusicTrack, 0x00);
}

/* ===== EntityInitFloatingItem2 (03:49E6) ===== */
void EntityInitFloatingItem2(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* call SetZPosForFloatingItem */
    SetZPosForFloatingItem(gb, bc);

    /* ldh a, [hActiveEntityPosX]; swap a; and $01; add $04; jp SetEntitySpriteVariant */
    uint8_t pos_x = gb_read_hram(gb, hActiveEntityPosX);
    uint8_t variant = ((pos_x >> 4) & 0x01) + 0x04;
    SetEntitySpriteVariant(gb, bc, variant);
}

/* ===== EntityInitFloatingItem (03:49F4) ===== */
void EntityInitFloatingItem(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ldh a, [hActiveEntityPosX]; swap a; and $01; ld e, a */
    uint8_t pos_x = gb_read_hram(gb, hActiveEntityPosX);

    uint8_t e = (pos_x >> 4) & 0x01;

    /* ldh a, [hActiveEntityPosY]; swap a; inc a; rla; and $02; or e */
    uint8_t pos_y = gb_read_hram(gb, hActiveEntityPosY);

    uint8_t a = ((pos_y >> 4) + 1) << 1;

    a = (a & 0x02) | e;

    /* call SetEntitySpriteVariant */
    SetEntitySpriteVariant(gb, bc, a);

    /* cp $01; jr nz, SetZPosForFloatingItem */
    if (a == 0x01) {
        /* ld a, [wHasToadstool]; and a; jp nz, UnloadEntityAndReturn */
        if (gb_read(gb, wHasToadstool) != 0) {
            UnloadEntityAndReturn(gb, bc);
            return;
        }
    }

    /* fallthrough / jr nz: SetZPosForFloatingItem */
    SetZPosForFloatingItem(gb, bc);
}

/* ===== SetZPosForFloatingItem (03:4A12) ===== */
void SetZPosForFloatingItem(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesPosZTable; add hl, bc; ld [hl], $13; ret */
    gb_write(gb, wEntitiesPosZTable + bc, 0x13);
}

/* ===== EntityInitKid71 (03:4A19) ===== */
void EntityInitKid71(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wEntitiesDirectionTable; add hl, bc; ld [hl], DIRECTION_UP */
    gb_write(gb, wEntitiesDirectionTable + bc, DIRECTION_UP);

    /* call IncrementEntityState */
    IncrementEntityState(gb, bc);

    /* call GetEntityTransitionCountdown; ld [hl], $20 */
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x20);
}

/* ===== EntityInitKid72 (03:4A27) ===== */
void EntityInitKid72(GBState *gb) {
    if (!gb) return;

    /* ret */
    (void)gb;
}

/* ===== EntityInitMrWrite (03:4A28) ===== */
void EntityInitMrWrite(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ldh a, [hMapRoom]; cp ROOM_INDOOR_B_CHRISTINE_HOUSE; ld a, $32; jr nz, .jr_4A32; ld a, $37 */
    uint8_t map_room = gb_read_hram(gb, hMapRoom);
    uint8_t a = 0x32;
    if (map_room == ROOM_INDOOR_B_CHRISTINE_HOUSE) {
        a = 0x37;
    }

    /* jr jr_003_4A4F: call SetMusicTrackIfHasSword */
    SetMusicTrackIfHasSword(gb, a);

    /* The function continues to EntityShiftPosition.shiftBy8 for X position */
    EntityShiftPosition_shiftBy8(gb, bc, wEntitiesPosXSignTable, wEntitiesPosXTable);
}

/* ===== EntityInitBigFairy (03:4A34) ===== */
void EntityInitBigFairy(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld hl, wEntitiesPosZTable; add hl, bc; ld [hl], $10 */
    gb_write(gb, wEntitiesPosZTable + bc, 0x10);

    /* ld a, [wIsIndoor]; and a; jr z, .indoorEnd */
    /* ldh a, [hMapId]; cp MAP_COLOR_DUNGEON; jr z, jr_003_4A4D */
    bool is_color_dungeon = (gb_read(gb, wIsIndoor) != 0 && gb_read_hram(gb, hMapId) == MAP_COLOR_DUNGEON);
    if (!is_color_dungeon) {
        /* .indoorEnd: ld a, [wFullHearts]; and a; jp nz, UnloadEntityAndReturn */
        if (gb_read(gb, wFullHearts) != 0) {
            UnloadEntityAndReturn(gb, bc);
            return;
        }
    }

    /* jr_003_4A4D: ld a, $0C */
    /* jr_003_4A4F: call SetMusicTrackIfHasSword */
    SetMusicTrackIfHasSword(gb, 0x0C);

    /* ld de, wEntitiesPosXSignTable; ld hl, wEntitiesPosXTable; jp EntityShiftPosition.shiftBy8 */
    EntityShiftPosition_shiftBy8(gb, bc, wEntitiesPosXSignTable, wEntitiesPosXTable);
}

/* ===== EntityInitBowWow (03:4A5B) ===== */
void EntityInitBowWow(GBState *gb) {
    if (!gb) return;

    /* ldh a, [hMapRoom]; cp UNKNOWN_ROOM_E2; jr nz, .jr_4A6B */
    uint8_t map_room = gb_read_hram(gb, hMapRoom);

    if (map_room != UNKNOWN_ROOM_E2) {
        goto jr_4A6B;

    }

    /* ld a, [wIsBowWowFollowingLink]; cp BOW_WOW_KIDNAPPED; jr z, return; jp UnloadEntityAndReturn */
    uint8_t bowwow_following = gb_read(gb, wIsBowWowFollowingLink);

    if (bowwow_following == BOW_WOW_KIDNAPPED) {
        return;
    }
    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    UnloadEntityAndReturn(gb, bc);

    return;

jr_4A6B:
    /* ld a, [wIsBowWowFollowingLink]; and a; jp nz, UnloadEntityAndReturn */
    if (gb_read(gb, wIsBowWowFollowingLink) != 0) {
        uint16_t bc = gb_read(gb, wActiveEntityIndex);

        UnloadEntityAndReturn(gb, bc);

        return;
    }

    /* return: ret */
}

/* ===== EntityInitOwlEvent (03:4A73) ===== */
void EntityInitOwlEvent(GBState *gb) {
    if (!gb) return;

    /* ldh a, [hRoomStatus]; rra; jr UnloadEntityIfRoomStatusSet */
    uint8_t room_status = gb_read_hram(gb, hRoomStatus);

    if (room_status & 0x10) { /* bit 4 after rra means bit 5 before */
        UnloadEntityIfRoomStatusSet(gb);

    }
}

/* ===== EntityInitSword (03:4A78) ===== */
void EntityInitSword(GBState *gb) {
    if (!gb) return;

    /* ldh a, [hRoomStatus]; fallthrough to UnloadEntityIfRoomStatusSet */
    /* and $10; jp nz, UnloadEntityAndReturn */
    if (gb_read_hram(gb, hRoomStatus) & 0x10) {
        UnloadEntityIfRoomStatusSet(gb);

    }
}

/* ===== UnloadEntityIfRoomStatusSet (03:4A7A) ===== */
void UnloadEntityIfRoomStatusSet(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* and $10; jp nz, UnloadEntityAndReturn */
    if (gb_read_hram(gb, hRoomStatus) & 0x10) {
        UnloadEntityAndReturn(gb, bc);

        return;
    }

    /* ret */
}

/* ===== EntityInitMarin (03:4A80) ===== */
void EntityInitMarin(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ldh a, [hMapRoom]; cp UNKNOWN_ROOM_C0; jr c, .checkMarinDebug */
    uint8_t map_room = gb_read_hram(gb, hMapRoom);

    if (map_room < UNKNOWN_ROOM_C0) {
        goto checkMarinDebug;

    }

    /* ld a, [wIsMarinInAnimalVillage]; and a; jp z, UnloadEntityAndReturn */
    if (gb_read(gb, wIsMarinInAnimalVillage) == 0) {
        UnloadEntityAndReturn(gb, bc);

        return;
    }

    /* ld a, [wIsMarinFollowingLink]; and a; jp nz, UnloadEntityAndReturn */
    if (gb_read(gb, wIsMarinFollowingLink) != 0) {
        UnloadEntityAndReturn(gb, bc);

        return;
    }

    /* inc a; ld [wIsMarinSinging], a */
    gb_write(gb, wIsMarinSinging, 0x01);

    /* ld a, MUSIC_MARIN_SING; ldh [hNextMusicTrackToFadeInto], a; ldh [hDefaultMusicTrack], a; ldh [hDefaultMusicTrackAlt], a; call ResetMusicFadeTimer */
    gb_write_hram(gb, hNextMusicTrackToFadeInto, MUSIC_MARIN_SING);

    gb_write_hram(gb, hDefaultMusicTrack, MUSIC_MARIN_SING);

    gb_write_hram(gb, hDefaultMusicTrackAlt, MUSIC_MARIN_SING);

    ResetMusicFadeTimer(gb);

checkMarinDebug:
    /* ld a, [ROM_DebugTool1]; and a; jp z, EntityInitNpcFacingDown */
    if (gb->rom[ROM_DebugTool1] == 0) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wName]; and a; jr nz, EntityInitNpcFacingDown */
    if (gb_read(gb, wName) != 0) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wName + 1]; and a; jr nz, .enableTextDebugger */
    if (gb_read(gb, wName + 1) != 0) {
        goto enableTextDebugger;

    }

    /* ld [wGameplaySubtype], a; ld a, GAMEPLAY_CREDITS; ld [wGameplayType], a; ret */
    gb_write(gb, wGameplaySubtype, 0x00);

    gb_write(gb, wGameplayType, GAMEPLAY_CREDITS);

    return;

enableTextDebugger:
    /* ld hl, wEntitiesTypeTable; add hl, bc; ld [hl], ENTITY_TEXT_DEBUGGER; ret */
    gb_write(gb, wEntitiesTypeTable + bc, ENTITY_TEXT_DEBUGGER);

}

/* ===== EntityInitTarin (03:4ACE) ===== */
void EntityInitTarin(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ldh a, [hIsGBC]; and a; jr z, EntityInitNpcFacingDown */
    if (gb_read_hram(gb, hIsGBC) == 0) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wIsIndoor]; and a; jr z, EntityInitNpcFacingDown */
    if (gb_read(gb, wIsIndoor) == 0) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wIsMarinFollowingLink]; and a; jr nz, EntityInitNpcFacingDown */
    if (gb_read(gb, wIsMarinFollowingLink) != 0) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wHasInstrument3]; and $02; jr nz, EntityInitNpcFacingDown */
    if (gb_read(gb, wHasInstrument3) & 0x02) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wTradeSequenceItem]; cp TRADING_ITEM_BANANAS; jr nc, EntityInitNpcFacingDown */
    if (gb_read(gb, wTradeSequenceItem) >= TRADING_ITEM_BANANAS) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wTarinFlag]; and a; jr z, EntityInitNpcFacingDown */
    if (gb_read(gb, wTarinFlag) == 0) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* cp $01; jr z, EntityInitNpcFacingDown */
    if (gb_read(gb, wTarinFlag) == 0x01) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, $02; ldh [rSVBK], a */
    /* ld hl, wObjPal8; ld de, Data_003_4AC6; loop: ld a, [de]; ld [hl+], a; inc de; ld a, l; and $07; jr nz, loop; xor a; ldh [rSVBK], a; jr EntityInitNpcFacingDown */
    static const uint8_t Data_003_4AC6[8] = { 0xFF, 0x7F, 0xBE, 0x0F, 0x13, 0x02, 0x00, 0x00 };
    for (int i = 0; i < 8; i++) {
        gb_write(gb, wObjPal8 + i, Data_003_4AC6[i]);
    }
    EntityInitNpcFacingDown(gb, bc);

}

/* ===== EntityInitMadamMeowMeow (03:4B0E) ===== */
void EntityInitMadamMeowMeow(GBState *gb) {
    if (!gb) return;

    /* ld a, [wIsBowWowFollowingLink]; cp BOW_WOW_KIDNAPPED; jr nz, return */
    if (gb_read(gb, wIsBowWowFollowingLink) != BOW_WOW_KIDNAPPED) {
        return;
    }

    /* ld a, MUSIC_BOWWOW_KIDNAPPED; ld [wMusicTrackToPlay], a */
    gb_write(gb, wMusicTrackToPlay, MUSIC_BOWWOW_KIDNAPPED);

    /* return: ret */
}

/* ===== EntityInitRaftRaftOwner (03:4B1B) ===== */
void EntityInitRaftRaftOwner(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* ld a, [wIsIndoor]; and a; jr nz, EntityInitNpcFacingDown */
    if (gb_read(gb, wIsIndoor) != 0) {
        EntityInitNpcFacingDown(gb, bc);

        return;
    }

    /* ld a, [wD477]; and a; ret nz */
    if (gb_read(gb, wD477) != 0) {
        return;
    }

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld a, [hl]; sub $10; ld [hl], a; ret */
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + bc);

    gb_write(gb, wEntitiesPosYTable + bc, (uint8_t)(pos_y - 0x10));

}

/* ===== EntityInitNpcFacingDown (03:4B2F) ===== */
void EntityInitNpcFacingDown(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesDirectionTable; add hl, bc; ld [hl], DIRECTION_DOWN; fallthrough to EntityInitStoreOwner */
    gb_write(gb, wEntitiesDirectionTable + bc, DIRECTION_DOWN);

}

/* ===== EntityInitStoreOwner (03:4B35) ===== */
void EntityInitStoreOwner(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wShieldLevel]; and a; jr nz, .noShieldEnd; ld a, $1C; call SetMusicTrack */
    if (gb_read(gb, wShieldLevel) == 0) {
        SetMusicTrack(gb, 0x1C);

    }

    /* .noShieldEnd: jr EntityInitShopOwner.setDirectionLeft */
    /* falls through to EntityInitShopOwner.setDirectionLeft */
    EntityInitShopOwner_setDirectionLeft(gb, bc);

}

/* ===== EntityInitWitch (03:4B42) ===== */
void EntityInitWitch(GBState *gb) {
    if (!gb) return;

    /* ret */
    (void)gb;

}

/* ===== EntityInitShopOwner (03:4B43) ===== */
void EntityInitShopOwner(GBState *gb) {
    if (!gb) return;

    /* ld a, MUSIC_SHOP; call SetMusicTrackIfHasSword */
    SetMusicTrackIfHasSword(gb, MUSIC_SHOP);

}

/* ===== EntityInitShopOwner_setDirectionLeft (03:4B48) ===== */
void EntityInitShopOwner_setDirectionLeft(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* .setDirectionLeft: ld a, DIRECTION_LEFT; jr SetEntityDirection */
    SetEntityDirection(gb, bc, DIRECTION_LEFT);

}

/* ===== EntityInitWithRandomDirection (03:4B4C) ===== */
void EntityInitWithRandomDirection(GBState *gb) {
    if (!gb) return;

    uint16_t bc = gb_read(gb, wActiveEntityIndex);

    /* call GetRandomByte; and $03; fallthrough to SetEntityDirection */
    uint8_t random = GetRandomByte(gb);

    SetEntityDirection(gb, bc, random & 0x03);

}

/* ===== SetEntityDirection (03:4B51) ===== */
void SetEntityDirection(GBState *gb, uint16_t bc, uint8_t direction) {
    if (!gb) return;

    /* ld hl, wEntitiesDirectionTable; add hl, bc; ld [hl], a; fallthrough to EntityInitNoop */
    gb_write(gb, wEntitiesDirectionTable + bc, direction);

}

/* ===== EntityInitNoop (03:4B56) ===== */
void EntityInitNoop(GBState *gb) {
    if (!gb) return;

    /* ret */
    (void)gb;

}

/* ===== EntityInitEntity13 (03:59D7) ===== */
void EntityInitEntity13(GBState *gb) {
    if (!gb) return;

    /* ret */
    (void)gb;
}

/* ===== EntityShiftPosition (03:4F83) ===== */
void EntityShiftPosition(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, wEntitiesPosXSignTable; ld hl, wEntitiesPosXTable; call .shiftBy8 */
    EntityShiftPosition_shiftBy8(gb, bc, wEntitiesPosXSignTable, wEntitiesPosXTable);

    /* ld de, wEntitiesPosYSignTable; ld hl, wEntitiesPosYTable; fallthrough to .shiftBy8 */
    EntityShiftPosition_shiftBy8(gb, bc, wEntitiesPosYSignTable, wEntitiesPosYTable);

}

/* ===== EntityShiftPosition.shiftBy8 (03:4F92) ===== */
void EntityShiftPosition_shiftBy8(GBState *gb, uint16_t bc, uint16_t sign_table, uint16_t pos_table) {
    if (!gb) return;

    /* add hl, bc; ld a, [hl]; add $08; ld [hl], a */
    uint8_t pos = gb_read(gb, pos_table + bc);

    uint8_t new_pos = (uint8_t)(pos + 0x08);

    gb_write(gb, pos_table + bc, new_pos);

    /* rla; ld l, e; ld h, d; add hl, bc; rra; ld a, [hl]; adc $00; ld [hl], a */
    uint8_t carry = (pos >= 0xF8) ? 1 : 0;  /* carry from add $08 */
    uint8_t sign = gb_read(gb, sign_table + bc);

    uint8_t new_sign = (uint8_t)(sign + carry);

    gb_write(gb, sign_table + bc, new_sign);

}