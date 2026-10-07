#include "test_bank2.h"

#include "gb.h"
#include "bank2/room_effects.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Independent oracle, read from:
 *   events.asm:104-149 (02:5DC2-5E02), guard at 02:5DAF-5DC1;
 *   bank0.asm:926-974 (00:0CC7-0D06), AddTranscientVfx;
 *   bank2.asm:4515-4546 (02:5B9F-5BC3), GetRoomStatusAddress;
 *   home/entities.asm:394-403, SpawnNewEntity_trampoline;
 *   bank0.asm:22-27, ReloadSavedBank.
 * All addresses below are literal instruction operands. Expected states use
 * direct GBState arrays, never production memory/address/VFX/guard helpers.
 * Current gb.c maps Cxxx to WRAM 0 and Dxxx to the selected WRAM bank.
 *
 * The allocator is an unfinished external dependency, represented by a mock,
 * not an allocator implementation or an assertion about allocator internals.
 * Its result is raw assembly DE: 0..15, or 00FF when full, NOT C's FFFF
 * failure convention. There is no carry test in this handler. CPU registers,
 * carry, cycle timing, and transient writes undone before observation are
 * outside GBState comparison coverage. The callback boundary is observable.
 */
enum {
    EVENT = 0xC18E - 0xC000,
    EXECUTED = 0xC18F - 0xC000,
    POS_X = 0xC200 - 0xC000,
    POS_Y = 0xC210 - 0xC000,
    SLOW_COUNTDOWN = 0xC450 - 0xC000,
    VFX_TYPE = 0xC510 - 0xC000,
    VFX_COUNTDOWN = 0xC520 - 0xC000,
    VFX_X = 0xC530 - 0xC000,
    VFX_Y = 0xC540 - 0xC000,
    VFX_RING = 0xC5C0 - 0xC000,
    INDOOR = 0xDBA5 - 0xD000,
    SAVED_BANK = 0xDBAF - 0xD000,
    MP0 = 0xFFD7 - 0xFF80,
    MP1 = 0xFFD8 - 0xFF80,
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
    /* Deterministic padding; every subsequent snapshot uses memcpy. */
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
    gb->wram[5][INDOOR] = 0;
    gb->wram[5][SAVED_BANK] = 2;
    gb->wram[5][0xD837 - 0xD000] = 0xA5;
    gb->hram[MAP_ROOM] = 0x37;
    gb->hram[MAP_ID] = 0xFF; /* Overworld must ignore even color map ID. */
    gb->hram[ROOM_STATUS] = 0x4A;
    gb->hram[MP0] = 0x17;
    gb->hram[MP1] = 0xE9;
    gb->wram[0][EVENT] = 0xE1;
    gb->wram[0][EXECUTED] = 0x80;
    memset(&gb->wram[0][VFX_TYPE], 0xA3, 16);
    gb->wram[0][VFX_TYPE + 15] = 0;
    gb->wram[0][VFX_RING] = 7;
}

static void expect_vfx(GBState *expected, unsigned slot, uint8_t type,
                       uint8_t x, uint8_t y) {
    expected->wram[0][VFX_TYPE + slot] = type;
    expected->wram[0][VFX_Y + slot] = y;
    expected->wram[0][VFX_X + slot] = x;
    expected->wram[0][VFX_COUNTDOWN + slot] = 0x0F;
}

static void expect_status(GBState *expected, unsigned bank, uint16_t address) {
    /* The address is supplied by a literal fixture, not recomputed from map
     * IDs. In particular the saved byte, not the HRAM cache, is ORed. */
    uint8_t value = (uint8_t)(expected->wram[bank][address - 0xD000] | 0x10);
    expected->wram[bank][address - 0xD000] = value;
    expected->hram[ROOM_STATUS] = value;
}

static void expect_fairy_writes(GBState *expected, uint16_t raw_de) {
    expected->wram[0][POS_X + raw_de] = 0x88;
    expected->wram[0][POS_Y + raw_de] = 0x30;
    expected->wram[0][SLOW_COUNTDOWN + raw_de] = 0x80;
    expected->hram[MP0] = 0x88;
    expected->hram[MP1] = 0x30;
}

