#include <stdio.h>
#include "bank3/test_bank3.h"

void run_bank3_tests(void) {
    printf("[TEST] Bank 3 Subsystems\n");

    test_bank3_entities();
    test_bank3_entities_physics();
    test_bank3_entities_collision();
    test_bank3_entities_droppable();
    test_bank3_entities_bomb();

    printf("[PASS] Bank 3 Subsystems\n\n");
}