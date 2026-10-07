#include "test_bank2.h"

#include "gb.h"
#include "bank2/room_triggers.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Independent oracle: events.asm:486-716 (02:5FC6-60D7), bank0.asm:834-858
 * (MarkTriggerAsResolved), events.asm:81-102,182-278,290-315 (effect helpers),
 * bank0.asm:926-974 (AddTranscientVfx), bank2.asm:4515-4546 and 02:5420.
 * Only literal assembly addresses/values construct expected memory. No
 * production memory, trigger, effect, VFX, bank or room-routing helpers.
 * Cxxx is fixed WRAM; Dxxx follows GBState's selected WRAM bank.
 * Full snapshots include deterministic padding and every bank/region. They
 * cannot observe CPU registers, scan order without side effects, or transient
 * writes subsequently undone. The dispatcher is deliberately not tested.
 */
#define C(g, address) ((g)->wram[0][(address) - 0xC000])
#define D(g, address) ((g)->wram[(g)->wram_bank][(address) - 0xD000])
#define H(g, address) ((g)->hram[(address) - 0xFF80])

typedef void (*Trigger)(GBState *);
static const Trigger triggers[] = {
    CheckKillSidescrollBossTrigger, CheckLightTorchesTrigger,
    CheckStepOnButtonTrigger, CheckKillInOrderTrigger,
    CheckKillEnemiesTrigger, CheckAnswerTunicsTrigger
};
static const uint8_t edges[] = {0, 1, 0x80, 0xFF};

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
    gb->rom_bank = 0x35;
    gb->wram_bank = 5;
    gb->vram_bank = 1;
    gb->sram_bank = 3;
    gb->sram_enabled = true;
    gb->joypad_input = 0xA6;
    C(gb, 0xC18F) = 0;
    C(gb, 0xC19D) = 0;
    C(gb, 0xC190) = 0;
    C(gb, 0xC18E) = 0xC1;
    C(gb, 0xC113) = 0;
    C(gb, 0xC1A2) = 2;
    C(gb, 0xC1CB) = 0x80;
    D(gb, 0xD460) = 1;
    D(gb, 0xDAE8) = 0x20;
    D(gb, 0xD9FF) = 0x20;
    D(gb, 0xDBB6) = 0;
    D(gb, 0xDBB7) = 1;
    D(gb, 0xDBB8) = 2;
    D(gb, 0xDBA5) = 1;
    H(gb, 0xFFD7) = 8;
    H(gb, 0xFFF6) = 8;
    H(gb, 0xFFF7) = 0xFF;
    H(gb, 0xFFF8) = 0xA5;
    H(gb, 0xFF98) = 0x88;
    H(gb, 0xFF99) = 0x30;
    for (unsigned slot = 0; slot < 16; ++slot) {
        C(gb, 0xC280 + slot) = 0;
        C(gb, 0xC430 + slot) = 0;
        C(gb, 0xC3A0 + slot) = 0;
    }
    C(gb, 0xC51F) = 0;
    C(gb, 0xC5C0) = 0;
}

static void expect_mark(GBState *gb) {
    if (C(gb, 0xC18F) != 0) {
        return;
    }
    C(gb, 0xC1CF) = 0;
    C(gb, 0xC18F) = 1;
    C(gb, 0xC5A6) = 1;
    if (C(gb, 0xC19D) == 0) {
        H(gb, 0xFFF2) = 2;
    }
}

/* Optimized working-memory-only copy/compare for room trigger tests. */
static void copy_working_memory(GBState *dst, const GBState *src) {
    memcpy(dst->wram, src->wram, sizeof(dst->wram));
    memcpy(dst->hram, src->hram, sizeof(dst->hram));
    dst->wram_bank = src->wram_bank;
    dst->rom_bank = src->rom_bank;
    dst->vram_bank = src->vram_bank;
    dst->sram_bank = src->sram_bank;
    dst->sram_enabled = src->sram_enabled;
    dst->joypad_input = src->joypad_input;
    dst->ie = src->ie;
    /* rom, rom_size, vram, sram, oam, io are not modified by room trigger tests */
}

