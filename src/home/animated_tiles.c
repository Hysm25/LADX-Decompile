#include "home/animated_tiles.h"
#include "home/bank.h"
#include "home/copy_data.h"
#include "constants/memory.h"
#include "constants/hardware.h"
#include "constants/gameplay.h"
#include "constants/link.h"
#include "constants/gfx.h"

static const uint8_t AnimatedTilesDataOffsets[8] = { 0, 0x40, 0x80, 0xC0, 0xC0, 0xC0, 0x80, 0x40 };

void AnimateMarinBeachTiles(GBState *gb) {
    if (!gb) return;
    if (gb_read(gb, wDrawCommand) != 0) {
        return;
    }

    uint8_t bank = AdjustBankNumberForGBC(gb, 0x10);
    gb_write(gb, rSelectROMBank, bank);

    uint16_t hl = 0x6500; /* MarinBeachWavesTiles */
    uint16_t de = 0x9500; /* vTiles2 + $500 */

    uint8_t frame = gb_read(gb, hFrameCounter);
    if ((frame & 0x0F) != 0) {
        if ((frame & 0x0F) != 0x08) {
            return;
        }
        hl = (hl & 0xFF00) | 0x40;
        de = (de & 0xFF00) | 0x40;
    }

    uint16_t bc = (uint16_t)((frame & 0x30) << 3);
    hl = (uint16_t)(hl + bc);

    CopyData(gb, de, hl, 0x40);
}

uint8_t IncrementAnimatedTilesDataOffset(GBState *gb) {
    if (!gb) return 0;
    uint8_t offset = (uint8_t)(gb_read(gb, hAnimatedTilesDataOffset) + 0x40);
    gb_write(gb, hAnimatedTilesDataOffset, offset);
    return offset;
}

void SkipTilesGroupAnimation(GBState *gb, void (*func_020_54f5)(GBState *)) {
    if (!gb) return;
    gb_write(gb, rSelectROMBank, 0x20);
    if (func_020_54f5) {
        func_020_54f5(gb);
    }
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);
}

void LoadAnimatedTilesFrame(GBState *gb, uint16_t src_hl, uint16_t dest_de,
                            void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;

    CopyData(gb, dest_de, src_hl, 0x40);

    if (gb_read(gb, hMapId) == MAP_COLOR_DUNGEON) {
        gb_write(gb, rSelectROMBank, 0x20);
        uint8_t bank1 = 0;
        if (configure_copy) {
            configure_copy(gb, 1, &bank1);
            if (bank1 != 0) {
                gb_write(gb, rSelectROMBank, bank1);
                CopyData(gb, dest_de, src_hl, 0x40);
            }
            gb_write(gb, rSelectROMBank, 0x20);
            uint8_t bank0 = 0;
            configure_copy(gb, 0, &bank0);
            if (bank0 != 0) {
                gb_write(gb, rSelectROMBank, bank0);
                CopyData(gb, 0x96C0, src_hl, 0x40);
            }
        }
    }
}

void LoadAnimatedTilesFrameAtOffset(GBState *gb, uint8_t group_high, uint8_t offset,
                                    void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    uint16_t src = (uint16_t)(((uint16_t)group_high << 8) | offset);
    LoadAnimatedTilesFrame(gb, src, 0x96C0, configure_copy);
}

void AnimateTilesSlowSpeed(GBState *gb, uint8_t group_high,
                           void (*func_020_54f5)(GBState *),
                           void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    if ((gb_read(gb, hAnimatedTilesFrameCount) & 0x0F) != 0) {
        SkipTilesGroupAnimation(gb, func_020_54f5);
        return;
    }
    uint8_t offset = IncrementAnimatedTilesDataOffset(gb);
    LoadAnimatedTilesFrameAtOffset(gb, group_high, offset, configure_copy);
}

void AnimateTilesMediumSpeed(GBState *gb, uint8_t group_high,
                             void (*func_020_54f5)(GBState *),
                             void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    if ((gb_read(gb, hAnimatedTilesFrameCount) & 0x07) != 0) {
        SkipTilesGroupAnimation(gb, func_020_54f5);
        return;
    }
    uint8_t offset = IncrementAnimatedTilesDataOffset(gb);
    LoadAnimatedTilesFrameAtOffset(gb, group_high, offset, configure_copy);
}

