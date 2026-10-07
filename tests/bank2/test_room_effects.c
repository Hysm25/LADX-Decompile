#include "test_bank2.h"

#include "gb.h"
#include "bank2/room_effects.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Independent oracle: LADX-Disassembly/src/code/events.asm,
 * KillAllEnemiesEffectHandler 02:5D79-5DAE and EventEffectGuard 02:5DAF-5DC1.
 * The guard's prose is misleading: JR NZ rejects room bit $10, and JR Z
 * rejects a zero executed byte. Neither path sets either of those bytes.
 * Offsets below come from constants/memory/{wram,hram}.asm and the encoded
 * instruction operands, not the C implementation or its address macros.
 */
enum {
    ROOM_EVENT = 0xC18E - 0xC000,
    EFFECT_EXECUTED = 0xC18F - 0xC000,
    ENTITY_STATUS = 0xC280 - 0xC000,
    ENTITY_PHYSICS = 0xC340 - 0xC000,
    ENTITY_COUNTDOWN3 = 0xC480 - 0xC000,
    NOISE_SFX = 0xFFF4 - 0xFF80,
    ROOM_STATUS = 0xFFF8 - 0xFF80,
    ENTITY_SLOTS = 16
};

static const uint8_t event_bytes[] = {0x00, 0x01, 0x40, 0xFF};

static void fill_sentinels(uint8_t *bytes, size_t size, unsigned salt) {
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = (uint8_t)(1u + (i * 37u + i / 251u + salt) % 255u);
    }
}

static void init_sentinels(GBState *gb) {
    static const uint8_t rom[] = {0x31, 0x73, 0xA5, 0xFE};

    /* Initialize padding too; use memcpy rather than struct assignment for
     * snapshots so whole-object comparisons include deterministic padding. */
    memset(gb, 0, sizeof(*gb));
    gb->rom = rom;
    gb->rom_size = sizeof(rom);
    for (size_t bank = 0; bank < 8; ++bank) {
        fill_sentinels(gb->wram[bank], sizeof(gb->wram[bank]), (unsigned)bank);
    }
    for (size_t bank = 0; bank < 2; ++bank) {
        fill_sentinels(gb->vram[bank], sizeof(gb->vram[bank]), (unsigned)bank + 19u);
    }
    for (size_t bank = 0; bank < 4; ++bank) {
        fill_sentinels(gb->sram[bank], sizeof(gb->sram[bank]), (unsigned)bank + 41u);
    }
    fill_sentinels(gb->oam, sizeof(gb->oam), 61);
    fill_sentinels(gb->io, sizeof(gb->io), 83);
    fill_sentinels(gb->hram, sizeof(gb->hram), 107);
    gb->ie = 0x1B;
    gb->rom_bank = 0x23;
    gb->wram_bank = 5;
    gb->vram_bank = 1;
    gb->sram_bank = 3;
    gb->sram_enabled = true;
    gb->joypad_input = 0xA6;

    /* If an off-by-one loop visits either adjacent pseudo-slot, it qualifies. */
    gb->wram[0][ENTITY_STATUS - 1] = 0xFF;
    gb->wram[0][ENTITY_STATUS + ENTITY_SLOTS] = 0xFF;
    gb->wram[0][ENTITY_PHYSICS - 1] = 0;
    gb->wram[0][ENTITY_PHYSICS + ENTITY_SLOTS] = 0;
    gb->wram[0][ENTITY_COUNTDOWN3 - 1] = 0xA7;
    gb->wram[0][ENTITY_COUNTDOWN3 + ENTITY_SLOTS] = 0xA7;
}

static void expect_explosion(GBState *expected, unsigned slot) {
    /* CP $05 is unsigned; AND $F0 preserves the upper physics nibble.
     * constants/entities.asm: ACTIVE=$05, HARMLESS=$80, MASK=$F0.
     * constants/sfx.asm: NOISE_SFX_ENEMY_DESTROYED=$13. */
    expected->wram[0][ENTITY_STATUS + slot] = 0x01;
    expected->wram[0][ENTITY_COUNTDOWN3 + slot] = 0x1F;
    expected->wram[0][ENTITY_PHYSICS + slot] =
        (uint8_t)((expected->wram[0][ENTITY_PHYSICS + slot] & 0xF0) | 0x02);
    expected->hram[NOISE_SFX] = 0x13;
}

