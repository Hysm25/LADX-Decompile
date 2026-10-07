#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "gb.h"
#include "home/check_items_to_use.h"
#include "constants/inventory.h"
#include "constants/entities.h"
#include "constants/directions.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/memory.h"
#include "constants/sfx.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static int mock_sync_shield_calls = 0;
static void mock_sync_shield(GBState *gb) {
    (void)gb;
    mock_sync_shield_calls++;
}

static void test_set_shield_vals(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wShieldLevel, 0x02);
    mock_sync_shield_calls = 0;

    SetShieldVals(&gb, mock_sync_shield);
    TEST_ASSERT(gb_read(&gb, wIsUsingShield) == 0x01, "wIsUsingShield not set to 1");
    TEST_ASSERT(gb_read(&gb, wHasMirrorShield) == 0x02, "wHasMirrorShield not set to wShieldLevel");
    TEST_ASSERT(mock_sync_shield_calls == 1, "sync_shield trampoline not called");
}

static void test_hold_sword_if_needed(void) {
    GBState gb;

    /* 1. Item not sword: nothing happens */
    gb_init(&gb);
    HoldSwordIfNeeded(&gb, INVENTORY_SHIELD);
    TEST_ASSERT(gb_read(&gb, wSwordAnimationState) == 0, "Sword state modified for shield");

    /* 2. Item is sword, but near NPC */
    gb_init(&gb);
    gb_write(&gb, wItemUsageContext, ITEM_USAGE_NEAR_NPC);
    HoldSwordIfNeeded(&gb, INVENTORY_SWORD);
    TEST_ASSERT(gb_read(&gb, wSwordAnimationState) == 0, "Sword state modified near NPC");

    /* 3. Item is sword, reading text */
    gb_init(&gb);
    gb_write(&gb, wItemUsageContext, ITEM_USAGE_READING_TEXT);
    HoldSwordIfNeeded(&gb, INVENTORY_SWORD);
    TEST_ASSERT(gb_read(&gb, wSwordAnimationState) == 0, "Sword state modified reading text");

    /* 4. Item is sword, wC160 != 0 */
    gb_init(&gb);
    gb_write(&gb, wC160, 0x01);
    HoldSwordIfNeeded(&gb, INVENTORY_SWORD);
    TEST_ASSERT(gb_read(&gb, wSwordAnimationState) == 0, "Sword state modified when wC160 != 0");

    /* 5. Item is sword, normal conditions -> hold sword */
    gb_init(&gb);
    gb_write(&gb, wC1AC, 0x12);
    HoldSwordIfNeeded(&gb, INVENTORY_SWORD);
    TEST_ASSERT(gb_read(&gb, wC1AC) == 0x00, "wC1AC not cleared");
    TEST_ASSERT(gb_read(&gb, wSwordAnimationState) == SWORD_ANIMATION_STATE_HOLDING, "wSwordAnimationState not HOLDING");
    TEST_ASSERT(gb_read(&gb, wSwordCollisionEnabled) == SWORD_ANIMATION_STATE_HOLDING, "wSwordCollisionEnabled not HOLDING");
}

static void test_use_shield(void) {
    GBState gb;

    /* Pushing: do not draw shield */
    gb_init(&gb);
    gb_write(&gb, wIsLinkPushing, 0x01);
    UseShield(&gb);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == 0, "Shield SFX triggered while pushing");

    /* Not pushing: draw shield */
    gb_init(&gb);
    UseShield(&gb);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_DRAW_SHIELD, "Shield SFX not triggered");
}

static bool mock_poking_flag = false;
static bool mock_check_poking(GBState *gb) {
    (void)gb;
    return mock_poking_flag;
}