void AnimateTilesFastSpeed(GBState *gb, uint8_t group_high,
                           void (*func_020_54f5)(GBState *),
                           void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    if ((gb_read(gb, hAnimatedTilesFrameCount) & 0x03) != 0) {
        SkipTilesGroupAnimation(gb, func_020_54f5);
        return;
    }
    uint8_t offset = IncrementAnimatedTilesDataOffset(gb);
    LoadAnimatedTilesFrameAtOffset(gb, group_high, offset, configure_copy);
}

void AnimateCounterTilesGroup(GBState *gb,
                             void (*load_counter_tiles)(GBState *),
                             void (*func_020_54f5)(GBState *)) {
    if (!gb) return;
    if ((gb_read(gb, hAnimatedTilesFrameCount) & 0x07) != 0) {
        SkipTilesGroupAnimation(gb, func_020_54f5);
        return;
    }
    gb_write(gb, rSelectROMBank, 0x01);
    if (load_counter_tiles) {
        load_counter_tiles(gb);
    }
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);
}

void AnimateTideTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                           void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesSlowSpeed(gb, 0x6B, func_020_54f5, configure_copy);
}

void AnimateVillageTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                              void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesSlowSpeed(gb, 0x6C, func_020_54f5, configure_copy);
}

void AnimateWaterDungeonTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                   void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesSlowSpeed(gb, 0x73, func_020_54f5, configure_copy);
}

void AnimateSlowWaterfallTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                    void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesSlowSpeed(gb, 0x6A, func_020_54f5, configure_copy);
}

void AnimateUndergroundTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                  void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesSlowSpeed(gb, 0x6E, func_020_54f5, configure_copy);
}

void AnimateDungeon1TilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                               void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    uint8_t count = gb_read(gb, hAnimatedTilesFrameCount);
    if ((count & 0x07) != 0) {
        SkipTilesGroupAnimation(gb, func_020_54f5);
        return;
    }
    uint8_t idx = (uint8_t)((count >> 3) & 0x07);
    uint8_t offset = AnimatedTilesDataOffsets[idx];
    uint16_t hl = (uint16_t)((0x6D << 8) | offset);
    LoadAnimatedTilesFrame(gb, hl, 0x96C0, configure_copy);
}

void AnimateLavaTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                           void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    uint8_t count = gb_read(gb, hAnimatedTilesFrameCount);
    if ((count & 0x07) != 0) {
        SkipTilesGroupAnimation(gb, func_020_54f5);
        return;
    }
    uint8_t idx = (uint8_t)((count >> 3) & 0x07);
    uint8_t offset = AnimatedTilesDataOffsets[idx];
    uint16_t hl = (uint16_t)((0x6F << 8) | offset);
    LoadAnimatedTilesFrame(gb, hl, 0x96C0, configure_copy);
}

void label_1CB8(GBState *gb, void (*func_020_54f5)(GBState *),
                void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    uint8_t next_count = (uint8_t)(gb_read(gb, hAnimatedTilesFrameCount) + 1);
    if ((next_count & 0x03) != 0) {
        AnimateDungeon1TilesGroup(gb, func_020_54f5, configure_copy);
        return;
    }
    LoadAnimatedTilesFrame(gb, wAnimatedScrollingTilesStorage, 0x90C0, configure_copy);
}

void AnimateDungeon2TilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                               void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    if (!gb) return;
    if (gb_read(gb, hMapId) != MAP_COLOR_DUNGEON) {
        label_1CB8(gb, func_020_54f5, configure_copy);
        return;
    }
    LoadAnimatedTilesFrame(gb, wAnimatedScrollingTilesStorage, 0x8400, configure_copy);
}

void AnimateWarpTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                           void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesMediumSpeed(gb, 0x70, func_020_54f5, configure_copy);
}

void AnimateWaterCurrentsTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                    void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesFastSpeed(gb, 0x71, func_020_54f5, configure_copy);
}

void AnimateWaterfallTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesFastSpeed(gb, 0x72, func_020_54f5, configure_copy);
}

void AnimateLightBeamTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesFastSpeed(gb, 0x75, func_020_54f5, configure_copy);
}

void AnimateBubblesTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                              void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesMediumSpeed(gb, 0x74, func_020_54f5, configure_copy);
}

void AnimateWeatherVaneTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                  void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesMediumSpeed(gb, 0x77, func_020_54f5, configure_copy);
}

void AnimateCrystalBlockTilesGroup(GBState *gb, void (*func_020_54f5)(GBState *),
                                   void (*configure_copy)(GBState *, uint8_t, uint8_t *)) {
    AnimateTilesMediumSpeed(gb, 0x76, func_020_54f5, configure_copy);
}

void AnimatePhotoTilesGroup(GBState *gb, void (*func_038_7830)(GBState *)) {
    if (!gb) return;
    gb_write(gb, rSelectROMBank, 0x38);
    if (func_038_7830) {
        func_038_7830(gb);
    }
}

void CopyLinkTilesPair(GBState *gb, uint16_t src_bc, uint16_t dest_hl, uint8_t length_d) {
    if (!gb) return;
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);

    while (length_d > 0) {
        uint8_t val = gb_read(gb, src_bc++);
        gb_write(gb, dest_hl++, val);
        length_d--;
    }

    gb_write(gb, rSelectROMBank, 0x20);
}

void DrawLinkSprite(GBState *gb) {
    if (!gb) return;

    uint8_t anim_state = gb_read(gb, hLinkAnimationState);
    if (anim_state == 0xFF) {
        return;
    }

    uint8_t invincibility = gb_read(gb, wInvincibilityCounter);
    uint8_t c135 = 0;
    if (gb_read(gb, hIsGBC) != 0) {
        if ((invincibility & 0x04) != 0) {
            c135 = 0x04;
        }
    } else {
        c135 = (uint8_t)((invincibility << 2) & 0x10);
    }
    gb_write(gb, wC135, c135);

    uint8_t c13b = gb_read(gb, wC13B);
    uint8_t c145 = gb_read(gb, wC145);
    uint8_t y = (uint8_t)(c13b + c145);
    if (y >= 0x88) {
        return;
    }

    uint8_t c13c = gb_read(gb, wC13C);
    uint8_t pos_x = gb_read(gb, hLinkPositionX);
    uint8_t x1 = (uint8_t)(pos_x + c13c);
    uint8_t x2 = (uint8_t)(x1 + 8);

    uint8_t flags1 = (uint8_t)(gb_read(gb, wC11D) | c135);
    uint8_t flags2 = (uint8_t)(gb_read(gb, wC11E) | c135);

    if (gb_read(gb, hIsGBC) != 0) {
        if ((invincibility & 0x04) == 0) {
            if (anim_state >= LINK_ANIMATION_STATE_REVOLVING_DOOR_1 && anim_state < LINK_ANIMATION_STATE_FALLING_PIT_1) {
                flags1 |= 0x07;
                flags2 |= 0x07;
            } else {
                uint8_t tunic = gb_read(gb, wTunicType);
                if (tunic != 0) {
                    flags1 |= (uint8_t)(tunic + 1);
                    flags2 |= (uint8_t)(tunic + 1);
                }
                if (anim_state == LINK_ANIMATION_STATE_HOLD_SWIMMING_2 || anim_state == LINK_ANIMATION_STATE_MOVING_SWIMMING_2) {
                    flags1 = 0x03;
                    flags2 = 0x23;
                }
            }
        }
    }

    /* Write sprite 0 */
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 8), y);
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 9), x1);
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 10), 0x00);
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 11), flags1);

    /* Write sprite 1 */
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 12), y);
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 13), x2);
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 14), 0x02);
    gb_write(gb, (uint16_t)(wLinkOAMBuffer + 15), flags2);
}

void DrawLinkSpriteAndReturn(GBState *gb) {
    DrawLinkSprite(gb);
}

void CopyDataAndDrawLinkSprite(GBState *gb, uint16_t src_hl, uint16_t dest_de, uint16_t size_bc) {
    if (!gb) return;
    CopyData(gb, dest_de, src_hl, size_bc);
    gb_write(gb, hReplaceTiles, 0);
    gb_write(gb, rSelectROMBank, 0x0C);
    DrawLinkSpriteAndReturn(gb);
}

void Copy4TilesAndDrawLinkSprite(GBState *gb, uint16_t src_hl, uint16_t dest_de) {
    CopyDataAndDrawLinkSprite(gb, src_hl, dest_de, 0x40);
}