static GBState callback_entry;
static unsigned callback_calls;
static uint16_t callback_de;
static int callback_mutates;

static uint16_t mock_spawn(GBState *gb, uint8_t type) {
    assert(gb != NULL);
    assert(type == 0x2F);
    assert(callback_calls == 0);
    ++callback_calls;
    assert(gb->rom_bank == 3);
    assert(gb->wram[0][EVENT] == 0);
    /* No coordinates, VFX, status, or other work may precede allocation. */
    assert(memcmp(gb, &callback_entry, sizeof(*gb)) == 0);
    if (callback_mutates) {
        /* Adversarial dependency side effects: the continuation must consume
         * the NEW map, bank, saved status, and VFX availability. It must not
         * run the guard again or clear the callback's new event. */
        gb->wram_bank = 6;
        gb->rom_bank = 0x61;
        gb->wram[6][SAVED_BANK] = 0xB5;
        gb->wram[6][INDOOR] = 1;
        gb->hram[MAP_ID] = 0xFF;
        gb->hram[MAP_ROOM] = 0x1F;
        gb->wram[6][0xDDFF - 0xD000] = 0x42;
        gb->hram[ROOM_STATUS] = 0xFF;
        gb->wram[0][EVENT] = 0x69;
        gb->wram[0][EXECUTED] = 0;
        gb->hram[MP0] = 0xE2;
        gb->hram[MP1] = 0x19;
        gb->wram[0][VFX_TYPE + 15] = 0xA1;
        gb->wram[0][VFX_TYPE + 14] = 0;
        gb->wram[0][VFX_RING] = 0;
        gb->wram[0][POS_X + callback_de] = 0xE3;
        gb->wram[0][POS_Y + callback_de] = 0xE4;
        gb->wram[0][SLOW_COUNTDOWN + callback_de] = 0xE5;
    }
    return callback_de;
}

static void prepare_callback(const GBState *gb, uint16_t raw_de, int mutates) {
    memcpy(&callback_entry, gb, sizeof(callback_entry));
    callback_entry.wram[0][EVENT] = 0;
    callback_entry.rom_bank = 3;
    callback_calls = 0;
    callback_de = raw_de;
    callback_mutates = mutates;
}

static void test_handler_guards(const GBState *seed) {
    GBState gb, expected;
    unsigned accepted = 0;
    static const uint8_t events[] = {0, 1, 0xA0, 0xFF};
    static const uint8_t rep_executed[] = {
        0x00, 0x01, 0x02, 0x10, 0x7F, 0x80, 0xFE, 0xFF
    };
    static const uint8_t rep_rooms[] = {
        0x00, 0x01, 0x0E, 0x0F, 0x10, 0x11, 0x1F, 0x20, 0x40, 0x7F, 0x80, 0xEF, 0xF0, 0xFE, 0xFF
    };

    /* Sweep all 256 room values with representative executed bytes */
    for (unsigned room = 0; room < 256; ++room) {
        for (size_t e = 0; e < sizeof(rep_executed); ++e) {
            unsigned executed = rep_executed[e];
            int allowed = (room & 0x10) == 0 && executed != 0;
            for (unsigned fairy = 0; fairy < 2; ++fairy) {
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[ROOM_STATUS] = (uint8_t)room;
                gb.wram[0][EXECUTED] = (uint8_t)executed;
                gb.wram[0][EVENT] = events[(room + executed) % 4];
                memcpy(&expected, &gb, sizeof(expected));
                prepare_callback(&gb, 9, 0);
                if (allowed) {
                    expected.wram[0][EVENT] = 0;
                    expected.hram[MP0] = 0x88;
                    expected.hram[MP1] = fairy ? 0x30 : 0x20;
                    if (fairy) {
                        expect_fairy_writes(&expected, 9);
                        expected.rom_bank = 2;
                    }
                    expect_vfx(&expected, 15, fairy ? 2 : 4, 0x88,
                               fairy ? 0x30 : 0x20);
                    expect_status(&expected, 5, 0xD837);
                    ++accepted;
                }
                if (fairy) {
                    DropFairyEffectHandler(&gb, mock_spawn);
                } else {
                    RevealStaircaseEffectHandler(&gb);
                }
                assert(callback_calls == (unsigned)(fairy && allowed));
                assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
            }
        }
    }

    /* Sweep all 256 executed values with representative room bytes */
    for (unsigned executed = 0; executed < 256; ++executed) {
        for (size_t r = 0; r < sizeof(rep_rooms); ++r) {
            unsigned room = rep_rooms[r];
            int allowed = (room & 0x10) == 0 && executed != 0;
            for (unsigned fairy = 0; fairy < 2; ++fairy) {
                memcpy(&gb, seed, sizeof(gb));
                gb.hram[ROOM_STATUS] = (uint8_t)room;
                gb.wram[0][EXECUTED] = (uint8_t)executed;
                gb.wram[0][EVENT] = events[(room + executed) % 4];
                memcpy(&expected, &gb, sizeof(expected));
                prepare_callback(&gb, 9, 0);
                if (allowed) {
                    expected.wram[0][EVENT] = 0;
                    expected.hram[MP0] = 0x88;
                    expected.hram[MP1] = fairy ? 0x30 : 0x20;
                    if (fairy) {
                        expect_fairy_writes(&expected, 9);
                        expected.rom_bank = 2;
                    }
                    expect_vfx(&expected, 15, fairy ? 2 : 4, 0x88,
                               fairy ? 0x30 : 0x20);
                    expect_status(&expected, 5, 0xD837);
                    ++accepted;
                }
                if (fairy) {
                    DropFairyEffectHandler(&gb, mock_spawn);
                } else {
                    RevealStaircaseEffectHandler(&gb);
                }
                assert(callback_calls == (unsigned)(fairy && allowed));
                assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
            }
        }
    }
    assert(accepted > 0);
}

