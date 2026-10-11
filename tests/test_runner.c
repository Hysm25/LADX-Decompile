#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_home.h"
#include "bank1/test_bank1.h"
#include "bank2/test_bank2.h"
#include "bank3/test_bank3.h"

static void print_usage(const char *prog_name) {
    printf("Usage: %s [target]\n", prog_name);
    printf("Available targets:\n");
    printf("  all    (default) Run full test suite (Home, Bank 1, Bank 2, Bank 3)\n");
    printf("  home   Run Bank 0 / Home tests only\n");
    printf("  bank1  Run Bank 1 tests only\n");
    printf("  bank2  Run Bank 2 tests only\n");
    printf("  bank3  Run Bank 3 tests only\n");
    printf("  -h, --help  Show this help message\n");
}

int main(int argc, char **argv) {
    const char *target = "all";
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        target = argv[1];
    }

    printf("========================================\n");
    printf("   LADX Decompilation Verification Tests\n");
    printf("========================================\n\n");

    int total_failures = 0;

    if (strcmp(target, "all") == 0) {
        total_failures += run_home_tests();
        run_bank1_tests();
        run_bank2_tests();
        run_bank3_tests();
    } else if (strcmp(target, "home") == 0 || strcmp(target, "bank0") == 0) {
        total_failures += run_home_tests();
    } else if (strcmp(target, "bank1") == 0) {
        run_bank1_tests();
    } else if (strcmp(target, "bank2") == 0) {
        run_bank2_tests();
    } else if (strcmp(target, "bank3") == 0) {
        run_bank3_tests();
    } else {
        fprintf(stderr, "Unknown test target: '%s'\n\n", target);
        print_usage(argv[0]);
        return 1;
    }

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