void ReplaceTilesPairAndDrawLinkSprite(GBState *gb, uint16_t src_hl, uint16_t dest_de) {
    if (!gb) return;
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);
    CopyDataAndDrawLinkSprite(gb, src_hl, dest_de, 0x20);
}

void ReplaceMarinTiles(GBState *gb, bool is_sitting) {
    if (!gb) return;
    uint16_t hl;
    uint8_t bank;
    if (is_sitting) {
        bank = AdjustBankNumberForGBC(gb, 0x12);
        hl = 0x6080; /* Npc3Tiles + $2080 */
    } else {
        bank = AdjustBankNumberForGBC(gb, 0x0E);
        hl = 0x4F00; /* Npc1Tiles + $F00 */
    }
    gb_write(gb, rSelectROMBank, bank);
    Copy4TilesAndDrawLinkSprite(gb, hl, 0x8400); /* vTiles0 + $400 */
}

void ReplaceTradingItemTiles(GBState *gb) {
    if (!gb) return;
    uint8_t item = gb_read(gb, wTradeSequenceItem);
    if (item < TRADING_ITEM_RIBBON) {
        gb_write(gb, hReplaceTiles, 0);
        gb_write(gb, rSelectROMBank, 0x0C);
        DrawLinkSpriteAndReturn(gb);
        return;
    }

    uint16_t de = (uint16_t)((item - TRADING_ITEM_RIBBON) * 0x40);
    uint16_t hl = (uint16_t)(0x4400 + de); /* Items1Tiles + de */
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);
    Copy4TilesAndDrawLinkSprite(gb, hl, 0x89A0); /* vTiles1 + $1A0 */
}

void ReplaceMagicPowderTilesByToadstool(GBState *gb) {
    ReplaceTilesPairAndDrawLinkSprite(gb, 0x68C0, 0x88E0);
}

void ReplaceSlimeKeyTilesByGoldenLeaf(GBState *gb) {
    ReplaceTilesPairAndDrawLinkSprite(gb, 0x68E0, 0x8CA0);
}

void ReplaceToadstoolTilesByMagicPowder(GBState *gb) {
    if (!gb) return;
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);
    CopyDataAndDrawLinkSprite(gb, 0x48E0, 0x88E0, 0x20);
}

void ReplaceTilesButtonPressed(GBState *gb) {
    if (!gb) return;
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x12);
    gb_write(gb, rSelectROMBank, bank);
    Copy4TilesAndDrawLinkSprite(gb, 0x7F00, 0x9140);
}

void ReplaceTiles_04(GBState *gb) {
    if (!gb) return;
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0D);
    gb_write(gb, rSelectROMBank, bank);
    Copy4TilesAndDrawLinkSprite(gb, 0x4CC0, 0x9140);
}

void ReplaceEndCreditsTiles(GBState *gb, uint16_t src_hl, uint16_t dest_de) {
    if (!gb) return;
    CopyData(gb, dest_de, src_hl, 0x40);
    gb_write(gb, hReplaceTiles, 0);
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);
}

void ReplaceDialogTilesByInstruments(GBState *gb) {
    if (!gb) return;
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x11);
    gb_write(gb, rSelectROMBank, bank);

    uint8_t scratch = gb_read(gb, wCreditsScratch0);
    uint16_t de = (uint16_t)(scratch * 0x40);
    uint16_t dest = (uint16_t)(0x8D00 + de);
    uint16_t src = (uint16_t)(0x5000 + de);

    ReplaceEndCreditsTiles(gb, src, dest);
}

void ReplaceTiles_08(GBState *gb) {
    if (!gb) return;
    uint8_t bank = AdjustBankNumberForGBC(gb, 0x13);
    gb_write(gb, rSelectROMBank, bank);

    uint8_t scratch = gb_read(gb, wCreditsScratch0);
    uint16_t de = (uint16_t)(scratch * 0x40);
    uint16_t dest = (uint16_t)(0x8D00 + de);
    uint16_t src = (uint16_t)(0x4D00 + de);

    ReplaceEndCreditsTiles(gb, src, dest);
}

