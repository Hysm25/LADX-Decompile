#include "home/lcd.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include <stdio.h>
#include <assert.h>

void test_lcd_off(void) {
    GBState gb;
    gb_init(&gb);

    /* Setup initial state: all interrupts enabled, LCD enabled */
    gb_write(&gb, rIE, 0x1F);
    gb_write(&gb, rLCDC, 0x93);
    gb_write(&gb, rLY, 50);
    gb_write(&gb, hInterrupts, 0x00);

    LCDOff(&gb);

    /* Verify hInterrupts has original rIE value */
    assert(gb_read(&gb, hInterrupts) == 0x1F);

    /* Verify rLY was set to SCRN_Y + 1 (145) */
    assert(gb_read(&gb, rLY) == SCRN_Y + 1);

    /* Verify rLCDC bit 7 is disabled (0x93 & ~0x80 == 0x13) */
    assert(gb_read(&gb, rLCDC) == 0x13);

    /* Verify rIE was restored back to 0x1F */
    assert(gb_read(&gb, rIE) == 0x1F);
}

void run_lcd_tests(void) {
    printf("[TEST] LCD\n");
    printf("[RUN ] LCDOff\n");
    test_lcd_off();
    printf("[PASS] LCDOff\n");
    printf("[PASS] LCD\n\n");
}
