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
#include "constants/link.h"
#include "home/entities.h"
#include "home/room.h"
#include "home/bank.h"
#include "home/audio.h"
#include "home/dialog.h"
#include "home/gameplay.h"
#include "home/link.h"
#include "home/vfx.h"

/* ===== HeartContainerSpriteVariants (03:59D8) ===== */
static const uint8_t HeartContainerSpriteVariants[4] = {
    0xAA, 0x14,
    0xAA, 0x34
};

/* ===== HeartContainerEntityHandler (03:59DC) ===== */
void HeartContainerEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, HeartContainerSpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, HeartContainerSpriteVariants, NULL);

    /* call GetEntityTransitionCountdown; jp z, PickableHandler */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    uint8_t countdown = gb_read(gb, countdown_addr);
    if (countdown == 0) {
        PickableHandler(gb, bc);
        return;
    }

    /* dec a; jr nz, HoldEntityAboveLink */
    if (countdown != 1) {
        HoldEntityAboveLink(gb, bc);
        return;
    }

    /* ld a, MUSIC_AFTER_BOSS; ld [wMusicTrackToPlay], a */
    gb_write(gb, wMusicTrackToPlay, MUSIC_AFTER_BOSS);

    /* ld hl, wMaxHearts; inc [hl] */
    uint8_t max_hearts = gb_read(gb, wMaxHearts);
    gb_write(gb, wMaxHearts, (uint8_t)(max_hearts + 1));

    /* ld hl, wAddHealthBuffer; ld [hl], $FF */
    gb_write(gb, wAddHealthBuffer, 0xFF);

    /* call GetRoomStatusAddressInHL; ld a, [hl]; or ROOM_STATUS_EVENT_2; ld [hl], a; ldh [hRoomStatus], a */
    uint16_t status_addr = GetRoomStatusAddressInHL(gb);
    uint8_t status = (uint8_t)(gb_read(gb, status_addr) | ROOM_STATUS_EVENT_2);
    gb_write(gb, status_addr, status);
    gb_write_hram(gb, hRoomStatus, status);

    /* ldh a, [hMapId]
       ld hl, wIndoorBRoomStatus + $2E
       cp MAP_EAGLES_TOWER; jr z, .inEaglesTower
       cp MAP_ANGLERS_TUNNEL; jr nz, .skipSecondRoomFlags
       ld hl, wIndoorARoomStatus + $66
       .inEaglesTower: set 5, [hl]
       .skipSecondRoomFlags: jp UnloadEntityAndReturn */
    uint8_t map_id = gb_read_hram(gb, hMapId);
    if (map_id == MAP_EAGLES_TOWER) {
        uint8_t val = gb_read(gb, wIndoorBRoomStatus + 0x2E);
        gb_write(gb, wIndoorBRoomStatus + 0x2E, (uint8_t)(val | 0x20));
    } else if (map_id == MAP_ANGLERS_TUNNEL) {
        uint8_t val = gb_read(gb, wIndoorARoomStatus + 0x66);
        gb_write(gb, wIndoorARoomStatus + 0x66, (uint8_t)(val | 0x20));
    }

    UnloadEntityAndReturn(gb, bc);
}

/* ===== HoldEntityAboveLink (03:5A17) ===== */
void HoldEntityAboveLink(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hLinkPositionX]; ld hl, wEntitiesPosXTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesPosXTable + bc, gb_read_hram(gb, hLinkPositionX));

    /* ldh a, [hLinkPositionY]; sub $0C; ld hl, wEntitiesPosYTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesPosYTable + bc, (uint8_t)(gb_read_hram(gb, hLinkPositionY) - 0x0C));

    /* ldh a, [hLinkPositionZ]; ld hl, wEntitiesPosZTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesPosZTable + bc, gb_read_hram(gb, hLinkPositionZ));

    /* jp func_003_5A2E */
    func_003_5A2E(gb, bc);
}

/* ===== func_003_5A2E (03:5A2E) ===== */
void func_003_5A2E(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, LINK_ANIMATION_STATE_GOT_ITEM; ldh [hLinkAnimationState], a */
    gb_write_hram(gb, hLinkAnimationState, LINK_ANIMATION_STATE_GOT_ITEM);

    /* ld a, DIRECTION_DOWN; ldh [hLinkDirection], a */
    gb_write_hram(gb, hLinkDirection, DIRECTION_DOWN);

    /* xor a; ld [wSwordAnimationState], a; ld [wC16A], a; ld [wSwordCharge], a; ld [wIsUsingSpinAttack], a */
    gb_write(gb, wSwordAnimationState, 0);
    gb_write(gb, wC16A, 0);
    gb_write(gb, wSwordCharge, 0);
    gb_write(gb, wIsUsingSpinAttack, 0);

    /* ld hl, wEntitiesGroundStatusTable; add hl, bc; ld [hl], a */
    gb_write(gb, wEntitiesGroundStatusTable + bc, 0);

    /* ld a, $02; ldh [hLinkInteractiveMotionBlocked], a; ret */
    gb_write_hram(gb, hLinkInteractiveMotionBlocked, 0x02);
}

/* ===== HeartPieceEntitySprite (03:5A44) ===== */
static const uint8_t HeartPieceEntitySprite[4] = {
    0xAC, OAM_GBC_PAL_2,
    0xAC, OAM_GBC_PAL_2 | OAMF_XFLIP
};

/* ===== HeartPieceSpriteVariants (03:5AF1) ===== */
static const uint8_t HeartPieceSpriteVariants[20] = {
    0x9A, OAM_GBC_PAL_2,
    0x9A, OAM_GBC_PAL_2 | OAMF_XFLIP,
    0x9C, OAM_GBC_PAL_2,
    0x9A, OAM_GBC_PAL_2 | OAMF_XFLIP,
    0x9E, OAM_GBC_PAL_2,
    0x9A, OAM_GBC_PAL_2 | OAMF_XFLIP,
    0x9E, OAM_GBC_PAL_2,
    0x9C, OAM_GBC_PAL_2 | OAMF_XFLIP,
    0x9E, OAM_GBC_PAL_2,
    0x9E, OAM_GBC_PAL_2 | OAMF_XFLIP
};

/* ===== DrawHeartPiecesInDialog (03:5B0E) ===== */
void DrawHeartPiecesInDialog(GBState *gb, uint16_t bc) {
    (void)bc;
    if (!gb) return;

    /* ld a, [wDialogState]; and a; ret z */
    uint8_t dialog_state = gb_read(gb, wDialogState);
    if (dialog_state == 0) {
        return;
    }

    /* ld a, [wDialogCharacterIndex]; cp $21; ret nc */
    uint8_t char_idx = gb_read(gb, wDialogCharacterIndex);
    if (char_idx >= 0x21) {
        return;
    }

    /* ld a, [wDialogState]; and DIALOG_BOX_BOTTOM_FLAG; ld a, $23; jr z, .positionSprite; ld a, $6B */
    uint8_t visual_pos_y = (dialog_state & DIALOG_BOX_BOTTOM_FLAG) ? 0x6B : 0x23;

    /* .positionSprite:
       ldh [hActiveEntityVisualPosY], a
       ld a, [wHeartPiecesCount]; ldh [hActiveEntitySpriteVariant], a
       ld a, $8E; ldh [hActiveEntityPosX], a
       ld de, HeartPieceSpriteVariants; jp RenderActiveEntitySpritesPair */
    gb_write_hram(gb, hActiveEntityVisualPosY, visual_pos_y);
    gb_write_hram(gb, hActiveEntitySpriteVariant, gb_read(gb, wHeartPiecesCount));
    gb_write_hram(gb, hActiveEntityPosX, 0x8E);
    RenderActiveEntitySpritesPair(gb, HeartPieceSpriteVariants, NULL);
}

/* ===== HeartPieceState0Handler (03:5B35) ===== */
void HeartPieceState0Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, HeartPieceEntitySprite; call RenderActiveEntitySpritesPair; jp PickableHandler */
    RenderActiveEntitySpritesPair(gb, HeartPieceEntitySprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== HeartPieceState1Handler (03:5A63) ===== */
void HeartPieceState1Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);

    /* call GetEntityTransitionCountdown; ret nz */
    if (GetEntityTransitionCountdown(gb, bc) != 0) {
        return;
    }

    /* ld a, $01; ld [wC167], a; jp IncrementEntityState */
    gb_write(gb, wC167, 0x01);
    IncrementEntityState(gb, bc);
}

/* ===== HeartPieceState2Handler (03:5A72) ===== */
void HeartPieceState2Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, TILESET_LOAD_PIECE_OF_HEART_1; ldh [hNeedsUpdatingBGTiles], a; jp IncrementEntityState */
    gb_write_hram(gb, hNeedsUpdatingBGTiles, TILESET_LOAD_PIECE_OF_HEART_1);
    IncrementEntityState(gb, bc);
}

/* ===== HeartPieceState3Handler (03:5A77) ===== */
void HeartPieceState3Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, TILESET_LOAD_PIECE_OF_HEART_2; ldh [hNeedsUpdatingBGTiles], a; jp IncrementEntityState */
    gb_write_hram(gb, hNeedsUpdatingBGTiles, TILESET_LOAD_PIECE_OF_HEART_2);
    IncrementEntityState(gb, bc);
}

/* ===== HeartPieceState4Handler (03:5A7C) ===== */
void HeartPieceState4Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call_open_dialog Dialog04F */
    OpenDialogInTable0(gb, Dialog04F);

    /* call IncrementEntityState */
    IncrementEntityState(gb, bc);

    /* ld a, $01; ld [wDialogInteractionLocked], a; ret */
    gb_write(gb, wDialogInteractionLocked, 0x01);
}

