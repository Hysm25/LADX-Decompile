#include <stdio.h>
#include "test_home.h"

int run_home_tests(void) {
    int total_failures = 0;

    printf("[TEST] Bank 0 / Home Subsystems\n");
    total_failures += run_clear_memory_tests();
    total_failures += run_copy_data_tests();
    total_failures += run_bank_tests();
    run_lcd_tests();
    run_entities_tests();
    run_audio_tests();
    run_dialog_tests();
    run_room_tests();
    run_gameplay_tests();
    run_link_tests();
    run_vfx_tests();
    run_ui_tests();
    total_failures += run_gfx_tests();
    run_check_items_to_use_tests();
    run_animated_tiles_tests();

    if (total_failures == 0) {
        printf("[PASS] Bank 0 / Home Subsystems\n\n");
    } else {
        printf("[FAIL] %d Bank 0 / Home Subsystem test(s) failed\n\n", total_failures);
    }
    return total_failures;
}
