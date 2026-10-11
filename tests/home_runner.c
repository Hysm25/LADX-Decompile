#include <stdio.h>
#include "test_home.h"

int main(void) {
    printf("========================================\n");
    printf("   LADX Bank 0 (Home) Verification Tests\n");
    printf("========================================\n\n");

    int failures = run_home_tests();

    printf("========================================\n");
    if (failures == 0) {
        printf("[PASS] ALL BANK 0 TESTS PASSED\n");
        printf("========================================\n");
        return 0;
    } else {
        printf("[FAIL] %d TEST(S) FAILED\n", failures);
        printf("========================================\n");
        return 1;
    }
}
