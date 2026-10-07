#include "test_bank2.h"

#include "gb.h"
#include "bank2/room_effects.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Independent oracle: events.asm:280-453 (02:5EA3-5F9F), with guard at
 * 02:5DAF-5DC1; bank0.asm:4361-4405 (label_2887), 184-245 (func_91D),
 * and 926-974 (AddTranscientVfx); object_attributes.asm:12-207 (1A:6576).
 * Addresses and tile bytes are literal assembly operands. No production
 * guard, VFX, address, bank, or table helper is used to construct expectations.
 * Dxxx accesses follow GBState's selected WRAM bank. CPU registers, timing,
 * and temporary bank switches are not observable in a final-state comparison.
 */
enum {
    EVENT = 0xC18E - 0xC000,
    EXECUTED = 0xC18F - 0xC000,
    VFX_TYPE = 0xC510 - 0xC000,
    VFX_TIMER = 0xC520 - 0xC000,
    VFX_X = 0xC530 - 0xC000,
    VFX_Y = 0xC540 - 0xC000,
    VFX_RING = 0xC5C0 - 0xC000,
    DRAW_SIZE = 0xD600 - 0xD000,
    DRAW = 0xD601 - 0xD000,
    OBJECTS = 0xD711 - 0xD000,
    INDOOR = 0xDBA5 - 0xD000,
    ATTR_SIZE = 0xDC90 - 0xD000,
    ATTR_DRAW = 0xDC91 - 0xD000,
    OBJECT_ID = 0xDDD8 - 0xD000,
    SCROLL_X = 0xFF96 - 0xFF80,
    SCROLL_Y = 0xFF97 - 0xFF80,
    LINK_X = 0xFF98 - 0xFF80,
    LINK_Y = 0xFF99 - 0xFF80,
    STAIRS = 0xFFAC - 0xFF80,
    STAIRS_X = 0xFFAD - 0xFF80,
    STAIRS_Y = 0xFFAE - 0xFF80,
    TOP = 0xFFCD - 0xFF80,
    LEFT = 0xFFCE - 0xFF80,
    BG_HI = 0xFFCF - 0xFF80,
    BG_LO = 0xFFD0 - 0xFF80,
    MP0 = 0xFFD7 - 0xFF80,
    MP1 = 0xFFD8 - 0xFF80,
    MP8 = 0xFFDF - 0xFF80,
    MP9 = 0xFFE0 - 0xFF80,
    MPA = 0xFFE1 - 0xFF80,
    ROOM = 0xFFF6 - 0xFF80,
    MAP = 0xFFF7 - 0xFF80,
    STATUS = 0xFFF8 - 0xFF80,
    GBC = 0xFFFE - 0xFF80
};

static const uint8_t chest_bytes[8] = {
    0x60, 0x70, 0x61, 0x71, 0x60, 0x70, 0x60, 0x70
};
static const uint8_t stair_bytes[8] = {
    0x6A, 0x7A, 0x6B, 0x7B, 0, 0, 0, 0
};

/* Literal lookup fixtures: bank-1A pointer-table address and selected bank.
 * The color dungeon bypasses the pointer table and uses group $6000.
 * Special rooms and adjacent ordinary rooms distinguish each lookup branch.
 */
struct AttrRoute {
    uint8_t indoor, map, room, bank;
    uint16_t pointer;
};
static const struct AttrRoute routes[] = {
    {0, 0xFF, 0x00, 0x21, 0x5E76},
    {0, 0x00, 0xFF, 0x22, 0x6074},
    {1, 0x00, 0x80, 0x23, 0x6076},
    {1, 0x08, 0xFE, 0x23, 0x6086},
    {1, 0x09, 0x00, 0x24, 0x6288},
    {1, 0xFF, 0xFF, 0x23, 0},
    {1, 0x0A, 0xFD, 0x24, 0x6294},
    {1, 0x0A, 0xFC, 0x24, 0x628A},
    {1, 0x11, 0xC0, 0x24, 0x6294},
    {1, 0x11, 0xC1, 0x24, 0x6294},
    {1, 0x11, 0xC2, 0x24, 0x6298},
    {1, 0x0F, 0xA0, 0x24, 0x6276},
    {1, 0x0F, 0xA1, 0x24, 0x6294},
    {1, 0x1F, 0xEB, 0x24, 0x629E},
    {1, 0x1F, 0xEC, 0x24, 0x629E},
    {1, 0x1F, 0xED, 0x24, 0x62B4},
    {1, 0x10, 0xE9, 0x24, 0x629C},
    {1, 0x10, 0xB5, 0x24, 0x6474},
    {1, 0x10, 0xB6, 0x24, 0x6296},
    {1, 0x16, 0x6F, 0x23, 0x62A2},
    {1, 0x16, 0x7F, 0x23, 0x62A2},
    {1, 0x16, 0x8F, 0x23, 0x62A2},
    {0x80, 0x16, 0x8E, 0x24, 0x62A2},
    {0xFF, 0x80, 0x00, 0x24, 0x6376}
};

