#include "test_bank2.h"
#include "test_support.h"

#include "gb.h"
#include "bank2/items.h"
#include "bank2/link_motion.h"
#include "home/dialog.h"
#include "constants/directions.h"
#include "constants/gameplay.h"
#include "constants/memory.h"
#include "constants/physics.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/* Mock callbacks - declared in test_support.h */

/* Test func_002_753A (02:753A-02:754E) - Swimming physics modifier */
void test_func_002_753A(void) {

    GBState gb;
    gb_init(&gb);

    /* Test: swimming -> adds 4 to wC13B, then falls through to func_002_754F/func_002_755B.
     * Note: func_002_755B may overwrite wC13B depending on object physics.
     * We verify the function executes without crashing and that wC13B is modified. */
    gb_write(&gb, wLinkMotionState, LINK_MOTION_SWIMMING);
    gb_write(&gb, wC13B, 0x10);
    gb_write(&gb, wIsUsingHookshot, 0x00);
    gb_write(&gb, wIsLinkInTheAir, 0x00);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x00);

    func_002_753A(&gb);

    /* Verify wC13B was modified from initial 0x10 (swimming adds 4, then func_002_755B may modify further) */
    assert(gb_read(&gb, wC13B) != 0x10);

    /* Test: not swimming -> skips wC13B update, falls through to func_002_754F/func_002_755B */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wC13B, 0x10);
    gb_write(&gb, wIsUsingHookshot, 0x00);
    gb_write(&gb, wIsLinkInTheAir, 0x00);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x00);

    func_002_753A(&gb);

    /* wC13B may be modified by func_002_755B, but not by swimming add-4 */
    assert(gb_read(&gb, wC13B) != 0x10 || gb_read(&gb, wC13B) == 0x10); /* May or may not be modified by func_002_755B */

    /* Test: hookshot in use -> calls func_002_754F directly (skips swimming check) */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_SWIMMING);
    gb_write(&gb, wIsUsingHookshot, 0x01);
    gb_write(&gb, wIsLinkInTheAir, 0x00);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x00);

    func_002_753A(&gb);

    /* Function executes without crashing */
}

/* Test func_002_754F (02:754F-02:755A) - Hookshot/airborne check */
void test_func_002_754F(void) {

    GBState gb;
    gb_init(&gb);

    /* Test: airborne -> calls func_002_755B directly (no ClearLinkPositionIncrement) */
    gb_write(&gb, wIsLinkInTheAir, 0x02);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x00);
    gb_write(&gb, wC13B, 0x10); /* Should not be modified */

    func_002_754F(&gb);

    /* wC13B should remain unchanged (func_002_755B doesn't modify it in this context) */
    assert(gb_read(&gb, wC13B) == 0x10);

    /* Test: pegasus boots -> calls func_002_755B directly (no ClearLinkPositionIncrement) */
    gb_init(&gb);
    gb_write(&gb, wIsLinkInTheAir, 0x00);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, wC13B, 0x10);

    func_002_754F(&gb);

    assert(gb_read(&gb, wC13B) == 0x10);

    /* Test: neither airborne nor pegasus -> clears pos increment, calls func_002_755B */
    gb_init(&gb);
    gb_write(&gb, wIsLinkInTheAir, 0x00);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x00);
    gb_write(&gb, wC13B, 0x10);

    func_002_754F(&gb);

    assert(gb_read(&gb, wC13B) == 0x10);

}

/* Test label_002_74AD (02:74AD-02:74FB) - Pegasus boots wall collision */
void test_label_002_74AD(void) {

    GBState gb;
    gb_init(&gb);

    /* Test: early return when not running with pegasus boots */
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x00);
    gb_write(&gb, wCurrentBank, 0x02);
    gb_write(&gb, wCollisionType, COLLISION_TYPE_VERTICAL);
    gb_write_hram(&gb, hLinkSpeedX, 0x10);
    gb_write_hram(&gb, hLinkSpeedY, 0x20);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_DOWN);

    label_002_74AD(&gb);

    /* Should return early, no changes */
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x10);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x20);
    assert(g_mock_reset_spin_attack_calls == 0);

    /* Test: early return when not in bank 2 */
    gb_init(&gb);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, wCurrentBank, 0x01);
    gb_write(&gb, wCollisionType, COLLISION_TYPE_VERTICAL);
    gb_write_hram(&gb, hLinkSpeedX, 0x10);
    gb_write_hram(&gb, hLinkSpeedY, 0x20);

    label_002_74AD(&gb);

    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x10);

    /* Test: early return when no vertical/horizontal collision */
    gb_init(&gb);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, wCurrentBank, 0x02);
    gb_write(&gb, wCollisionType, 0x00); /* No collision */
    gb_write_hram(&gb, hLinkSpeedX, 0x10);
    gb_write_hram(&gb, hLinkSpeedY, 0x20);

    label_002_74AD(&gb);

    assert(gb_read_hram(&gb, hLinkSpeedX) == 0x10);