/* ===== HeartPieceState5Handler (03:5A89) ===== */
void HeartPieceState5Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);

    /* ld de, HeartPieceEntitySprite; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, HeartPieceEntitySprite, NULL);

    /* call DrawHeartPiecesInDialog */
    DrawHeartPiecesInDialog(gb, bc);

    /* ld hl, wEntitiesInertiaTable; add hl, bc; inc [hl]; ld a, [hl] */
    uint16_t inertia_addr = wEntitiesInertiaTable + bc;
    uint8_t inertia = (uint8_t)(gb_read(gb, inertia_addr) + 1);
    gb_write(gb, inertia_addr, inertia);

    /* cp $A8; jp z, IncrementEntityState */
    if (inertia == 0xA8) {
        IncrementEntityState(gb, bc);
        return;
    }

    /* cp $38; jr nz, .ret_5ABA */
    if (inertia == 0x38) {
        /* ld a, [wHeartPiecesCount]; inc a; ld [wHeartPiecesCount], a */
        uint8_t count = gb_read(gb, wHeartPiecesCount);
        gb_write(gb, wHeartPiecesCount, (uint8_t)(count + 1));
    }
}

/* ===== HeartPieceState6Handler (03:5AAD) ===== */
void HeartPieceState6Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);

    /* ld de, HeartPieceEntitySprite; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, HeartPieceEntitySprite, NULL);

    /* xor a; ld [wDialogInteractionLocked], a */
    gb_write(gb, wDialogInteractionLocked, 0x00);

    /* call DrawHeartPiecesInDialog */
    DrawHeartPiecesInDialog(gb, bc);

    /* ld a, [wDialogState]; and a; ret nz */
    if (gb_read(gb, wDialogState) != 0) {
        return;
    }

    /* ld a, [wHeartPiecesCount]; cp $04; jr nz, .jr_5AED */
    if (gb_read(gb, wHeartPiecesCount) == 4) {
        /* ld a, JINGLE_NEW_HEART; ldh [hJingle], a */
        gb_write_hram(gb, hJingle, JINGLE_NEW_HEART);

        /* xor a; ld [wHeartPiecesCount], a */
        gb_write(gb, wHeartPiecesCount, 0x00);

        /* ld hl, wAddHealthBuffer; ld [hl], $40 */
        gb_write(gb, wAddHealthBuffer, 0x40);

        /* ld hl, wMaxHearts; inc [hl] */
        uint8_t max_hearts = gb_read(gb, wMaxHearts);
        gb_write(gb, wMaxHearts, (uint8_t)(max_hearts + 1));

        /* call_open_dialog Dialog050 */
        OpenDialogInTable0(gb, Dialog050);
    }

    /* .jr_5AED: jp IncrementEntityState */
    IncrementEntityState(gb, bc);
}

/* ===== HeartPieceState7Handler (03:5ACE) ===== */
void HeartPieceState7Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);

    /* ld de, HeartPieceEntitySprite; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, HeartPieceEntitySprite, NULL);

    /* ld a, [wDialogState]; and a; ret nz */
    if (gb_read(gb, wDialogState) != 0) {
        return;
    }

    /* ld a, TILESET_CLEAR_PIECE_OF_HEART_1; ldh [hNeedsUpdatingBGTiles], a; jp IncrementEntityState */
    gb_write_hram(gb, hNeedsUpdatingBGTiles, TILESET_CLEAR_PIECE_OF_HEART_1);
    IncrementEntityState(gb, bc);
}

/* ===== HeartPieceState8Handler (03:5AE0) ===== */
void HeartPieceState8Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, TILESET_CLEAR_PIECE_OF_HEART_2; ldh [hNeedsUpdatingBGTiles], a */
    gb_write_hram(gb, hNeedsUpdatingBGTiles, TILESET_CLEAR_PIECE_OF_HEART_2);

    /* call UnloadEntity */
    UnloadEntity(gb, bc);

    /* ld a, REPLACE_TILES_TRADING_ITEM; ldh [hReplaceTiles], a */
    gb_write_hram(gb, hReplaceTiles, REPLACE_TILES_TRADING_ITEM);

    /* xor a; ld [wC167], a */
    gb_write(gb, wC167, 0x00);

    /* jp MarkRoomCompleted */
    MarkRoomCompleted(gb);
}

/* ===== HeartPieceEntityHandler (03:5A48) ===== */
void HeartPieceEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hRoomStatus]; and ROOM_STATUS_EVENT_1; jp nz, UnloadEntityAndReturn */
    if (gb_read_hram(gb, hRoomStatus) & ROOM_STATUS_EVENT_1) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ldh a, [hActiveEntityState]; JP_TABLE */
    uint8_t state = gb_read_hram(gb, hActiveEntityState);
    switch (state) {
        case 0: HeartPieceState0Handler(gb, bc); break;
        case 1: HeartPieceState1Handler(gb, bc); break;
        case 2: HeartPieceState2Handler(gb, bc); break;
        case 3: HeartPieceState3Handler(gb, bc); break;
        case 4: HeartPieceState4Handler(gb, bc); break;
        case 5: HeartPieceState5Handler(gb, bc); break;
        case 6: HeartPieceState6Handler(gb, bc); break;
        case 7: HeartPieceState7Handler(gb, bc); break;
        case 8: HeartPieceState8Handler(gb, bc); break;
        default: break;
    }
}

/* ===== Data_003_5B5B (03:5B5B) ===== */
static const uint8_t Data_003_5B5B[2] = {
    0xAE, 0x14
};

/* ===== GuardianAcornEntityHandler (03:5B5D) ===== */
void GuardianAcornEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, Data_003_5B5B; call RenderActiveEntitySprite; jp PickableHandler */
    RenderActiveEntitySprite(gb, Data_003_5B5B, NULL);
    PickableHandler(gb, bc);
}

/* ===== PieceOfPowerSpriteVariants (03:5B65) ===== */
static const uint8_t PieceOfPowerSpriteVariants[8] = {
    0x14, 0x02, 0x14, 0x22,
    0x14, 0x14, 0x14, 0x34
};

/* ===== PieceOfPowerEntityHandler (03:5B6D) ===== */
void PieceOfPowerEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, PieceOfPowerSpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, PieceOfPowerSpriteVariants, NULL);

    /* ldh a, [hFrameCounter]; rra; rra; rra; and $01; call SetEntitySpriteVariant */
    uint8_t variant = (uint8_t)((gb_read_hram(gb, hFrameCounter) >> 3) & 0x01);
    SetEntitySpriteVariant(gb, bc, variant);

    /* jp PickableHandler */
    PickableHandler(gb, bc);
}

/* ===== IronMasksMaskSpriteVariants (03:5B80) ===== */
static const uint8_t IronMasksMaskSpriteVariants[8] = {
    0x74, 0x00, 0x76, 0x00,
    0x76, 0x20, 0x74, 0x20
};

/* ===== IronMasksMaskEntityHandler (03:5B88) ===== */
void IronMasksMaskEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, IronMasksMaskSpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, IronMasksMaskSpriteVariants, NULL);

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* call PickableHandleGrabbedByItemIfNeeded; ret */
    PickableHandleGrabbedByItemIfNeeded(gb, bc);
}

/* ===== Data_003_5B95 (03:5B95) ===== */
static const uint8_t Data_003_5B95[2] = {
    0x86, 0x17
};

/* ===== Data_003_5B97 (03:5B97) ===== */
static const uint8_t Data_003_5B97[2] = {
    0x84, 0x17
};

/* ===== SwordShieldPickableState0Handler (03:5BAE) ===== */
void SwordShieldPickableState0Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; jp z, PickableHandler */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    uint8_t countdown = gb_read(gb, countdown_addr);
    if (countdown == 0) {
        PickableHandler(gb, bc);
        return;
    }

    /* cp $10; jr nz, .playSwordFanfare */
    if (countdown == 0x10) {
        /* dec [hl] */
        gb_write(gb, countdown_addr, 0x0F);

        /* call_open_dialog Dialog09B */
        OpenDialogInTable0(gb, Dialog09B);

        /* xor a; fallthrough to .playSwordFanfare: dec a (a becomes $FF, nz) */
        HoldEntityAboveLink(gb, bc);
        return;
    }

    /* .playSwordFanfare: dec a; jr nz, .holdItemAboveLink */
    if (countdown == 1) {
        /* ld a, MUSIC_OVERWORLD_INTRO; ld [wMusicTrackToPlay], a */
        gb_write(gb, wMusicTrackToPlay, MUSIC_OVERWORLD_INTRO);

        /* ld a, MUSIC_OVERWORLD; ldh [hDefaultMusicTrack], a; ldh [hNextDefaultMusicTrack], a */
        gb_write_hram(gb, hDefaultMusicTrack, MUSIC_OVERWORLD);
        gb_write_hram(gb, hNextDefaultMusicTrack, MUSIC_OVERWORLD);

        /* call GetEntitySlowTransitionCountdown; ld [hl], $52 */
        gb_write(gb, wEntitiesSlowTransitionCountdownTable + bc, 0x52);

        /* call IncrementEntityState */
        IncrementEntityState(gb, bc);
    }

    /* .holdItemAboveLink: jp HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);
}

/* ===== SwordShieldPickableState1Handler (03:5BCD) ===== */
void SwordShieldPickableState1Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);

    /* call GetEntitySlowTransitionCountdown; ret nz */
    if (GetEntitySlowTransitionCountdown(gb, bc) != 0) {
        return;
    }

    /* ld a, $FF; call SetEntitySpriteVariant */
    SetEntitySpriteVariant(gb, bc, 0xFF);

    /* call GetEntityTransitionCountdown; ld [hl], $20 */
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x20);

    /* ld a, USING_SPIN_ATTACK_MAX; ld [wIsUsingSpinAttack], a */
    gb_write(gb, wIsUsingSpinAttack, USING_SPIN_ATTACK_MAX);

    /* ld a, NOISE_SFX_SPIN_ATTACK; ldh [hNoiseSfx], a */
    gb_write_hram(gb, hNoiseSfx, NOISE_SFX_SPIN_ATTACK);

    /* jp IncrementEntityState */
    IncrementEntityState(gb, bc);
}

