#include "test_bank2.h"

#include "gb.h"
#include "bank2/room_effects.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Independent instruction-derived oracle:
 * events.asm:81-102,151-165 (guard and 02:5E03-5E15);
 * bank2.asm:3123-3176,4515-4546 (key continuation and status address);
 * home/entities.asm:394-403 and bank0.asm:22-27 (bank 3 / saved bank).
 * Literal operands and direct arrays deliberately avoid production helpers.
 * Cxxx uses WRAM 0; Dxxx uses the selected WRAM bank in current gb.c.
 * The callback models the existing label_002_5425 C contract: slots 0..15,
 * FFFF for carry failure, NOT the fairy handler's raw-DE failure convention.
 * It preserves map ID, as the real allocator does and the verified C label
 * requires. Registers, cycles and unobservable transient writes are not tested.
 */
enum {
    EVENT = 0xC18E - 0xC000,
    EXECUTED = 0xC18F - 0xC000,
    POS_X = 0xC200 - 0xC000,
    POS_Y = 0xC210 - 0xC000,
    POS_Z = 0xC310 - 0xC000,
    INDOOR = 0xDBA5 - 0xD000,
    SAVED_BANK = 0xDBAF - 0xD000,
    MAP_ROOM = 0xFFF6 - 0xFF80,
    MAP_ID = 0xFFF7 - 0xFF80,
    ROOM_STATUS = 0xFFF8 - 0xFF80
};

static void fill_bytes(uint8_t *bytes, size_t size, unsigned salt) {
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = (uint8_t)(1u + (i * 37u + i / 251u + salt) % 255u);
    }
}

static void init_seed(GBState *gb) {
    static const uint8_t rom[] = {0x31, 0x73, 0xA5, 0xFE};
    /* Zero padding too, so memcpy snapshots and whole-struct memcmp agree. */
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
    for (unsigned bank = 1; bank < 8; ++bank) {
        gb->wram[bank][INDOOR] = 1;
        gb->wram[bank][SAVED_BANK] = 2;
        gb->wram[bank][0xD869 - 0xD000] = 0x21;
        gb->wram[bank][0xD969 - 0xD000] = 0x42;
        gb->wram[bank][0xDA69 - 0xD000] = 0x84;
        gb->wram[bank][0xDB69 - 0xD000] = 0x09;
        gb->wram[bank][0xDE49 - 0xD000] = 0xA5;
    }
    memset(&gb->wram[0][POS_X], 0xE1, 16);
    memset(&gb->wram[0][POS_Y], 0xE2, 16);
    memset(&gb->wram[0][POS_Z], 0xE3, 16);
    gb->wram[0][EVENT] = 0xE1;
    gb->wram[0][EXECUTED] = 0x80;
    gb->hram[MAP_ROOM] = 0x69;
    gb->hram[MAP_ID] = 3;
    gb->hram[ROOM_STATUS] = 0x4A;
}

static GBState callback_entry;
static unsigned callback_calls;
static uint16_t callback_result;
static uint8_t callback_type;
static int callback_initializes;

static uint16_t mock_spawn(GBState *gb, uint8_t type) {
    assert(gb != NULL);
    assert(callback_calls == 0);
    ++callback_calls;
    assert(type == callback_type);
    assert(gb->rom_bank == 3);
    assert(gb->wram[0][EVENT] == 0);
    /* Includes conditional saved status/cache updates, BEFORE allocation,
     * with no key coordinates written yet and no unrelated side effects. */
    assert(memcmp(gb, &callback_entry, sizeof(*gb)) == 0);
    if (callback_initializes && callback_result != 0xFFFF) {
        unsigned slot = callback_result;
        assert(slot < 16);
        gb->wram[0][0xC280 - 0xC000 + slot] = 2;
        gb->wram[0][0xC3A0 - 0xC000 + slot] = type;
        gb->wram[0][0xC240 - 0xC000 + slot] = 0x97;
        gb->wram[0][POS_X + slot] = 0xD1;
        gb->wram[0][POS_Y + slot] = 0xD2;
        gb->wram[0][POS_Z + slot] = 0xD3;
    }
    return callback_result;
}

/* status_address is a literal fixture for room 69, zero for all other rooms.
 * Neither GetRoomStatusAddress nor label_002_5425 builds the expected state. */
