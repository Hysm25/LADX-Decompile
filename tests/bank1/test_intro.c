#include "test_bank1.h"

#include "gb.h"
#include "bank1/intro.h"
#include "constants/audio.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/memory.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdio.h>

void test_intro_subsystem(void) {
    GBState gb;

    /* Test 1: IntroHandlerEntryPoint buttons inactive delay */
    gb_init(&gb);
    gb_write(&gb, hButtonsInactiveDelay, 5);
    gb_write(&gb, wGameplaySubtype, 1);
    IntroHandlerEntryPoint(&gb);
    assert(gb_read(&gb, hButtonsInactiveDelay) == 4);
    assert(gb_read(&gb, wGameplaySubtype) == 2); /* RenderIntroFrame stepped subtype */

    /* Test 2: IntroHandlerEntryPoint Start button pressed on Title Screen */
    gb_init(&gb);
    gb_write(&gb, hButtonsInactiveDelay, 0);
    gb_write(&gb, hJoypadState, J_START);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_INTRO_TITLE);
    IntroHandlerEntryPoint(&gb);
    /* TransitionToFileMenu sets wGameplayType to GAMEPLAY_FILE_SELECT (1) */
    assert(gb_read(&gb, wGameplayType) == GAMEPLAY_FILE_SELECT);

    /* Test 3: IntroHandlerEntryPoint Start button pressed before Title Screen (DMG mode) */
    gb_init(&gb);
    gb_write(&gb, hButtonsInactiveDelay, 0);
    gb_write(&gb, hJoypadState, J_START);
    gb_write(&gb, wGameplaySubtype, 0);
    gb_write(&gb, hIsGBC, 0);
    IntroHandlerEntryPoint(&gb);
    assert(gb_read(&gb, hButtonsInactiveDelay) == 40);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_TITLE);
    assert(gb_read(&gb, wOBJ0Palette) == Data_001_789B);
    assert(gb_read(&gb, wOBJ1Palette) == Data_001_789F);
    assert(gb_read(&gb, wD013) == 4);
    assert(gb_read(&gb, wGameplaySubtype) == 0x0D);
    assert(gb_read(&gb, wEntitiesStatusTable + 0) == 0);
    assert(gb_read(&gb, wEntitiesStatusTable + 4) == 0);
    assert(gb_read(&gb, rBGP) == 0);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_TITLE_SCREEN_NO_INTRO);
    assert(gb_read(&gb, rIE) == IEF_VBLANK);
    assert(gb_read(&gb, rLYC) == 0x4F);

    /* Test 4: IntroHandlerEntryPoint Start button pressed (CGB mode) */
    gb_init(&gb);
    gb_write(&gb, hButtonsInactiveDelay, 0);
    gb_write(&gb, hJoypadState, J_START);
    gb_write(&gb, wGameplaySubtype, 0);
    gb_write(&gb, hIsGBC, 1);
    IntroHandlerEntryPoint(&gb);
    assert(gb_read(&gb, wD013) == 8);
    assert(gb_read(&gb, wPaletteUnknownE) == 0);

    /* Test 5: RenderIntroFrame lightning palette modulation */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_INTRO_SEA);
    gb_write(&gb, wIntroLightningVisibleCountdown, 4);
    RenderIntroFrame(&gb);
    assert(gb_read(&gb, wIntroLightningVisibleCountdown) == 3);
    /* (3 >> 1) & 3 = 1 -> IntroSeaPaletteTable[1] == 0xC2 */
    assert(gb_read(&gb, wBGPalette) == IntroSeaPaletteTable[1]);

    /* Test 6: IntroSceneStage0Handler */
    gb_init(&gb);
    gb_write(&gb, rLCDC, 0xFF);
    gb_write(&gb, wGameplaySubtype, 0);
    IntroSceneStage0Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_CLEAR_TILEMAP);
    assert(gb_read(&gb, hFrameCounter) == 0);
    assert(gb_read(&gb, wRandomSeed) == 0xA2);
    assert((gb_read(&gb, rLCDC) & LCDCF_WINON) == 0);
    assert(gb_read(&gb, wD016) == 0xB4);
    assert(gb_read(&gb, wD017) == 0x00);
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    /* Test 7: IntroSceneStage1Handler */
    gb_write(&gb, wPaletteUnknownE, 5);
    IntroSceneStage1Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_INTRO);
    assert(gb_read(&gb, wPaletteUnknownE) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 2);

    /* Test 8: IntroSceneStage2Handler (CGB) */
    gb_write(&gb, hIsGBC, 1);
    IntroSceneStage2Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_INTRO_SEA_CGB);
    assert(gb_read(&gb, wOBJ0Palette) == 0x1C);
    assert(gb_read(&gb, wOBJ1Palette) == 0xE0);
    assert(gb_read(&gb, rIE) == (IEF_STAT | IEF_VBLANK));
    assert(gb_read(&gb, wEntitiesStatusTable + 2) == 0x05);
    assert(gb_read(&gb, wEntitiesPosXTable + 2) == 0xC0);
    assert(gb_read(&gb, wEntitiesPosYTable + 2) == 0x4E);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    /* Test 9: IntroShipOnSeaHandler ship reaching position 0x50 */
    gb_init(&gb);
    gb_write(&gb, wIntroSubTimer, 0);
    gb_write(&gb, wEntitiesPosXTable + 2, 0x50);
    IntroShipOnSeaHandler(&gb);
    assert(gb_read(&gb, rBGP) == 0xFF);
    assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_INTRO_LINK_FACE);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_INTRO_LINK_FACE);
    assert(gb_read(&gb, rIE) == IEF_VBLANK);
    assert(gb_read(&gb, hBaseScrollX) == 0);

    /* Test 10: IntroShipOnSeaHandler scrolling & lightning trigger */
    gb_init(&gb);
    gb_write(&gb, wIntroSubTimer, 0);
    gb_write(&gb, wEntitiesPosXTable + 2, 0x90);
    gb_write(&gb, hBaseScrollX, 0x0F);
    gb_write(&gb, hFrameCounter, 0); /* (0 & 7) == 0 */
    gb_write(&gb, wEntitiesStatusTable + 1, 0); /* slot 1 available for lightning */
    IntroShipOnSeaHandler(&gb);
    assert(gb_read(&gb, hBaseScrollX) == 0x10);
    /* scroll_x == 0x10 -> c = 0 -> Data_001_7081[0] = 4, Data_001_707B[0] = 0x28 */
    assert(gb_read(&gb, wEntitiesStatusTable + 1) == 4);
    assert(gb_read(&gb, wEntitiesPosXTable + 1) == 0x28);
    assert(gb_read(&gb, wEntitiesPosYTable + 1) == 0x30);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 1) == 0x20);
    assert(gb_read(&gb, wIntroLightningVisibleCountdown) == 0x1C);

    /* Test 11: IntroShipOnSeaHandler subtimer sequence to lightning */
    gb_init(&gb);
    gb_write(&gb, wIntroSubTimer, 0x18 + (8 << 3) - 1); /* subtimer increment reaches idx 8 */
    IntroShipOnSeaHandler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_INTRO_LIGHTNING);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_TITLE);
    assert(gb_read(&gb, wIntroTimer) == 0xFF);
    assert(gb_read(&gb, wScrollXOffsetForSection + 1) == 0x92);

    /* Test 12: IntroLinkFaceHandler scream and wrap around */
    gb_init(&gb);
    gb_write(&gb, wIntroTimer, 127);
    IntroLinkFaceHandler(&gb);
    assert(gb_read(&gb, wIntroTimer) == 128);
    assert(gb_read(&gb, wDrawCommand) == 0x99); /* scream command loaded */

    gb_write(&gb, wIntroTimer, 199);
    gb_write(&gb, hIsGBC, 0);
    IntroLinkFaceHandler(&gb);
    assert(gb_read(&gb, wIntroTimer) == 0x00);
    assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_INTRO_SEA);
    assert(gb_read(&gb, wIntroSubTimer) == 1);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_INTRO_SEA_DMG);
    assert(gb_read(&gb, rBGP) == 0xFF);

    /* Test 13: LoadTileMapZero_trampoline */
    gb_init(&gb);
    LoadTileMapZero_trampoline(&gb);
    assert(gb_read(&gb, wFarcallBank) == 0x00);
    assert(gb_read(&gb, wFarcallAdressHigh) == 0x04);
    assert(gb_read(&gb, wFarcallAdressLow) == 0x3A);

    /* Test 14: ResetIntroTimers */
    gb_init(&gb);
    ResetIntroTimers(&gb);
    assert(gb_read(&gb, wIntroTimer) == 0xA0);
    assert(gb_read(&gb, wIntroSubTimer) == 0x00);
    assert(gb_read(&gb, wD003) == 0xFF);

    /* Test 15: RenderRain */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, GAMEPLAY_INTRO_SEA);
    RenderRain(&gb);
    /* Rain entries written starting at wDynamicOAMBuffer + 0x1C (0xC04C) */
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x1C) != 0);

    /* Test 16: RenderIntroShip and heave */
    gb_init(&gb);
    gb_write(&gb, wIntroSubTimer, 0);
    gb_write(&gb, hFrameCounter, 0);
    gb_write(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write(&gb, hActiveEntityPosX, 0x60);
    RenderIntroShip(&gb);
    /* Heave added: index ((0 + 0xD0) >> 4) & 7 = 5 -> ShipHeaveTable[5] = 1 -> visual_y = 0x41 */
    assert(gb_read(&gb, hActiveEntityVisualPosY) == 0x41);
    assert(gb_read(&gb, wOAMBuffer + 0) == 0x41);
    assert(gb_read(&gb, wOAMBuffer + 1) == 0x60);

    /* Test 17: Parallax wave scrolling func_001_7D01 and wave tile transfer func_001_7D4E */
    gb_init(&gb);
    gb_write(&gb, hFrameCounter, 0);
    gb_write(&gb, wD004, 0x20);
    func_001_7D01(&gb);
    assert(gb_read(&gb, wScrollXOffsetForSection + 0) == 1);
    assert(gb_read(&gb, wScrollXOffsetForSection + 1) == 1);
    assert(gb_read(&gb, wScrollXOffsetForSection + 2) == 1);
    assert(gb_read(&gb, wScrollXOffsetForSection + 3) == 1);
    assert(gb_read(&gb, wD004) == 0x48);

    gb_write(&gb, hFrameCounter, 0x10);
    gb_write(&gb, wD00F, 0);
    func_001_7D4E(&gb);
    assert(gb_read(&gb, wD009) == 0x89);

    /* Test 18: IntroStage5Handler */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 5);
    IntroStage5Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_INTRO_BEACH);
    assert(gb_read(&gb, wPaletteUnknownE) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 6);

    /* Test 19: func_001_71C7 noise sfx sea waves */
    gb_init(&gb);
    gb_write(&gb, wEntitiesStateTable + 1, 0x02);
    gb_write(&gb, wNoiseSfxSeaWavesCounter, 0x10);
    func_001_71C7(&gb);
    assert(gb_read(&gb, wNoiseSfxSeaWavesCounter) == 0x10); /* skipped if state >= 2 */

    gb_write(&gb, wEntitiesStateTable + 1, 0x01);
    gb_write(&gb, wNoiseSfxSeaWavesCounter, 0x9F);
    func_001_71C7(&gb);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_SEA_WAVES);
    assert(gb_read(&gb, wNoiseSfxSeaWavesCounter) == 0x00);

    /* Test 20: IntroStage6Handler timer progression and transition */
    gb_init(&gb);
    gb_write(&gb, wIntroTimer, 0xA0);
    IntroStage6Handler(&gb);
    assert(gb_read(&gb, rLYC) == 0x02);
    assert(gb_read(&gb, wIntroTimer) == 0x9F);

    gb_write(&gb, wIntroTimer, 0x01);
    IntroStage6Handler(&gb);
    assert(gb_read(&gb, wIntroTimer) == 0x00);
    assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_INTRO_BEACH);
    assert(gb_read(&gb, wEntitiesStatusTable) == 0x06);
    assert(gb_read(&gb, wEntitiesPosXTable) == 0xB0);
    assert(gb_read(&gb, wEntitiesPosYTable) == 0x68);
    assert(gb_read(&gb, wEntitiesInertiaTable) == 0x01);

    /* Test 21: IntroStage6Handler palette update */
    gb_init(&gb);
    gb_write(&gb, wIntroTimer, 0x30);
    gb_write(&gb, wD010, 0x04);
    gb_write(&gb, hFrameCounter, 0x00);
    IntroStage6Handler(&gb);
    assert(gb_read(&gb, wIntroTimer) == 0x2F);
    assert(gb_read(&gb, wD010) == 0x04);
    assert(gb_read(&gb, wBGPalette) == Data_001_7128[4]);
    assert(gb_read(&gb, wOBJ0Palette) == Data_001_7138[4]);
    assert(gb_read(&gb, wOBJ1Palette) == Data_001_7148[4]);

    /* Test 22: IntroBeachHandler */
    gb_init(&gb);
    IntroBeachHandler(&gb);
    assert(gb_read(&gb, wOAMNextAvailableSlot) == 0);

    /* Test 23: IntroStage8Handler */
    gb_init(&gb);
    gb_write(&gb, wIntroSubTimer, 0);
    gb_write(&gb, hIsGBC, 1);
    IntroStage8Handler(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == TitleTileMap4[0]);
    assert(gb_read(&gb, wDrawCommand + 1) == TitleTileMap4[1]);
    assert(gb_read(&gb, wDrawCommand + 19) == 0x00);
    assert(gb_read(&gb, wDrawCommandVRAM1 + 0) == TitleAttrMap4[0]);
    assert(gb_read(&gb, wDrawCommandVRAM1 + 19) == 0x00);
    assert(gb_read(&gb, wIntroSubTimer) == 1);

    /* Test 23b: IntroStage8Handler completion to stage 9 */
    gb_write(&gb, wIntroSubTimer, 6);
    gb_write(&gb, wGameplaySubtype, 8);
    IntroStage8Handler(&gb);
    assert(gb_read(&gb, wIntroSubTimer) == 7);
    assert(gb_read(&gb, wGameplaySubtype) == 9);

    /* Test 24: TitleScreenSfxHandler */
    gb_init(&gb);
    gb_write(&gb, wC17E, 0x0F);
    TitleScreenSfxHandler(&gb);
    assert(gb_read_hram(&gb, hNoiseSfx) == 0);

    gb_write(&gb, wC17E, 0x10);
    gb_write(&gb, wGameplaySubtype, 9);
    TitleScreenSfxHandler(&gb);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_PING);
    assert(gb_read(&gb, wGameplaySubtype) == 10);

    /* Test 25: IntroStageAHandler (DMG & GBC) */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wGameplaySubtype, 10);
    IntroStageAHandler(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x9B);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xB7);
    assert(gb_read(&gb, wOBJ0Palette) == Data_001_7898[0]);
    assert(gb_read(&gb, wOBJ1Palette) == Data_001_789C[0]);
    assert(gb_read(&gb, wD015) == 0x3C);
    assert(gb_read(&gb, wGameplaySubtype) == 11);
    assert(gb_read(&gb, wIntroTimer) == 0xA0);

    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wGameplaySubtype, 10);
    IntroStageAHandler(&gb);
    assert(gb_read(&gb, wPalettePartialCopyColorIndexStart) == 0x14);
    assert(gb_read(&gb, wPalettePartialCopyColorCount) == 0x08);
    assert(gb_read(&gb, wPaletteDataFlags) == 0x82);
    assert(gb_read(&gb, wDrawCommandVRAM1 + 0) == 0x9B);

    /* Test 26: TitleScreenHandler sparkle spawn and timer countdown */
    gb_init(&gb);
    gb_write(&gb, hFrameCounter, 0); /* 0 & 0x3F == 0 */
    gb_write(&gb, wEntitiesStatusTable + 1, 0); /* free slot */
    gb_write(&gb, wD003, 0);
    gb_write(&gb, wIntroSubTimer, 0x0E);
    gb_write(&gb, wIntroTimer, 0x05);
    TitleScreenHandler(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 1) == 0x08); /* Sparkle */
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 1) == 0x3F);
    assert(gb_read(&gb, wEntitiesPosXTable + 1) == Data_001_73C0[1]);
    assert(gb_read(&gb, wEntitiesPosYTable + 1) == Data_001_73C8[1]);
    assert(gb_read(&gb, wIntroSubTimer) == 0x0F);
    assert(gb_read(&gb, wIntroTimer) == 0x05);

    /* Test 26b: TitleScreenHandler timer reaches 0 -> subtype 12 */
    gb_write(&gb, hFrameCounter, 1);
    gb_write(&gb, wIntroSubTimer, 0x0F);
    gb_write(&gb, wIntroTimer, 0x01);
    gb_write(&gb, wGameplaySubtype, 11);
    TitleScreenHandler(&gb);
    assert(gb_read(&gb, wIntroSubTimer) == 0x10);
    assert(gb_read(&gb, wIntroTimer) == 0x00);
    assert(gb_read(&gb, wGameplaySubtype) == 12);
    assert(gb_read(&gb, wTransitionSequenceCounter) == 0);
    assert(gb_read(&gb, wC16C) == 0);

    /* Test 27: IntroStageCHandler */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 3);
    IntroStageCHandler(&gb);
    assert(gb_read(&gb, wFarcallBank) == 0);

    /* Test 28: IntroStageDHandler */
    gb_init(&gb);
    IntroStageDHandler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == 0x11);
    assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_INTRO_TITLE);
    assert(gb_read(&gb, wBGPalette) == 0xC9);
    assert(gb_read(&gb, wOBJ0Palette) == 0x1C);
    assert(gb_read_hram(&gb, hBaseScrollX) == 0);
    assert(gb_read_hram(&gb, hBaseScrollY) == 0);
    assert(gb_read(&gb, wD018) == 0xFF);

    /* Test 29: func_001_79AE */
    gb_init(&gb);
    func_001_79AE(&gb, 2);
    assert(gb_read(&gb, wOBJ0Palette) == Data_001_7898[2]);
    assert(gb_read(&gb, wOBJ1Palette) == Data_001_789C[2]);

    /* Test 30: func_001_79C2 */
    gb_init(&gb);
    func_001_79C2(&gb, 1);
    assert(gb_read(&gb, wObjPal6 + 0) == DXFadeInPalette[16]);
    assert(gb_read(&gb, wPalettePartialCopyColorIndexStart) == 0x14);
    assert(gb_read(&gb, wPalettePartialCopyColorCount) == 0x08);
    assert(gb_read(&gb, wPaletteDataFlags) == 0x82);

    /* Test 31: func_001_7920 (DMG & GBC) */
    gb_init(&gb);
    gb_write(&gb, wD015, 5);
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wD013, 0);
    gb_write(&gb, wD014, 0x0B);
    func_001_7920(&gb);
    assert(gb_read(&gb, wD015) == 4);
    assert(gb_read(&gb, wD014) == 0);
    assert(gb_read(&gb, wD013) == 1);
    assert(gb_read(&gb, wOBJ0Palette) == Data_001_7898[1]);

    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wD013, 0);
    gb_write(&gb, wD014, 0x07);
    func_001_7920(&gb);
    assert(gb_read(&gb, wD014) == 0);
    assert(gb_read(&gb, wD013) == 1);
    assert(gb_read(&gb, wPaletteDataFlags) == 0x82);

    /* Test 32: func_001_7D9C */
    gb_init(&gb);
    gb_write(&gb, hFrameCounter, 0);
    gb_write(&gb, wD004, 0xC0);
    gb_write(&gb, wD005, 0xB0);
    gb_write(&gb, wD00D, 0x60);
    func_001_7D9C(&gb);
    assert(gb_read(&gb, wScrollXOffsetForSection + 0) == 1);
    assert(gb_read(&gb, wScrollXOffsetForSection + 1) == 1); /* 0xC0 + 0x50 = 0x110 */
    assert(gb_read(&gb, wScrollXOffsetForSection + 2) == 1); /* 0xB0 + 0x58 = 0x108 */
    assert(gb_read(&gb, wScrollXOffsetForSection + 3) == 1); /* 0x60 + 0xB0 = 0x110 */

    /* Test 33: func_001_7DCF */
    gb_init(&gb);
    gb_write(&gb, hFrameCounter, 0);
    gb_write(&gb, wD004, 0xE0);
    gb_write(&gb, wD005, 0xE0);
    gb_write(&gb, wD00D, 0xB0);
    func_001_7DCF(&gb);
    assert(gb_read(&gb, wScrollXOffsetForSection + 0) == 1);
    assert(gb_read(&gb, wScrollXOffsetForSection + 1) == 1); /* 0xE0 + 0x28 = 0x108 */
    assert(gb_read(&gb, wScrollXOffsetForSection + 2) == 1); /* 0xE0 + 0x2C = 0x10C */
    assert(gb_read(&gb, wScrollXOffsetForSection + 3) == 1); /* 0xB0 + 0x58 = 0x108 */

    /* Test 34: RenderIntroFrame stage dispatch 5..13 */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 5);
    RenderIntroFrame(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 6);

    gb_write(&gb, wGameplaySubtype, 13);
    RenderIntroFrame(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_INTRO_TITLE);

    /* Test 35: func_001_7A11 & func_001_7A16 */
    gb_init(&gb);
    func_001_7A11(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == Data_001_79FD[0]);
    assert(gb_read(&gb, wDrawCommand + 23) == Data_001_79FD[23]);

    func_001_7A16(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == Data_001_79EC[0]);
    assert(gb_read(&gb, wDrawCommand + 23) == Data_001_79EC[23]);

    /* Test 36: IntroMarinState0 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, wEntitiesInertiaTable + 0, 1);
    gb_write(&gb, wEntitiesPosXTable + 0, 0x50);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    IntroMarinState0(&gb);
    assert(gb_read(&gb, wEntitiesInertiaTable + 0) == 4);
    assert(gb_read(&gb, wEntitiesPosXTable + 0) == 0x4F);

    gb_write_hram(&gb, hActiveEntityPosX, 0x40); /* < 0x48 */
    IntroMarinState0(&gb);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 0) == 0x40);
    assert(gb_read(&gb, wEntitiesStateTable + 0) == 1);

    /* Test 37: IntroMarinState1 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0, 5);
    IntroMarinState1(&gb);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 0) == 4);

    gb_write(&gb, wEntitiesTransitionCountdownTable + 0, 0);
    IntroMarinState1(&gb);
    assert(gb_read(&gb, wEntitiesStateTable + 0) == 1);
    assert(gb_read(&gb, wEntitiesStatusTable + 1) == ENTITY_INTRO_INERT_LINK);
    assert(gb_read(&gb, wEntitiesPosXTable + 1) == 0xFE);
    assert(gb_read(&gb, wEntitiesPosYTable + 1) == 0x6E);

    /* Test 38: IntroMarinState2 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, wEntitiesPosXTable + 1, 0x50);
    gb_write_hram(&gb, hBaseScrollX, 0x1F);
    gb_write_hram(&gb, hFrameCounter, 0x00); /* even */
    IntroMarinState2(&gb);
    assert(gb_read(&gb, wEntitiesPosXTable + 1) == 0x4F);
    assert(gb_read_hram(&gb, hBaseScrollX) == 0x20);
    assert(gb_read(&gb, wDrawCommand + 0) == Data_001_79EC[0]); /* func_001_7A16 triggered */

    gb_write_hram(&gb, hBaseScrollX, 0x2F);
    IntroMarinState2(&gb);
    assert(gb_read_hram(&gb, hBaseScrollX) == 0x30);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 0) == 0x40);
    assert(gb_read(&gb, wEntitiesStateTable + 0) == 1);

    /* Test 39: IntroMarinState3 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0, 2);
    IntroMarinState3(&gb);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 0) == 1);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0) == 1);

    gb_write(&gb, wEntitiesTransitionCountdownTable + 0, 0);
    gb_write_hram(&gb, hFrameCounter, 0x00);
    gb_write_hram(&gb, hBaseScrollX, 0x55);
    IntroMarinState3(&gb);
    assert(gb_read_hram(&gb, hBaseScrollX) == 0xA0);
    assert(gb_read(&gb, rSCX) == 0xA0);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 0) == 0xE0);
    assert(gb_read(&gb, wEntitiesStateTable + 0) == 1);

    /* Test 40: IntroMarinState4 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write_hram(&gb, hFrameCounter, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0, 0x95);
    IntroMarinState4(&gb);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 0) == 0x94);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0) == 3);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 1) == 1);

    gb_write(&gb, wEntitiesTransitionCountdownTable + 0, 0x60);
    IntroMarinState4(&gb);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0) == 2);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 1) == 0);

    /* Test 41: RenderIntroMarin */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write_hram(&gb, hActiveEntityState, 0);
    gb_write(&gb, wOAMNextAvailableSlot, 0);
    RenderIntroMarin(&gb);
    assert(gb_read(&gb, wOAMNextAvailableSlot) == 8);

    /* Test 42: RenderIntroSparkle */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write(&gb, wEntitiesStatusTable + 1, ENTITY_INTRO_SPARKLE);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 1, 0x10);
    RenderIntroSparkle(&gb);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 1) == 0x0F);
    assert(gb_read_hram(&gb, hActiveEntitySpriteVariant) == 1);

    gb_write(&gb, wEntitiesTransitionCountdownTable + 1, 1);
    RenderIntroSparkle(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 1) == 0); /* destroyed */

    /* Test 43: InertLinkState0Handler */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 1, 1);
    InertLinkState0Handler(&gb);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 1) == 0x90);
    assert(gb_read(&gb, wEntitiesStateTable + 1) == 1);

    /* Test 44: InertLinkState1Handler */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write_hram(&gb, hFrameCounter, 0);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 1, 1);
    InertLinkState1Handler(&gb);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 1) == 0);
    assert(gb_read(&gb, wEntitiesStateTable + 1) == 1);

    /* Test 45: InertLinkState2Handler */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write(&gb, wD00A, 0x13);
    InertLinkState2Handler(&gb);
    assert(gb_read(&gb, wEntitiesStateTable + 1) == 1);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 1) == 0x17);
    assert(gb_read_hram(&gb, hVolumeRight) == 0x07);
    assert(gb_read_hram(&gb, hVolumeLeft) == 0x70);

    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write(&gb, wD00A, 0x0A);
    gb_write(&gb, wCreditsSubscene, 0x03); /* + 1 = 4 -> & 3 == 0 */
    gb_write_hram(&gb, hBaseScrollY, 0x10); /* & 7 == 0 */
    InertLinkState2Handler(&gb);
    assert(gb_read(&gb, wD00A) == 0x0B);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_TITLE_SCREEN);

    /* Test 46: InertLinkState3Handler */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write_hram(&gb, hFrameCounter, 0);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 1, 1);
    gb_write(&gb, wGameplaySubtype, 7);
    InertLinkState3Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 8);
    assert(gb_read(&gb, wEntitiesStatusTable + 0) == 0);
    assert(gb_read(&gb, wEntitiesStatusTable + 1) == 0);

    /* Test 47: RenderIntroInertLink */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityState, 0);
    gb_write(&gb, wOAMNextAvailableSlot, 0);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 1, 5);
    RenderIntroInertLink(&gb);
    assert(gb_read(&gb, wOAMNextAvailableSlot) == 8);

    /* Test 48: func_7C60 & func_001_7CCB */
    gb_init(&gb);
    gb_write(&gb, wD00A, 0);
    gb_write(&gb, hIsGBC, 1);
    func_7C60(&gb);
    assert(gb_read(&gb, wDrawCommand + 0) == 0x9B);
    assert(gb_read(&gb, wDrawCommand + 1) == 0xF4);
    assert(gb_read(&gb, wDrawCommand + 2) == 0x13);
    assert(gb_read(&gb, wDrawCommand + 3) == TitleScreenPostBeachTilemap[0]);
    assert(gb_read(&gb, wDrawCommand + 23) == 0x00);
    assert(gb_read(&gb, wDrawCommandVRAM1 + 0) == 0x9B);
    assert(gb_read(&gb, wD00A) == 1);
    assert(gb_read(&gb, wD00B) == 0xD4);
    assert(gb_read(&gb, wD00C) == 0x9B);

    /* Test 49: RenderIntroEntity dispatch */
    gb_init(&gb);
    gb_write(&gb, wEntitiesStatusTable + 0, ENTITY_INTRO_MARIN);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, wEntitiesInertiaTable + 0, 5);
    RenderIntroEntity(&gb, 0);
    assert(gb_read(&gb, wEntitiesInertiaTable + 0) == 4);
}