/* ===== SwordShieldPickableState2Handler (03:5BE7) ===== */
void SwordShieldPickableState2Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; ret nz */
    if (GetEntityTransitionCountdown(gb, bc) != 0) {
        return;
    }

    /* ld [hl], 32 */
    gb_write(gb, wEntitiesTransitionCountdownTable + bc, 32);

    /* ld a, $00; call SetEntitySpriteVariant */
    SetEntitySpriteVariant(gb, bc, 0x00);

    /* jp IncrementEntityState */
    IncrementEntityState(gb, bc);
}

/* ===== SwordShieldPickableState3Handler (03:5BF7) ===== */
void SwordShieldPickableState3Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);

    /* ld a, LINK_ANIMATION_STATE_UNKNOWN_6B; ldh [hLinkAnimationState], a */
    gb_write_hram(gb, hLinkAnimationState, LINK_ANIMATION_STATE_UNKNOWN_6B);

    /* ldh a, [hLinkPositionX]; sub $04; ld hl, wEntitiesPosXTable; add hl, bc; ld [hl], a */
    uint8_t link_x = gb_read_hram(gb, hLinkPositionX);
    gb_write(gb, wEntitiesPosXTable + bc, (uint8_t)(link_x - 0x04));

    /* call GetEntityTransitionCountdown; jr nz, .continueToRaiseSword */
    uint8_t countdown = GetEntityTransitionCountdown(gb, bc);
    if (countdown == 0) {
        /* ld [wC167], a */
        gb_write(gb, wC167, 0x00);

        /* ld d, INVENTORY_SWORD; call GiveInventoryItem */
        GiveInventoryItem(gb, INVENTORY_SWORD);

        /* ld a, $01; ld [wSwordLevel], a */
        gb_write(gb, wSwordLevel, 0x01);

        /* call MarkRoomCompleted */
        MarkRoomCompleted(gb);

        /* jp UnloadEntityAndReturn */
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* .continueToRaiseSword: cp 26; jr nz, .return */
    if (countdown == 26) {
        /* ldh a, [hActiveEntityPosY]; sub $0C; call CheckLinkCollisionWithProjectile.showSwordPokeVfx */
        uint8_t vfx_y = (uint8_t)(gb_read_hram(gb, hActiveEntityPosY) - 0x0C);
        gb_write_hram(gb, hMultiPurpose1, vfx_y);
        gb_write_hram(gb, hMultiPurpose0, gb_read_hram(gb, hActiveEntityPosX));
        AddTranscientVfx(gb, TRANSCIENT_VFX_SWORD_POKE);

        /* ld a, JINGLE_SWORD_POKING; ldh [hJingle], a */
        gb_write_hram(gb, hJingle, JINGLE_SWORD_POKING);
    }
}

/* ===== SwordShieldPickableEntityHandler (03:5B99) ===== */
void SwordShieldPickableEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, Data_003_5B95
       ld a, [wSwordLevel]; and a; jr nz, .jr_5BAC
       ldh a, [hRoomStatus]; and ROOM_STATUS_EVENT_1; jp nz, UnloadEntityAndReturn
       ld de, Data_003_5B97 */
    const uint8_t *sprite = Data_003_5B95;
    if (gb_read(gb, wSwordLevel) == 0) {
        if (gb_read_hram(gb, hRoomStatus) & ROOM_STATUS_EVENT_1) {
            UnloadEntityAndReturn(gb, bc);
            return;
        }
        sprite = Data_003_5B97;
    }

    /* .jr_5BAC: call RenderActiveEntitySprite */
    RenderActiveEntitySprite(gb, sprite, NULL);

    /* ldh a, [hActiveEntityState]; JP_TABLE */
    uint8_t state = gb_read_hram(gb, hActiveEntityState);
    switch (state) {
        case 0: SwordShieldPickableState0Handler(gb, bc); break;
        case 1: SwordShieldPickableState1Handler(gb, bc); break;
        case 2: SwordShieldPickableState2Handler(gb, bc); break;
        case 3: SwordShieldPickableState3Handler(gb, bc); break;
        default: break;
    }
}

/* ===== HookshotSpriteData (03:5C47) ===== */
static const uint8_t HookshotSpriteData[2] = {
    0x8A, 0x14
};

/* ===== HookshotDropEntityHandler (03:5C49) ===== */
void HookshotDropEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hRoomStatus]; and ROOM_STATUS_EVENT_1; jp nz, UnloadEntityAndReturn */
    if ((gb_read_hram(gb, hRoomStatus) & ROOM_STATUS_EVENT_1) != 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ld de, HookshotSpriteData; call RenderActiveEntitySprite */
    RenderActiveEntitySprite(gb, HookshotSpriteData, NULL);

    /* call GetEntityTransitionCountdown; jp z, PickableHandler */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    uint8_t countdown = gb_read(gb, countdown_addr);
    if (countdown == 0) {
        PickableHandler(gb, bc);
        return;
    }

    /* cp $10; jr nz, .skipUpdateSpeedY */
    if (countdown == 0x10) {
        /* dec [hl] */
        gb_write(gb, countdown_addr, 0x0F);
        /* call_open_dialog Dialog093 */
        OpenDialogInTable0(gb, Dialog093);
        /* xor a; dec a; jr nz, .decSpeedX (HoldEntityAboveLink) */
        HoldEntityAboveLink(gb, bc);
        return;
    }

    /* .skipUpdateSpeedY: dec a; jr nz, .decSpeedX */
    if (countdown == 1) {
        /* ld d, INVENTORY_HOOKSHOT; call GiveInventoryItem
           call MarkRoomCompleted
           jp UnloadEntityAndReturn */
        GiveInventoryItem(gb, INVENTORY_HOOKSHOT);
        MarkRoomCompleted(gb);
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* .decSpeedX: jp HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);
}

/* ===== KeyDropSpriteTable (03:5C78) ===== */
static const uint8_t KeyDropSpriteTable[12] = {
    0xCA, 0x17,
    0xC0, 0x17,
    0xC2, 0x14,
    0xC4, 0x17,
    0xC6, 0x14,
    0xCA, 0x17
};

/* ===== KeyCollectDialogs (03:5C84) ===== */
static const uint8_t KeyCollectDialogs[5] = {
    Dialog000, Dialog0A3, Dialog0A4, Dialog0A5, Dialog000
};

/* ===== KeyDropPointEntityHandler (03:5C89) ===== */
void KeyDropPointEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call CheckForEntityFallingDownQuicksandHole; jr nc, .jr_5C99 */
    if (CheckForEntityFallingDownQuicksandHole(gb, bc)) {
        /* ld hl, wOverworldRoomStatus + ROOM_OW_YARNA_LANMOLA; set OW_ROOM_STATUS_FLAG_CHANGED, [hl] */
        uint8_t s_ow = gb_read(gb, wOverworldRoomStatus + ROOM_OW_YARNA_LANMOLA);
        gb_write(gb, wOverworldRoomStatus + ROOM_OW_YARNA_LANMOLA, (uint8_t)(s_ow | (1 << OW_ROOM_STATUS_FLAG_CHANGED)));

        /* ld hl, wIndoorARoomStatus + ROOM_INDOOR_A_QUICKSAND_CAVE; set 5, [hl]; ret */
        uint8_t s_in = gb_read(gb, wIndoorARoomStatus + ROOM_INDOOR_A_QUICKSAND_CAVE);
        gb_write(gb, wIndoorARoomStatus + ROOM_INDOOR_A_QUICKSAND_CAVE, (uint8_t)(s_in | 0x20));
        return;
    }

    /* ldh a, [hMapRoom]; cp ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_4; jp z, label_003_5C49 */
    if (gb_read_hram(gb, hMapRoom) == ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_4) {
        HookshotDropEntityHandler(gb, bc);
        return;
    }

    /* ld de, KeyDropSpriteTable; call RenderActiveEntitySprite */
    RenderActiveEntitySprite(gb, KeyDropSpriteTable, NULL);

    /* call GetEntityTransitionCountdown; jp z, label_003_5CD6 */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    uint8_t countdown = gb_read(gb, countdown_addr);

    if (countdown == 0) {
        /* label_003_5CD6:
           call ReturnIfNonInteractive_03
           call PickableHandleGrabbedByItemIfNeeded
           ld hl, wEntitiesPosZTable; add hl, bc; ld a, [hl]; and a; jr nz, .jr_5CE7
           call PickableCollectIfNeeded
           .jr_5CE7:
           jp BouncingEntityPhysics */
        if (ReturnIfNonInteractive_03(gb, false)) {
            return;
        }

        if (PickableHandleGrabbedByItemIfNeeded(gb, bc)) {
            return;
        }

        if (gb_read(gb, wEntitiesPosZTable + bc) == 0) {
            if (PickableCollectIfNeeded(gb, bc)) {
                return;
            }
        }

        BouncingEntityPhysics(gb, bc);
        return;
    }

    /* cp $10; jr nz, .jr_5CCD */
    if (countdown == 0x10) {
        /* dec [hl] */
        gb_write(gb, countdown_addr, 0x0F);

        /* ldh a, [hActiveEntitySpriteVariant]; dec a; ld e, a; ld d, b; ld hl, KeyCollectDialogs; add hl, de; ld a, [hl]; call OpenDialogInTable0 */
        uint8_t variant = gb_read_hram(gb, hActiveEntitySpriteVariant);
        if (variant == 0) {
            variant = gb_read(gb, wEntitiesSpriteVariantTable + bc);
        }
        uint8_t idx = (uint8_t)(variant - 1);

        if (idx < sizeof(KeyCollectDialogs)) {
            OpenDialogInTable0(gb, KeyCollectDialogs[idx]);
        }

        /* ldh a, [hActiveEntitySpriteVariant]; dec a; ld e, a; ld d, b; ld hl, wHasTailKey; add hl, de; ld [hl], $01 */
        gb_write(gb, wHasTailKey + idx, 0x01);

        /* call MarkRoomCompleted */
        MarkRoomCompleted(gb);

        /* xor a; dec a; jr nz, .jr_5CD3 (HoldEntityAboveLink) */
        HoldEntityAboveLink(gb, bc);
        return;
    }

    /* .jr_5CCD: dec a; jr nz, .jr_5CD3 */
    if (countdown == 1) {
        /* jp UnloadEntityAndReturn */
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* .jr_5CD3: jp HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);
}