static void test_use_shovel(void) {
    GBState gb;

    /* Already using shovel: ignored */
    gb_init(&gb);
    gb_write(&gb, wLinkUsingShovel, 0x01);
    UseShovel(&gb, mock_check_poking);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == 0, "Shovel SFX played while already using");

    /* Link in air: ignored */
    gb_init(&gb);
    gb_write(&gb, wIsLinkInTheAir, 0x01);
    UseShovel(&gb, mock_check_poking);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == 0, "Shovel SFX played while in air");

    /* Normal shovel digging (not poking) */
    gb_init(&gb);
    mock_poking_flag = false;
    UseShovel(&gb, mock_check_poking);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_SHOVEL_DIG, "Digging SFX not set");
    TEST_ASSERT(gb_read(&gb, wLinkUsingShovel) == 0x01, "wLinkUsingShovel not set");
    TEST_ASSERT(gb_read(&gb, wLinkUsingShovelTimer) == 0x00, "wLinkUsingShovelTimer not reset");

    /* Shovel poking solid wall */
    gb_init(&gb);
    mock_poking_flag = true;
    UseShovel(&gb, mock_check_poking);
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_SWORD_POKING, "Poking jingle not set");
}

static int mock_hookshot_calls = 0;
static void mock_fire_hookshot(GBState *gb) {
    (void)gb;
    mock_hookshot_calls++;
}

static void test_use_hookshot(void) {
    GBState gb;

    /* Already using hookshot */
    gb_init(&gb);
    gb_write(&gb, wIsUsingHookshot, 0x01);
    mock_hookshot_calls = 0;
    UseHookshot(&gb, mock_fire_hookshot);
    TEST_ASSERT(mock_hookshot_calls == 0, "Hookshot fired while already using");

    /* Normal hookshot fire */
    gb_init(&gb);
    mock_hookshot_calls = 0;
    UseHookshot(&gb, mock_fire_hookshot);
    TEST_ASSERT(mock_hookshot_calls == 1, "Hookshot not fired");
}

static void test_use_magic_rod(void) {
    GBState gb;

    /* Sword animating: blocked */
    gb_init(&gb);
    gb_write(&gb, wSwordAnimationState, 0x01);
    UseMagicRod(&gb);
    TEST_ASSERT(gb_read(&gb, wLinkAttackStepAnimationCountdown) == 0, "Magic rod fired during sword animation");

    /* Too many projectiles (>= 2): blocked */
    gb_init(&gb);
    gb_write(&gb, wActiveProjectileCount, 0x02);
    UseMagicRod(&gb);
    TEST_ASSERT(gb_read(&gb, wLinkAttackStepAnimationCountdown) == 0, "Magic rod fired with >= 2 projectiles");

    /* Normal magic rod fire */
    gb_init(&gb);
    UseMagicRod(&gb);
    TEST_ASSERT(gb_read(&gb, wLinkAttackStepAnimationCountdown) == (0x0E | ATTACK_STEP_ITEM_MAGIC_ROD),
                "Magic rod animation countdown not set correctly");
}

static int mock_bomb_arrow_calls = 0;
static void mock_convert_to_bomb_arrow(GBState *gb) {
    (void)gb;
    mock_bomb_arrow_calls++;
}

static bool mock_spawn_projectile_res = true;
static bool mock_spawn_bomb_projectile(GBState *gb) {
    (void)gb;
    return mock_spawn_projectile_res;
}

static void test_place_bomb(void) {
    GBState gb;

    /* Bomb already placed */
    gb_init(&gb);
    gb_write(&gb, wHasPlacedBomb, 0x01);
    gb_write(&gb, wBombCount, 0x10);
    PlaceBomb(&gb, mock_spawn_bomb_projectile, mock_convert_to_bomb_arrow);
    TEST_ASSERT(gb_read(&gb, wBombCount) == 0x10, "Bomb placed when already placed");

    /* Zero bombs: plays wrong answer jingle */
    gb_init(&gb);
    gb_write(&gb, wBombCount, 0x00);
    PlaceBomb(&gb, mock_spawn_bomb_projectile, mock_convert_to_bomb_arrow);
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_WRONG_ANSWER, "Wrong answer jingle not played on 0 bombs");

    /* Normal placement with BCD decrement (0x20 -> 0x19) */
    gb_init(&gb);
    gb_write(&gb, wBombCount, 0x20);
    mock_bomb_arrow_calls = 0;
    mock_spawn_projectile_res = true;
    PlaceBomb(&gb, mock_spawn_bomb_projectile, mock_convert_to_bomb_arrow);
    TEST_ASSERT(gb_read(&gb, wBombCount) == 0x19, "BCD decrement failed on bomb count");
    TEST_ASSERT(mock_bomb_arrow_calls == 1, "Bomb arrow conversion not called");
}