static uint8_t fixture_rom[0x25 * 0x4000];

static uint16_t route_group(const struct AttrRoute *route) {
    return route->pointer ? (uint16_t)(0x4800 + (route->pointer & 0x03FF))
                          : 0x6000;
}

static void fill_bytes(uint8_t *bytes, size_t size, unsigned salt) {
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = (uint8_t)(1u + (i * 37u + i / 251u + salt) % 255u);
    }
}

static void init_seed(GBState *gb) {
    fill_bytes(fixture_rom, sizeof(fixture_rom), 139);
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); ++i) {
        const struct AttrRoute *route = &routes[i];
        if (route->pointer) {
            size_t offset = 0x1A * 0x4000 + route->pointer - 0x4000;
            uint16_t group = route_group(route);
            fixture_rom[offset] = (uint8_t)group;
            fixture_rom[offset + 1] = (uint8_t)(group >> 8);
        }
        if (!route->indoor) {
            fixture_rom[0x1A * 0x4000 + 0x6476 - 0x4000 + route->room] =
                route->bank;
        }
    }
    /* Deterministic padding and nonzero sentinels in every memory region. */
    memset(gb, 0, sizeof(*gb));
    gb->rom = fixture_rom;
    gb->rom_size = sizeof(fixture_rom);
    for (unsigned bank = 0; bank < 8; ++bank) {
        fill_bytes(gb->wram[bank], sizeof(gb->wram[bank]), bank);
    }
    for (unsigned bank = 0; bank < 2; ++bank) {
        fill_bytes(gb->vram[bank], sizeof(gb->vram[bank]), bank + 19);
    }
    for (unsigned bank = 0; bank < 4; ++bank) {
        fill_bytes(gb->sram[bank], sizeof(gb->sram[bank]), bank + 41);
    }
    fill_bytes(gb->oam, sizeof(gb->oam), 61);
    fill_bytes(gb->io, sizeof(gb->io), 83);
    fill_bytes(gb->hram, sizeof(gb->hram), 107);
    gb->ie = 0x1B;
    gb->rom_bank = 0x35;
    gb->wram_bank = 5;
    gb->vram_bank = 1;
    gb->sram_bank = 3;
    gb->sram_enabled = true;
    gb->joypad_input = 0xA6;
    gb->hram[STATUS] = 0xEF;
    gb->wram[0][EXECUTED] = 0x80;
    gb->wram[0][EVENT] = 0xE1;
    gb->hram[LINK_X] = 0x88;
    gb->hram[LINK_Y] = 0x30;
    gb->hram[GBC] = 0;
    gb->wram[0][VFX_TYPE + 15] = 0;
    gb->wram[0][VFX_RING] = 0;
}

static void set_route(GBState *gb, unsigned index) {
    gb->wram[gb->wram_bank][INDOOR] = routes[index].indoor;
    gb->hram[MAP] = routes[index].map;
    gb->hram[ROOM] = routes[index].room;
}

static int link_overlaps(const GBState *gb) {
    /* Equivalent unsigned-byte SUB/ADD/CP intervals, expressed as literal
     * coordinate ranges rather than copying production arithmetic. */
    return gb->hram[LINK_Y] >= 0x28 && gb->hram[LINK_Y] <= 0x37 &&
           gb->hram[LINK_X] >= 0x78 && gb->hram[LINK_X] <= 0x97;
}

