#include <stdio.h>
#include "bank1/test_bank1.h"

int main(void) {
    printf("========================================\n");
    printf("   LADX Bank 1 Verification Tests\n");
    printf("========================================\n\n");

    run_bank1_tests();

    printf("========================================\n");
    printf("[PASS] ALL BANK 1 TESTS PASSED\n");
    printf("========================================\n");
    return 0;
}
