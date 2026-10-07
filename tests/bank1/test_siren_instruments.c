#include "test_bank1.h"

#include "gb.h"
#include "bank1/siren_instruments.h"
#include "constants/memory.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void test_siren_instruments_subsystem(void) {
    GBState gb;

    /* Test 1: GetInstrumentNextBGAddress */
    uint16_t base = 0x9D00;
    uint16_t next1 = GetInstrumentNextBGAddress(base, 0);
    assert(next1 == 0x9D01);
    uint16_t next2 = GetInstrumentNextBGAddress(next1, 1);
    assert(next2 == 0x9D20);
    uint16_t next3 = GetInstrumentNextBGAddress(next2, 2);
    assert(next3 == 0x9D21);

    /* Test 2: LoadInstrumentsBG right side (instruments 0..3) */
    gb_init(&gb);
    gb_write(&gb, wHasInstrument1 + 0, 0x02); /* Obtained: tile 0xD0 */
    gb_write(&gb, wHasInstrument1 + 1, 0x00); /* Missing: placeholder 0xB2 */
    LoadInstrumentsBG(&gb, 1);

    /* Instrument 0 at 0x9D00 + 0x0F = 0x9D0F */
    assert(gb_read(&gb, 0x9D0F) == 0xD0);
    assert(gb_read(&gb, 0x9D10) == 0xD1);
    assert(gb_read(&gb, 0x9D2F) == 0xE0);
    assert(gb_read(&gb, 0x9D30) == 0xE1);

    /* Instrument 1 at 0x9D00 + 0x51 = 0x9D51 */
    assert(gb_read(&gb, 0x9D51) == 0x7C);
    assert(gb_read(&gb, 0x9D52) == 0x7C);
    assert(gb_read(&gb, 0x9D71) == 0x7C);
    assert(gb_read(&gb, 0x9D72) == 0xB2);

    /* Test 3: LoadSirenInstrumentTiles */
    gb_init(&gb);
    gb_write(&gb, hBGTilesLoadingStage, 2);
    LoadSirenInstrumentTiles(&gb, 2);
    assert(gb_read(&gb, hBGTilesLoadingStage) == 3);

    /* Test 4: LoadSirenInstruments state machine */
    gb_init(&gb);
    gb_write(&gb, hBGTilesLoadingStage, 0);
    LoadSirenInstruments(&gb);
    assert(gb_read(&gb, hBGTilesLoadingStage) == 1);

    gb_write(&gb, hBGTilesLoadingStage, 8);
    LoadSirenInstruments(&gb);
    assert(gb_read(&gb, hBGTilesLoadingStage) == 9);

    gb_write(&gb, hBGTilesLoadingStage, 9);
    gb_write(&gb, hNeedsUpdatingBGTiles, 1);
    LoadSirenInstruments(&gb);
    assert(gb_read(&gb, hBGTilesLoadingStage) == 0);
    assert(gb_read(&gb, hNeedsUpdatingBGTiles) == 0);
}