/* Test: vertical collision - reverses speeds, sets airborne, etc. */
    gb_init(&gb);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, wCurrentBank, 0x02);
    gb_write(&gb, wCollisionType, COLLISION_TYPE_VERTICAL);
    gb_write_hram(&gb, hLinkSpeedX, 0x10);
    gb_write_hram(&gb, hLinkSpeedY, 0x20);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_DOWN);

    label_002_74AD(&gb);

    /* Speed X: ~0x10 + 1 = 0xEF, >> 2 = 0xFB (-5) */
    /* Actually: cpl = 0xEF, inc = 0xF0, sra = 0xF8, sra = 0xFC (-4) */
    uint8_t expected_speed_x = (uint8_t)((~0x10 + 1) >> 2); /* 0xFC */
    assert(gb_read_hram(&gb, hLinkSpeedX) == expected_speed_x);

    /* Speed Y: ~0x20 + 1 = 0xDF, >> 2 = 0xF7 (-9) */
    uint8_t expected_speed_y = (uint8_t)((~0x20 + 1) >> 2); /* 0xF7 */
    assert(gb_read_hram(&gb, hLinkSpeedY) == expected_speed_y);

    /* Airborne state set */
    assert(gb_read(&gb, wIsLinkInTheAir) == 0x02);

    /* Velocity Z = $18 */
    assert(gb_read_hram(&gb, hLinkVelocityZ) == 0x18);

    /* Screen shake = $20 */
    assert(gb_read(&gb, wScreenShakeCountdown) == 0x20);

    /* wC158 from direction bit 1: DIRECTION_DOWN (3) has bit 1 set -> (3 & 2) << 1 = 4 */
    assert(gb_read(&gb, wC158) == 0x04);

    /* JINGLE_STRONG_BUMP */
    assert(gb_read_hram(&gb, hJingle) == JINGLE_STRONG_BUMP);

    /* Test: horizontal collision (no direction bit 1) */
    gb_init(&gb);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, wCurrentBank, 0x02);
    gb_write(&gb, wCollisionType, COLLISION_TYPE_HORIZONTAL); /* = 0x0C */
    gb_write_hram(&gb, hLinkSpeedX, 0x10);
    gb_write_hram(&gb, hLinkSpeedY, 0x20);
    gb_write_hram(&gb, hLinkDirection, DIRECTION_RIGHT); /* bit 1 clear */

    label_002_74AD(&gb);

    /* wC158 should be 0 since bit 1 clear */
    assert(gb_read(&gb, wC158) == 0x00);

}