static void expect_reveal(GBState *expected) {
    if ((expected->hram[STATUS] & 0x10) || !expected->wram[0][EXECUTED]) {
        return;
    }
    expected->wram[0][EVENT] = 0;
    expected->hram[MP0] = 0x88;
    expected->hram[MP1] = link_overlaps(expected) ? 0x40 : 0x30;
    int slot = 15;
    while (slot >= 0 && expected->wram[0][VFX_TYPE + slot]) {
        --slot;
    }
    if (slot < 0) {
        uint8_t ring = expected->wram[0][VFX_RING];
        slot = ring ? ring - 1 : 15;
        expected->wram[0][VFX_RING] = (uint8_t)slot;
    }
    expected->wram[0][VFX_TYPE + slot] = 3;
    expected->wram[0][VFX_Y + slot] = expected->hram[MP1];
    expected->wram[0][VFX_X + slot] = 0x88;
    expected->wram[0][VFX_TIMER + slot] = 0x0F;
}

static void expect_command(uint8_t *out, uint16_t bg, const uint8_t tiles[4]) {
    const uint8_t command[11] = {
        (uint8_t)(bg >> 8), (uint8_t)bg, 0x81, tiles[0], tiles[1],
        (uint8_t)(bg >> 8), (uint8_t)(bg + 1), 0x81, tiles[2], tiles[3], 0
    };
    /* Ten queued bytes plus terminator; INC A never carries into bgHigh. */
    memcpy(out, command, sizeof(command));
}

static void expect_object(GBState *expected, int stairs, unsigned route_index) {
    uint8_t *ram = expected->wram[expected->wram_bank];
    const uint8_t top = stairs ? 0x10 : (link_overlaps(expected) ? 0x30 : 0x20);
    const uint8_t object = stairs ? 0xBE : 0xA0;
    if (stairs) {
        expected->hram[STAIRS] = 1;
        expected->hram[STAIRS_X] = 0x88;
        expected->hram[STAIRS_Y] = 0x20;
    }
    expected->hram[TOP] = top;
    expected->hram[LEFT] = 0x80;
    /* Chest swaps $80; stairs swaps $88. Both yield low nibble $08. */
    ram[OBJECTS + top + 8] = object;
    ram[OBJECT_ID] = object;
    unsigned row = ((top + expected->hram[SCROLL_Y]) % 256u) / 8u;
    unsigned col = ((0x80u + expected->hram[SCROLL_X]) % 256u) / 8u;
    uint16_t bg = (uint16_t)(0x9800 + row * 32 + col);
    expected->hram[BG_HI] = (uint8_t)(bg >> 8);
    expected->hram[BG_LO] = (uint8_t)bg;
    unsigned size = ram[DRAW_SIZE];
    ram[DRAW_SIZE] = (uint8_t)(size + 10);
    const uint8_t *tiles = stairs ? stair_bytes :
        chest_bytes + (expected->hram[GBC] ? 4 : 0);
    expect_command(&ram[DRAW + size], bg, tiles);

    if (expected->hram[GBC]) {
        const struct AttrRoute *route = &routes[route_index];
        uint16_t address = (uint16_t)(route_group(route) + object * 4);
        expected->hram[MP8] = route->bank;
        expected->hram[MP9] = (uint8_t)(address >> 8);
        expected->hram[MPA] = (uint8_t)address;
        const uint8_t *attrs = &fixture_rom[route->bank * 0x4000u + address - 0x4000];
        /* Attribute records are row-major, unlike the column-major tile IDs. */
        const uint8_t columns[4] = {attrs[0], attrs[2], attrs[1], attrs[3]};
        size = ram[ATTR_SIZE];
        ram[ATTR_SIZE] = (uint8_t)(size + 10);
        expect_command(&ram[ATTR_DRAW + size], bg, columns);
        expected->rom_bank = 2; /* Stacked A is $02, NOT the incoming bank. */
    }
}

static unsigned active_route;

/* Materializes the unfinished bank-1A attribute lookup for the fixture route:
 * MP8 = attribute bank, MP9/A = group + object * 4. The real lookup is only
 * reached on CGB, after func_91D selects bank $1A. */