void UpdateSwitchBlockTiles(GBState *gb, uint8_t stage) {
    if (!gb) return;

    uint8_t bank = AdjustBankNumberForGBC(gb, 0x0C);
    gb_write(gb, rSelectROMBank, bank);
    gb_write(gb, hLinkInteractiveMotionBlocked, 1);

    uint8_t state = gb_read(gb, wSwitchBlocksState);
    stage++;

    if (stage == 3) {
        state ^= 0x02;
        gb_write(gb, wSwitchBlocksState, state);
    }
    gb_write(gb, wSwitchableObjectAnimationStage, stage);

    if (stage == 4) {
        /* SwitchBlockTransitionTilesTable: half-raised at 0x9040 */
        CopyData(gb, vTilesSwitchBlockA, 0x4040, 0x40);
    } else if (stage == 8) {
        /* SwitchBlockState0TilesTable: state 0 (or inverted by state) */
        uint16_t src = (state == 0) ? 0x4000 : 0x4080;
        CopyData(gb, vTilesSwitchBlockA, src, 0x40);
    } else if (stage == 6) {
        /* SwitchBlockTransitionTilesTable: half-raised at 0x9080 */
        CopyData(gb, vTilesSwitchBlockB, 0x4040, 0x40);
    } else if (stage == 10) {
        gb_write(gb, wSwitchableObjectAnimationStage, 0);
        uint16_t src = (state == 0) ? 0x4080 : 0x4000;
        CopyData(gb, vTilesSwitchBlockB, src, 0x40);
    } else {
        DrawLinkSpriteAndReturn(gb);
    }
}

void AnimateTilesGroup(GBState *gb,
                       void (*load_counter_tiles)(GBState *),
                       void (*func_020_54f5)(GBState *),
                       void (*configure_copy)(GBState *, uint8_t, uint8_t *),
                       void (*func_038_7830)(GBState *)) {
    uint8_t group = gb_read(gb, hAnimatedTilesGroup);
    switch (group) {
        case ANIMATED_TILES_COUNTER:
            AnimateCounterTilesGroup(gb, load_counter_tiles, func_020_54f5);
            break;
        case ANIMATED_TILES_TIDE:
            AnimateTideTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_VILLAGE:
            AnimateVillageTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_DUNGEON_1:
            AnimateDungeon1TilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_UNDERGROUND:
            AnimateUndergroundTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_LAVA:
            AnimateLavaTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_DUNGEON_2:
            AnimateDungeon2TilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_WARP_TILE:
            AnimateWarpTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_CURRENTS:
            AnimateWaterCurrentsTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_WATERFALL:
            AnimateWaterfallTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_WATERFALL_SLOW:
            AnimateSlowWaterfallTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_WATER_DUNGEON:
            AnimateWaterDungeonTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_LIGHT_BEAM:
            AnimateLightBeamTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_CRYSTAL_BLOCK:
            AnimateCrystalBlockTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_BUBBLES:
            AnimateBubblesTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_WEATHER_VANE:
            AnimateWeatherVaneTilesGroup(gb, func_020_54f5, configure_copy);
            break;
        case ANIMATED_TILES_PHOTO:
            AnimatePhotoTilesGroup(gb, func_038_7830);
            break;
        default:
            SkipTilesGroupAnimation(gb, func_020_54f5);
            break;
    }

}

