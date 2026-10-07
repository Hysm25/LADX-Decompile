#include <stdio.h>
#include <stdlib.h>

#include "bank1/test_bank1.h"
#include "bank2/test_bank2.h"
#include "bank3/test_bank3.h"

extern int run_clear_memory_tests(void);
extern int run_copy_data_tests(void);
extern int run_bank_tests(void);
extern void run_lcd_tests(void);
extern void run_entities_tests(void);
extern void run_audio_tests(void);
extern void run_dialog_tests(void);
extern void run_room_tests(void);
extern void run_gameplay_tests(void);
extern void run_link_tests(void);
extern void run_vfx_tests(void);
extern void run_ui_tests(void);
extern int run_gfx_tests(void);
extern void run_check_items_to_use_tests(void);
extern void run_animated_tiles_tests(void);


int main(void) {
    int total_failures = 0;

    printf("========================================\n");
    printf("   LADX Decompilation Verification Tests\n");
    printf("========================================\n\n");

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
    run_bank1_tests();
    run_bank2_tests();
    run_bank3_tests();

    printf("========================================\n");
    if (total_failures == 0) {
        printf("[PASS] ALL TESTS PASSED\n");
        printf("========================================\n");
        return 0;
    } else {
        printf("[FAIL] %d TEST(S) FAILED\n", total_failures);
        printf("========================================\n");
        return 1;
    }
}