static int compare_working(const GBState *a, const GBState *b) {
    if (memcmp(a->wram, b->wram, sizeof(a->wram)) != 0) return 1;
    if (memcmp(a->hram, b->hram, sizeof(a->hram)) != 0) return 1;
    if (a->wram_bank != b->wram_bank) return 1;
    if (a->rom_bank != b->rom_bank) return 1;
    if (a->vram_bank != b->vram_bank) return 1;
    if (a->sram_bank != b->sram_bank) return 1;
    if (a->sram_enabled != b->sram_enabled) return 1;
    if (a->joypad_input != b->joypad_input) return 1;
    if (a->ie != b->ie) return 1;
    return 0;
}

static void compare(const GBState *gb, const GBState *expected, unsigned api) {
    if (compare_working(gb, expected) != 0) {
        const unsigned char *actual = (const unsigned char *)gb;
        const unsigned char *wanted = (const unsigned char *)expected;
        for (size_t i = 0; i < sizeof(*gb); ++i) {
            if (((const unsigned char *)gb)[i] != ((const unsigned char *)expected)[i]) {
                fprintf(stderr, "room trigger API %u: GBState byte %zu: %02X != %02X\n",
                        api, i, (unsigned)actual[i], (unsigned)wanted[i]);
                break;
            }
        }
        assert(!"room trigger full-state mismatch");
    }
}



static void check_simple(GBState *gb, unsigned api, bool resolved) {
    GBState expected;
    copy_working_memory(&expected, gb);
    if (resolved) {
        expect_mark(&expected);
    }
    triggers[api](gb);
    compare(gb, &expected, api);
}

/* Count is supplied by each independently constructed fixture, not obtained
 * by copying the production entity scan. status_address is a literal route
 * fixture, used only by room $12 after its effect guard succeeds. */
static void check_tunics(GBState *gb, unsigned count, uint16_t status_address) {
    GBState expected;
    copy_working_memory(&expected, gb);
    if (C(&expected, 0xC18F) == 0) {
        uint8_t room = H(&expected, 0xFFF6);
        H(&expected, 0xFFD7) = (uint8_t)count;
        unsigned required = room == 0x12 ? 2u : room == 0x0A ? 9u : 4u;
        if (count == required) {
            expect_mark(&expected);
            if (room != 8 && !(H(&expected, 0xFFF8) & 0x10)) {
                C(&expected, 0xC18E) = 0;
                if (room == 0x0A) {
                    /* Guard clears C1 before shutter dispatch: no miniboss
                     * progress/jingle even if the incoming event was C1.
                     * CloseDoors cannot close after Mark set C18F to 1. */
                    if (C(&expected, 0xC190) != 0) {
                        C(&expected, 0xC18C) = 1;
                        H(&expected, 0xFFF4) = 4;
                    }
                    D(&expected, 0xDDEA) |= 0x10;
                    /* Unlike room 12, the cached status is not updated. */
                } else if (room == 0x12) {
                    assert(status_address >= 0xD000 && status_address < 0xE000);
                    D(&expected, status_address) |= 0x10;
                    H(&expected, 0xFFF8) = D(&expected, status_address);
                } else {
                    H(&expected, 0xFFD7) = 0x88;
                    uint8_t x = H(&expected, 0xFF98);
                    uint8_t y = H(&expected, 0xFF99);
                    H(&expected, 0xFFD8) =
                        x >= 0x78 && x <= 0x97 && y >= 0x28 && y <= 0x37
                        ? 0x40 : 0x30;
                    int slot = 15;
                    while (slot >= 0 && C(&expected, 0xC510 + slot) != 0) {
                        --slot;
                    }
                    if (slot < 0) {
                        uint8_t ring = C(&expected, 0xC5C0);
                        ring = ring == 0 ? 15 : (uint8_t)(ring - 1);
                        C(&expected, 0xC5C0) = ring;
                        slot = ring;
                    }
                    C(&expected, 0xC510 + slot) = 3;
                    C(&expected, 0xC520 + slot) = 0x0F;
                    C(&expected, 0xC530 + slot) = 0x88;
                    C(&expected, 0xC540 + slot) = H(&expected, 0xFFD8);
                }
            }
        }
    }
    CheckAnswerTunicsTrigger(gb);
    compare(gb, &expected, 5);
}