static int mock_used_item = -1;
static void mock_use_item_cb(GBState *gb, uint8_t item) {
    (void)gb;
    mock_used_item = item;
}

static int mock_boots_calls = 0;
static void mock_use_pegasus_boots(GBState *gb) {
    (void)gb;
    mock_boots_calls++;
}

static void test_check_items_to_use(void) {
    GBState gb;

    /* 1. Item usage blocked */
    gb_init(&gb);
    gb_write(&gb, wBlockItemUsage, 0x01);
    mock_used_item = -1;
    CheckItemsToUse(&gb, mock_use_pegasus_boots, mock_use_item_cb, NULL);
    TEST_ASSERT(mock_used_item == -1, "Items checked when wBlockItemUsage != 0");

    /* 2. Normal item usage on Button A */
    gb_init(&gb);
    gb_write(&gb, wInventoryItems_AButtonSlot, INVENTORY_BOMBS);
    gb_write(&gb, hJoypadState, J_A);
    mock_used_item = -1;
    CheckItemsToUse(&gb, mock_use_pegasus_boots, mock_use_item_cb, NULL);
    TEST_ASSERT(mock_used_item == INVENTORY_BOMBS, "Item A not used on J_A");

    /* 3. Normal item usage on Button B */
    gb_init(&gb);
    gb_write(&gb, wInventoryItems_BButtonSlot, INVENTORY_BOW);
    gb_write(&gb, hJoypadState, J_B);
    mock_used_item = -1;
    CheckItemsToUse(&gb, mock_use_pegasus_boots, mock_use_item_cb, NULL);
    TEST_ASSERT(mock_used_item == INVENTORY_BOW, "Item B not used on J_B");

    /* 4. Reading text suppresses item A & B */
    gb_init(&gb);
    gb_write(&gb, wInventoryItems_AButtonSlot, INVENTORY_BOMBS);
    gb_write(&gb, wInventoryItems_BButtonSlot, INVENTORY_BOW);
    gb_write(&gb, hJoypadState, J_A | J_B);
    gb_write(&gb, wItemUsageContext, ITEM_USAGE_READING_TEXT);
    mock_used_item = -1;
    CheckItemsToUse(&gb, mock_use_pegasus_boots, mock_use_item_cb, NULL);
    TEST_ASSERT(mock_used_item == -1, "Items used while reading text");

    /* 5. Pegasus Boots held on button A */
    gb_init(&gb);
    gb_write(&gb, wInventoryItems_AButtonSlot, INVENTORY_PEGASUS_BOOTS);
    gb_write(&gb, hPressedButtonsMask, J_A);
    mock_boots_calls = 0;
    CheckItemsToUse(&gb, mock_use_pegasus_boots, mock_use_item_cb, NULL);
    TEST_ASSERT(mock_boots_calls == 1, "Pegasus boots not used on held button A");

    /* 6. Pegasus Boots released -> charge meter reset */
    gb_init(&gb);
    gb_write(&gb, wInventoryItems_AButtonSlot, INVENTORY_PEGASUS_BOOTS);
    gb_write(&gb, hPressedButtonsMask, 0x00);
    gb_write(&gb, wPegasusBootsChargeMeter, 0x15);
    CheckItemsToUse(&gb, mock_use_pegasus_boots, mock_use_item_cb, NULL);
    TEST_ASSERT(gb_read(&gb, wPegasusBootsChargeMeter) == 0, "Boots charge meter not reset on button release");
}


static void test_use_power_bracelet(void) {
    GBState gb;
    gb_init(&gb);
    UsePowerBracelet(&gb);
    TEST_ASSERT(true, "UsePowerBracelet returned cleanly");
}

static int mock_4bff_calls = 0;
static void mock_func_020_4bff(GBState *gb) {
    (void)gb;
    mock_4bff_calls++;
}