static void test_guard_exhaustive(const GBState *seed) {
    GBState gb = *seed;
    GBState expected = *seed;
    unsigned calls = 0;
    unsigned accepted = 0;

    static const uint8_t rep_rooms[] = {
        0x00, 0x01, 0x0E, 0x0F, 0x10, 0x11, 0x1F, 0x20, 0x40, 0x7F, 0x80, 0xEF, 0xF0, 0xFE, 0xFF
    };
    static const uint8_t rep_executed[] = {
        0x00, 0x01, 0x02, 0x0F, 0x10, 0x7F, 0x80, 0xFE, 0xFF
    };

    for (size_t event = 0; event < sizeof(event_bytes); ++event) {
        /* Sweep all 256 room values with representative executed bytes */
        for (unsigned room = 0; room < 256; ++room) {
            for (size_t e = 0; e < sizeof(rep_executed); ++e) {
                unsigned executed = rep_executed[e];
                gb.hram[ROOM_STATUS] = (uint8_t)room;
                gb.wram[0][EFFECT_EXECUTED] = (uint8_t)executed;
                gb.wram[0][ROOM_EVENT] = event_bytes[event];
                expected.hram[ROOM_STATUS] = (uint8_t)room;
                expected.wram[0][EFFECT_EXECUTED] = (uint8_t)executed;
                const bool should_run = (room & 0x10) == 0 && executed != 0;
                expected.wram[0][ROOM_EVENT] = should_run ? 0 : event_bytes[event];
                if (should_run) {
                    ++accepted;
                }

                const bool result = EventEffectGuard(&gb);
                assert(result == should_run);
                assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
                ++calls;
            }
        }

        /* Sweep all 256 executed values with representative room bytes */
        for (unsigned executed = 0; executed < 256; ++executed) {
            for (size_t r = 0; r < sizeof(rep_rooms); ++r) {
                unsigned room = rep_rooms[r];
                gb.hram[ROOM_STATUS] = (uint8_t)room;
                gb.wram[0][EFFECT_EXECUTED] = (uint8_t)executed;
                gb.wram[0][ROOM_EVENT] = event_bytes[event];
                expected.hram[ROOM_STATUS] = (uint8_t)room;
                expected.wram[0][EFFECT_EXECUTED] = (uint8_t)executed;
                const bool should_run = (room & 0x10) == 0 && executed != 0;
                expected.wram[0][ROOM_EVENT] = should_run ? 0 : event_bytes[event];
                if (should_run) {
                    ++accepted;
                }

                const bool result = EventEffectGuard(&gb);
                assert(result == should_run);
                assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
                ++calls;
            }
        }
    }
    assert(calls > 0);
    assert(accepted > 0);
}