/* ===== DroppableHeartSprite (03:5D36) ===== */
static const uint8_t DroppableHeartSprite[2] = {
    0xA8, 0x14
};

/* ===== DroppableHeartEntityHandler (03:5D38) ===== */
void DroppableHeartEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableHeartSprite; call RenderActiveEntitySprite; jp PickableHandler */
    RenderActiveEntitySprite(gb, DroppableHeartSprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== SleepyToadstoolSprite (03:5D47) ===== */
static const uint8_t SleepyToadstoolSprite[4] = {
    0x5E, 0x02,
    0x5E, 0x22
};

/* ===== SleepyToadstoolEntityHandler (03:5D4B) ===== */
void SleepyToadstoolEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wHasToadstool; ld a, [wMagicPowderCount]; or [hl]; jp nz, UnloadEntityAndReturn */
    if ((gb_read(gb, wMagicPowderCount) | gb_read(gb, wHasToadstool)) != 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ld de, SleepyToadstoolSprite; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, SleepyToadstoolSprite, NULL);

    /* call GetEntityTransitionCountdown; jp z, PickableHandler */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    uint8_t countdown = gb_read(gb, countdown_addr);
    if (countdown == 0) {
        PickableHandler(gb, bc);
        return;
    }

    /* cp $10; jr nz, .jr_5D6C */
    if (countdown == 0x10) {
        /* dec [hl] */
        gb_write(gb, countdown_addr, 0x0F);
        /* call_open_dialog Dialog00F */
        OpenDialogInTable0(gb, Dialog00F);
        /* xor a; dec a; jr nz, .jr_5D80 (HoldEntityAboveLink) */
        HoldEntityAboveLink(gb, bc);
        return;
    }

    /* .jr_5D6C: dec a; jr nz, .jr_5D80 */
    if (countdown == 1) {
        /* ld a, REPLACE_TILES_TOADSTOOL; ldh [hReplaceTiles], a */
        gb_write_hram(gb, hReplaceTiles, REPLACE_TILES_TOADSTOOL);
        /* ld d, INVENTORY_MAGIC_POWDER; call GiveInventoryItem */
        GiveInventoryItem(gb, INVENTORY_MAGIC_POWDER);
        /* ld a, TRUE; ld [wHasToadstool], a */
        gb_write(gb, wHasToadstool, 0x01);
        /* jp UnloadEntityAndReturn */
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* .jr_5D80: jp HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);
}

/* ===== SirensInstrument2SpriteVariants (03:5D83) ===== */
static const uint8_t SirensInstrument2SpriteVariants[16] = {
    0x70, 0x01, 0x72, 0x01,  /* variant 0 */
    0x74, 0x01, 0x76, 0x01,  /* variant 1 */
    0x78, 0x01, 0x7A, 0x01,  /* variant 2 */
    0x7C, 0x01, 0x7E, 0x01   /* variant 3 */
};

/* ===== SirensInstrumentEntityHandler (03:5D93) ===== */
void SirensInstrumentEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesPrivateState1Table; add hl, bc; ld a, [hl]; JP_TABLE */
    uint8_t priv1 = gb_read(gb, wEntitiesPrivateState1Table + bc);
    switch (priv1) {
        case 0: SirensInstrumentState0Handler(gb, bc); break;
        case 1: SirensInstrumentState1Handler(gb, bc); break;
        case 2: SirensInstrumentState2Handler(gb, bc); break;
        default: break;
    }
}

/* ===== Data_003_5D9F (03:5D9F) ===== */
static const uint8_t Data_003_5D9F[29] = {
    0xE4, 0xE4, 0xE4, 0xE4, 0x90, 0x90, 0x90, 0x90, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* ===== Data_003_5DBC (03:5DBC) ===== */
static const uint8_t Data_003_5DBC[29] = {
    0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* ===== AfterSirensInstrumentD1 (03:5E0C) ===== */
void AfterSirensInstrumentD1(GBState *gb) {
    if (!gb) return;
    /* Mark Bow-Wow as kidnapped */
    gb_write(gb, wIsBowWowFollowingLink, BOW_WOW_KIDNAPPED);
}

/* ===== AfterSirensInstrumentD2 (03:5E12) ===== */
void AfterSirensInstrumentD2(GBState *gb) {
    if (!gb) return;
    gb_write(gb, wTarinFlag, 0x02);
}

/* ===== AfterSirensInstrumentD3 (03:5E18) ===== */
void AfterSirensInstrumentD3(GBState *gb) {
    (void)gb;
}

/* ===== AfterSirensInstrumentD4 (03:5E19) ===== */
void AfterSirensInstrumentD4(GBState *gb) {
    if (!gb) return;
    /* Mark Ghost as following Link */
    gb_write(gb, wIsGhostFollowingLink, 0x02);
}

/* ===== AfterSirensInstrumentNone (03:5E1E) ===== */
void AfterSirensInstrumentNone(GBState *gb) {
    (void)gb;
}

/* ===== AfterSirensInstrumentD6 (03:5E1F) ===== */
void AfterSirensInstrumentD6(GBState *gb) {
    if (!gb) return;
    /* Mark Marin as disappeared */
    gb_write(gb, wIsMarinInAnimalVillage, 0x00);
}

/* ===== AfterSirensInstrumentD7 (03:5E24) ===== */
void AfterSirensInstrumentD7(GBState *gb) {
    if (!gb) return;
    /* Make the rooster not following Link anymore */
    gb_write(gb, wIsRoosterFollowingLink, 0x00);
}

/* ===== animateSirensInstrumentPickup (03:5E29) ===== */
void animateSirensInstrumentPickup(GBState *gb, uint16_t bc, uint8_t slow_countdown) {
    if (!gb) return;

    /* cp $50; jr nc, ret_003_5E8A */
    if (slow_countdown >= 0x50) {
        return;
    }

    /* ld hl, wEntitiesPrivateState2Table; add hl, bc; ld a, [hl]; cp $19; jr nc, ret_003_5E8A */
    uint16_t priv2_addr = wEntitiesPrivateState2Table + bc;
    uint8_t priv2 = gb_read(gb, priv2_addr);
    if (priv2 >= 0x19) {
        return;
    }

    /* ldh a, [hFrameCounter]; and $07; jr nz, jr_003_5E5B */
    if ((gb_read_hram(gb, hFrameCounter) & 0x07) == 0) {
        /* ld a, [hl]; and a; jr nz, .jr_5E45; ld a, NOISE_SFX_INSTRUMENT_WARP; ldh [hNoiseSfx], a */
        if (priv2 == 0) {
            gb_write_hram(gb, hNoiseSfx, NOISE_SFX_INSTRUMENT_WARP);
        }

        /* .jr_5E45: inc [hl] */
        gb_write(gb, priv2_addr, (uint8_t)(priv2 + 1));

        /* cp $18; jr nz, jr_003_5E5B */
        if (priv2 == 0x18) {
            /* ld a, ENTITY_GOOMBA; call SpawnNewEntity */
            uint16_t new_entity = SpawnNewEntityInRange_impl(gb, ENTITY_GOOMBA, bc, MAX_ENTITIES - 1);
            if (new_entity != 0xFFFF) {
                /* ld hl, wEntitiesPrivateState1Table; add hl, de; ld [hl], $01 */
                gb_write(gb, (uint16_t)(wEntitiesPrivateState1Table + new_entity), 0x01);
                /* ld hl, wEntitiesTransitionCountdownTable; add hl, de; ld [hl], $60 */
                gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + new_entity), 0x60);
            }
        }
    }

    /* jr_003_5E5B:
       ldh a, [hFrameCounter]; and $03; ld hl, wEntitiesPrivateState2Table; add hl, bc; add [hl]; ld e, a; ld d, b */
    uint8_t current_priv2 = gb_read(gb, priv2_addr);
    uint8_t palette_idx = (uint8_t)((gb_read_hram(gb, hFrameCounter) & 0x03) + current_priv2);

    /* ldh a, [hIsGBC]; and a; jr z, .jr_5E76 */
    if (gb_read_hram(gb, hIsGBC) != 0) {
        func_020_6D0E_trampoline(gb, 0x03, NULL);
    } else {
        /* .jr_5E76:
           ld hl, Data_003_5D9F; add hl, de; ld a, [hl]; ld [wBGPalette], a
           ld hl, Data_003_5DBC; add hl, de; ld a, [hl]; ld [wOBJ0Palette], a
           xor a; ld [wOBJ1Palette], a */
        if (palette_idx < sizeof(Data_003_5D9F)) {
            gb_write(gb, wBGPalette, Data_003_5D9F[palette_idx]);
            gb_write(gb, wOBJ0Palette, Data_003_5DBC[palette_idx]);
            gb_write(gb, wOBJ1Palette, 0);
        }
    }
}