static void test_make_room_status(const GBState *seed) {
    static const struct {
        uint8_t indoor, map;
        uint16_t base;
    } maps[] = {
        {0, 0x00, 0xD800}, {0, 0x06, 0xD800}, {0, 0x19, 0xD800},
        {0, 0x1A, 0xD800}, {0, 0xFF, 0xD800},
        {1, 0x00, 0xD900}, {1, 0x05, 0xD900}, {1, 0x06, 0xDA00},
        {1, 0x19, 0xDA00}, {1, 0x1A, 0xD900}, {1, 0xFE, 0xD900},
        {1, 0xFF, 0xDDE0},
        {2, 0x05, 0xDA00}, {2, 0x06, 0xDB00}, {2, 0xFF, 0xDDE0}
    };
    static const uint8_t rooms[] = {0, 1, 0x1F, 0x20, 0x7F, 0xFF};
    static const uint8_t rep_saved[] = {
        0x00, 0x01, 0x0E, 0x0F, 0x10, 0x11, 0x1F, 0x20, 0x40, 0x7F, 0x80, 0xEF, 0xF0, 0xFE, 0xFF
    };
    GBState gb, expected;

    /* Grid across maps and rooms with representative saved status values */
    for (size_t map = 0; map < sizeof(maps) / sizeof(maps[0]); ++map) {
        for (size_t room = 0; room < sizeof(rooms); ++room) {
            for (size_t s = 0; s < sizeof(rep_saved); ++s) {
                unsigned saved = rep_saved[s];
                memcpy(&gb, seed, sizeof(gb));
                unsigned bank = 1u + saved % 7u;
                uint16_t address = (uint16_t)(maps[map].base + rooms[room]);
                gb.wram_bank = (uint8_t)bank;
                gb.wram[bank][INDOOR] = maps[map].indoor;
                gb.hram[MAP_ID] = maps[map].map;
                gb.hram[MAP_ROOM] = rooms[room];
                gb.wram[bank][address - 0xD000] = (uint8_t)saved;
                gb.hram[ROOM_STATUS] = (uint8_t)(saved ^ 0xFF);
                gb.wram[0][EXECUTED] = 0;
                gb.hram[MP0] = (uint8_t)saved;
                gb.hram[MP1] = (uint8_t)(255u - saved);
                memcpy(&expected, &gb, sizeof(expected));
                expect_vfx(&expected, 15, (uint8_t)saved,
                           (uint8_t)saved, (uint8_t)(255u - saved));
                expect_status(&expected, bank, address);
                MakeEffectObjectAppear(&gb, (uint8_t)saved);
                assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
            }
        }
    }

    /* Sweep all 256 saved values on a representative map/room */
    for (unsigned saved = 0; saved < 256; ++saved) {
        memcpy(&gb, seed, sizeof(gb));
        unsigned bank = 1u + saved % 7u;
        uint16_t address = 0xD800;
        gb.wram_bank = (uint8_t)bank;
        gb.wram[bank][INDOOR] = 0;
        gb.hram[MAP_ID] = 0;
        gb.hram[MAP_ROOM] = 0;
        gb.wram[bank][address - 0xD000] = (uint8_t)saved;
        gb.hram[ROOM_STATUS] = (uint8_t)(saved ^ 0xFF);
        gb.wram[0][EXECUTED] = 0;
        gb.hram[MP0] = (uint8_t)saved;
        gb.hram[MP1] = (uint8_t)(255u - saved);
        memcpy(&expected, &gb, sizeof(expected));
        expect_vfx(&expected, 15, (uint8_t)saved,
                   (uint8_t)saved, (uint8_t)(255u - saved));
        expect_status(&expected, bank, address);
        MakeEffectObjectAppear(&gb, (uint8_t)saved);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
    }
}

