#include "test_bank2.h"

#include "bank2/room_effects.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Independent oracle: events.asm:167-278 (02:5E18-5EA2), plus
 * bank2.asm EnqueueDoorUnlockedSfx (02:5420). Literal operands and constants
 * from constants/{events,maps,gameplay,sfx,memory/wram,memory/hram}.asm.
 * Cxxx uses WRAM 0; Dxxx uses the selected bank, as defined by GBState.
 * No production memory helpers or room-status helpers construct expectations.
 * Full-object comparisons cover memory, bank selectors and deterministic
 * padding, but not CPU registers, timing or transient writes later undone.
 */
enum {
    OPENING = 0xC18C - 0xC000,
    CLOSING = 0xC18D - 0xC000,
    EVENT = 0xC18E - 0xC000,
    EXECUTED = 0xC18F - 0xC000,
    LATCH = 0xC190 - 0xC000,
    C111 = 0xC111 - 0xC000,
    INSTRUMENT = 0xDB65 - 0xD000,
    INDOOR = 0xDBA5 - 0xD000,
    X = 0xFF98 - 0xFF80,
    Y = 0xFF99 - 0xFF80,
    JINGLE = 0xFFF2 - 0xFF80,
    NOISE = 0xFFF4 - 0xFF80,
    ROOM = 0xFFF6 - 0xFF80,
    MAP = 0xFFF7 - 0xFF80,
    CACHE = 0xFFF8 - 0xFF80
};

static const uint8_t flags[] = {0, 1, 0x80, 0xFF};

static void fill_bytes(uint8_t *bytes, size_t size, unsigned salt) {
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = (uint8_t)(1u + (i * 37u + i / 251u + salt) % 255u);
    }
}

static void init_seed(GBState *gb) {
    static const uint8_t rom[] = {0x31, 0x73, 0xA5, 0xFE};
    memset(gb, 0, sizeof(*gb));
    gb->rom = rom;
    gb->rom_size = sizeof(rom);
    for (unsigned bank = 0; bank < 8; ++bank) {
        fill_bytes(gb->wram[bank], sizeof(gb->wram[bank]), bank);
    }
    for (unsigned bank = 0; bank < 2; ++bank) {
        fill_bytes(gb->vram[bank], sizeof(gb->vram[bank]), bank + 19u);
    }
    for (unsigned bank = 0; bank < 4; ++bank) {
        fill_bytes(gb->sram[bank], sizeof(gb->sram[bank]), bank + 41u);
    }
    fill_bytes(gb->oam, sizeof(gb->oam), 61);
    fill_bytes(gb->io, sizeof(gb->io), 83);
    fill_bytes(gb->hram, sizeof(gb->hram), 107);
    gb->ie = 0x1B;
    gb->rom_bank = 0x23;
    gb->wram_bank = 5;
    gb->vram_bank = 1;
    gb->sram_bank = 3;
    gb->sram_enabled = true;
    gb->joypad_input = 0xA6;
    gb->hram[X] = 0x50;
    gb->hram[Y] = 0x40;
    gb->hram[MAP] = 5;
    gb->hram[ROOM] = 0x37;
    gb->hram[CACHE] = 0xDA; /* EVENT_1 set must not act as a guard. */
    gb->hram[JINGLE] = 0xB7;
    gb->hram[NOISE] = 0xA9;
    gb->wram[0][OPENING] = 0xA7;
    gb->wram[0][CLOSING] = 0xB8;
    gb->wram[0][C111] = 0xC9;
    gb->wram[0][EVENT] = 0xC1;
    gb->wram[0][EXECUTED] = 0;
    gb->wram[0][LATCH] = 0;
    gb->wram[5][INSTRUMENT + 5] = 0xA4;
}

static void oracle_close(GBState *expected) {
    /* SUB wraps to a byte before unsigned CP. Both axes must qualify. */
    if ((uint8_t)(expected->hram[X] - 0x11) >= 0x7E ||
        (uint8_t)(expected->hram[Y] - 0x16) >= 0x5E ||
        expected->wram[0][EXECUTED] != 0) {
        return;
    }
    expected->wram[0][CLOSING] = 1;
    expected->wram[0][LATCH] = 1;
    expected->wram[0][C111] = 4;
    expected->hram[NOISE] = 0x10;
}

