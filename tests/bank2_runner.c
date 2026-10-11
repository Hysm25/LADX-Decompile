#include <stdio.h>
#include "bank2/test_bank2.h"

int main(void) {
    printf("========================================\n");
    printf("   LADX Bank 2 Verification Tests\n");
    printf("========================================\n\n");

    run_bank2_tests();

    printf("========================================\n");
    printf("[PASS] ALL BANK 2 TESTS PASSED\n");
    printf("========================================\n");
    return 0;
}