/* ===== SirensInstrumentState2Handler (03:5DD9) ===== */
void SirensInstrumentState2Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call func_006_783C_trampoline */
    func_006_783C_trampoline(gb, NULL);

    /* ld a, $01; ld [wC167], a */
    gb_write(gb, wC167, 0x01);

    /* call GetEntitySlowTransitionCountdown; jr nz, animateSirensInstrumentPickup */
    uint8_t slow_countdown = GetEntitySlowTransitionCountdown(gb, bc);
    if (slow_countdown != 0) {
        animateSirensInstrumentPickup(gb, bc, slow_countdown);
        return;
    }

    /* call UnloadEntity */
    UnloadEntity(gb, bc);

    /* xor a; ldh [hLinkAnimationState], a */
    gb_write_hram(gb, hLinkAnimationState, 0);

    /* ld a, [wD201]; ld e, a; ld d, b; ld hl, wEntitiesStateTable; add hl, de; inc [hl] */
    uint8_t target_entity = gb_read(gb, wD201);
    uint8_t state = gb_read(gb, (uint16_t)(wEntitiesStateTable + target_entity));
    gb_write(gb, (uint16_t)(wEntitiesStateTable + target_entity), (uint8_t)(state + 1));

    /* call disableMovementInTransition */
    disableMovementInTransition(gb);

    /* ldh a, [hMapId]; JP_TABLE */
    uint8_t map_id = gb_read_hram(gb, hMapId);
    switch (map_id) {
        case 0: AfterSirensInstrumentD1(gb); break;
        case 1: AfterSirensInstrumentD2(gb); break;
        case 2: AfterSirensInstrumentD3(gb); break;
        case 3: AfterSirensInstrumentD4(gb); break;
        case 4: AfterSirensInstrumentNone(gb); break;
        case 5: AfterSirensInstrumentD6(gb); break;
        case 6: AfterSirensInstrumentD7(gb); break;
        case 7: AfterSirensInstrumentNone(gb); break;
        default: break;
    }
}

/* ===== SirensInstrument1SpriteVariants (03:5E8B) ===== */
static const uint8_t SirensInstrument1SpriteVariants[8] = {
    0x6C, 0x00, 0xFF, 0xFF,  /* variant 0 */
    0x6C, 0x00, 0x6E, 0x00   /* variant 1 */
};

/* ===== SirensInstrumentState1Handler (03:5E93) ===== */
void SirensInstrumentState1Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld de, SirensInstrument1SpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, SirensInstrument1SpriteVariants, NULL);

    /* call UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);

    /* call GetEntityTransitionCountdown; jp z, UnloadEntityAndReturn */
    if (GetEntityTransitionCountdown(gb, bc) == 0) {
        UnloadEntityAndReturn(gb, bc);
    }
}

/* ===== InstrumentMusicTable (03:5F04) ===== */
static const uint8_t InstrumentMusicTable[8] = {
    MUSIC_INSTRUMENT_FULL_MOON_CELLO,
    MUSIC_INSTRUMENT_CONCH_HORN,
    MUSIC_INSTRUMENT_SEA_LILYS_BELL,
    MUSIC_INSTRUMENT_SURF_HARP,
    MUSIC_INSTRUMENT_WIND_MARIMBA,
    MUSIC_INSTRUMENT_CORAL_TRIANGLE,
    MUSIC_INSTRUMENT_ORGAN_OF_EVENING_CALM,
    MUSIC_INSTRUMENT_THUNDER_DRUM
};

/* ===== func_003_5ED5 (03:5ED5) ===== */
void func_003_5ED5(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; jp z, PickableHandler */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    uint8_t countdown = gb_read(gb, countdown_addr);
    if (countdown == 0) {
        PickableHandler(gb, bc);
        return;
    }

    /* cp $10; jr nz, .jr_5EFE */
    if (countdown == 0x10) {
        /* dec [hl] */
        gb_write(gb, countdown_addr, 0x0F);

        /* call IncrementEntityState */
        IncrementEntityState(gb, bc);

        /* ldh a, [hMapId]; add $00; call OpenDialogInTable1 */
        uint8_t map_id = gb_read_hram(gb, hMapId);
        OpenDialogInTable1(gb, map_id);

        /* ldh a, [hMapId]; ld e, a; ld d, b; ld hl, wHasInstrument1; add hl, de; ld a, [hl]; or $02; ld [hl], a */
        uint16_t inst_addr = (uint16_t)(wHasInstrument1 + map_id);
        gb_write(gb, inst_addr, (uint8_t)(gb_read(gb, inst_addr) | 0x02));

        /* call GetRoomStatusAddressInHL; ld a, [hl]; or $10; ld [hl], a */
        uint16_t room_status_addr = GetRoomStatusAddressInHL(gb);
        gb_write(gb, room_status_addr, (uint8_t)(gb_read(gb, room_status_addr) | ROOM_STATUS_EVENT_1));
    }

    /* .jr_5EFE: dec a; jp HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);
}

/* ===== func_003_5F0C (03:5F0C) ===== */
void func_003_5F0C(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wActiveMusicIndex]; and a; jr nz, .jr_5F2C
       ld a, [wDialogState]; and a; jr nz, .jr_5F2C */
    if (gb_read(gb, wActiveMusicIndex) == 0 && gb_read(gb, wDialogState) == 0) {
        /* ldh a, [hMapId]; ld e, a; ld d, b; ld hl, InstrumentMusicTable; add hl, de; ld a, [hl]; ld [wMusicTrackToPlay], a */
        uint8_t map_id = gb_read_hram(gb, hMapId);
        if (map_id < sizeof(InstrumentMusicTable)) {
            gb_write(gb, wMusicTrackToPlay, InstrumentMusicTable[map_id]);
        }

        /* call IncrementEntityState */
        IncrementEntityState(gb, bc);

        /* call GetEntityTransitionCountdown; ld [hl], $FF */
        gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0xFF);
    }

    /* .jr_5F2C: jp HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);
}

/* ===== Data_003_5F2F (03:5F2F) ===== */
static const uint8_t Data_003_5F2F[2] = {
    0x0A, 0xFA
};

/* ===== Data_003_5F31 (03:5F31) ===== */
static const uint8_t Data_003_5F31[2] = {
    0x04, 0xFC
};

/* ===== func_003_5F33 (03:5F33) ===== */
void func_003_5F33(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call GetEntityTransitionCountdown; jr nz, .jr_5F5F */
    if (GetEntityTransitionCountdown(gb, bc) == 0) {
        /* ld a, JINGLE_INSTRUMENT_WARP; ldh [hJingle], a */
        gb_write_hram(gb, hJingle, JINGLE_INSTRUMENT_WARP);

        /* ld a, ENTITY_INSTRUMENT_OF_THE_SIRENS; call SpawnNewEntity */
        uint16_t new_entity = SpawnNewEntityInRange_impl(gb, ENTITY_INSTRUMENT_OF_THE_SIRENS, bc, MAX_ENTITIES - 1);
        if (new_entity != 0xFFFF) {
            /* ldh a, [hMultiPurpose0]; dec a; ld hl, wEntitiesPosXTable; add hl, de; ld [hl], a */
            uint8_t pos_x = (uint8_t)(gb_read_hram(gb, hMultiPurpose0) - 1);
            gb_write(gb, (uint16_t)(wEntitiesPosXTable + new_entity), pos_x);

            /* ldh a, [hMultiPurpose1]; ld hl, wEntitiesPosYTable; add hl, de; ld [hl], a */
            uint8_t pos_y = gb_read_hram(gb, hMultiPurpose1);
            gb_write(gb, (uint16_t)(wEntitiesPosYTable + new_entity), pos_y);

            /* ld hl, wEntitiesPrivateState1Table; add hl, de; ld [hl], $02 */
            gb_write(gb, (uint16_t)(wEntitiesPrivateState1Table + new_entity), 0x02);

            /* ld hl, wEntitiesSlowTransitionCountdownTable; add hl, de; ld [hl], $80 */
            gb_write(gb, (uint16_t)(wEntitiesSlowTransitionCountdownTable + new_entity), 0x80);
        }

        /* jp IncrementEntityState */
        IncrementEntityState(gb, bc);
        return;
    }

    /* .jr_5F5F: ld hl, wEntitiesPrivateState3Table; add hl, bc; dec [hl]; ld a, [hl]; cp $FF; jr nz, .jr_5FB9 */
    uint16_t priv3_addr = wEntitiesPrivateState3Table + bc;
    uint8_t priv3 = (uint8_t)(gb_read(gb, priv3_addr) - 1);
    gb_write(gb, priv3_addr, priv3);

    if (priv3 == 0xFF) {
        /* ld [hl], $17 */
        gb_write(gb, priv3_addr, 0x17);

        /* ld hl, wEntitiesPrivateState4Table; add hl, bc; inc [hl]; ld a, [hl]; and $01; ldh [hMultiPurposeG], a */
        uint16_t priv4_addr = wEntitiesPrivateState4Table + bc;
        uint8_t priv4 = (uint8_t)(gb_read(gb, priv4_addr) + 1);
        gb_write(gb, priv4_addr, priv4);
        uint8_t bit0 = priv4 & 0x01;
        gb_write_hram(gb, hMultiPurposeG, bit0);

        /* ld a, ENTITY_INSTRUMENT_OF_THE_SIRENS; call SpawnNewEntity */
        uint16_t new_entity = SpawnNewEntityInRange_impl(gb, ENTITY_INSTRUMENT_OF_THE_SIRENS, bc, MAX_ENTITIES - 1);
        if (new_entity != 0xFFFF) {
            /* ld hl, wEntitiesPrivateState1Table; add hl, de; inc [hl] */
            uint8_t priv1 = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesPrivateState1Table + new_entity)) + 1);
            gb_write(gb, (uint16_t)(wEntitiesPrivateState1Table + new_entity), priv1);

            /* ldh a, [hMultiPurposeG]; ld c, a; ld hl, Data_003_5F2F; add hl, bc; ldh a, [hMultiPurpose0]; add [hl]; ld hl, wEntitiesPosXTable; add hl, de; ld [hl], a */
            uint8_t offset_x = Data_003_5F2F[bit0];
            uint8_t pos_x = (uint8_t)(gb_read_hram(gb, hMultiPurpose0) + offset_x);
            gb_write(gb, (uint16_t)(wEntitiesPosXTable + new_entity), pos_x);

            /* ld hl, Data_003_5F31; add hl, bc; ld a, [hl]; ld hl, wEntitiesSpeedXTable; add hl, de; ld [hl], a */
            uint8_t speed_x = Data_003_5F31[bit0];
            gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + new_entity), speed_x);

            /* ldh a, [hMultiPurpose1]; ld hl, wEntitiesPosYTable; add hl, de; add $F8; ld [hl], a */
            uint8_t pos_y = (uint8_t)(gb_read_hram(gb, hMultiPurpose1) + 0xF8);
            gb_write(gb, (uint16_t)(wEntitiesPosYTable + new_entity), pos_y);

            /* ld hl, wEntitiesSpeedYTable; add hl, de; ld [hl], $FD */
            gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + new_entity), 0xFD);

            /* ld hl, wEntitiesTransitionCountdownTable; add hl, de; ld [hl], $38 */
            gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + new_entity), 0x38);

            /* call GetRandomByte; and $01; ld hl, wEntitiesSpriteVariantTable; add hl, de; ld [hl], a */
            uint8_t variant = (uint8_t)(GetRandomByte(gb) & 0x01);
            gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + new_entity), variant);
        }
    }

    /* .jr_5FB9: jp HoldEntityAboveLink */
    HoldEntityAboveLink(gb, bc);
}