static void test_vfx_slots_and_raw_de(const GBState *seed) {
    GBState gb, expected;
    /* 0=Make, 1=Staircase, 2=Fairy. Each handler sees every sole free slot,
     * every highest-of-many free slot, and every full-ring cursor 0..15.
     * Fairy additionally crosses every case with all 16 slots and raw 00FF. */
    for (unsigned api = 0; api < 3; ++api) {
        for (unsigned spawn = 0; spawn < (api == 2 ? 17u : 1u); ++spawn) {
            uint16_t raw_de = spawn == 16 ? 0x00FF : (uint16_t)spawn;
            for (unsigned shape = 0; shape < 3; ++shape) {
                for (unsigned index = 0; index < 16; ++index) {
                    memcpy(&gb, seed, sizeof(gb));
                    memset(&gb.wram[0][VFX_TYPE], 0xA3, 16);
                    unsigned slot = index;
                    if (shape == 0) {
                        gb.wram[0][VFX_TYPE + index] = 0;
                    } else if (shape == 1) {
                        memset(&gb.wram[0][VFX_TYPE], 0, index + 1);
                    } else {
                        gb.wram[0][VFX_RING] = (uint8_t)index;
                        slot = index == 0 ? 15 : index - 1;
                    }
                    prepare_callback(&gb, raw_de, 0);
                    memcpy(&expected, &gb, sizeof(expected));
                    uint8_t type = 0xD6, x = 0x17, y = 0xE9;
                    if (api != 0) {
                        expected.wram[0][EVENT] = 0;
                        expected.hram[MP0] = x = 0x88;
                        expected.hram[MP1] = y = api == 2 ? 0x30 : 0x20;
                        type = api == 2 ? 2 : 4;
                    }
                    if (api == 2) {
                        expect_fairy_writes(&expected, raw_de);
                        expected.rom_bank = 2;
                    }
                    if (shape == 2) {
                        expected.wram[0][VFX_RING] = (uint8_t)slot;
                    }
                    /* raw FF writes C2FF/C30F/C54F. C54F aliases VFX Y[15],
                     * so allocating VFX slot 15 must overwrite that $80. */
                    expect_vfx(&expected, slot, type, x, y);
                    expect_status(&expected, 5, 0xD837);
                    if (api == 0) {
                        MakeEffectObjectAppear(&gb, type);
                    } else if (api == 1) {
                        RevealStaircaseEffectHandler(&gb);
                    } else {
                        DropFairyEffectHandler(&gb, mock_spawn);
                    }
                    assert(callback_calls == (unsigned)(api == 2));
                    assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
                }
            }
        }
    }

    /* Consecutive full allocations traverse the ring twice, including wrap.
     * MakeEffect still runs after it has set the cache's EVENT_1 bit. */
    memcpy(&gb, seed, sizeof(gb));
    memset(&gb.wram[0][VFX_TYPE], 0xA3, 16);
    gb.wram[0][VFX_RING] = 0;
    memcpy(&expected, &gb, sizeof(expected));
    for (unsigned call = 0; call < 32; ++call) {
        unsigned slot = 15u - call % 16u;
        expected.wram[0][VFX_RING] = (uint8_t)slot;
        expect_vfx(&expected, slot, 4, 0x17, 0xE9);
        expect_status(&expected, 5, 0xD837);
        MakeEffectObjectAppear(&gb, 4);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
    }
}