static void set_answer(GBState *gb, uint8_t room, unsigned count, unsigned first) {
    H(gb, 0xFFF6) = room;
    for (unsigned i = 0; i < 16; ++i) {
        unsigned slot = (first + i) % 16;
        C(gb, 0xC280 + slot) = i < count ? 0x80 : 0;
        C(gb, 0xC3A0 + slot) = room == 0x12 ? (uint8_t)(0xF6 + i % 2)
                                                         : (uint8_t)(0xEF + i % 3);
        C(gb, 0xC3B0 + slot) = 8;
        C(gb, 0xC290 + slot) = 4;
    }
}

static void test_simple_bytes(const GBState *seed) {
    GBState gb;
    for (unsigned value = 0; value < 256; ++value) {
        copy_working_memory(&gb, seed);
        C(&gb, 0xC1A2) = (uint8_t)value;
        check_simple(&gb, 1, value == 2);
        copy_working_memory(&gb, seed);
        C(&gb, 0xC1CB) = (uint8_t)value;
        check_simple(&gb, 2, value != 0);
    }

    /* Map == 6 sweep across all 256 status bytes */
    for (unsigned status = 0; status < 256; ++status) {
        copy_working_memory(&gb, seed);
        H(&gb, 0xFFF7) = 6;
        D(&gb, 0xDAE8) = (uint8_t)status;
        D(&gb, 0xD9FF) = (uint8_t)(status ^ 0x20);
        check_simple(&gb, 0, (status & 0x20) != 0);
    }

    /* Representative non-6 maps sweep across all 256 status bytes */
    static const uint8_t rep_maps[] = {0, 1, 5, 7, 0x0A, 0x7F, 0x80, 0xFE, 0xFF};
    for (size_t mi = 0; mi < sizeof(rep_maps); ++mi) {
        unsigned map = rep_maps[mi];
        for (unsigned status = 0; status < 256; ++status) {
            copy_working_memory(&gb, seed);
            H(&gb, 0xFFF7) = (uint8_t)map;
            D(&gb, 0xDAE8) = (uint8_t)(status ^ 0x20);
            D(&gb, 0xD9FF) = (uint8_t)status;
            check_simple(&gb, 0, (status & 0x20) != 0);
        }
    }

    /* Sweep all 256 maps with bit 5 set and bit 5 clear */
    for (unsigned map = 0; map < 256; ++map) {
        for (unsigned s = 0; s < 2; ++s) {
            unsigned status = s ? 0x20 : 0x00;
            copy_working_memory(&gb, seed);
            H(&gb, 0xFFF7) = (uint8_t)map;
            D(&gb, 0xDAE8) = (uint8_t)(map == 6 ? status : status ^ 0x20);
            D(&gb, 0xD9FF) = (uint8_t)(map == 6 ? status ^ 0x20 : status);
            check_simple(&gb, 0, (status & 0x20) != 0);
        }
    }

    /* CheckKillInOrderTrigger: sweep x and y across all 256 values with edge pairs */
    for (unsigned fixed = 0; fixed < 3; ++fixed) {
        unsigned a = (fixed + 1) % 3, b = (fixed + 2) % 3;
        unsigned rep_y_vals[] = {b, (b + 1) % 256, 0, 0xFF};
        for (unsigned x = 0; x < 256; ++x) {
            for (size_t yi = 0; yi < sizeof(rep_y_vals) / sizeof(rep_y_vals[0]); ++yi) {
                unsigned y = rep_y_vals[yi];
                copy_working_memory(&gb, seed);
                D(&gb, 0xDBB6 + a) = (uint8_t)x;
                D(&gb, 0xDBB6 + b) = (uint8_t)y;
                check_simple(&gb, 3, x == a && y == b);
            }
        }
        unsigned rep_x_vals[] = {a, (a + 1) % 256, 0, 0xFF};
        for (unsigned y = 0; y < 256; ++y) {
            for (size_t xi = 0; xi < sizeof(rep_x_vals) / sizeof(rep_x_vals[0]); ++xi) {
                unsigned x = rep_x_vals[xi];
                copy_working_memory(&gb, seed);
                D(&gb, 0xDBB6 + a) = (uint8_t)x;
                D(&gb, 0xDBB6 + b) = (uint8_t)y;
                check_simple(&gb, 3, x == a && y == b);
            }
        }
    }
}