/* ===== func_003_5FBC (03:5FBC) ===== */
void func_003_5FBC(GBState *gb, uint16_t bc) {
    if (!gb) return;
    HoldEntityAboveLink(gb, bc);
}

/* ===== func_003_5FBF (03:5FBF) ===== */
void func_003_5FBF(GBState *gb, uint16_t bc) {
    (void)gb;
    (void)bc;
}

/* ===== SirensInstrumentState0Handler (03:5EA3) ===== */
void SirensInstrumentState0Handler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hActiveEntityState]; cp $03; jr nc, .jr_5EAE */
    uint8_t active_state = gb_read_hram(gb, hActiveEntityState);
    if (active_state < 0x03) {
        /* ld a, $03; call cycleInstrumentItemColor_trampoline */
        cycleInstrumentItemColor_trampoline(gb, 0x03, NULL);
    }

    /* .jr_5EAE: ld a, c; ld [wD201], a */
    gb_write(gb, wD201, (uint8_t)(bc & 0xFF));

    /* ldh a, [hRoomStatus]; and ROOM_STATUS_EVENT_1; jp nz, UnloadEntityAndReturn */
    if (gb_read_hram(gb, hRoomStatus) & ROOM_STATUS_EVENT_1) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ldh a, [hMapId]; and $03; ldh [hActiveEntitySpriteVariant], a */
    uint8_t map_id = gb_read_hram(gb, hMapId);
    gb_write_hram(gb, hActiveEntitySpriteVariant, (uint8_t)(map_id & 0x03));

    /* call label_394D */
    label_394D(gb, NULL);

    /* ld de, SirensInstrument2SpriteVariants; call RenderActiveEntitySpritesPair */
    RenderActiveEntitySpritesPair(gb, SirensInstrument2SpriteVariants, NULL);

    /* ldh a, [hActiveEntityState]; JP_TABLE */
    active_state = gb_read_hram(gb, hActiveEntityState);
    switch (active_state) {
        case 0: func_003_5ED5(gb, bc); break;
        case 1: func_003_5F0C(gb, bc); break;
        case 2: func_003_5F33(gb, bc); break;
        case 3: func_003_5FBC(gb, bc); break;
        case 4: func_003_5FBF(gb, bc); break;
        default: break;
    }
}

/* ===== DroppableBombsSprite (03:5FC0) ===== */
static const uint8_t DroppableBombsSprite[2] = {
    0x80, 0x15
};

/* ===== DroppableBombsEntityHandler (03:5FC2) ===== */
void DroppableBombsEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableBombsSprite; call RenderActiveEntitySprite; jp PickableHandler */
    RenderActiveEntitySprite(gb, DroppableBombsSprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== DroppableSeashellSprite (03:5FD1) ===== */
static const uint8_t DroppableSeashellSprite[2] = {
    0x9E, 0x14
};

/* ===== DroppableSeashellEntityHandler (03:5FD3) ===== */
void DroppableSeashellEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wSwordLevel]; cp $02; jp nc, UnloadEntityAndReturn */
    if (gb_read(gb, wSwordLevel) >= 2) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ldh a, [hRoomStatus]; and ROOM_STATUS_EVENT_1; jp nz, UnloadEntityAndReturn */
    if ((gb_read_hram(gb, hRoomStatus) & ROOM_STATUS_EVENT_1) != 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* ldh a, [hMapRoom]; cp UNKNOWN_ROOM_E3; jr nz, .jr_5FEF */
    if (gb_read_hram(gb, hMapRoom) == UNKNOWN_ROOM_E3) {
        /* ldh a, [hRoomStatus]; and ROOM_STATUS_EVENT_3; jp z, UnloadEntityAndReturn */
        if ((gb_read_hram(gb, hRoomStatus) & ROOM_STATUS_EVENT_3) == 0) {
            UnloadEntityAndReturn(gb, bc);
            return;
        }
    }

    /* .jr_5FEF: call DroppableRevealOrReturnIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;

    /* ld de, DroppableSeashellSprite; call RenderActiveEntitySprite; jp PickableHandler */
    RenderActiveEntitySprite(gb, DroppableSeashellSprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== HidingSlimeKeySprite (03:5FFB) ===== */
static const uint8_t HidingSlimeKeySprite[2] = {
    0xCA, 0x14
};

/* ===== HidingSlimeKeyEntityHandler (03:5FFD) ===== */
void HidingSlimeKeyEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hRoomStatus]; and ROOM_STATUS_EVENT_1; jp nz, UnloadEntityAndReturn */
    if ((gb_read_hram(gb, hRoomStatus) & ROOM_STATUS_EVENT_1) != 0) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    /* call DroppableRevealOrReturnIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;

    /* ld de, HidingSlimeKeySprite; call RenderActiveEntitySprite */
    RenderActiveEntitySprite(gb, HidingSlimeKeySprite, NULL);

    /* call GetEntityTransitionCountdown; jp z, PickableHandler */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    uint8_t countdown = gb_read(gb, countdown_addr);
    if (countdown == 0) {
        PickableHandler(gb, bc);
        return;
    }

    /* cp $10; jr nz, jr_003_604C */
    if (countdown == 0x10) {
        /* dec [hl] */
        gb_write(gb, countdown_addr, 0x0F);

        /* ld a, [wIsIndoor]; and a; jr nz, .jr_6029 */
        if (gb_read(gb, wIsIndoor) == 0) {
            /* ldh a, [hMapRoom]; cp ROOM_OW_POTHOLE_FIELD_SLIME_KEY; jr nz, .jr_6029 */
            if (gb_read_hram(gb, hMapRoom) == ROOM_OW_POTHOLE_FIELD_SLIME_KEY) {
                /* ld a, GOLDEN_LEAVES_5; ld [wGoldenLeavesCount], a */
                gb_write(gb, wGoldenLeavesCount, GOLDEN_LEAVES_5);
            }
        }

        /* .jr_6029:
           ld hl, wGoldenLeavesCount; call IncreaseValueAtHLClampAt99 */
        IncreaseValueAtHLClampAt99_addr(gb, wGoldenLeavesCount);

        /* call MarkRoomCompleted */
        MarkRoomCompleted(gb);

        /* ld hl, hRoomStatus; res 4, [hl] */
        uint8_t room_status = gb_read_hram(gb, hRoomStatus);
        room_status &= (uint8_t)(~(1 << OW_ROOM_STATUS_FLAG_CHANGED));
        gb_write_hram(gb, hRoomStatus, room_status);

        /* ld_dialog_low e, Dialog0A2; ld a, [wGoldenLeavesCount]; cp SLIME_KEY; jr z, .openDialog */
        uint8_t leaves = gb_read(gb, wGoldenLeavesCount);
        uint8_t dialog = Dialog0A2;
        if (leaves != SLIME_KEY) {
            /* ld_dialog_low e, Dialog0E8; cp GOLDEN_LEAVES_5; jr nz, .openDialog; inc e */
            dialog = (leaves == GOLDEN_LEAVES_5) ? Dialog0E9 : Dialog0E8;
        }

        /* .openDialog: ld a, e; call OpenDialogInTable0 */
        OpenDialogInTable0(gb, dialog);

        /* xor a; jr_003_604C: dec a; jp nz, HoldEntityAboveLink */
        HoldEntityAboveLink(gb, bc);
        return;
    }

    /* jr_003_604C: dec a; jp nz, HoldEntityAboveLink; jp UnloadEntityAndReturn */
    if (countdown == 1) {
        UnloadEntityAndReturn(gb, bc);
        return;
    }

    HoldEntityAboveLink(gb, bc);
}

/* ===== data_003_6157 (03:6157) ===== */
static const uint8_t data_003_6157[4] = {
    0x20, 0x21,
    0x20, 0x01
};