static void test_callback_continuation(const GBState *seed) {
    GBState gb, expected;
    for (unsigned spawn = 0; spawn < 17; ++spawn) {
        uint16_t raw_de = spawn == 16 ? 0x00FF : (uint16_t)spawn;
        memcpy(&gb, seed, sizeof(gb));
        prepare_callback(&gb, raw_de, 1);
        memcpy(&expected, &gb, sizeof(expected));
        /* Explicit external side-effect contract, independent of mock code. */
        expected.wram_bank = 6;
        expected.rom_bank = 0x35; /* Reload mutated DBAF=B5, gb.c masks to 7 bits. */
        expected.wram[6][SAVED_BANK] = 0xB5;
        expected.wram[6][INDOOR] = 1;
        expected.hram[MAP_ID] = 0xFF;
        expected.hram[MAP_ROOM] = 0x1F;
        expected.wram[6][0xDDFF - 0xD000] = 0x52;
        expected.hram[ROOM_STATUS] = 0x52;
        expected.wram[0][EVENT] = 0x69;
        expected.wram[0][EXECUTED] = 0;
        expected.wram[0][VFX_TYPE + 15] = 0xA1;
        expected.wram[0][VFX_RING] = 0;
        expect_fairy_writes(&expected, raw_de);
        expect_vfx(&expected, 14, 2, 0x88, 0x30);
        DropFairyEffectHandler(&gb, mock_spawn);
        assert(callback_calls == 1);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
    }
}

static void test_repeated_and_null(const GBState *seed) {
    GBState gb, expected;
    for (unsigned fairy = 0; fairy < 2; ++fairy) {
        memcpy(&gb, seed, sizeof(gb));
        prepare_callback(&gb, 0, 0);
        if (fairy) {
            DropFairyEffectHandler(&gb, mock_spawn);
        } else {
            RevealStaircaseEffectHandler(&gb);
        }
        assert((gb.hram[ROOM_STATUS] & 0x10) != 0);
        memcpy(&expected, &gb, sizeof(expected));
        callback_calls = 0;
        if (fairy) {
            DropFairyEffectHandler(&gb, mock_spawn);
        } else {
            RevealStaircaseEffectHandler(&gb);
        }
        assert(callback_calls == 0);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
    }
    /* NULL callback is a no-op BEFORE the guard, even if it would accept. */
    for (unsigned room = 0; room < 256; ++room) {
        for (unsigned executed = 0; executed < 2; ++executed) {
            memcpy(&gb, seed, sizeof(gb));
            gb.hram[ROOM_STATUS] = (uint8_t)room;
            gb.wram[0][EXECUTED] = (uint8_t)executed;
            memcpy(&expected, &gb, sizeof(expected));
            DropFairyEffectHandler(&gb, NULL);
            assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
        }
    }
    callback_calls = 0;
    MakeEffectObjectAppear(NULL, 0);
    MakeEffectObjectAppear(NULL, 0xFF);
    RevealStaircaseEffectHandler(NULL);
    DropFairyEffectHandler(NULL, mock_spawn);
    DropFairyEffectHandler(NULL, NULL);
    assert(callback_calls == 0);
}

void test_bank2_room_effect_appearance(void) {
    GBState seed;
    init_seed(&seed);
    /* Whole-state assertions also prohibit extra sound, item, staircase
     * activation, tilemap/OAM, entity status/type, or bank-selector writes.
     * Only appearance is scheduled here; no VFX update/activation is run. */
    test_handler_guards(&seed);
    test_make_room_status(&seed);
    test_vfx_slots_and_raw_de(&seed);
    test_callback_continuation(&seed);
    test_repeated_and_null(&seed);
}