static size_t saved_status_offset(uint8_t map, uint8_t room) {
    /* Deliberately not GetRoomStatusAddress: wIsIndoor is irrelevant. */
    if (map == 0xFF) {
        return 0xDDE0 - 0xD000 + (size_t)room;
    }
    return (map >= 0x06 && map < 0x1A ? 0xDA00 : 0xD900) -
           0xD000 + (size_t)room;
}

static void oracle_shutter(GBState *expected, bool midboss_wrapper) {
    uint8_t *ram = expected->wram[expected->wram_bank];
    const uint8_t map = expected->hram[MAP];
    /* AND $01: reject bit 0, not the instrument-owned bit ($02). */
    if (midboss_wrapper && (ram[INSTRUMENT + map] & 1) != 0) {
        return;
    }
    if (expected->wram[0][LATCH] == 0) {
        oracle_close(expected);
    }
    if (expected->wram[0][EXECUTED] == 0) {
        return;
    }
    if (expected->wram[0][EVENT] == 0xC1) {
        /* Raw DE index, including DB65 + FF = DC64, with no clamping. */
        ram[INSTRUMENT + map] |= 1;
        ram[saved_status_offset(map, expected->hram[ROOM])] |= 0x20;
        expected->hram[JINGLE] = 0x1B;
        /* Assembly does not update hRoomStatus. */
    }
    if (expected->wram[0][LATCH] == 0) {
        return;
    }
    expected->wram[0][EVENT] = 0;
    expected->wram[0][OPENING] = 1;
    expected->hram[NOISE] = 4;
}

static void check_shutter(GBState *gb, bool wrapper) {
    GBState expected;
    memcpy(&expected, gb, sizeof(expected));
    oracle_shutter(&expected, wrapper);
    if (wrapper) {
        ClearMidbossEffectHandler(gb);
    } else {
        OpenShutterDoorsEffectHandler(gb);
    }
    assert(memcmp(gb, &expected, sizeof(*gb)) == 0);
}

static void test_close_coordinates(const GBState *seed) {
    GBState gb, expected;
    unsigned closed = 0;
    static const uint8_t rep_x[] = {
        0x00, 0x10, 0x11, 0x12, 0x50, 0x8D, 0x8E, 0x8F, 0xFF
    };
    static const uint8_t rep_y[] = {
        0x00, 0x15, 0x16, 0x17, 0x40, 0x72, 0x73, 0x74, 0xFF
    };

    for (size_t guard = 0; guard < sizeof(flags); ++guard) {
        /* Sweep all 256 X values against representative Y values */
        for (unsigned x = 0; x < 256; ++x) {
            for (size_t yi = 0; yi < sizeof(rep_y); ++yi) {
                unsigned y = rep_y[yi];
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[X] = (uint8_t)x;
                gb.hram[Y] = (uint8_t)y;
                gb.wram[0][EXECUTED] = flags[guard];
                gb.wram[0][LATCH] = flags[(x + y) % sizeof(flags)];
                memcpy(&expected, &gb, sizeof(expected));
                oracle_close(&expected);
                if (expected.wram[0][CLOSING] == 1) {
                    ++closed;
                }
                CloseDoors(&gb);
                assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
            }
        }

        /* Sweep all 256 Y values against representative X values */
        for (unsigned y = 0; y < 256; ++y) {
            for (size_t xi = 0; xi < sizeof(rep_x); ++xi) {
                unsigned x = rep_x[xi];
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[X] = (uint8_t)x;
                gb.hram[Y] = (uint8_t)y;
                gb.wram[0][EXECUTED] = flags[guard];
                gb.wram[0][LATCH] = flags[(x + y) % sizeof(flags)];
                memcpy(&expected, &gb, sizeof(expected));
                oracle_close(&expected);
                if (expected.wram[0][CLOSING] == 1) {
                    ++closed;
                }
                CloseDoors(&gb);
                assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
            }
        }
    }
    assert(closed > 0);

    /* Every executed byte at an interior position, not just booleans. */
    for (unsigned executed = 0; executed < 256; ++executed) {
        memcpy(&gb, seed, sizeof(gb));
        gb.wram[0][EXECUTED] = (uint8_t)executed;
        memcpy(&expected, &gb, sizeof(expected));
        oracle_close(&expected);
        CloseDoors(&gb);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
    }
}

