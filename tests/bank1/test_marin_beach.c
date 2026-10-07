#include "test_bank1.h"

#include "gb.h"
#include "bank1/marin_beach.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/vfx.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void test_marin_beach_subsystem(void) {
    GBState gb;

    /* Test 1: MarinBeachPrepare0 */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 0);
    gb_write_hram(&gb, hIsGBC, 1);
    gb_write(&gb, wBGPal1, 0x55);
    MarinBeachPrepare0(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    gb_write(&gb, rSVBK, 3);
    assert(gb_read(&gb, wBGPal1) == 0x55);
    gb_write(&gb, rSVBK, 0);

    /* Test 2: MarinBeachPrepare1 */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 1);
    gb_write(&gb, wTransitionSequenceCounter, 0);
    MarinBeachPrepare1(&gb);
    assert(gb_read(&gb, wC167) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 1);

    gb_write(&gb, wTransitionSequenceCounter, 4);
    MarinBeachPrepare1(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 2);
    assert(gb_read(&gb, wScrollXOffset) == 0);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_0F);

    /* Test 3: MarinBeachPrepare2 & Prepare3 */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    MarinBeachPrepare2(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_MARIN_BEACH);
    assert(gb_read(&gb, wC13F) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    MarinBeachPrepare3(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_MARIN_BEACH);
    assert(gb_read(&gb, wWindowY) == 0xFF);
    assert(gb_read_hram(&gb, hBaseScrollY) == 0x90);
    assert(gb_read(&gb, wNoiseSfxSeaWavesCounter) == 0x40);
    assert(gb_read(&gb, wD466) == 0xA0);
    assert(gb_read(&gb, wPaletteUnknownE) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 4);
    /* Check seagull 0 initialization */
    assert(gb_read(&gb, wTranscientVfxPosYTable) == 0xE0);
    assert(gb_read(&gb, wTranscientVfxPosXTable) == 0x00);
    assert(gb_read(&gb, wTranscientVfxTypeTable) == TRANSCIENT_VFX_WATER_SPLASH);
    assert(gb_read(&gb, wC560) == 0x0C);
    assert(gb_read(&gb, wC550) == 0x08);

    /* Test 4: func_001_658B (OAM generation) */
    gb_init(&gb);
    gb_write_hram(&gb, hBaseScrollY, 0x10);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x30);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    const uint8_t test_desc[4] = { 0x11, 0x02, 0x13, 0x04 };
    func_001_658B(&gb, 0xC030, test_desc);
    assert(gb_read_hram(&gb, hMultiPurposeG) == 0x20); /* 0x30 - 0x10 */
    assert(gb_read(&gb, 0xC030) == 0x20); /* Y0 */
    assert(gb_read(&gb, 0xC031) == 0x40); /* X0 */
    assert(gb_read(&gb, 0xC032) == 0x11); /* Tile0 */
    assert(gb_read(&gb, 0xC033) == 0x02); /* Attr0 */
    assert(gb_read(&gb, 0xC034) == 0x20); /* Y1 */
    assert(gb_read(&gb, 0xC035) == 0x48); /* X1 = X0 + 8 */
    assert(gb_read(&gb, 0xC036) == 0x13); /* Tile1 */
    assert(gb_read(&gb, 0xC037) == 0x04); /* Attr1 */

    /* Test 5: func_001_67B5 (Fixed-point movement) */
    gb_init(&gb);
    /* Velocity +1.0 = 0x10 */
    gb_write(&gb, wC550, 0x10);
    gb_write(&gb, wC570, 0x00);
    gb_write(&gb, wTranscientVfxPosXTable, 0x50);
    func_001_67B5(&gb, 0);
    assert(gb_read(&gb, wTranscientVfxPosXTable) == 0x51);
    assert(gb_read(&gb, wC570) == 0x00);

    /* Test 6: Dialog progression and player choices */
    gb_init(&gb);
    /* Stage 6: MarinBeachScrollStop */
    gb_write(&gb, wGameplaySubtype, 6);
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wC3C7, 0);
    MarinBeachScrollStop(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 7);
    assert(gb_read(&gb, wDialogState) != 0);

    /* Stage 7: MarinBeachDialog1 */
    gb_write(&gb, wDialogState, 0);
    MarinBeachDialog1(&gb);
    assert(gb_read(&gb, wC3C4) == 0x80);
    assert(gb_read(&gb, wC3C7) == 0xC0);
    assert(gb_read(&gb, wGameplaySubtype) == 8);

    /* Stage 8: MarinBeachPause1 */
    gb_write(&gb, wC3C7, 1);
    MarinBeachPause1(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 8);
    gb_write(&gb, wC3C7, 0);
    MarinBeachPause1(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 9);

    /* Stage 11: MarinBeachAreYouListening choice Yes (0) */
    gb_write(&gb, wGameplaySubtype, 11);
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wDialogAskSelectionIndex, 0);
    MarinBeachAreYouListening(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 12);

    /* Stage 11: MarinBeachAreYouListening choice No (1) */
    gb_write(&gb, wGameplaySubtype, 11);
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wDialogAskSelectionIndex, 1);
    MarinBeachAreYouListening(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 6);
    assert(gb_read(&gb, wC3C7) == 5);

    /* Stage 13: MarinBeachDialog4 completion */
    gb_write(&gb, wGameplaySubtype, 13);
    gb_write(&gb, wC3C7, 0);
    MarinBeachDialog4(&gb);
    assert(gb_read(&gb, wIsMarinFollowingLink) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 14);

    /* Test 7: MarinBeachEntryPoint timer countdown */
    gb_init(&gb);
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wC3C7, 10);
    gb_write(&gb, wC3C4, 5);
    gb_write(&gb, wGameplaySubtype, 2);
    MarinBeachEntryPoint(&gb);
    assert(gb_read(&gb, wC3C7) == 9);
    assert(gb_read(&gb, wC3C4) == 4);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}