static void test_resolution_guards(const GBState *seed) {
    GBState gb;
    /* All executed/suppress-jingle bytes individually for every entry point.
     * A satisfied tunic puzzle must return BEFORE resetting MP0 when executed. */
    for (unsigned api = 0; api < 6; ++api) {
        for (unsigned field = 0; field < 2; ++field) {
            for (unsigned value = 0; value < 256; ++value) {
                copy_working_memory(&gb, seed);
                set_answer(&gb, 8, 4, 0);
                if (api == 4) {
                    for (unsigned slot = 0; slot < 16; ++slot) {
                        C(&gb, 0xC280 + slot) = 0;
                    }
                }
                C(&gb, field ? 0xC19D : 0xC18F) = (uint8_t)value;
                if (api == 5) {
                    check_tunics(&gb, 4, 0);
                } else {
                    check_simple(&gb, api, true);
                }
            }
        }
    }
}

static void test_enemies(const GBState *seed) {
    GBState gb;
    static const uint8_t rep_options[] = {0, 1, 2, 3, 0x0E, 0x0F, 0x7F, 0x80, 0xFD, 0xFE, 0xFF};
    static const uint8_t rep_statuses[] = {0, 1, 2, 5, 0x7F, 0x80, 0xFE, 0xFF};

    /* Sweep all 256 status bytes with representative options */
    for (unsigned status = 0; status < 256; ++status) {
        for (size_t oi = 0; oi < sizeof(rep_options); ++oi) {
            unsigned options = rep_options[oi];
            copy_working_memory(&gb, seed);
            unsigned slot = (status + options) % 16;
            C(&gb, 0xC280 + slot) = (uint8_t)status;
            C(&gb, 0xC430 + slot) = (uint8_t)options;
            check_simple(&gb, 4, status == 0 || (options & 2) != 0);
        }
    }

    /* Sweep all 256 options bytes with representative statuses */
    for (unsigned options = 0; options < 256; ++options) {
        for (size_t si = 0; si < sizeof(rep_statuses); ++si) {
            unsigned status = rep_statuses[si];
            copy_working_memory(&gb, seed);
            unsigned slot = (status + options) % 16;
            C(&gb, 0xC280 + slot) = (uint8_t)status;
            C(&gb, 0xC430 + slot) = (uint8_t)options;
            check_simple(&gb, 4, status == 0 || (options & 2) != 0);
        }
    }

    /* All slots independently, mixed ignored/disabled entities elsewhere. */
    for (unsigned slot = 0; slot < 16; ++slot) {
        for (unsigned value = 0; value < 256; ++value) {
            for (unsigned excluded = 0; excluded < 2; ++excluded) {
                copy_working_memory(&gb, seed);
                for (unsigned other = 0; other < 16; ++other) {
                    C(&gb, 0xC280 + other) = other & 1 ? 0xFF : 0;
                    C(&gb, 0xC430 + other) = other & 1 ? 0x02 : 0xFD;
                }
                C(&gb, 0xC280 + slot) = (uint8_t)value;
                C(&gb, 0xC430 + slot) = excluded ? 0xFF : 0xFD;
                check_simple(&gb, 4, value == 0 || excluded != 0);
            }
        }
    }

    /* Sweep all 256 ready bytes with representative killed bytes */
    static const uint8_t rep_killed[] = {0, 1, 2, 0x7F, 0x80, 0xFF};
    for (unsigned ready = 0; ready < 256; ++ready) {
        for (size_t ki = 0; ki < sizeof(rep_killed); ++ki) {
            unsigned killed = rep_killed[ki];
            copy_working_memory(&gb, seed);
            D(&gb, 0xD460) = (uint8_t)ready;
            C(&gb, 0xC113) = (uint8_t)killed;
            check_simple(&gb, 4, ready != 0 && killed == 0);
        }
    }

    /* Sweep all 256 killed bytes with representative ready bytes */
    for (unsigned killed = 0; killed < 256; ++killed) {
        for (size_t ri = 0; ri < sizeof(rep_killed); ++ri) {
            unsigned ready = rep_killed[ri];
            copy_working_memory(&gb, seed);
            D(&gb, 0xD460) = (uint8_t)ready;
            C(&gb, 0xC113) = (uint8_t)killed;
            check_simple(&gb, 4, ready != 0 && killed == 0);
        }
    }

    for (unsigned id = 0; id < 256; ++id) {
        for (unsigned combination = 0; combination < 4; ++combination) {
            copy_working_memory(&gb, seed);
            H(&gb, 0xFFD7) = (uint8_t)id;
            D(&gb, 0xD460) = (uint8_t)(combination & 1 ? 0x80 : 0);
            C(&gb, 0xC113) = combination & 2 ? 0xFF : 0;
            check_simple(&gb, 4, id != 8 || combination == 1);
        }
    }
}