static void test_shutter_matrix(const GBState *seed) {
    GBState gb;
    /* Exact C1 versus every other byte, both entry points, closed/open latch,
     * unresolved/resolved effects and positions inside/outside the bounds. */
    for (unsigned wrapper = 0; wrapper < 2; ++wrapper) {
        for (unsigned event = 0; event < 256; ++event) {
            for (size_t latch = 0; latch < sizeof(flags); ++latch) {
                for (size_t executed = 0; executed < sizeof(flags); ++executed) {
                    for (unsigned outside = 0; outside < 2; ++outside) {
                        memcpy(&gb, seed, sizeof(gb));
                        gb.wram[0][EVENT] = (uint8_t)event;
                        gb.wram[0][LATCH] = flags[latch];
                        gb.wram[0][EXECUTED] = flags[executed];
                        gb.hram[X] = outside ? 0xFF : 0x50;
                        check_shutter(&gb, wrapper != 0);
                    }
                }
            }
        }
    }
}

static void test_midboss_maps_and_bits(const GBState *seed) {
    static const uint8_t maps[] = {0x05, 0x06, 0x19, 0x1A, 0xFF};
    static const uint8_t rooms[] = {0, 0x1F, 0x20, 0xFF};
    static const uint8_t rep_bits[] = {
        0x00, 0x01, 0x02, 0x0F, 0x10, 0x20, 0x21, 0x3F, 0x55, 0x7F, 0x80, 0xDF, 0xE0, 0xFE, 0xFF
    };
    GBState gb;

    /* Multi-dimensional grid with representative bit patterns */
    for (size_t m = 0; m < sizeof(maps); ++m) {
        for (size_t r = 0; r < sizeof(rooms); ++r) {
            for (size_t indoor = 0; indoor < sizeof(flags); ++indoor) {
                for (size_t bi = 0; bi < sizeof(rep_bits); ++bi) {
                    unsigned bits = rep_bits[bi];
                    for (unsigned latch = 0; latch < 2; ++latch) {
                        for (unsigned wrapper = 0; wrapper < 2; ++wrapper) {
                            memcpy(&gb, seed, sizeof(gb));
                            gb.wram_bank = (bits & 1) ? 1 : 5;
                            uint8_t *ram = gb.wram[gb.wram_bank];
                            gb.hram[MAP] = maps[m];
                            gb.hram[ROOM] = rooms[r];
                            gb.hram[CACHE] = (uint8_t)bits;
                            gb.wram[0][EXECUTED] = 0x80;
                            gb.wram[0][LATCH] = latch ? 0xFF : 0;
                            ram[INDOOR] = flags[indoor];
                            ram[INSTRUMENT + maps[m]] = (uint8_t)bits;
                            ram[saved_status_offset(maps[m], rooms[r])] =
                                (uint8_t)(bits ^ 0xFF);
                            check_shutter(&gb, wrapper != 0);
                        }
                    }
                }
            }
        }
    }

    /* Sweep all 256 bits on representative maps for both wrappers */
    for (unsigned bits = 0; bits < 256; ++bits) {
        for (unsigned wrapper = 0; wrapper < 2; ++wrapper) {
            memcpy(&gb, seed, sizeof(gb));
            gb.wram_bank = (bits & 1) ? 1 : 5;
            uint8_t *ram = gb.wram[gb.wram_bank];
            gb.hram[MAP] = 0x05;
            gb.hram[ROOM] = 0x20;
            gb.hram[CACHE] = (uint8_t)bits;
            gb.wram[0][EXECUTED] = 0x80;
            gb.wram[0][LATCH] = 0;
            ram[INDOOR] = 1;
            ram[INSTRUMENT + 0x05] = (uint8_t)bits;
            ram[saved_status_offset(0x05, 0x20)] = (uint8_t)(bits ^ 0xFF);
            check_shutter(&gb, wrapper != 0);
        }
    }

    /* Wrapper's early AND $01 return precedes closing and event dispatch,
     * even for non-C1 events and when the effect has not executed. */
    for (unsigned event = 0; event < 256; ++event) {
        for (size_t executed = 0; executed < sizeof(flags); ++executed) {
            memcpy(&gb, seed, sizeof(gb));
            gb.wram[0][EVENT] = (uint8_t)event;
            gb.wram[0][EXECUTED] = flags[executed];
            gb.wram[5][INSTRUMENT + 5] = 0x03;
            check_shutter(&gb, true);
        }
    }
}

void test_bank2_shutter_effects(void) {
    GBState seed;
    CloseDoors(NULL);
    OpenShutterDoorsEffectHandler(NULL);
    ClearMidbossEffectHandler(NULL);
    init_seed(&seed);
    test_close_coordinates(&seed);
    test_shutter_matrix(&seed);
    test_midboss_maps_and_bits(&seed);
}