/* ===== DroppableFairyEntityHandler (03:615B) ===== */
void DroppableFairyEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, data_003_6157; call RenderActiveEntitySprite */
    RenderActiveEntitySprite(gb, data_003_6157, NULL);

    /* call ReturnIfNonInteractive_03 */
    if (ReturnIfNonInteractive_03(gb, false)) {
        return;
    }

    /* call PickableHandleGrabbedByItemIfNeeded */
    if (PickableHandleGrabbedByItemIfNeeded(gb, bc)) {
        return;
    }

    /* call PickableCollectIfNeeded */
    if (PickableCollectIfNeeded(gb, bc)) {
        return;
    }

    /* ld hl, wEntitiesSpeedXTable; add hl, bc; ld a, [hl]; rlca; and $01; call SetEntitySpriteVariant */
    uint8_t speed_x = gb_read(gb, wEntitiesSpeedXTable + bc);
    uint8_t variant = (uint8_t)((speed_x >> 7) & 0x01);
    SetEntitySpriteVariant(gb, bc, variant);

    /* call UpdateEntityPosWithSpeed_03 */
    UpdateEntityPosWithSpeed_03(gb, bc);

    /* call func_003_61C0 */
    func_003_61C0(gb, bc);

    /* call ApplyEntityInteractionWithBackground */
    ApplyEntityInteractionWithBackground(gb, bc);

    /* call GetEntityXDistanceToLink_03; ld a, d; bit 7, a; jr z, .jr_618C; .jr_618C: cp $20; jr c, jr_003_619C */
    uint8_t dir_x, dist_x;
    GetEntityXDistanceToLink_03_idx(gb, bc, &dir_x, &dist_x);
    if (dist_x >= 0x20) {
        /* call GetEntityYDistanceToLink_03; ld a, d; bit 7, a; jr z, .jr_6198; .jr_6198: cp $20; jr nc, jr_003_61BB */
        uint8_t dir_y, dist_y;
        GetEntityYDistanceToLink_03_idx(gb, bc, &dir_y, &dist_y);
        if (dist_y >= 0x20) {
            /* jr_003_61BB: ld a, $09; jp ApplyVectorTowardsLinkAndReturn */
            ApplyVectorTowardsLink_with_length(gb, bc, 0x09);
            return;
        }
    }

    /* jr_003_619C: call GetEntityTransitionCountdown; ret nz */
    uint16_t countdown_addr = wEntitiesTransitionCountdownTable + bc;
    if (gb_read(gb, countdown_addr) != 0) {
        return;
    }

    /* ld [hl], $30 */
    gb_write(gb, countdown_addr, 0x30);

    /* call GetRandomByte; and $0F; sub $08; ld hl, wEntitiesSpeedXTable; add hl, bc; ld [hl], a */
    uint8_t rand_x = (uint8_t)((GetRandomByte(gb) & 0x0F) - 0x08);
    gb_write(gb, wEntitiesSpeedXTable + bc, rand_x);

    /* call GetRandomByte; and $0F; sub $08; ld hl, wEntitiesSpeedYTable; add hl, bc; ld [hl], a; ret */
    uint8_t rand_y = (uint8_t)((GetRandomByte(gb) & 0x0F) - 0x08);
    gb_write(gb, wEntitiesSpeedYTable + bc, rand_y);
}

/* ===== DroppableMagicPowderSprite (03:6055) ===== */
static const uint8_t DroppableMagicPowderSprite[2] = {
    0x8E, 0x16  /* tile $8E, OAM_GBC_PAL_6 | OAMF_PAL1 */
};

/* ===== DroppableMagicPowderEntityHandler (03:6057) ===== */
void DroppableMagicPowderEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wIsIndoor]; and a; jr z, .jr_6063 */
    /* ldh a, [hMapId]; cp MAP_COLOR_DUNGEON; jr z, jr_003_606A */
    /* .jr_6063: ld a, [wHasToadstool]; and a; jp nz, UnloadEntityAndReturn */
    if (gb_read(gb, wIsIndoor) != 0) {
        if (gb_read_hram(gb, hMapId) != MAP_COLOR_DUNGEON) {
            if (gb_read(gb, wHasToadstool) != 0) {
                UnloadEntityAndReturn(gb, bc);
                return;
            }
        }
    } else {
        if (gb_read(gb, wHasToadstool) != 0) {
            UnloadEntityAndReturn(gb, bc);
            return;
        }
    }

    /* jr_003_606A: call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableMagicPowderSprite; call RenderActiveEntitySprite; jp PickableHandler */
    RenderActiveEntitySprite(gb, DroppableMagicPowderSprite, NULL);
    PickableHandler(gb, bc);
}

/* ===== DroppableArrowSprite (03:6079) ===== */
static const uint8_t DroppableArrowSprite[4] = {
    0x2A, 0x41,  /* tile $2A, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_YFLIP */
    0x2A, 0x61   /* tile $2A, OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_YFLIP | OAMF_XFLIP */
};

/* ===== DroppableArrowsEntityHandler (03:607D) ===== */
void DroppableArrowsEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableArrowSprite; call RenderActiveEntitySpritesPair; jp PickableHandler */
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
    uint8_t variant = (uint8_t)((countdown & 0x01) - 1);
    SetEntitySpriteVariant(gb, bc, variant);
}

/* ===== DroppableRupeeSprite (03:609C) ===== */
static const uint8_t DroppableRupeeSprite[2] = {
    0xA6, 0x15  /* tile $A6, OAM_GBC_PAL_5 | OAMF_PAL1 */
};

/* ===== DroppableRupeeEntityHandler (03:609E) ===== */
void DroppableRupeeEntityHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call DroppableRevealOrReturnIfNeeded; call DroppableDisappearIfNeeded */
    if (DroppableRevealOrReturnIfNeeded(gb, bc)) return;
    DroppableDisappearIfNeeded(gb, bc);

    /* ld de, DroppableRupeeSprite; call RenderActiveEntitySprite; fallthrough to PickableHandler */
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
    if (PickableHandleGrabbedByItemIfNeeded(gb, bc)) {
        return;
    }

    /* call PickableCollectIfNeeded */
    if (PickableCollectIfNeeded(gb, bc)) {
        return;
    }

    /* fallthrough to BouncingEntityPhysics */
    BouncingEntityPhysics(gb, bc);
}

/* ===== DroppableRevealOrReturnIfNeeded (03:61DE) ===== */
bool DroppableRevealOrReturnIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    /* ld hl, wEntitiesPrivateState3Table; add hl, bc; ld a, [hl]; and a; jp z, .return */
    uint8_t private_state3 = gb_read(gb, wEntitiesPrivateState3Table + bc);
    if (private_state3 == 0) {
        return false;
    }

    /* ld a, [wRoomTransitionState]; and a; jp nz, .remainInvisible */
    if (gb_read(gb, wRoomTransitionState) != 0) {
        return true;
    }

    /* ld a, [hl]; cp $02; jr nz, .checkPegasusBootsCollision */
    if (private_state3 != 0x02) {
        goto checkPegasusBootsCollision;
    }

    /* Items buried, hidden in bushes, or indoors: */
    /* ldh a, [hActiveEntityType]; cp ENTITY_DROPPABLE_SECRET_SEASHELL; jr z, .skipNotActiveIfIndoors */
    if (gb_read_hram(gb, hActiveEntityType) != ENTITY_DROPPABLE_SECRET_SEASHELL) {
        /* If indoors and not a seashell, the item can't be dug up or dropped by bushes. */
        /* ld a, [wIsIndoor]; and a; jp nz, .remainInvisible */
        if (gb_read(gb, wIsIndoor) != 0) {
            return true;
        }
    }

    /* .skipNotActiveIfIndoors: call func_003_7E0E */
    func_003_7E0E(gb, bc);

    /* ldh a, [hActiveEntityType]; cp ENTITY_DROPPABLE_HEART; jr z, .activeIfOnShortGrass */
    uint8_t active_type = gb_read_hram(gb, hActiveEntityType);
    if (active_type == ENTITY_DROPPABLE_HEART) {
        goto activeIfOnShortGrass;
    }

    /* cp ENTITY_DROPPABLE_SECRET_SEASHELL; jr nz, .activeIfOnShortGrassEnd */
    if (active_type != ENTITY_DROPPABLE_SECRET_SEASHELL) {
        goto activeIfOnShortGrassEnd;
    }

    /* Seashells buried under short grass (some of these don't exist) */
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
        return true;
    }

setOptionsAndReveal:
    /* ld hl, wEntitiesOptions1Table; add hl, bc; ld [hl], ENTITY_OPT1_SPLASH_IN_WATER|ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL; jr .reveal */
    gb_write(gb, wEntitiesOptions1Table + bc, ENTITY_OPT1_SPLASH_IN_WATER | ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL);
    goto reveal;

checkPegasusBootsCollision:
    /* Items knocked down with the Pegasus Boots: */
    /* ld a, [wScreenShakeCountdown]; and a; jr z, .remainInvisible */
    if (gb_read(gb, wScreenShakeCountdown) == 0) {
        return true;
    }

    /* ld a, [wPegasusBootsCollisionCountdown]; and a; jr z, .remainInvisible */
    if (gb_read(gb, wPegasusBootsCollisionCountdown) == 0) {
        return true;
    }

    /* ldh a, [hActiveEntityPosX]; add $08; ld hl, wPegasusBootsCollisionPosX; sub [hl]; add $10; cp $20; jr nc, .remainInvisible */
    uint8_t pos_x = gb_read_hram(gb, hActiveEntityPosX);
    uint8_t pegasus_x = gb_read(gb, wPegasusBootsCollisionPosX);
    uint8_t diff_x = (uint8_t)((uint8_t)(pos_x + 8) - pegasus_x) + 0x10;
    if (diff_x >= 0x20) {
        return true;
    }

    /* ldh a, [hActiveEntityPosY]; add $08; ld hl, wPegasusBootsCollisionPosY; sub [hl]; add $10; cp $20; jr nc, .remainInvisible */
    uint8_t pos_y = gb_read_hram(gb, hActiveEntityPosY);
    uint8_t pegasus_y = gb_read(gb, wPegasusBootsCollisionPosY);
    uint8_t diff_y = (uint8_t)((uint8_t)(pos_y + 8) - pegasus_y) + 0x10;
    if (diff_y >= 0x20) {
        return true;
    }