static void test_kill_exhaustive(const GBState *seed) {
    GBState gb, expected;
    unsigned combinations = 0;
    unsigned explosions = 0;

    /* Sample batches across the space covering all status/physics pairs. */
    for (unsigned b = 0; b < 256; ++b) {
        unsigned batch = (b * 16u + (b % 17u)) % 4096u;
        memcpy(&gb, seed, sizeof(gb));
        gb.hram[ROOM_STATUS] = (uint8_t)((batch >> 4) & 0xEF);
        gb.wram[0][EFFECT_EXECUTED] = (uint8_t)(1u + batch % 255u);
        gb.wram[0][ROOM_EVENT] = event_bytes[batch % sizeof(event_bytes)];
        gb.hram[NOISE_SFX] = 0xA9;
        for (unsigned pair = 0; pair < ENTITY_SLOTS; ++pair) {
            const unsigned value = batch * ENTITY_SLOTS + pair;
            const unsigned slot = (pair + batch) % ENTITY_SLOTS;
            gb.wram[0][ENTITY_STATUS + slot] = (uint8_t)(value >> 8);
            gb.wram[0][ENTITY_PHYSICS + slot] = (uint8_t)value;
            gb.wram[0][ENTITY_COUNTDOWN3 + slot] = (uint8_t)(0x80u + slot);
        }
        memcpy(&expected, &gb, sizeof(expected));
        expected.wram[0][ROOM_EVENT] = 0;
        for (int slot = ENTITY_SLOTS - 1; slot >= 0; --slot) {
            const uint8_t status = gb.wram[0][ENTITY_STATUS + slot];
            const uint8_t flags = gb.wram[0][ENTITY_PHYSICS + slot];
            if (flags < 0x80 && status >= 5) {
                expect_explosion(&expected, (unsigned)slot);
                ++explosions;
            }
            ++combinations;
        }

        KillAllEnemiesEffectHandler(&gb);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
    }
    assert(combinations == 256u * ENTITY_SLOTS);
    assert(explosions > 0);
}

static void test_kill_guard_rejection(const GBState *seed) {
    GBState gb, expected;
    unsigned calls = 0;
    static const uint8_t reject_rooms[] = {
        0x10, 0x11, 0x1F, 0x30, 0x50, 0x70, 0x90, 0xB0, 0xD0, 0xF0, 0xFF
    };

    /* All 256 rooms with executed=0 */
    for (unsigned room = 0; room < 256; ++room) {
        memcpy(&gb, seed, sizeof(gb));
        gb.hram[ROOM_STATUS] = (uint8_t)room;
        gb.wram[0][EFFECT_EXECUTED] = 0;
        gb.wram[0][ROOM_EVENT] = event_bytes[calls % sizeof(event_bytes)];
        gb.hram[NOISE_SFX] = 0xA9;
        for (unsigned slot = 0; slot < ENTITY_SLOTS; ++slot) {
            gb.wram[0][ENTITY_STATUS + slot] = (uint8_t)(0xF0u + slot);
            gb.wram[0][ENTITY_PHYSICS + slot] = (uint8_t)(0x70u + slot);
        }
        memcpy(&expected, &gb, sizeof(expected));
        KillAllEnemiesEffectHandler(&gb);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
        ++calls;
    }

    /* All 256 executed values with bit-4 set rooms */
    for (size_t r = 0; r < sizeof(reject_rooms); ++r) {
        for (unsigned executed = 0; executed < 256; ++executed) {
            memcpy(&gb, seed, sizeof(gb));
            gb.hram[ROOM_STATUS] = reject_rooms[r];
            gb.wram[0][EFFECT_EXECUTED] = (uint8_t)executed;
            gb.wram[0][ROOM_EVENT] = event_bytes[calls % sizeof(event_bytes)];
            gb.hram[NOISE_SFX] = 0xA9;
            for (unsigned slot = 0; slot < ENTITY_SLOTS; ++slot) {
                gb.wram[0][ENTITY_STATUS + slot] = (uint8_t)(0xF0u + slot);
                gb.wram[0][ENTITY_PHYSICS + slot] = (uint8_t)(0x70u + slot);
            }
            memcpy(&expected, &gb, sizeof(expected));
            KillAllEnemiesEffectHandler(&gb);
            assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
            ++calls;
        }
    }
    assert(calls == 256u + sizeof(reject_rooms) * 256u);
}

static void test_kill_single_slots(const GBState *seed) {
    GBState gb, expected;

    /* Each slot, especially 0 and 15, must work as the sole eligible entity. */
    for (unsigned active = 0; active < ENTITY_SLOTS; ++active) {
        memcpy(&gb, seed, sizeof(gb));
        gb.hram[ROOM_STATUS] = 0xEF;
        gb.wram[0][EFFECT_EXECUTED] = 0x80;
        gb.wram[0][ROOM_EVENT] = 0;
        gb.hram[NOISE_SFX] = 0xA9;
        for (unsigned slot = 0; slot < ENTITY_SLOTS; ++slot) {
            gb.wram[0][ENTITY_STATUS + slot] = 4;
            gb.wram[0][ENTITY_PHYSICS + slot] = 0x7F;
        }
        gb.wram[0][ENTITY_STATUS + active] = 5;
        memcpy(&expected, &gb, sizeof(expected));
        expect_explosion(&expected, active);
        KillAllEnemiesEffectHandler(&gb);
        assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
    }
}