static void run_case(GBState *gb, uint16_t status_address, uint8_t type,
                     uint8_t x, uint16_t result, uint8_t restored_bank,
                     int initializes) {
    GBState expected;
    int allowed = (gb->hram[ROOM_STATUS] & 0x10) == 0 &&
                  gb->wram[0][EXECUTED] != 0;
    memcpy(&expected, gb, sizeof(expected));
    callback_calls = 0;
    callback_result = result;
    callback_type = type;
    callback_initializes = initializes;
    if (allowed) {
        expected.wram[0][EVENT] = 0;
        if (status_address != 0) {
            uint8_t value = (uint8_t)(expected.wram[gb->wram_bank]
                                      [status_address - 0xD000] | 0x10);
            expected.wram[gb->wram_bank][status_address - 0xD000] = value;
            expected.hram[ROOM_STATUS] = value;
        }
        memcpy(&callback_entry, &expected, sizeof(callback_entry));
        callback_entry.rom_bank = 3;
        expected.rom_bank = restored_bank;
        if (result != 0xFFFF) {
            assert(result < 16);
            if (initializes) {
                expected.wram[0][0xC280 - 0xC000 + result] = 2;
                expected.wram[0][0xC3A0 - 0xC000 + result] = type;
                expected.wram[0][0xC240 - 0xC000 + result] = 0x97;
            }
            expected.wram[0][POS_X + result] = x;
            expected.wram[0][POS_Y + result] = 0x3C;
            expected.wram[0][POS_Z + result] = 0x70;
        }
    }
    DropKeyEffectHandler(gb, mock_spawn);
    assert(callback_calls == (unsigned)allowed);
    assert(memcmp(gb, &expected, sizeof(*gb)) == 0);
}

static void test_guard_pairs(const GBState *seed) {
    GBState gb, unchanged;
    static const uint8_t events[] = {0, 1, 0xA0, 0xFF};
    static const uint8_t rep_executed[] = {
        0x00, 0x01, 0x10, 0x7F, 0x80, 0xFF
    };
    static const uint8_t rep_status[] = {
        0x00, 0x01, 0x0F, 0x10, 0x11, 0x4A, 0x80, 0xEF, 0xFF
    };
    unsigned accepted = 0;

    /* Sweep all 256 status values with representative executed bytes */
    for (unsigned status = 0; status < 256; ++status) {
        for (size_t e = 0; e < sizeof(rep_executed); ++e) {
            unsigned executed = rep_executed[e];
            for (unsigned special = 0; special < 2; ++special) {
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[MAP_ROOM] = special ? 0x69 : 0x68;
                gb.hram[ROOM_STATUS] = (uint8_t)status;
                gb.wram[0][EXECUTED] = (uint8_t)executed;
                gb.wram[0][EVENT] = events[(status + executed) % 4];
                memcpy(&unchanged, &gb, sizeof(unchanged));
                DropKeyEffectHandler(&gb, NULL);
                assert(memcmp(&gb, &unchanged, sizeof(gb)) == 0);
                run_case(&gb, special ? 0xD969 : 0, 0x30, 0x28, 9, 2, 0);
                accepted += callback_calls;
            }
        }
    }

    /* Sweep all 256 executed values with representative status bytes */
    for (unsigned executed = 0; executed < 256; ++executed) {
        for (size_t s = 0; s < sizeof(rep_status); ++s) {
            unsigned status = rep_status[s];
            for (unsigned special = 0; special < 2; ++special) {
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[MAP_ROOM] = special ? 0x69 : 0x68;
                gb.hram[ROOM_STATUS] = (uint8_t)status;
                gb.wram[0][EXECUTED] = (uint8_t)executed;
                gb.wram[0][EVENT] = events[(status + executed) % 4];
                memcpy(&unchanged, &gb, sizeof(unchanged));
                DropKeyEffectHandler(&gb, NULL);
                assert(memcmp(&gb, &unchanged, sizeof(gb)) == 0);
                run_case(&gb, special ? 0xD969 : 0, 0x30, 0x28, 9, 2, 0);
                accepted += callback_calls;
            }
        }
    }
    assert(accepted > 0);
}