void AnimateTiles(GBState *gb,
                  void (*load_counter_tiles)(GBState *),
                  void (*func_020_54f5)(GBState *),
                  void (*configure_copy)(GBState *, uint8_t, uint8_t *),
                  void (*func_038_7830)(GBState *)) {
    if (!gb) return;

    uint8_t gameplay = gb_read(gb, wGameplayType);
    if (gameplay == GAMEPLAY_MARIN_BEACH) {
        AnimateMarinBeachTiles(gb);
        return;
    }

    if (gameplay == GAMEPLAY_INTRO) {
        if (gb_read(gb, wDrawCommand) != 0) return;
        if ((gb_read(gb, hFrameCounter) & 0x0F) < 4) return;
        uint8_t b = AdjustBankNumberForGBC(gb, 0x10);
        gb_write(gb, rSelectROMBank, b);
        uint16_t hl = (uint16_t)(gb_read(gb, wD006) | (gb_read(gb, wD007) << 8));
        uint16_t de = (uint16_t)(gb_read(gb, wD008) | (gb_read(gb, wD009) << 8));
        CopyData(gb, de, hl, 0x20);
        return;
    }

    if (gameplay == GAMEPLAY_CREDITS) {
        if (gb_read(gb, hReplaceTiles) != 0) {
            goto replace_tiles_label;
        }
        return;
    }

    if (gameplay < GAMEPLAY_WORLD) {
        return;
    }

    if (gb_read(gb, wWindowY) != 0x80) {
        return;
    }

    if (gb_read(gb, wInventoryAppearing) != 0) {
        DrawLinkSpriteAndReturn(gb);
        return;
    }

    if (gb_read(gb, wRoomTransitionState) != 0 || gb_read(gb, wDrawCommand) != 0) {
        DrawLinkSpriteAndReturn(gb);
        return;
    }

    uint8_t stage = gb_read(gb, wSwitchableObjectAnimationStage);
    if (stage != 0) {
        UpdateSwitchBlockTiles(gb, stage);
        DrawLinkSpriteAndReturn(gb);
        return;
    }

replace_tiles_label:;
    uint8_t replace = gb_read(gb, hReplaceTiles);
    if (replace != 0) {
        switch (replace) {
            case REPLACE_TILES_BUTTON_PRESSED:
                ReplaceTilesButtonPressed(gb);
                return;
            case REPLACE_TILES_UNKNOWN_04:
                ReplaceEvilEagleRiderVisibleTiles(gb);
                return;
            case REPLACE_TILES_UNKNOWN_08:
                ReplaceEvilEagleRiderHiddenTiles(gb);
                return;
            case REPLACE_TILES_GOLDEN_LEAF:
                ReplaceSlimeKeyTilesByGoldenLeaf(gb);
                return;
            case REPLACE_TILES_TOADSTOOL:
                ReplaceMagicPowderTilesByToadstool(gb);
                return;
            case REPLACE_TILES_MAGIC_POWDER:
                ReplaceToadstoolTilesByMagicPowder(gb);
                return;
            case REPLACE_TILES_SIRENS_INSTRUMENT:
                ReplaceDialogTilesByInstruments(gb);
                return;
            case REPLACE_TILES_TRADING_ITEM:
                ReplaceTradingItemTiles(gb);
                return;
            case REPLACE_TILES_MARIN_SITTING:
                ReplaceMarinTiles(gb, true);
                return;
            case REPLACE_TILES_MARIN_STANDING:
                ReplaceMarinTiles(gb, false);
                return;
            default:
                DrawLinkSpriteAndReturn(gb);
                return;
        }
    }

    uint8_t frame_count = (uint8_t)(gb_read(gb, hAnimatedTilesFrameCount) + 1);
    gb_write(gb, hAnimatedTilesFrameCount, frame_count);

    AnimateTilesGroup(gb, load_counter_tiles, func_020_54f5, configure_copy, func_038_7830);
    DrawLinkSpriteAndReturn(gb);
}

void ReplaceEvilEagleRiderVisibleTiles(GBState *gb) {
    if (!gb) return;
    gb_write(gb, rSelectROMBank, BANK_EvilEagleRiderVisibleTiles);
    CopyData(gb, (uint16_t)(vTiles0 + 0x460), EvilEagleRiderVisibleTiles, 0x10);
    CopyData(gb, (uint16_t)(vTiles0 + 0x480), (uint16_t)(EvilEagleRiderVisibleTiles + 0x10), 0x10);
    gb_write(gb, hReplaceTiles, 0);
    gb_write(gb, rSelectROMBank, BANK_LinkCharacterTiles);
    DrawLinkSpriteAndReturn(gb);
}

void ReplaceEvilEagleRiderHiddenTiles(GBState *gb) {
    if (!gb) return;
    gb_write(gb, rSelectROMBank, BANK_EvilEagleRiderHiddenTiles);
    CopyData(gb, (uint16_t)(vTiles0 + 0x460), EvilEagleRiderHiddenTiles, 0x10);
    CopyData(gb, (uint16_t)(vTiles0 + 0x480), (uint16_t)(EvilEagleRiderHiddenTiles + 0x10), 0x10);
    gb_write(gb, hReplaceTiles, 0);
    gb_write(gb, rSelectROMBank, BANK_LinkCharacterTiles);
    DrawLinkSpriteAndReturn(gb);
}