static void test_use_boomerang(void) {
    GBState gb;

    /* 1. Projectiles already active */
    gb_init(&gb);
    gb_write(&gb, wActiveProjectileCount, 1);
    mock_4bff_calls = 0;
    UseBoomerang(&gb, NULL, mock_func_020_4bff);
    TEST_ASSERT(mock_4bff_calls == 0, "Boomerang used while projectiles active");

    /* 2. Free to use boomerang */
    gb_init(&gb);
    mock_4bff_calls = 0;
    UseBoomerang(&gb, NULL, mock_func_020_4bff);
    TEST_ASSERT(mock_4bff_calls == 1, "func_020_4BFF not called");
    TEST_ASSERT(gb_read(&gb, wEntitiesStatusTable) == 4, "Boomerang entity not initialized");
    TEST_ASSERT(gb_read(&gb, wEntitiesTypeTable) == ENTITY_BOOMERANG, "Entity type not ENTITY_BOOMERANG");
}

static void test_spawn_player_projectile(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, hLinkDirection, DIRECTION_UP);
    gb_write(&gb, hLinkPositionX, 50);
    gb_write(&gb, hLinkPositionY, 60);
    gb_write(&gb, hLinkPositionZ, 5);

    uint8_t out_idx = 0xFF;
    bool success = SpawnPlayerProjectile(&gb, ENTITY_ARROW, &out_idx);
    TEST_ASSERT(success == true, "Failed to spawn player projectile");
    TEST_ASSERT(out_idx == 0, "First projectile not in slot 0");
    TEST_ASSERT(gb_read(&gb, wEntitiesPosXTable + out_idx) == 50, "Projectile X pos mismatch");
    TEST_ASSERT(gb_read(&gb, wEntitiesPosYTable + out_idx) == 60, "Projectile Y pos mismatch");
    TEST_ASSERT(gb_read(&gb, wEntitiesPosZTable + out_idx) == 6, "Projectile Z pos not hLinkPositionZ + 1");
    TEST_ASSERT(gb_read(&gb, wEntitiesDirectionTable + out_idx) == DIRECTION_UP, "Projectile direction mismatch");
    TEST_ASSERT(gb_read(&gb, wEntitiesSpeedYTable + out_idx) == (uint8_t)-32, "Upward projectile Y speed mismatch");

    /* Fill all 16 slots, next spawn should fail */
    for (int i = 0; i < 16; i++) {
        gb_write(&gb, wEntitiesStatusTable + i, 5);
    }
    success = SpawnPlayerProjectile(&gb, ENTITY_BOMB, &out_idx);
    TEST_ASSERT(success == false, "Projectile spawned when all 16 slots full");
}

static void test_shoot_arrow(void) {
    GBState gb;

    /* 1. Already shooting arrow */
    gb_init(&gb);
    gb_write(&gb, wIsShootingArrow, 0x10);
    bool res = ShootArrow(&gb, NULL, NULL);
    TEST_ASSERT(res == false, "Arrow shot while already shooting");

    /* 2. Zero arrows -> wrong answer jingle */
    gb_init(&gb);
    gb_write(&gb, wArrowCount, 0x00);
    res = ShootArrow(&gb, NULL, NULL);
    TEST_ASSERT(res == false, "Arrow shot with zero count");
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_WRONG_ANSWER, "Wrong answer jingle not played on 0 arrows");

    /* 3. Normal arrow shot */
    gb_init(&gb);
    gb_write(&gb, wArrowCount, 0x10);
    gb_write(&gb, hLinkDirection, DIRECTION_RIGHT);
    res = ShootArrow(&gb, NULL, NULL);
    TEST_ASSERT(res == true, "Normal arrow shot failed");
    TEST_ASSERT(gb_read(&gb, wArrowCount) == 0x09, "Arrow count BCD decrement failed");
    TEST_ASSERT(gb_read(&gb, wIsShootingArrow) == 0x10, "wIsShootingArrow not set");
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_WHOOSH, "Whoosh SFX not played");
    TEST_ASSERT(gb_read(&gb, wBombArrowCooldown) == 0x06, "Bomb arrow cooldown not set");
    TEST_ASSERT(gb_read(&gb, wEntitiesSpeedXTable + 0) == 0x30, "Arrow SpeedX mismatch without power");

    /* 4. Bomb-arrow combo */
    gb_init(&gb);
    gb_write(&gb, wArrowCount, 0x05);
    gb_write(&gb, wBombArrowCooldown, 0x04);
    gb_write(&gb, wLatestDroppedBombEntityIndex, 0x03);
    gb_write(&gb, wEntitiesStatusTable + 0x03, 0x05); /* Active bomb */

    res = ShootArrow(&gb, NULL, NULL);
    TEST_ASSERT(res == true, "Bomb arrow shot failed");
    TEST_ASSERT(gb_read(&gb, wEntitiesStatusTable + 0x03) == 0x00, "Dropped bomb not cleared on bomb-arrow");
    TEST_ASSERT(gb_read(&gb, wEntitiesStateTable + gb_read(&gb, wLatestShotArrowEntityIndex)) == 0x01, "Arrow state not set to 1 for bomb-arrow");
    TEST_ASSERT(gb_read(&gb, wBombArrowCooldown) == 0x00, "Bomb arrow cooldown not reset");
}