static void mock_attr_lookup(GBState *gb) {
    assert(gb->hram[GBC] != 0);
    assert(gb->rom_bank == 0x1A);
    const struct AttrRoute *route = &routes[active_route];
    uint16_t bc = (uint16_t)(gb->wram[gb->wram_bank][OBJECT_ID] * 4u);
    uint16_t address = (uint16_t)(route_group(route) + bc);
    gb->hram[MP8] = route->bank;
    gb->hram[MP9] = (uint8_t)(address >> 8);
    gb->hram[MPA] = (uint8_t)address;
}

static void check_reveal(GBState *gb) {
    GBState expected;
    memcpy(&expected, gb, sizeof(expected));
    expect_reveal(&expected);
    RevealChestEffectHandler(gb);
    assert(memcmp(gb, &expected, sizeof(*gb)) == 0);
}

static void check_object(GBState *gb, int stairs, unsigned route) {
    GBState expected;
    memcpy(&expected, gb, sizeof(expected));
    expect_object(&expected, stairs, route);
    active_route = route;
    if (stairs) {
        func_002_5F5C(gb, mock_attr_lookup);
    } else {
        func_002_5ED3(gb, mock_attr_lookup);
    }
    assert(memcmp(gb, &expected, sizeof(*gb)) == 0);
}

static void test_guards(const GBState *seed) {
    GBState gb;
    unsigned accepted = 0;
    static const uint8_t events[] = {0, 1, 0x40, 0xFF};
    static const uint8_t rep_executed[] = {
        0x00, 0x01, 0x02, 0x10, 0x7F, 0x80, 0xFE, 0xFF
    };
    static const uint8_t rep_status[] = {
        0x00, 0x01, 0x0E, 0x0F, 0x10, 0x11, 0x1F, 0x20, 0x40, 0x7F, 0x80, 0xEF, 0xF0, 0xFE, 0xFF
    };

    /* Sweep all 256 status values with representative executed bytes */
    for (unsigned status = 0; status < 256; ++status) {
        for (size_t e = 0; e < sizeof(rep_executed); ++e) {
            unsigned executed = rep_executed[e];
            memcpy(&gb, seed, sizeof(gb));
            gb.hram[STATUS] = (uint8_t)status;
            gb.wram[0][EXECUTED] = (uint8_t)executed;
            gb.wram[0][EVENT] = events[(status + executed) % 4];
            gb.hram[GBC] = (uint8_t)executed;
            check_reveal(&gb);
            accepted += !(status & 0x10) && executed != 0;
        }
    }

    /* Sweep all 256 executed values with representative status bytes */
    for (unsigned executed = 0; executed < 256; ++executed) {
        for (size_t s = 0; s < sizeof(rep_status); ++s) {
            unsigned status = rep_status[s];
            memcpy(&gb, seed, sizeof(gb));
            gb.hram[STATUS] = (uint8_t)status;
            gb.wram[0][EXECUTED] = (uint8_t)executed;
            gb.wram[0][EVENT] = events[(status + executed) % 4];
            gb.hram[GBC] = (uint8_t)executed;
            check_reveal(&gb);
            accepted += !(status & 0x10) && executed != 0;
        }
    }
    assert(accepted > 0);
}

static void test_coordinates(const GBState *seed) {
    GBState gb;
    unsigned overlaps = 0;
    static const uint8_t rep_x[] = {
        0x00, 0x77, 0x78, 0x79, 0x88, 0x96, 0x97, 0x98, 0xFF
    };
    static const uint8_t rep_y[] = {
        0x00, 0x27, 0x28, 0x29, 0x30, 0x36, 0x37, 0x38, 0xFF
    };

    /* Sweep all 256 X values against representative Y values */
    for (unsigned x = 0; x < 256; ++x) {
        for (size_t yi = 0; yi < sizeof(rep_y); ++yi) {
            unsigned y = rep_y[yi];
            for (unsigned cgb = 0; cgb < 2; ++cgb) {
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[LINK_X] = (uint8_t)x;
                gb.hram[LINK_Y] = (uint8_t)y;
                gb.hram[GBC] = (uint8_t)cgb;
                set_route(&gb, 2);
                overlaps += link_overlaps(&gb);
                check_reveal(&gb);
                gb.hram[STATUS] = 0xFF;
                gb.wram[0][EXECUTED] = 0;
                gb.wram[0][EVENT] = 0xE1;
                check_object(&gb, 0, 2);
            }
        }
    }

    /* Sweep all 256 Y values against representative X values */
    for (unsigned y = 0; y < 256; ++y) {
        for (size_t xi = 0; xi < sizeof(rep_x); ++xi) {
            unsigned x = rep_x[xi];
            for (unsigned cgb = 0; cgb < 2; ++cgb) {
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[LINK_X] = (uint8_t)x;
                gb.hram[LINK_Y] = (uint8_t)y;
                gb.hram[GBC] = (uint8_t)cgb;
                set_route(&gb, 2);
                overlaps += link_overlaps(&gb);
                check_reveal(&gb);
                gb.hram[STATUS] = 0xFF;
                gb.wram[0][EXECUTED] = 0;
                gb.wram[0][EVENT] = 0xE1;
                check_object(&gb, 0, 2);
            }
        }
    }
    assert(overlaps > 0);
}