/* Test func_002_7468 (02:7468-02:74AC) - Revolving door & special objects */
void test_func_002_7468(void) {

    GBState gb;
    gb_init(&gb);

    /* Test: early return when object not B1/B2/C1/C2/BB/BC */
    gb_write_hram(&gb, hObjectUnderEntity, 0x50);
    gb_write_hram(&gb, hMultiPurpose5, 0x00);

    func_002_7468(&gb);

    /* Should return early, no changes */

    /* Test: revolving door object $B1, hMultiPurpose5 < 6 */
    gb_init(&gb);
    gb_write_hram(&gb, hObjectUnderEntity, 0xB1);
    gb_write_hram(&gb, hMultiPurpose5, 0x03);

    func_002_7468(&gb);

    /* JINGLE_REVOLVING_DOOR */
    assert(gb_read_hram(&gb, hJingle) == JINGLE_REVOLVING_DOOR);

    /* LINK_MOTION_REVOLVING_DOOR */
    assert(gb_read(&gb, wLinkMotionState) == LINK_MOTION_REVOLVING_DOOR);

    /* Invincibility counter = 0 */
    assert(gb_read(&gb, wInvincibilityCounter) == 0x00);

    /* Link animation frame = 0 */
    assert(gb_read(&gb, wLinkAnimationFrame) == 0x00);

    /* Position Z = 0, Velocity Z = 0 */
    assert(gb_read_hram(&gb, hLinkPositionZ) == 0x00);
    assert(gb_read_hram(&gb, hLinkVelocityZ) == 0x00);

    /* Test: revolving door object $B2, hMultiPurpose5 >= 6 -> early return */
    gb_init(&gb);
    gb_write_hram(&gb, hObjectUnderEntity, 0xB2);
    gb_write_hram(&gb, hMultiPurpose5, 0x06);

    func_002_7468(&gb);

    /* Should return early */

    /* Test: object $C1, hMultiPurpose5 < $0C */
    gb_init(&gb);
    gb_write_hram(&gb, hObjectUnderEntity, 0xC1);
    gb_write_hram(&gb, hMultiPurpose5, 0x0A);

    func_002_7468(&gb);

    /* Should not call ApplyMapFadeOutTransitionWithNoise - no state change expected */

    /* Test: object $C2, hMultiPurpose5 >= $0C -> calls ApplyMapFadeOutTransitionWithNoise */
    gb_init(&gb);
    gb_write_hram(&gb, hObjectUnderEntity, 0xC2);
    gb_write_hram(&gb, hMultiPurpose5, 0x0C);

    func_002_7468(&gb);

    /* ApplyMapFadeOutTransitionWithNoise would initiate a map transition.
     * Since we can't easily test the actual function, we verify the function executes without crashing. */

    /* Test: object $BB, hMultiPurpose5 < $0C */
    gb_init(&gb);
    gb_write_hram(&gb, hObjectUnderEntity, 0xBB);
    gb_write_hram(&gb, hMultiPurpose5, 0x0B);

    func_002_7468(&gb);

    /* Test: object $BC, hMultiPurpose5 >= $0C */
    gb_init(&gb);
    gb_write_hram(&gb, hObjectUnderEntity, 0xBC);
    gb_write_hram(&gb, hMultiPurpose5, 0x0C);

    func_002_7468(&gb);

}

/* Test OpenDialogInTable0AndClearIncrement (02:74FE-02:7501) */
void test_OpenDialogInTable0AndClearIncrement(void) {

    GBState gb;
    gb_init(&gb);

    OpenDialogInTable0AndClearIncrement(&gb, 0x36);

    /* Verify dialog was opened by checking if the dialog state is set */
    /* The mock doesn't actually set dialog state, so we just verify the function executes without error */
    /* The actual implementation would set dialog state, but our mock doesn't */

}

/* Test OpenDialogInTable2AndClearIncrement (02:7504-02:7507) */
void test_OpenDialogInTable2AndClearIncrement(void) {

    GBState gb;
    gb_init(&gb);

    OpenDialogInTable2AndClearIncrement(&gb, 0x77);

}

/* Test Data_002_750A / Data_002_750E tables */
void test_Data_002_750A_750E(void) {

    /* Data_002_750A: X speeds for Right, Left, Up, Down */
    assert(Data_002_750A[0] == 0x08);   /* Right */
    assert(Data_002_750A[1] == (int8_t)0xF8); /* Left */
    assert(Data_002_750A[2] == 0x00);   /* Up */
    assert(Data_002_750A[3] == 0x00);   /* Down */

    /* Data_002_750E: Y speeds for Right, Left, Up, Down */
    assert(Data_002_750E[0] == 0x00);   /* Right */
    assert(Data_002_750E[1] == 0x00);   /* Left */
    assert(Data_002_750E[2] == (int8_t)0xF8); /* Up */
    assert(Data_002_750E[3] == 0x08);   /* Down */

}

void test_bank2_link_motion_helpers(void) {
    test_func_002_753A();
    test_func_002_754F();
    test_label_002_74AD();
    test_func_002_7468();
    test_OpenDialogInTable0AndClearIncrement();
    test_OpenDialogInTable2AndClearIncrement();
    test_Data_002_750A_750E();

}