reveal:
    /* Items revealed are thrown away from Link */
    /* ld hl, wEntitiesPrivateState3Table; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesPrivateState3Table + bc, 0);
    /* ld hl, wEntitiesPrivateState4Table; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesPrivateState4Table + bc, 0);
    /* call GetEntityPrivateCountdown1; ld [hl], $18 */
    gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x18);

    /* ld a, $0C; call GetVectorTowardsLink */
    GetVectorTowardsLink_with_length(gb, 0x0C, NULL, NULL);
    uint8_t vx = gb_read_hram(gb, hMultiPurpose1);
    uint8_t vy = gb_read_hram(gb, hMultiPurpose0);
    gb_write(gb, wEntitiesSpeedXTable + bc, (uint8_t)(~vx + 1));
    gb_write(gb, wEntitiesSpeedYTable + bc, (uint8_t)(~vy + 1));

    /* ld hl, wEntitiesSpeedZTable; add hl, bc; ld [hl], $20 */
    gb_write(gb, wEntitiesSpeedZTable + bc, 0x20);

    /* call GetEntitySlowTransitionCountdown; ld [hl], $80 */
    gb_write(gb, wEntitiesSlowTransitionCountdownTable + bc, 0x80);

    return true;
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
    if ((pos_z & 0x80) != 0) {
        /* inc [hl]; jr ret_003_61DD */
        gb_write(gb, wEntitiesPosZTable + bc, (uint8_t)(pos_z + 1));
        return;
    }

    /* .jr_61D6: cp $10; jr nc, .jr_61DC */
    if (pos_z >= 0x10) {
        /* dec [hl]; ret */
        gb_write(gb, wEntitiesPosZTable + bc, (uint8_t)(pos_z - 1));
        return;
    }

    /* inc [hl]; ret */
    gb_write(gb, wEntitiesPosZTable + bc, (uint8_t)(pos_z + 1));
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
bool PickableHandleGrabbedByItemIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    uint8_t grabbed = gb_read(gb, wEntitiesPrivateState5Table + bc);
    if (grabbed == 0) {
        return false;
    }

    uint8_t grabber = (uint8_t)(grabbed - 1);
    uint8_t status = gb_read(gb, wEntitiesStatusTable + grabber);
    if (status == 0) {
        PickableCollect(gb, bc);
        return true;
    }

    uint8_t type = gb_read(gb, wEntitiesTypeTable + grabber);
    if (type != ENTITY_BOOMERANG && type != ENTITY_HOOKSHOT_CHAIN) {
        PickableCollect(gb, bc);
        return true;
    }

    /* Snap to boomerang or hookshot */
    uint8_t pos_x = gb_read(gb, wEntitiesPosXTable + grabber);
    gb_write(gb, wEntitiesPosXTable + bc, pos_x);
    uint8_t pos_y = gb_read(gb, wEntitiesPosYTable + grabber);
    gb_write(gb, wEntitiesPosYTable + bc, pos_y);
    gb_write(gb, wEntitiesPosZTable + bc, 0);
    return true;
}

/* ===== PickableCollectIfNeeded (03:62EB) ===== */
bool PickableCollectIfNeeded(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    if (GetEntityPrivateCountdown1(gb, bc) != 0) {
        return false;
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
        return false;
    }

    PickableCollect(gb, bc);
    return true;
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

/* ===== Chest Data Tables (03:504F-03:5068) ===== */

/* OpenChestTilesGBC (03:504F) */
const uint8_t OpenChestTilesGBC[4] = {
    0x62, 0x70, 0x63, 0x71
};

/* OpenChestTiles (03:5053) */
const uint8_t OpenChestTiles[4] = {
    0x62, 0x70, 0x62, 0x70
};

/* ChestToInventoryMappingTable (03:5057) */
const uint8_t ChestToInventoryMappingTable[12] = {
    INVENTORY_POWER_BRACELET, /* CHEST_POWER_BRACELET */
    INVENTORY_SHIELD,         /* CHEST_SHIELD */
    INVENTORY_BOW,            /* CHEST_BOW */
    INVENTORY_HOOKSHOT,       /* CHEST_HOOKSHOT */
    INVENTORY_MAGIC_ROD,      /* CHEST_MAGIC_ROD */
    INVENTORY_PEGASUS_BOOTS,  /* CHEST_PEGASUS_BOOTS */
    INVENTORY_OCARINA,        /* CHEST_OCARINA */
    INVENTORY_ROCS_FEATHER,   /* CHEST_FEATHER */
    INVENTORY_SHOVEL,         /* CHEST_SHOVEL */
    INVENTORY_MAGIC_POWDER,   /* CHEST_MAGIC_POWDER_BAG */
    INVENTORY_BOMBS,          /* CHEST_BOMB */
    INVENTORY_SWORD           /* CHEST_SWORD */
};

/* ChestRupeeCountHigh (03:5063) */
const uint8_t ChestRupeeCountHigh[5] = {
    0, 0, 0, 0, 1
};

/* ChestRupeeCountLow (03:5068) */
const uint8_t ChestRupeeCountLow[5] = {
    50, 20, 100, 200, 244
};

/* ===== ChestGiveNoneInventoryItem (03:5125) ===== */
void ChestGiveNoneInventoryItem(GBState *gb, uint8_t variant) {
    if (!gb) return;
    uint8_t count = gb_read(gb, (uint16_t)(wInventoryItems + variant));
    gb_write(gb, (uint16_t)(wInventoryItems + variant), (uint8_t)(count + 1));
    MarkRoomCompleted(gb);
}

/* ===== EntityInitChestWithItem (03:506D) ===== */
void EntityInitChestWithItem(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, $2A; ld [wC111], a */
    gb_write(gb, wC111, 0x2A);

    /* ld a, NOISE_SFX_DOOR_UNLOCKED; ldh [hNoiseSfx], a */
    gb_write_hram(gb, hNoiseSfx, NOISE_SFX_DOOR_UNLOCKED);

    /* ld de, OpenChestTilesGBC; ldh a, [hIsGBC]; and a; jr z, .jr_5081; ld de, OpenChestTiles */
    const uint8_t *tiles = (gb_read_hram(gb, hIsGBC) == 0) ? OpenChestTilesGBC : OpenChestTiles;

    /* ld b, $A1; call func_003_51C9 */
    func_003_51C9(gb, bc, tiles, OBJECT_CHEST_OPEN);

    /* ld hl, wEntitiesPosYTable; add hl, bc; ld a, [hl]; sub $08; ld [hl], a */
    uint8_t pos_y = gb_read(gb, (uint16_t)(wEntitiesPosYTable + bc));
    gb_write(gb, (uint16_t)(wEntitiesPosYTable + bc), (uint8_t)(pos_y - 8));

    /* ld hl, wEntitiesSpeedYTable; add hl, bc; ld [hl], $FC */
    gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + bc), 0xFC);

    /* ld hl, wEntitiesSpriteVariantTable; add hl, bc; ld a, [hl]; ldh [hMultiPurposeG], a */
    uint8_t variant = gb_read(gb, (uint16_t)(wEntitiesSpriteVariantTable + bc));
    gb_write_hram(gb, hMultiPurposeG, variant);

    /* cp CHEST_TAIL_KEY; jr nz, .jr_50AC */
    if (variant == CHEST_TAIL_KEY) {
        /* ld a, [wOwlEntityIndex]; ld e, a; ld hl, wEntitiesPrivateCountdown1Table; add hl, de; ld [hl], $38 */
        uint8_t owl_index = gb_read(gb, wOwlEntityIndex);
        gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown1Table + owl_index), 0x38);
    }

    /* .jr_50AC: cp CHEST_MESSAGE; jp nc, MarkRoomCompleted */
    if (variant >= CHEST_MESSAGE) {
        MarkRoomCompleted(gb);
        return;
    }

    /* cp CHEST_SEASHELL; jr nz, .jr_50B9; jp label_003_636D */
    if (variant == CHEST_SEASHELL) {
        MarkRoomCompleted(gb);
        IncreaseValueAtHLClampAt99(gb);
        return;
    }

    /* .jr_50B9: cp CHEST_RUPEES_50; jr c, .jr_50D8; cp $20; jr nc, .jr_50D8 */
    if (variant >= CHEST_RUPEES_50 && variant < 0x20) {
        uint8_t rupee_idx = (uint8_t)(variant - CHEST_RUPEES_50);
        gb_write(gb, wAddRupeeBufferLow, ChestRupeeCountLow[rupee_idx]);
        gb_write(gb, wAddRupeeBufferHigh, ChestRupeeCountHigh[rupee_idx]);
        gb_write(gb, wC3CE, 0x18);
        MarkRoomCompleted(gb);
        return;
    }

    /* .jr_50D8: cp CHEST_MAP; jr c, .jr_50EF; cp CHEST_RUPEES_50; jr nc, .jr_50EF */
    if (variant >= CHEST_MAP && variant < CHEST_RUPEES_50) {
        uint8_t map_offset = (uint8_t)(variant - CHEST_MAP);
        uint8_t cur_val = gb_read(gb, (uint16_t)(wHasDungeonMap + map_offset));
        gb_write(gb, (uint16_t)(wHasDungeonMap + map_offset), (uint8_t)(cur_val + 1));
        SynchronizeDungeonsItemFlags_trampoline(gb, SynchronizeDungeonsItemFlags);
        MarkRoomCompleted(gb);
        return;
    }

    /* .jr_50EF: cp CHEST_FLIPPERS; jr nc, ChestGiveNoneInventoryItem */
    if (variant >= CHEST_FLIPPERS) {
        ChestGiveNoneInventoryItem(gb, variant);
        return;
    }

    /* ldh a, [hMultiPurposeG]; cp CHEST_SHIELD; jr nz, .shieldEnd; inc [hl] */
    if (variant == CHEST_SHIELD) {
        uint8_t shield = gb_read(gb, wShieldLevel);
        gb_write(gb, wShieldLevel, (uint8_t)(shield + 1));
    }

    /* cp CHEST_POWER_BRACELET; jr nz, .powerBraceletEnd */
    if (variant == CHEST_POWER_BRACELET) {
        /* ld a, [wPowerBraceletLevel]; cp $02; jr z, .powerBraceletEnd; inc [hl] */
        uint8_t bracelet = gb_read(gb, wPowerBraceletLevel);
        if (bracelet != 0x02) {
            gb_write(gb, wPowerBraceletLevel, (uint8_t)(bracelet + 1));
        }
    }

    /* cp CHEST_BOMB; jr nz, .bombsEnd */
    if (variant == CHEST_BOMB) {
        uint8_t bomb_count = gb_read(gb, wBombCount);
        gb_write(gb, wBombCount, bcd_add(bomb_count, 1));
    }

    /* ld d, [hl]; call GiveInventoryItem; jr MarkRoomCompleted */
    if (variant < 12) {
        uint8_t inv_item = ChestToInventoryMappingTable[variant];
        GiveInventoryItem(gb, inv_item);
    }
    MarkRoomCompleted(gb);
}