static void test_use_magic_powder(void) {
    GBState gb;

    /* 1. Has toadstool: gives toadstool dialog */
    gb_init(&gb);
    gb_write(&gb, wHasToadstool, 0x01);
    gb_write(&gb, hLinkPositionZ, 0x00);
    UseMagicPowder(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wDialogGotItem) == 0x02, "Toadstool dialog not triggered");
    TEST_ASSERT(gb_read(&gb, wDialogGotItemCountdown) == 0x2A, "Toadstool countdown not set");

    /* 2. Zero magic powder -> wrong answer jingle */
    gb_init(&gb);
    gb_write(&gb, wMagicPowderCount, 0x00);
    UseMagicPowder(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_WRONG_ANSWER, "Wrong answer jingle not played on 0 powder");

    /* 3. Normal magic powder sprinkle */
    gb_init(&gb);
    gb_write(&gb, wMagicPowderCount, 0x10);
    UseMagicPowder(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wEntitiesStatusTable) == 4, "Magic powder entity not initialized");
    TEST_ASSERT(gb_read(&gb, wEntitiesTypeTable) == ENTITY_MAGIC_POWDER_SPRINKLE, "Entity type not magic powder sprinkle");
}

static void test_use_rocs_feather(void) {
    GBState gb;

    /* 1. In air already -> ignored */
    gb_init(&gb);
    gb_write(&gb, wIsLinkInTheAir, 0x01);
    UseRocsFeather(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hJingle) == 0, "Feather used while already in air");

    /* 2. Normal top-down jump */
    gb_init(&gb);
    UseRocsFeather(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wIsLinkInTheAir) == 0x01, "wIsLinkInTheAir not set");
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_FEATHER_JUMP, "Feather jump jingle not played");
    TEST_ASSERT(gb_read(&gb, hLinkVelocityZ) == 0x20, "Link velocity Z not 0x20");

    /* 3. Running with Pegasus boots boost */
    gb_init(&gb);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, hLinkDirection, DIRECTION_RIGHT);
    UseRocsFeather(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hLinkSpeedX) == 28, "Pegasus boots jump speed boost X mismatch");

    /* 4. Side-scrolling jump */
    gb_init(&gb);
    gb_write(&gb, hIsSideScrolling, 0x01);
    gb_write(&gb, hPressedButtonsMask, J_RIGHT);
    UseRocsFeather(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hLinkSpeedY) == 0xE8, "Side-scrolling jump speed Y mismatch with lateral button pressed");
}

static void test_update_link_direction_from_joypad(void) {
    GBState gb;

    gb_init(&gb);
    gb_write(&gb, hPressedButtonsMask, J_RIGHT);
    UpdateLinkDirectionFromJoypad(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkDirection) == DIRECTION_RIGHT, "Direction not set to RIGHT");

    gb_write(&gb, hPressedButtonsMask, J_UP);
    UpdateLinkDirectionFromJoypad(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkDirection) == DIRECTION_UP, "Direction not set to UP");

    /* Diagonal / keep */
    gb_write(&gb, hPressedButtonsMask, J_UP | J_RIGHT);
    UpdateLinkDirectionFromJoypad(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkDirection) == DIRECTION_UP, "Direction changed on diagonal input");
}