static void test_tunic_counts_and_entities(const GBState *seed) {
    static const uint8_t rooms[] = {8, 0x0A, 0x12, 0, 9, 0xFF};
    static const uint8_t types[] = {0xEF, 0xF0, 0xF1, 0xF6, 0xF7};
    GBState gb;
    for (size_t r = 0; r < sizeof(rooms); ++r) {
        for (unsigned count = 0; count <= 16; ++count) {
            for (unsigned first = 0; first < 16; ++first) {
                copy_working_memory(&gb, seed);
                set_answer(&gb, rooms[r], count, first);
                check_tunics(&gb, count, 0xDDF2);
            }
        }
    }
    /* For each path, put one candidate in every slot and enough known-good
     * companions in other slots to make that candidate decisive. */
    for (unsigned special = 0; special < 2; ++special) {
        unsigned baseline = special ? 1 : 3;
        uint8_t room = special ? 0x12 : 8;
        for (unsigned slot = 0; slot < 16; ++slot) {
            static const uint8_t rep_types[] = {0, 1, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF5, 0xF6, 0xF7, 0xF8, 0xFF};
            for (size_t ti = 0; ti < sizeof(rep_types); ++ti) {
                unsigned type = rep_types[ti];
                for (size_t s = 0; s < sizeof(edges); ++s) {
                    copy_working_memory(&gb, seed);
                    set_answer(&gb, room, baseline, (slot + 1) % 16);
                    C(&gb, 0xC3A0 + slot) = (uint8_t)type;
                    C(&gb, 0xC280 + slot) = edges[s];
                    bool matches = special ? type == 0xF6 || type == 0xF7
                                           : type == 0xEF || type == 0xF0 || type == 0xF1;
                    check_tunics(&gb, baseline + (matches && edges[s] != 0), 0xDDF2);
                }
            }
            if (slot == 0 || slot == 15) {
                for (unsigned type = 0; type < 256; ++type) {
                    for (size_t s = 0; s < sizeof(edges); ++s) {
                        copy_working_memory(&gb, seed);
                        set_answer(&gb, room, baseline, (slot + 1) % 16);
                        C(&gb, 0xC3A0 + slot) = (uint8_t)type;
                        C(&gb, 0xC280 + slot) = edges[s];
                        bool matches = special ? type == 0xF6 || type == 0xF7
                                               : type == 0xEF || type == 0xF0 || type == 0xF1;
                        check_tunics(&gb, baseline + (matches && edges[s] != 0), 0xDDF2);
                    }
                }
            }

            static const uint8_t rep_values[] = {0, 1, 2, 4, 8, 16, 0x7F, 0x80, 0xFE, 0xFF};
            for (size_t t = 0; t < sizeof(types); ++t) {
                for (size_t vi = 0; vi < sizeof(rep_values); ++vi) {
                    unsigned value = rep_values[vi];
                    for (unsigned field = 0; field < 2; ++field) {
                        copy_working_memory(&gb, seed);
                        set_answer(&gb, room, baseline, (slot + 1) % 16);
                        C(&gb, 0xC3A0 + slot) = types[t];
                        C(&gb, 0xC280 + slot) = field ? 0xFF : (uint8_t)value;
                        C(&gb, (special ? 0xC290 : 0xC3B0) + slot) =
                            field ? (uint8_t)value : special ? 4 : 8;
                        C(&gb, (special ? 0xC3B0 : 0xC290) + slot) = (uint8_t)~value;
                        bool matches = special ? types[t] == 0xF6 || types[t] == 0xF7
                                               : types[t] >= 0xEF && types[t] <= 0xF1;
                        bool qualifies = field ? value == (special ? 4u : 8u) : value != 0;
                        check_tunics(&gb, baseline + (matches && qualifies), 0xDDF2);
                    }
                }
                if (slot == 0 || slot == 15) {
                    for (unsigned value = 0; value < 256; ++value) {
                        for (unsigned field = 0; field < 2; ++field) {
                            copy_working_memory(&gb, seed);
                            set_answer(&gb, room, baseline, (slot + 1) % 16);
                            C(&gb, 0xC3A0 + slot) = types[t];
                            C(&gb, 0xC280 + slot) = field ? 0xFF : (uint8_t)value;
                            C(&gb, (special ? 0xC290 : 0xC3B0) + slot) =
                                field ? (uint8_t)value : special ? 4 : 8;
                            C(&gb, (special ? 0xC3B0 : 0xC290) + slot) = (uint8_t)~value;
                            bool matches = special ? types[t] == 0xF6 || types[t] == 0xF7
                                                   : types[t] >= 0xEF && types[t] <= 0xF1;
                            bool qualifies = field ? value == (special ? 4u : 8u) : value != 0;
                            check_tunics(&gb, baseline + (matches && qualifies), 0xDDF2);
                        }
                    }
                }
            }
        }
    }
}

