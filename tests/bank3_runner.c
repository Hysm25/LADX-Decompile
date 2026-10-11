#include <stdio.h>
#include "bank3/test_bank3.h"

int main(void) {
    printf("========================================\n");
    printf("   LADX Bank 3 Verification Tests\n");
    printf("========================================\n\n");

    run_bank3_tests();

    printf("========================================\n");
    printf("[PASS] ALL BANK 3 TESTS PASSED\n");
    printf("========================================\n");
    return 0;
}