static void test_vfx_slots(const GBState *seed) {
    GBState gb;

    /* Stride across all masks, plus boundary and dedicated priority patterns */
    for (unsigned mask = 0; mask < 65536; mask += 64) {
        memcpy(&gb, seed, sizeof(gb));
        for (unsigned slot = 0; slot < 16; ++slot) {
            gb.wram[0][VFX_TYPE + slot] =
                (mask & (1u << slot)) ? (uint8_t)(0x80 + slot) : 0;
        }
        gb.hram[LINK_Y] = (mask & 1) ? 0x30 : 0xFF;
        check_reveal(&gb);
    }

    /* Every sole-free-slot and sole-occupied-slot pattern */
    for (unsigned slot = 0; slot < 16; ++slot) {
        unsigned free_mask = (unsigned)(~(1u << slot) & 0xFFFFu);
        unsigned occ_mask = (1u << slot);
        memcpy(&gb, seed, sizeof(gb));
        for (unsigned s = 0; s < 16; ++s) {
            gb.wram[0][VFX_TYPE + s] =
                (free_mask & (1u << s)) ? (uint8_t)(0x80 + s) : 0;
        }
        gb.hram[LINK_Y] = 0x30;
        check_reveal(&gb);

        memcpy(&gb, seed, sizeof(gb));
        for (unsigned s = 0; s < 16; ++s) {
            gb.wram[0][VFX_TYPE + s] =
                (occ_mask & (1u << s)) ? (uint8_t)(0x80 + s) : 0;
        }
        gb.hram[LINK_Y] = 0xFF;
        check_reveal(&gb);
    }

    /* Highest-free-slot priority patterns: slot k free, higher occupied, lower varied */
    for (unsigned k = 0; k < 16; ++k) {
        unsigned high_occ = 0xFFFFu & ~((1u << (k + 1)) - 1u);
        unsigned variations[] = {0, ((1u << k) - 1u), 0x5555u & ((1u << k) - 1u)};
        for (size_t vi = 0; vi < 3; ++vi) {
            unsigned mask = high_occ | variations[vi];
            memcpy(&gb, seed, sizeof(gb));
            for (unsigned s = 0; s < 16; ++s) {
                gb.wram[0][VFX_TYPE + s] =
                    (mask & (1u << s)) ? (uint8_t)(0x80 + s) : 0;
            }
            gb.hram[LINK_Y] = 0x30;
            check_reveal(&gb);
        }
    }
    for (unsigned ring = 0; ring < 256; ++ring) {
        memcpy(&gb, seed, sizeof(gb));
        memset(&gb.wram[0][VFX_TYPE], 0xFE, 16);
        gb.wram[0][VFX_RING] = (uint8_t)ring;
        check_reveal(&gb);
    }
    /* No room-status latch: repeated calls allocate another VFX. */
    memcpy(&gb, seed, sizeof(gb));
    memset(&gb.wram[0][VFX_TYPE], 0, 16);
    for (unsigned call = 0; call < 34; ++call) {
        check_reveal(&gb);
    }
}