static void test_tunic_guards_and_routes(const GBState *seed) {
    /* Literal results of bank2.asm GetRoomStatusAddress, including raw
     * nonboolean indoor-byte arithmetic rather than normalizing it to 1. */
    static const struct {
        uint8_t indoor, map;
        uint16_t address;
    } routes[] = {
        {0, 0, 0xD812}, {0, 0xFF, 0xD812},
        {1, 0, 0xD912}, {1, 5, 0xD912}, {1, 6, 0xDA12},
        {1, 0x19, 0xDA12}, {1, 0x1A, 0xD912},
        {1, 0x80, 0xD912}, {1, 0xFF, 0xDDF2},
        {2, 0, 0xDA12}, {2, 6, 0xDB12}, {0xFF, 6, 0xD812},
        {0xFF, 0, 0xD712}, {0x80, 0xFF, 0xDDF2}
    };
    static const uint8_t rooms[] = {8, 0x0A, 0x12, 9};
    GBState gb;
    for (size_t r = 0; r < sizeof(rooms); ++r) {
        for (unsigned cache = 0; cache < 256; ++cache) {
            for (size_t latch = 0; latch < sizeof(edges); ++latch) {
                copy_working_memory(&gb, seed);
                unsigned count = rooms[r] == 0x12 ? 2 : rooms[r] == 0x0A ? 9 : 4;
                set_answer(&gb, rooms[r], count, 0);
                H(&gb, 0xFFF8) = (uint8_t)cache;
                C(&gb, 0xC190) = edges[latch];
                C(&gb, 0xC19D) = edges[(cache + latch) % sizeof(edges)];
                check_tunics(&gb, count, 0xDDF2);
                /* Re-entry must preserve count (or chest scratch), even when
                 * saved/cached event flags did not change on the first call. */
                check_tunics(&gb, count, 0xDDF2);
            }
        }
    }
    static const uint8_t rep_saved_routes[] = {
        0x00, 0x01, 0x0E, 0x0F, 0x10, 0x11, 0x1F, 0x20, 0x40, 0x7F, 0x80, 0xEF, 0xF0, 0xFE, 0xFF
    };
    for (size_t r = 0; r < sizeof(routes) / sizeof(routes[0]); ++r) {
        for (unsigned bank = 1; bank <= 7; ++bank) {
            for (size_t si = 0; si < sizeof(rep_saved_routes); ++si) {
                unsigned saved = rep_saved_routes[si];
                copy_working_memory(&gb, seed);
                gb.wram_bank = (uint8_t)bank;
                D(&gb, 0xDBA5) = routes[r].indoor;
                H(&gb, 0xFFF7) = routes[r].map;
                set_answer(&gb, 0x12, 2, 15);
                D(&gb, routes[r].address) = (uint8_t)saved;
                check_tunics(&gb, 2, routes[r].address);
            }
        }
    }
    for (unsigned saved = 0; saved < 256; ++saved) {
        copy_working_memory(&gb, seed);
        gb.wram_bank = 1;
        D(&gb, 0xDBA5) = routes[0].indoor;
        H(&gb, 0xFFF7) = routes[0].map;
        set_answer(&gb, 0x12, 2, 15);
        D(&gb, routes[0].address) = (uint8_t)saved;
        check_tunics(&gb, 2, routes[0].address);
    }
    /* Room 0A uses color status regardless of map/indoor; all other room IDs
     * except 08/12 take the chest branch. No color-map validation is allowed. */
    for (unsigned value = 0; value < 256; ++value) {
        copy_working_memory(&gb, seed);
        H(&gb, 0xFFF7) = (uint8_t)value;
        D(&gb, 0xDBA5) = (uint8_t)value;
        D(&gb, 0xDDEA) = (uint8_t)value;
        C(&gb, 0xC190) = 0x80;
        C(&gb, 0xC18E) = (uint8_t)value;
        set_answer(&gb, 0x0A, 9, 15);
        check_tunics(&gb, 9, 0);
        copy_working_memory(&gb, seed);
        H(&gb, 0xFFF7) = (uint8_t)value;
        unsigned count = value == 0x12 ? 2 : value == 0x0A ? 9 : 4;
        set_answer(&gb, (uint8_t)value, count, 15);
        check_tunics(&gb, count, 0xDA12);
    }
}