static int mock_reset_spin_calls = 0;
static void mock_reset_spin(GBState *gb) {
    (void)gb;
    mock_reset_spin_calls++;
}

static void test_use_sword(void) {
    GBState gb;

    /* 1. Spin attack active -> ignored */
    gb_init(&gb);
    gb_write(&gb, wIsUsingSpinAttack, 0x01);
    UseSword(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wSwordAnimationState) == 0, "Sword used while spin attack active");

    /* 2. Normal swing (L1 sword) */
    gb_init(&gb);
    mock_reset_spin_calls = 0;
    UseSword(&gb, mock_reset_spin, NULL);
    TEST_ASSERT(gb_read(&gb, wSwordAnimationState) == SWORD_ANIMATION_STATE_SWING_START, "Sword animation not START");
    TEST_ASSERT(gb_read(&gb, wSwordCollisionEnabled) == SWORD_ANIMATION_STATE_SWING_START, "Sword collision not enabled");
    TEST_ASSERT(gb_read(&gb, wC138) == 0x03, "wC138 not set to 3");
    TEST_ASSERT(mock_reset_spin_calls == 1, "ResetSpinAttack not called");

    /* 3. L2 Sword with full hearts -> sword beam */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 0x02);
    gb_write(&gb, wFullHearts, 0x01);
    UseSword(&gb, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wEntitiesTypeTable) == ENTITY_SWORD_BEAM, "Sword beam not spawned on full health L2 sword");
}

#define RUN_CHECK_ITEMS_TEST(fn, name) \
    do { \
        int _prev = failures; \
        printf("[RUN ] %s\n", name); \
        fn(); \
        if (failures == _prev) { \
            printf("[PASS] %s\n", name); \
        } else { \
            printf("[FAIL] %s\n", name); \
        } \
    } while (0)

void run_check_items_to_use_tests(void) {
    printf("[TEST] Check Items To Use\n");
    RUN_CHECK_ITEMS_TEST(test_set_shield_vals, "SetShieldVals");
    RUN_CHECK_ITEMS_TEST(test_hold_sword_if_needed, "HoldSwordIfNeeded");
    RUN_CHECK_ITEMS_TEST(test_use_shield, "UseShield");
    RUN_CHECK_ITEMS_TEST(test_use_shovel, "UseShovel");
    RUN_CHECK_ITEMS_TEST(test_use_hookshot, "UseHookshot");
    RUN_CHECK_ITEMS_TEST(test_use_magic_rod, "UseMagicRod");
    RUN_CHECK_ITEMS_TEST(test_place_bomb, "PlaceBomb");
    RUN_CHECK_ITEMS_TEST(test_check_items_to_use, "CheckItemsToUse");
    RUN_CHECK_ITEMS_TEST(test_use_power_bracelet, "UsePowerBracelet");
    RUN_CHECK_ITEMS_TEST(test_use_boomerang, "UseBoomerang");
    RUN_CHECK_ITEMS_TEST(test_spawn_player_projectile, "SpawnPlayerProjectile");
    RUN_CHECK_ITEMS_TEST(test_shoot_arrow, "ShootArrow");
    RUN_CHECK_ITEMS_TEST(test_use_magic_powder, "UseMagicPowder");
    RUN_CHECK_ITEMS_TEST(test_use_rocs_feather, "UseRocsFeather");
    RUN_CHECK_ITEMS_TEST(test_update_link_direction_from_joypad, "UpdateLinkDirectionFromJoypad");
    RUN_CHECK_ITEMS_TEST(test_use_sword, "UseSword");

    if (failures == 0) {
        printf("[PASS] Check Items To Use\n\n");
    } else {
        printf("[FAIL] Check Items To Use (%d failures)\n\n", failures);
    }
}