static void test_kill_mixed_and_repeated(const GBState *seed) {
    static const uint8_t statuses[ENTITY_SLOTS] = {
        5, 4, 0, 1, 2, 3, 5, 6, 0x7F, 0x80, 0xFE, 0xFF, 5, 0xFF, 4, 0xFF
    };
    static const uint8_t flags[ENTITY_SLOTS] = {
        0x7F, 0x00, 0x70, 0x01, 0x12, 0x23, 0x80, 0x0F,
        0x10, 0x21, 0x3E, 0x40, 0xFF, 0x81, 0x7F, 0x6D
    };
    static const unsigned killed[] = {0, 7, 8, 9, 10, 11, 15};
    GBState gb, expected;

    memcpy(&gb, seed, sizeof(gb));
    gb.hram[ROOM_STATUS] = 0xEF;
    gb.wram[0][EFFECT_EXECUTED] = 0xFF;
    gb.wram[0][ROOM_EVENT] = 0x40;
    gb.hram[NOISE_SFX] = 0xA9;
    memcpy(&gb.wram[0][ENTITY_STATUS], statuses, sizeof(statuses));
    memcpy(&gb.wram[0][ENTITY_PHYSICS], flags, sizeof(flags));
    for (unsigned slot = 0; slot < ENTITY_SLOTS; ++slot) {
        gb.wram[0][ENTITY_COUNTDOWN3 + slot] = (uint8_t)(0xB0u + slot);
    }
    memcpy(&expected, &gb, sizeof(expected));
    expected.wram[0][ROOM_EVENT] = 0;
    for (size_t i = 0; i < sizeof(killed) / sizeof(killed[0]); ++i) {
        expect_explosion(&expected, killed[i]);
    }
    KillAllEnemiesEffectHandler(&gb);
    assert(memcmp(&gb, &expected, sizeof(gb)) == 0);

    /* Already exploded statuses are now 1; no timer reset or noise request. */
    gb.hram[NOISE_SFX] = 0xA9;
    for (size_t i = 0; i < sizeof(killed) / sizeof(killed[0]); ++i) {
        gb.wram[0][ENTITY_COUNTDOWN3 + killed[i]] = (uint8_t)i;
    }
    memcpy(&expected, &gb, sizeof(expected));
    KillAllEnemiesEffectHandler(&gb);
    assert(memcmp(&gb, &expected, sizeof(gb)) == 0);

    /* No global executed/status latch was set. Even with event still zero,
     * newly qualifying unsigned high statuses must explode on another call. */
    gb.wram[0][ENTITY_STATUS] = 0x80;
    gb.wram[0][ENTITY_STATUS + 15] = 0xFF;
    gb.wram[0][ENTITY_PHYSICS + 6] = 0x70; /* Previously harmless status 5. */
    memcpy(&expected, &gb, sizeof(expected));
    expect_explosion(&expected, 0);
    expect_explosion(&expected, 6);
    expect_explosion(&expected, 15);
    KillAllEnemiesEffectHandler(&gb);
    assert(memcmp(&gb, &expected, sizeof(gb)) == 0);
}

void test_bank2_room_effects(void) {
    GBState seed;
    init_sentinels(&seed);
    test_guard_exhaustive(&seed);
    test_kill_exhaustive(&seed);
    test_kill_guard_rejection(&seed);
    test_kill_single_slots(&seed);
    test_kill_mixed_and_repeated(&seed);

    /* C API contract; NULL has no corresponding assembly input. */
    const bool result = EventEffectGuard(NULL);
    assert(result == false);
    KillAllEnemiesEffectHandler(NULL);
}
