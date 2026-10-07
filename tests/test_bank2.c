#include <stdio.h>
#include <time.h>
#include "bank2/test_bank2.h"

#define RUN_TIMED_TEST(fn) \
    do { \
        printf("[RUN ] %s\n", #fn); \
        struct timespec start, end; \
        clock_gettime(CLOCK_MONOTONIC, &start); \
        fn(); \
        clock_gettime(CLOCK_MONOTONIC, &end); \
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; \
        printf("[PASS] %-36s (%6.3fs)\n", #fn, elapsed); \
    } while (0)

void run_bank2_tests(void) {
    printf("[TEST] Bank 2 Subsystems\n");

    RUN_TIMED_TEST(test_bank2_tables);
    RUN_TIMED_TEST(test_bank2_audio);
    RUN_TIMED_TEST(test_bank2_chest);
    RUN_TIMED_TEST(test_bank2_ocarina_use);
    RUN_TIMED_TEST(test_bank2_hookshot);
    RUN_TIMED_TEST(test_bank2_link_motion);
    RUN_TIMED_TEST(test_bank2_sword);
    RUN_TIMED_TEST(test_bank2_link_animation);
    RUN_TIMED_TEST(test_bank2_ocarina_playing);
    RUN_TIMED_TEST(test_bank2_shovel);
    RUN_TIMED_TEST(test_bank2_revolving_door);
    RUN_TIMED_TEST(test_bank2_swimming);
    RUN_TIMED_TEST(test_bank2_falling);
    RUN_TIMED_TEST(test_bank2_got_item);
    RUN_TIMED_TEST(test_bank2_recovery);
    RUN_TIMED_TEST(test_bank2_magic_rod);
    RUN_TIMED_TEST(test_bank2_room);
    RUN_TIMED_TEST(test_bank2_vfx);
    RUN_TIMED_TEST(test_bank2_room_events);
    RUN_TIMED_TEST(test_bank2_room_effects);
    RUN_TIMED_TEST(test_bank2_room_effect_appearance);
    RUN_TIMED_TEST(test_bank2_key_drop_effect);
    RUN_TIMED_TEST(test_bank2_object_reveal);
    RUN_TIMED_TEST(test_bank2_shutter_effects);
    RUN_TIMED_TEST(test_bank2_room_triggers);
    RUN_TIMED_TEST(test_bank2_room_dispatch);
    RUN_TIMED_TEST(test_bank2_clamp_item_count);
    RUN_TIMED_TEST(test_bank2_func_002_60E0);
    RUN_TIMED_TEST(test_bank2_room_transition);
    RUN_TIMED_TEST(test_bank2_link_motion_helpers);
    RUN_TIMED_TEST(test_bank2_link_ground_physics);

    printf("[PASS] Bank 2 Subsystems\n\n");
}