static void test_chest_dependency(const GBState *seed) {
    static const uint8_t xs[] = {0, 0x77, 0x78, 0x88, 0x97, 0x98, 0xFF};
    static const uint8_t ys[] = {0, 0x27, 0x28, 0x30, 0x37, 0x38, 0xFF};
    GBState gb;
    for (size_t x = 0; x < sizeof(xs); ++x) {
        for (size_t y = 0; y < sizeof(ys); ++y) {
            for (int free_slot = -1; free_slot < 16; ++free_slot) {
                copy_working_memory(&gb, seed);
                set_answer(&gb, 9, 4, 15);
                H(&gb, 0xFF98) = xs[x];
                H(&gb, 0xFF99) = ys[y];
                for (unsigned slot = 0; slot < 16; ++slot) {
                    C(&gb, 0xC510 + slot) = slot & 1 ? 1 : 0xFF;
                }
                if (free_slot >= 0) {
                    C(&gb, 0xC510 + free_slot) = 0;
                    /* A second lower free slot must lose the descending scan. */
                    C(&gb, 0xC510 + free_slot / 2) = 0;
                }
                check_tunics(&gb, 4, 0);
            }
        }
    }
    /* Full VFX table replacement, including ring wrap and byte-wide indices. */
    for (unsigned ring = 0; ring < 256; ++ring) {
        copy_working_memory(&gb, seed);
        set_answer(&gb, 0, 4, 0);
        for (unsigned slot = 0; slot < 16; ++slot) {
            C(&gb, 0xC510 + slot) = 0x80;
        }
        C(&gb, 0xC5C0) = (uint8_t)ring;
        check_tunics(&gb, 4, 0);
    }
}

static void test_simple_banks(const GBState *seed) {
    GBState gb;
    for (unsigned bank = 1; bank <= 7; ++bank) {
        for (unsigned api = 0; api < 5; ++api) {
            for (unsigned resolved = 0; resolved < 2; ++resolved) {
                copy_working_memory(&gb, seed);
                gb.wram_bank = (uint8_t)bank;
                gb.rom_bank = (uint8_t)(bank * 9);
                D(&gb, 0xDBB6) = 0;
                D(&gb, 0xDBB7) = 1;
                D(&gb, 0xDBB8) = resolved ? 2 : 3;
                H(&gb, 0xFFF7) = bank & 1 ? 6 : 0xFF;
                D(&gb, 0xDAE8) = D(&gb, 0xD9FF) = resolved ? 0x20 : 0;
                C(&gb, 0xC1A2) = resolved ? 2 : 0xFF;
                C(&gb, 0xC1CB) = resolved ? 0xFF : 0;
                D(&gb, 0xD460) = resolved ? 0xFF : 0;
                check_simple(&gb, api, resolved != 0);
            }
        }
    }
}

void test_bank2_room_triggers(void) {
    GBState seed;
    for (size_t api = 0; api < sizeof(triggers) / sizeof(triggers[0]); ++api) {
        triggers[api](NULL);
    }
    init_seed(&seed);
    test_simple_bytes(&seed);
    test_resolution_guards(&seed);
    test_enemies(&seed);
    test_tunic_counts_and_entities(&seed);
    test_tunic_guards_and_routes(&seed);
    test_chest_dependency(&seed);
    test_simple_banks(&seed);
}
