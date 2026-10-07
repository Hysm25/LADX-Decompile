#include "test_bank1.h"

#include "gb.h"
#include "bank1/face_shrine_mural.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/memory.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdio.h>

void test_face_shrine_mural_subsystem(void) {
    GBState gb;

    /* Test 1: FaceShrineMuralStage0Handler CGB */
    gb_init(&gb);
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wBGPal1, 0x42);
    FaceShrineMuralStage0Handler(&gb);
    /* Subtype was 0, stage 0 increments to 1, then falls into stage 1 */
    assert(gb_read(&gb, wGameplaySubtype) == 1);
    gb_write(&gb, rSVBK, 3);
    assert(gb_read(&gb, wBGPal1) == 0x42);
    gb_write(&gb, rSVBK, 0);

    /* Test 2: FaceShrineMuralStage1Handler transition completion */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 1);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    for (int i = 0; i < 12; i++) {
        gb_write(&gb, wRoomTransitionState + i, 0xFF);
    }
    FaceShrineMuralStage1Handler(&gb);
    assert(gb_read(&gb, wC167) == 1);
    assert(gb_read(&gb, wRoomTransitionState) == 0);
    assert(gb_read(&gb, wRoomTransitionState + 11) == 0);
    assert(gb_read(&gb, hVolumeRight) == 0x03);
    assert(gb_read(&gb, hVolumeLeft) == 0x30);
    assert(gb_read(&gb, wGameplaySubtype) == 2);
    assert(gb_read(&gb, wScrollXOffset) == 0);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_FACE_SHRINE_MURAL);

    /* Test 3: FaceShrineMuralStage2Handler */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    FaceShrineMuralStage2Handler(&gb);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_FACE_SHRINE_MURAL);
    assert(gb_read(&gb, wWindowY) == 0xFF);
    assert(gb_read(&gb, hBaseScrollX) == 0);
    assert(gb_read(&gb, hBaseScrollY) == 0);
    assert(gb_read(&gb, wTransitionSequenceCounter) == 0);
    assert(gb_read(&gb, wC16C) == 0);
    assert(gb_read(&gb, wPaletteUnknownE) == 1);
    assert(gb_read(&gb, wGameplaySubtype) == 3);

    /* Test 4: FaceShrineMuralStage3Handler */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 3);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    FaceShrineMuralStage3Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 4);
    assert(gb_read(&gb, wC3C4) == 0);

    /* Test 5: FaceShrineMuralStage4Handler */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 4);
    gb_write(&gb, wDialogState, 1);
    gb_write(&gb, wC3C4, 0x10);
    FaceShrineMuralStage4Handler(&gb);
    assert(gb_read(&gb, wC3C4) == 0x10); /* unchanged if dialog open */

    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wC3C4, 0x7F);
    FaceShrineMuralStage4Handler(&gb);
    assert(gb_read(&gb, wC3C4) == 0x80);
    assert(gb_read(&gb, wDialogState) != 0); /* opened dialog 0xE7 */

    /* Wrapped to 0 */
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wC3C4, 0xFF);
    FaceShrineMuralStage4Handler(&gb);
    assert(gb_read(&gb, wC3C4) == 0);
    assert(gb_read(&gb, wGameplaySubtype) == 5);

    /* Test 6: FaceShrineMuralStage5Handler */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 5);
    gb_write(&gb, hJoypadState, 0);
    FaceShrineMuralStage5Handler(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 5);

    gb_write(&gb, hJoypadState, J_A);
    FaceShrineMuralStage5Handler(&gb);
    assert(gb_read(&gb, hJingle) == JINGLE_VALIDATE);
    assert(gb_read(&gb, wGameplaySubtype) == 6);
    assert(gb_read(&gb, wTransitionSequenceCounter) == 0);
    assert(gb_read(&gb, wC16C) == 0);

    /* Test 7: FaceShrineMuralEntryPoint */
    gb_init(&gb);
    gb_write(&gb, wGameplaySubtype, 2);
    FaceShrineMuralEntryPoint(&gb);
    assert(gb_read(&gb, wGameplaySubtype) == 3);
}