static void test_draw_commands(const GBState *seed) {
    static const uint8_t scroll[] = {
        0, 1, 7, 8, 0x1F, 0x20, 0x3F, 0x40, 0x77, 0x78,
        0x7F, 0x80, 0xCF, 0xD0, 0xDF, 0xE0, 0xEF, 0xF0, 0xF7, 0xF8, 0xFF
    };
    static const uint8_t modes[] = {0, 1, 0x80, 0xFF};
    GBState gb;
    for (unsigned kind = 0; kind < 3; ++kind) {
        for (size_t cgb = 0; cgb < sizeof(modes); ++cgb) {
            for (size_t y = 0; y < sizeof(scroll); ++y) {
                for (size_t x = 0; x < sizeof(scroll); ++x) {
                    memcpy(&gb, seed, sizeof(gb));
                    gb.wram_bank = (uint8_t)(1 + (x + y) % 7);
                    set_route(&gb, 2);
                    gb.hram[GBC] = modes[cgb];
                    gb.hram[LINK_Y] = kind == 0 ? 0xFF : 0x30;
                    gb.hram[SCROLL_X] = scroll[x];
                    gb.hram[SCROLL_Y] = scroll[y];
                    check_object(&gb, kind == 2, 2);
                }
            }
            /* Every initial byte size, including $F6..$FF wrap. The write
             * pointer uses the OLD size without wrapping its 16-bit address.
             * The two queues have different sizes to detect cross-use. */
            for (unsigned size = 0; size < 256; ++size) {
                memcpy(&gb, seed, sizeof(gb));
                set_route(&gb, 2);
                gb.hram[GBC] = modes[cgb];
                gb.hram[LINK_Y] = kind == 0 ? 0 : 0x30;
                gb.wram[5][DRAW_SIZE] = (uint8_t)size;
                gb.wram[5][ATTR_SIZE] = (uint8_t)(255 - size);
                /* Force BG low $FF: next column is same high, low $00. */
                gb.hram[SCROLL_X] = 0x78;
                gb.hram[SCROLL_Y] = (uint8_t)(0xF8 -
                    (kind == 2 ? 0x10 : kind == 0 ? 0x20 : 0x30));
                check_object(&gb, kind == 2, 2);
            }
        }
    }
}

static void test_attribute_routes_and_repetition(const GBState *seed) {
    GBState gb;
    for (unsigned route = 0; route < sizeof(routes) / sizeof(routes[0]); ++route) {
        for (unsigned stairs = 0; stairs < 2; ++stairs) {
            memcpy(&gb, seed, sizeof(gb));
            gb.wram_bank = (uint8_t)(1 + route % 7);
            set_route(&gb, route);
            gb.hram[GBC] = 0xFF;
            gb.hram[STATUS] = 0xFF;
            gb.wram[0][EXECUTED] = 0;
            gb.wram[gb.wram_bank][DRAW_SIZE] = 3;
            gb.wram[gb.wram_bank][ATTR_SIZE] = 17;
            check_object(&gb, (int)stairs, route);
            /* Append over the old terminator, retaining all previous bytes;
             * stairs ignores Link position and prior staircase latch. */
            gb.hram[LINK_X] = 0;
            gb.hram[LINK_Y] = 0xFF;
            check_object(&gb, (int)stairs, route);
        }
    }
    /* Reveal and actual chest creation occur at different times. Moving Link
     * must change the object row, not reuse the earlier VFX coordinates. */
    memcpy(&gb, seed, sizeof(gb));
    set_route(&gb, 2);
    check_reveal(&gb);
    gb.hram[LINK_Y] = 0;
    check_object(&gb, 0, 2);
}

void test_bank2_object_reveal(void) {
    assert(sizeof(ChestTileIds) == sizeof(chest_bytes));
    assert(sizeof(StaircaseTileIds) == sizeof(stair_bytes));
    assert(memcmp(ChestTileIds, chest_bytes, sizeof(chest_bytes)) == 0);
    assert(memcmp(StaircaseTileIds, stair_bytes, sizeof(stair_bytes)) == 0);

    GBState seed;
    init_seed(&seed);
    test_guards(&seed);
    test_coordinates(&seed);
    test_vfx_slots(&seed);
    test_draw_commands(&seed);
    test_attribute_routes_and_repetition(&seed);

    /* C API contract; NULL has no corresponding assembly input. */
    RevealChestEffectHandler(NULL);
    func_002_5ED3(NULL, mock_attr_lookup);
    func_002_5F5C(NULL, mock_attr_lookup);
}