static void test_rooms_maps_slots(const GBState *seed) {
    /* Boundaries are literal fixtures from the two distinct assembly tests:
     * status routing uses [06,1A); entity type uses 0A, with FF special. */
    static const struct {
        uint8_t indoor, first_map, last_map, type, x;
        uint16_t status_address;
    } routes[] = {
        {0, 0x00, 0x09, 0x30, 0x28, 0xD869},
        {0, 0x0A, 0xFE, 0x3C, 0x28, 0xD869},
        {0, 0xFF, 0xFF, 0x30, 0x48, 0xD869},
        {1, 0x00, 0x05, 0x30, 0x28, 0xD969},
        {1, 0x06, 0x09, 0x30, 0x28, 0xDA69},
        {1, 0x0A, 0x19, 0x3C, 0x28, 0xDA69},
        {1, 0x1A, 0xFE, 0x3C, 0x28, 0xD969},
        {1, 0xFF, 0xFF, 0x30, 0x48, 0xDE49},
        /* D is the indoor byte, not a boolean; color ignores its magnitude. */
        {2, 0x05, 0x05, 0x30, 0x28, 0xDA69},
        {2, 0x06, 0x06, 0x30, 0x28, 0xDB69},
        {2, 0xFF, 0xFF, 0x30, 0x48, 0xDE49}
    };
    static const uint8_t key_rooms[] = {0x00, 0x08, 0x69, 0xFF};
    GBState gb;
    for (size_t route = 0; route < sizeof(routes) / sizeof(routes[0]); ++route) {
        for (unsigned map = routes[route].first_map;
             map <= routes[route].last_map; ++map) {
            bool is_endpoint = (map == routes[route].first_map ||
                                map == routes[route].last_map);
            if (!is_endpoint) {
                /* Interior maps: test special and coordinate rooms with all outcomes */
                for (size_t kr = 0; kr < sizeof(key_rooms); ++kr) {
                    unsigned room = key_rooms[kr];
                    uint16_t address = room == 0x69 ? routes[route].status_address : 0;
                    uint8_t x = map == 0xFF && room == 8 ? 0x58 : routes[route].x;
                    for (unsigned outcome = 0; outcome <= 16; ++outcome) {
                        memcpy(&gb, seed, sizeof(gb));
                        gb.wram[5][INDOOR] = routes[route].indoor;
                        gb.hram[MAP_ID] = (uint8_t)map;
                        gb.hram[MAP_ROOM] = (uint8_t)room;
                        run_case(&gb, address, routes[route].type, x,
                                 outcome == 16 ? 0xFFFF : (uint16_t)outcome, 2,
                                 (int)(outcome & 1));
                    }
                }
                continue;
            }

            /* Endpoint maps: test all 17 outcomes on key boundary/special rooms */
            for (size_t kr = 0; kr < sizeof(key_rooms); ++kr) {
                unsigned room = key_rooms[kr];
                uint16_t address = room == 0x69 ? routes[route].status_address : 0;
                uint8_t x = map == 0xFF && room == 8 ? 0x58 : routes[route].x;
                for (unsigned outcome = 0; outcome <= 16; ++outcome) {
                    memcpy(&gb, seed, sizeof(gb));
                    gb.wram[5][INDOOR] = routes[route].indoor;
                    gb.hram[MAP_ID] = (uint8_t)map;
                    gb.hram[MAP_ROOM] = (uint8_t)room;
                    run_case(&gb, address, routes[route].type, x,
                             outcome == 16 ? 0xFFFF : (uint16_t)outcome, 2,
                             (int)(outcome & 1));
                }
            }

            /* And sweep all 256 rooms with success (slot 9) and failure (16) */
            for (unsigned room = 0; room < 256; ++room) {
                uint16_t address = room == 0x69 ? routes[route].status_address : 0;
                uint8_t x = map == 0xFF && room == 8 ? 0x58 : routes[route].x;
                for (unsigned oc_idx = 0; oc_idx < 2; ++oc_idx) {
                    unsigned outcome = oc_idx == 0 ? 9 : 16;
                    memcpy(&gb, seed, sizeof(gb));
                    gb.wram[5][INDOOR] = routes[route].indoor;
                    gb.hram[MAP_ID] = (uint8_t)map;
                    gb.hram[MAP_ROOM] = (uint8_t)room;
                    run_case(&gb, address, routes[route].type, x,
                             outcome == 16 ? 0xFFFF : (uint16_t)outcome, 2,
                             (int)(outcome & 1));
                }
            }
        }
    }
}

static void test_saved_status_and_banks(const GBState *seed) {
    static const struct {
        uint8_t indoor, map;
        uint16_t address;
    } routes[] = {
        {0, 0xFF, 0xD869}, {1, 0x03, 0xD969},
        {1, 0x06, 0xDA69}, {1, 0xFF, 0xDE49}
    };
    static const struct { uint8_t saved, restored; } banks[] = {
        {0, 1}, {2, 2}, {0x23, 0x23}, {0x80, 0}, {0xB5, 0x35}, {0xFF, 0x7F}
    };
    static const uint8_t caches[] = {0, 0x01, 0x4A, 0xEF};
    static const uint8_t rep_saved[] = {
        0x00, 0x01, 0x0F, 0x10, 0x11, 0x40, 0x80, 0xEF, 0xFF
    };
    GBState gb;

    /* Representative saved status sweep across all banks, routes, and caches */
    for (unsigned bank = 1; bank < 8; ++bank) {
        for (size_t route = 0; route < sizeof(routes) / sizeof(routes[0]); ++route) {
            for (size_t s = 0; s < sizeof(rep_saved); ++s) {
                unsigned saved = rep_saved[s];
                for (size_t cache = 0; cache < sizeof(caches); ++cache) {
                    const size_t b = (saved + cache) % (sizeof(banks) / sizeof(banks[0]));
                    memcpy(&gb, seed, sizeof(gb));
                    gb.wram_bank = (uint8_t)bank;
                    gb.wram[bank][INDOOR] = routes[route].indoor;
                    gb.wram[bank][SAVED_BANK] = banks[b].saved;
                    gb.wram[bank][routes[route].address - 0xD000] = (uint8_t)saved;
                    gb.hram[MAP_ID] = routes[route].map;
                    gb.hram[ROOM_STATUS] = caches[cache];
                    run_case(&gb, routes[route].address, 0x30,
                             routes[route].map == 0xFF ? 0x48 : 0x28,
                             cache & 1 ? 0xFFFF : 15, banks[b].restored, 1);
                }
            }
        }
    }

    /* Full 256 saved byte sweep on bank 1, route 0 */
    for (unsigned saved = 0; saved < 256; ++saved) {
        for (size_t cache = 0; cache < sizeof(caches); ++cache) {
            const size_t b = (saved + cache) % (sizeof(banks) / sizeof(banks[0]));
            memcpy(&gb, seed, sizeof(gb));
            gb.wram_bank = 1;
            gb.wram[1][INDOOR] = routes[0].indoor;
            gb.wram[1][SAVED_BANK] = banks[b].saved;
            gb.wram[1][routes[0].address - 0xD000] = (uint8_t)saved;
            gb.hram[MAP_ID] = routes[0].map;
            gb.hram[ROOM_STATUS] = caches[cache];
            run_case(&gb, routes[0].address, 0x30, 0x48,
                     cache & 1 ? 0xFFFF : 15, banks[b].restored, 1);
        }
    }
}

static void test_repeated_and_null(const GBState *seed) {
    GBState gb, unchanged;
    for (unsigned special = 0; special < 2; ++special) {
        for (unsigned fail = 0; fail < 2; ++fail) {
            memcpy(&gb, seed, sizeof(gb));
            gb.hram[MAP_ROOM] = special ? 0x69 : 0x08;
            run_case(&gb, special ? 0xD969 : 0, 0x30, 0x28,
                     fail ? 0xFFFF : 0, 2, 1);
            /* Room 69's new cache blocks even after allocation failure.
             * Other rooms can spawn again despite EVENT now being zero. */
            run_case(&gb, special ? 0xD969 : 0, 0x30, 0x28, 15, 2, 1);
            assert(callback_calls == (unsigned)!special);
            memcpy(&unchanged, &gb, sizeof(unchanged));
            callback_calls = 0;
            DropKeyEffectHandler(&gb, NULL);
            DropKeyEffectHandler(NULL, mock_spawn);
            DropKeyEffectHandler(NULL, NULL);
            assert(callback_calls == 0);
            assert(memcmp(&gb, &unchanged, sizeof(gb)) == 0);
        }
    }
}

void test_bank2_key_drop_effect(void) {
    GBState seed;
    init_seed(&seed);
    test_guard_pairs(&seed);
    test_rooms_maps_slots(&seed);
    test_saved_status_and_banks(&seed);
    test_repeated_and_null(&seed);
}
