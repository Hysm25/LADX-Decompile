#include "test_bank3.h"
#include "gb.h"
#include "bank3/entities_droppable.h"
#include "bank3/entities_pushed_block.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/maps.h"
#include "constants/gameplay.h"
#include "constants/inventory.h"
#include "constants/sfx.h"
#include "constants/audio.h"
#include "constants/dialog.h"
#include "constants/vfx.h"
#include "constants/gfx.h"
#include "constants/link.h"
#include "constants/directions.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test 1: PickableCanBeCollectedBySwordTable values */
static void test_PickableCanBeCollectedBySwordTable(void) {
    printf("[RUN ] PickableCanBeCollectedBySwordTable\n");

    GBState gb;
    gb_init(&gb);
    PickableCanBeCollectedBySwordTable(&gb);

    /* Verify table behaviors via PickableCollectIfNeeded */
    /* Heart (0x2D) can be collected by sword */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_DROPPABLE_HEART);
    gb_write(&gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + 3), 0x05);
    /* Countdown 0, so collection check proceeds */
    gb_write(&gb, (uint16_t)(wEntitiesPrivateCountdown1Table + 3), 0x00);
    PickableCollectIfNeeded(&gb, 3);
    /* Ignore hits should have been temporarily cleared and restored during sword check */
    assert(gb_read(&gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + 3)) == 0x05);

    printf("[PASS] PickableCanBeCollectedBySwordTable\n");
}

/* Test 2: PickableHandleGrabbedByItemIfNeeded */
static void test_PickableHandleGrabbedByItemIfNeeded(void) {
    printf("[RUN ] PickableHandleGrabbedByItemIfNeeded\n");

    GBState gb;
    gb_init(&gb);

    /* Case A: Not grabbed -> does nothing */
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState5Table + 2), 0);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 2), 40);
    PickableHandleGrabbedByItemIfNeeded(&gb, 2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 2)) == 40);

    /* Case B: Grabbed by active boomerang -> snaps position */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState5Table + 2), 0x05); /* grabber = index 4 */
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 4), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTypeTable + 4), ENTITY_BOOMERANG);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 4), 88);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 4), 66);
    gb_write(&gb, (uint16_t)(wEntitiesPosZTable + 2), 12);

    PickableHandleGrabbedByItemIfNeeded(&gb, 2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 2)) == 88);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYTable + 2)) == 66);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosZTable + 2)) == 0);

    /* Case C: Grabbed by disabled item -> collects immediately */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_DROPPABLE_HEART);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState5Table + 1), 0x02); /* grabber = index 1 */
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_DISABLED);
    gb_write(&gb, (uint16_t)(wEntitiesLoadOrderTable + 1), 0xFF);
    PickableHandleGrabbedByItemIfNeeded(&gb, 1);
    /* Should have added health buffer 8 for heart */
    assert(gb_read(&gb, wAddHealthBuffer) == 8);

    /* Case D: Grabbed by non-boomerang non-hookshot entity -> collects */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_DROPPABLE_RUPEE);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState5Table + 3), 0x06); /* grabber = index 5 */
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 5), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTypeTable + 5), ENTITY_BOMB);
    gb_write(&gb, (uint16_t)(wEntitiesLoadOrderTable + 3), 0xFF);
    PickableHandleGrabbedByItemIfNeeded(&gb, 3);
    assert(gb_read(&gb, wAddRupeeBufferLow) == 1);

    printf("[PASS] PickableHandleGrabbedByItemIfNeeded\n");
}

/* Test 3: PickableCollectIfNeeded */
static void test_PickableCollectIfNeeded(void) {
    printf("[RUN ] PickableCollectIfNeeded\n");

    GBState gb;
    gb_init(&gb);

    /* If private countdown1 != 0, return without collecting */
    gb_write_hram(&gb, hActiveEntityType, ENTITY_DROPPABLE_HEART);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateCountdown1Table + 2), 0x04);
    PickableCollectIfNeeded(&gb, 2);
    assert(gb_read(&gb, wAddHealthBuffer) == 0);

    /* When collision succeeds: room cleared bit updated for load order < 8 and entity unloaded */
    gb_init(&gb);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_DROPPABLE_FAIRY);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateCountdown1Table + 1), 0);
    gb_write(&gb, (uint16_t)(wEntitiesLoadOrderTable + 1), 0x02); /* bit 2 = 0x04 */
    gb_write_hram(&gb, hMapRoom, 0x34);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);

    /* Set up Link collision so func_003_6C6B returns true: */
    /* Position entity 1 at Link's position */
    uint8_t link_x = 0x50;
    uint8_t link_y = 0x40;
    gb_write_hram(&gb, hLinkPositionX, link_x);
    gb_write_hram(&gb, hLinkPositionY, link_y);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), link_x);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), link_y);
    gb_write(&gb, (uint16_t)(wEntitiesPhysicsFlagsTable + 1), 0);

    /* In func_003_6C6B, (hFrameCounter ^ c) bit 0 must be 1 */
    gb_write_hram(&gb, hFrameCounter, 0x00);

    PickableCollectIfNeeded(&gb, 1);
    /* Should have added 0x30 health buffer for fairy */
    assert(gb_read(&gb, wAddHealthBuffer) == 0x30);
    /* Cleared room bit 2 = 0x04 */
    assert(gb_read(&gb, (uint16_t)(wEntitiesClearedRooms + 0x34)) & 0x04);
    /* Entity 1 should be unloaded */
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    printf("[PASS] PickableCollectIfNeeded\n");
}

/* Test 4: PickDroppableMagicPowder */
static void test_PickDroppableMagicPowder(void) {
    printf("[RUN ] PickDroppableMagicPowder\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wMagicPowderCount, 0x05);
    gb_write(&gb, wMaxMagicPowder, 0x20);

    PickDroppableMagicPowder(&gb, 0);

    assert(gb_read_hram(&gb, hReplaceTiles) == REPLACE_TILES_MAGIC_POWDER);
    assert(gb_read(&gb, wInventoryBButtonSlot) == 0x0C);
    assert(gb_read(&gb, wMagicPowderCount) == 0x06);

    /* Test BCD addition 0x09 -> 0x10 */
    gb_write(&gb, wMagicPowderCount, 0x09);
    PickDroppableMagicPowder(&gb, 0);
    assert(gb_read(&gb, wMagicPowderCount) == 0x10);

    /* Test clamp at max */
    gb_write(&gb, wMagicPowderCount, 0x20);
    PickDroppableMagicPowder(&gb, 0);
    assert(gb_read(&gb, wMagicPowderCount) == 0x20);

    printf("[PASS] PickDroppableMagicPowder\n");
}

/* Test 5: PickSecretSeashell & IncreaseValueAtHLClampAt99 */
static void test_PickSecretSeashell(void) {
    printf("[RUN ] PickSecretSeashell\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wSeashellsCount, 0x08);
    gb_write_hram(&gb, hMapRoom, 0x15);
    gb_write(&gb, wIsIndoor, 0);

    PickSecretSeashell(&gb, 0);

    /* Seashell count incremented to 9 */
    assert(gb_read(&gb, wSeashellsCount) == 0x09);
    /* Room completed flag set */
    assert(gb_read(&gb, (uint16_t)(wOverworldRoomStatus + 0x15)) & ROOM_STATUS_EVENT_1);
    assert(gb_read_hram(&gb, hRoomStatus) & ROOM_STATUS_EVENT_1);

    /* Test clamp at 99 (0x99) */
    gb_write(&gb, wSeashellsCount, 0x99);
    IncreaseValueAtHLClampAt99(&gb);
    assert(gb_read(&gb, wSeashellsCount) == 0x99);

    /* Test BCD rollover 0x09 -> 0x10 */
    gb_write(&gb, wSeashellsCount, 0x09);
    IncreaseValueAtHLClampAt99(&gb);
    assert(gb_read(&gb, wSeashellsCount) == 0x10);

    printf("[PASS] PickSecretSeashell\n");
}

/* Test 6: PickDroppableArrows and Bombs */
static void test_PickDroppableArrows_and_Bombs(void) {
    printf("[RUN ] PickDroppableArrows_and_Bombs\n");

    GBState gb;
    gb_init(&gb);

    /* Arrows */
    gb_write(&gb, wArrowCount, 0x19);
    gb_write(&gb, wMaxArrows, 0x30);
    PickDroppableArrows(&gb, 0);
    assert(gb_read(&gb, wArrowCount) == 0x20);

    /* Clamp arrows */
    gb_write(&gb, wArrowCount, 0x30);
    PickDroppableArrows(&gb, 0);
    assert(gb_read(&gb, wArrowCount) == 0x30);

    /* Bombs */
    gb_write(&gb, wBombCount, 0x09);
    gb_write(&gb, wMaxBombs, 0x30);
    PickDroppableBombs(&gb, 0);
    assert(gb_read(&gb, wInventoryBButtonSlot) == 0x02);
    assert(gb_read(&gb, wBombCount) == 0x10);

    /* Clamp bombs */
    gb_write(&gb, wBombCount, 0x30);
    PickDroppableBombs(&gb, 0);
    assert(gb_read(&gb, wBombCount) == 0x30);

    printf("[PASS] PickDroppableArrows_and_Bombs\n");
}

/* Test 7: PickSirensInstrument and HoldPickupInTheAir */
static void test_PickSirensInstrument(void) {
    printf("[RUN ] PickSirensInstrument\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wBossDefeated, 0x01);
    gb_write(&gb, wObjectAffectingBGPalette, 0x05);
    gb_write_hram(&gb, hLinkPositionX, 50);
    gb_write_hram(&gb, hLinkPositionY, 60);

    PickSirensInstrument(&gb, 3);

    assert(gb_read(&gb, wBossDefeated) == 0);
    assert(gb_read(&gb, wObjectAffectingBGPalette) == 0);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OBTAIN_INSTRUMENT);
    assert(gb_read(&gb, wC167) == MUSIC_OBTAIN_INSTRUMENT);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 3)) == 0x68);
    assert(gb_read(&gb, wC111) == 0x68);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 3)) == ENTITY_STATUS_ACTIVE);
    /* hLinkPositionX should be restored */
    assert(gb_read_hram(&gb, hLinkPositionX) == 50);

    printf("[PASS] PickSirensInstrument\n");
}

/* Test 8: PickHeartContainer */
static void test_PickHeartContainer(void) {
    printf("[RUN ] PickHeartContainer\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActivePowerUp, 0x01);
    PickHeartContainer(&gb, 2);

    assert(gb_read(&gb, wActivePowerUp) == 0);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_HEART_CONTAINER);
    assert(gb_read(&gb, wBossDefeated) == MUSIC_HEART_CONTAINER);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2)) == 0x70);
    assert(gb_read(&gb, wC111) == 0x70);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 2)) == ENTITY_STATUS_ACTIVE);

    printf("[PASS] PickHeartContainer\n");
}

/* Test 9: PickToadstoolOrDungeonKey & HeartPiece */
static void test_PickToadstool_and_HeartPiece(void) {
    printf("[RUN ] PickToadstool_and_HeartPiece\n");

    GBState gb;
    gb_init(&gb);

    /* Toadstool or dungeon key */
    PickToadstoolOrDungeonKey(&gb, 1);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OBTAIN_ITEM);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x68);
    assert(gb_read(&gb, wC111) == 0x68);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_ACTIVE);

    /* Heart Piece */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesStateTable + 4), 0x02);
    PickHeartPiece(&gb, 4);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OBTAIN_ITEM);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 4)) == 0x03);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 4)) == ENTITY_STATUS_ACTIVE);

    printf("[PASS] PickToadstool_and_HeartPiece\n");
}

/* Test 10: PickGuardianAcorn & PieceOfPower */
static void test_PowerUps(void) {
    printf("[RUN ] PowerUps\n");

    GBState gb;
    gb_init(&gb);

    /* Guardian Acorn */
    PickGuardianAcorn(&gb, 0);
    assert(gb_read(&gb, wActivePowerUp) == ACTIVE_POWER_UP_GUARDIAN_ACORN);
    assert(gb_read(&gb, wDialogGotItem) == DIALOG_GOT_GUARDIAN_ACORN);
    assert(gb_read(&gb, wDialogGotItemCountdown) == 0x30);
    assert(gb_read(&gb, wC111) == 0x30);
    assert(gb_read(&gb, wPowerUpHits) == 0);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OBTAIN_POWERUP);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_ACTIVE_POWER_UP);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_ACTIVE_POWER_UP);

    /* Piece of Power */
    gb_init(&gb);
    PickPieceOfPower(&gb, 1);
    assert(gb_read(&gb, wActivePowerUp) == ACTIVE_POWER_UP_PIECE_OF_POWER);
    assert(gb_read(&gb, wDialogGotItem) == DIALOG_GOT_PIECE_OF_POWER);

    printf("[PASS] PowerUps\n");
}

/* Test 11: PickSword */
static void test_PickSword(void) {
    printf("[RUN ] PickSword\n");

    GBState gb;
    gb_init(&gb);

    /* Sword level == 0: obtain sword music, countdown 0xA0, silence next */
    gb_write(&gb, wSwordLevel, 0);
    PickSword(&gb, 2);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OBTAIN_SWORD);
    assert(gb_read(&gb, wC167) == MUSIC_OBTAIN_SWORD);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2)) == 0xA0);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_SILENCE);

    /* Sword level > 0: shields restored */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 1);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState1Table + 2), 0x02); /* shield level 2 */
    PickSword(&gb, 2);
    assert(gb_read(&gb, wShieldLevel) == 0x02);
    assert(gb_read(&gb, wInventoryBButtonSlot) == INVENTORY_SHIELD);

    printf("[PASS] PickSword\n");
}

/* Test 12: GiveInventoryItem */
static void test_GiveInventoryItem(void) {
    printf("[RUN ] GiveInventoryItem\n");

    GBState gb;
    gb_init(&gb);

    /* Adds to first empty slot */
    GiveInventoryItem(&gb, INVENTORY_BOOMERANG);
    assert(gb_read(&gb, wInventoryBButtonSlot) == INVENTORY_BOOMERANG);

    /* Doesn't add duplicate */
    GiveInventoryItem(&gb, INVENTORY_BOOMERANG);
    assert(gb_read(&gb, (uint16_t)(wInventoryBButtonSlot + 1)) == 0);

    /* Adds another item to slot 1 */
    GiveInventoryItem(&gb, INVENTORY_HOOKSHOT);
    assert(gb_read(&gb, (uint16_t)(wInventoryBButtonSlot + 1)) == INVENTORY_HOOKSHOT);

    /* Full inventory doesn't overflow */
    for (uint16_t i = 0; i < 12; i++) {
        gb_write(&gb, (uint16_t)(wInventoryBButtonSlot + i), (uint8_t)(i + 1));
    }
    GiveInventoryItem(&gb, INVENTORY_MAGIC_POWDER);
    assert(gb_read(&gb, (uint16_t)(wInventoryBButtonSlot + 11)) == 12);

    printf("[PASS] GiveInventoryItem\n");
}

/* Test 13: PickDroppableKey */
static void test_PickDroppableKey(void) {
    printf("[RUN ] PickDroppableKey\n");

    GBState gb;
    gb_init(&gb);

    /* Room Catfish's Maw Master Stalfos 4 */
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_4);
    PickDroppableKey(&gb, 1);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OBTAIN_ITEM);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x68);

    /* Room Angler's Tunnel Key Fall: sets bit 4 in indoor A room 0x69 */
    gb_init(&gb);
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_FALL);
    gb_write(&gb, (uint16_t)(wIndoorARoomStatus + 0x69), 0x00);
    gb_write_hram(&gb, hActiveEntitySpriteVariant, 0);
    PickDroppableKey(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wIndoorARoomStatus + 0x69)) & 0x10);
    assert(gb_read(&gb, wSmallKeysCount) == 1);

    /* Variant != 0 */
    gb_init(&gb);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write_hram(&gb, hActiveEntitySpriteVariant, 1);
    PickDroppableKey(&gb, 2);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OBTAIN_ITEM);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2)) == 0x68);

    printf("[PASS] PickDroppableKey\n");
}

/* Test 14: PickDroppableHeart, Rupee, Fairy */
static void test_PickDroppableHeart_Rupee_Fairy(void) {
    printf("[RUN ] PickDroppableHeart_Rupee_Fairy\n");

    GBState gb;
    gb_init(&gb);

    PickDroppableHeart(&gb, 0);
    assert(gb_read(&gb, wAddHealthBuffer) == 8);

    PickDroppableRupee(&gb, 0);
    assert(gb_read(&gb, wAddRupeeBufferLow) == 1);

    PickDroppableFairy(&gb, 0);
    assert(gb_read(&gb, wAddHealthBuffer) == 8 + 0x30);

    printf("[PASS] PickDroppableHeart_Rupee_Fairy\n");
}

/* Test 15: SpawnNewEntity and ConfigureNewEntity_helper */
static void test_SpawnNewEntity(void) {
    printf("[RUN ] SpawnNewEntity\n");

    GBState gb;
    gb_init(&gb);

    /* Set up parent entity in slot 0 */
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 0), 100);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 0), 120);
    gb_write(&gb, (uint16_t)(wEntitiesDirectionTable + 0), 2);
    gb_write(&gb, (uint16_t)(wEntitiesPosZTable + 0), 5);
    gb_write(&gb, (uint16_t)(wEntitiesPosXSignTable + 0), 1);
    gb_write(&gb, (uint16_t)(wEntitiesPosYSignTable + 0), 0);

    /* All slots initially disabled */
    uint16_t slot = SpawnNewEntity_slot(&gb, ENTITY_DROPPABLE_FAIRY);
    assert(slot == 15); /* Should allocate slot 15 */
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 15)) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTypeTable + 15)) == ENTITY_DROPPABLE_FAIRY);
    assert(gb_read_hram(&gb, hMultiPurpose0) == 100);
    assert(gb_read_hram(&gb, hMultiPurpose1) == 120);
    assert(gb_read_hram(&gb, hMultiPurpose2) == 2);
    assert(gb_read_hram(&gb, hMultiPurpose3) == 5);
    assert(gb_read(&gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + 15)) == 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXSignTable + 15)) == 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYSignTable + 15)) == 0);

    /* Next allocation should pick slot 14 */
    slot = SpawnNewEntity_slot(&gb, ENTITY_DROPPABLE_HEART);
    assert(slot == 14);

    /* When all slots active, returns 0xFFFF */
    for (uint16_t i = 0; i < MAX_ENTITIES; i++) {
        gb_write(&gb, (uint16_t)(wEntitiesStatusTable + i), ENTITY_STATUS_ACTIVE);
    }
    slot = SpawnNewEntity_slot(&gb, ENTITY_BOMB);
    assert(slot == 0xFFFF);

    printf("[PASS] SpawnNewEntity\n");
}

/* Test 16: MarkRoomCompleted & GetRoomStatusAddressInHL */
static void test_MarkRoomCompleted_and_GetRoomStatusAddressInHL(void) {
    printf("[RUN ] MarkRoomCompleted_and_GetRoomStatusAddressInHL\n");

    GBState gb;
    gb_init(&gb);

    /* Overworld room 0x2A */
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hMapRoom, 0x2A);
    gb_write_hram(&gb, hMapId, 0x00);
    assert(GetRoomStatusAddressInHL(&gb) == (uint16_t)(wOverworldRoomStatus + 0x2A));

    MarkRoomCompleted(&gb);
    assert(gb_read(&gb, (uint16_t)(wOverworldRoomStatus + 0x2A)) & ROOM_STATUS_EVENT_1);
    assert(gb_read_hram(&gb, hRoomStatus) & ROOM_STATUS_EVENT_1);

    /* Indoors A room 0x12 */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, 0x12);
    gb_write_hram(&gb, hMapId, 0x01); /* MAP_TAIL_CAVE */
    assert(GetRoomStatusAddressInHL(&gb) == (uint16_t)(wIndoorARoomStatus + 0x12));

    /* Indoors B room 0x34 (MAP_INDOORS_B_START <= map < MAP_INDOORS_B_END) */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, 0x34);
    gb_write_hram(&gb, hMapId, 0x08); /* Indoors B map */
    assert(GetRoomStatusAddressInHL(&gb) == (uint16_t)(wIndoorBRoomStatus + 0x34));

    /* Color Dungeon room 0x05 */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, 0x05);
    gb_write_hram(&gb, hMapId, MAP_COLOR_DUNGEON);
    assert(GetRoomStatusAddressInHL(&gb) == (uint16_t)(wColorDungeonRoomStatus + 0x05));

    printf("[PASS] MarkRoomCompleted_and_GetRoomStatusAddressInHL\n");
}

/* Test 17: HoldEntityAboveLink and func_003_5A2E */
static void test_HoldEntityAboveLink_and_func_003_5A2E(void) {
    printf("[RUN ] HoldEntityAboveLink_and_func_003_5A2E\n");

    GBState gb;
    gb_init(&gb);

    gb_write_hram(&gb, hLinkPositionX, 0x40);
    gb_write_hram(&gb, hLinkPositionY, 0x50);
    gb_write_hram(&gb, hLinkPositionZ, 0x05);
    gb_write(&gb, wSwordAnimationState, 1);
    gb_write(&gb, wC16A, 2);
    gb_write(&gb, wSwordCharge, 3);
    gb_write(&gb, wIsUsingSpinAttack, 4);
    gb_write(&gb, (uint16_t)(wEntitiesGroundStatusTable + 1), 5);

    HoldEntityAboveLink(&gb, 1);

    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 1)) == 0x40);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYTable + 1)) == 0x44); /* 0x50 - 0x0C */
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosZTable + 1)) == 0x05);
    assert(gb_read_hram(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_GOT_ITEM);
    assert(gb_read_hram(&gb, hLinkDirection) == DIRECTION_DOWN);
    assert(gb_read(&gb, wSwordAnimationState) == 0);
    assert(gb_read(&gb, wC16A) == 0);
    assert(gb_read(&gb, wSwordCharge) == 0);
    assert(gb_read(&gb, wIsUsingSpinAttack) == 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesGroundStatusTable + 1)) == 0);
    assert(gb_read_hram(&gb, hLinkInteractiveMotionBlocked) == 0x02);

    printf("[PASS] HoldEntityAboveLink_and_func_003_5A2E\n");
}

/* Test 18: HeartContainerEntityHandler */
static void test_HeartContainerEntityHandler(void) {
    printf("[RUN ] HeartContainerEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* Case A: Countdown == 1, Boss defeated pickup completion */
    gb_write(&gb, wActiveEntityIndex, 2);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 2), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2), 1);
    gb_write(&gb, wMaxHearts, 3);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write_hram(&gb, hMapId, MAP_EAGLES_TOWER);
    gb_write(&gb, (uint16_t)(wIndoorBRoomStatus + 0x2E), 0x00);

    HeartContainerEntityHandler(&gb, 2);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_AFTER_BOSS);
    assert(gb_read(&gb, wMaxHearts) == 4);
    assert(gb_read(&gb, wAddHealthBuffer) == 0xFF);
    assert(gb_read_hram(&gb, hRoomStatus) & ROOM_STATUS_EVENT_2);
    assert(gb_read(&gb, (uint16_t)(wIndoorBRoomStatus + 0x2E)) == 0x20); /* Eagles Tower staircase flag */
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 2)) == ENTITY_STATUS_DISABLED);

    /* Case B: Angler's Tunnel boss staircase flag */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 0), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 0), 1);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, 0x20);
    gb_write_hram(&gb, hMapId, MAP_ANGLERS_TUNNEL);
    gb_write(&gb, (uint16_t)(wIndoorARoomStatus + 0x66), 0x00);

    HeartContainerEntityHandler(&gb, 0);
    assert(gb_read(&gb, (uint16_t)(wIndoorARoomStatus + 0x66)) == 0x20);

    /* Case C: Countdown > 1 holds above Link */
    gb_init(&gb);
    gb_write_hram(&gb, hLinkPositionX, 0x20);
    gb_write_hram(&gb, hLinkPositionY, 0x30);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 5);
    HeartContainerEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 1)) == 0x20);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYTable + 1)) == 0x24);

    printf("[PASS] HeartContainerEntityHandler\n");
}

static void setup_interactive_entity(GBState *gb, uint16_t entity_index) {
    gb_write_hram(gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(gb, (uint16_t)(wEntitiesStatusTable + entity_index), ENTITY_STATUS_ACTIVE);
    gb_write(gb, wActiveEntityIndex, entity_index);
    gb_write(gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(gb, wTransitionSequenceCounter, 0x04);
    gb_write(gb, wDialogState, 0x00);
    gb_write(gb, wC1A8, 0x00);
    gb_write(gb, wInventoryAppearing, 0x00);
    gb_write(gb, wRoomTransitionState, 0x00);
}

/* Test 19: GuardianAcorn, PieceOfPower, IronMasksMask */
static void test_GuardianAcorn_PieceOfPower_IronMasksMask(void) {
    printf("[RUN ] GuardianAcorn_PieceOfPower_IronMasksMask\n");

    GBState gb;
    gb_init(&gb);

    /* Guardian acorn with countdown 0 delegates to PickableHandler */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 0), 0);
    GuardianAcornEntityHandler(&gb, 0);

    /* Piece of power updates sprite variant from frame counter */
    gb_init(&gb);
    gb_write_hram(&gb, hFrameCounter, 0x08); /* 8 >> 3 = 1 */
    PieceOfPowerEntityHandler(&gb, 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 0)) == 1);

    gb_write_hram(&gb, hFrameCounter, 0x10); /* 16 >> 3 = 2 & 1 = 0 */
    PieceOfPowerEntityHandler(&gb, 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 0)) == 0);

    /* IronMasksMask: non-interactive returns */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_DISABLED);
    IronMasksMaskEntityHandler(&gb, 1);

    /* Interactive: calls PickableHandleGrabbedByItemIfNeeded */
    gb_init(&gb);
    setup_interactive_entity(&gb, 1);
    gb_write(&gb, (uint16_t)(wEntitiesPrivateState5Table + 1), 0x05);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 4), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTypeTable + 4), ENTITY_BOOMERANG);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 4), 0x55);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 4), 0x66);
    IronMasksMaskEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 1)) == 0x55);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYTable + 1)) == 0x66);

    printf("[PASS] GuardianAcorn_PieceOfPower_IronMasksMask\n");
}

/* Test 20: HookshotDropEntityHandler */
static void test_HookshotDropEntityHandler(void) {
    printf("[RUN ] HookshotDropEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* Already collected in room -> unloads */
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    HookshotDropEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    /* Countdown == 0x10 -> triggers dialog 0x93, decrements to 0x0F, holds above Link */
    gb_init(&gb);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2), 0x10);
    HookshotDropEntityHandler(&gb, 2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2)) == 0x0F);
    assert(gb_read_hram(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_GOT_ITEM);

    /* Countdown == 1 -> gives Hookshot, marks room completed, unloads */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 2), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 2), 1);
    HookshotDropEntityHandler(&gb, 2);
    assert(gb_read(&gb, wInventoryBButtonSlot) == INVENTORY_HOOKSHOT);
    assert(gb_read_hram(&gb, hRoomStatus) & ROOM_STATUS_EVENT_1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 2)) == ENTITY_STATUS_DISABLED);

    printf("[PASS] HookshotDropEntityHandler\n");
}

/* Test 21: KeyDropPointEntityHandler */
static void test_KeyDropPointEntityHandler(void) {
    printf("[RUN ] KeyDropPointEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* Case A: In Catfish's Maw Master Stalfos room (0x80) -> behaves as Hookshot drop */
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_4);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 0), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 0), 1);
    KeyDropPointEntityHandler(&gb, 0);
    assert(gb_read(&gb, wInventoryBButtonSlot) == INVENTORY_HOOKSHOT);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 0)) == ENTITY_STATUS_DISABLED);

    /* Case B: Quicksand hole drop -> sets Yarna Lanmola flag and Quicksand Cave flag */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hMapRoom, ROOM_OW_YARNA_LANMOLA);
    gb_write_hram(&gb, hActiveEntityPosX, 0x50);
    gb_write_hram(&gb, hActiveEntityPosY, 0x48);
    gb_write(&gb, (uint16_t)(wEntitiesPosZTable + 0), 0);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 0), 5);
    KeyDropPointEntityHandler(&gb, 0);
    assert(gb_read(&gb, (uint16_t)(wOverworldRoomStatus + ROOM_OW_YARNA_LANMOLA)) & (1 << OW_ROOM_STATUS_FLAG_CHANGED));
    assert(gb_read(&gb, (uint16_t)(wIndoorARoomStatus + ROOM_INDOOR_A_QUICKSAND_CAVE)) & 0x20);

    /* Case C: Normal key drop pickup at countdown 0x10 */
    gb_init(&gb);
    gb_write_hram(&gb, hMapRoom, 0x05);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0x10);
    gb_write(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 1), 2); /* Variant 2 = Face Key (index 1) */
    KeyDropPointEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x0F);
    assert(gb_read(&gb, (uint16_t)(wHasTailKey + 1)) == 1); /* wHasAnglerKey */
    assert(gb_read_hram(&gb, hRoomStatus) & ROOM_STATUS_EVENT_1);

    /* Countdown == 1 -> unloads */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 1);
    KeyDropPointEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    printf("[PASS] KeyDropPointEntityHandler\n");
}

/* Test 22: DroppableHeart, Bombs, Seashell */
static void test_DroppableHeart_Bombs_Seashell(void) {
    printf("[RUN ] DroppableHeart_Bombs_Seashell\n");

    GBState gb;
    gb_init(&gb);

    /* Droppable heart and bombs delegate to PickableHandler */
    DroppableHeartEntityHandler(&gb, 0);
    DroppableBombsEntityHandler(&gb, 0);

    /* Seashell unloads if sword level >= 2 */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 2);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    DroppableSeashellEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    /* Seashell unloads if room status bit 4 is set */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 1);
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    DroppableSeashellEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    /* Seashell in room E3 unloads if ROOM_STATUS_EVENT_3 is 0 */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 1);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_E3);
    gb_write_hram(&gb, hRoomStatus, 0);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    DroppableSeashellEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    printf("[PASS] DroppableHeart_Bombs_Seashell\n");
}

/* Test 23: SleepyToadstoolEntityHandler */
static void test_SleepyToadstoolEntityHandler(void) {
    printf("[RUN ] SleepyToadstoolEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* If Link already has toadstool or powder, entity unloads */
    gb_write(&gb, wHasToadstool, 1);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 0), ENTITY_STATUS_ACTIVE);
    SleepyToadstoolEntityHandler(&gb, 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 0)) == ENTITY_STATUS_DISABLED);

    /* Countdown == 0x10 -> opens Dialog00F and holds above Link */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 0), 0x10);
    SleepyToadstoolEntityHandler(&gb, 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 0)) == 0x0F);
    assert(gb_read_hram(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_GOT_ITEM);

    /* Countdown == 1 -> gives powder, sets wHasToadstool, sets replace tiles, unloads */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 0), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 0), 1);
    SleepyToadstoolEntityHandler(&gb, 0);
    assert(gb_read_hram(&gb, hReplaceTiles) == REPLACE_TILES_TOADSTOOL);
    assert(gb_read(&gb, wInventoryBButtonSlot) == INVENTORY_MAGIC_POWDER);
    assert(gb_read(&gb, wHasToadstool) == 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 0)) == ENTITY_STATUS_DISABLED);

    printf("[PASS] SleepyToadstoolEntityHandler\n");
}

/* Test 24: HidingSlimeKeyEntityHandler */
static void test_HidingSlimeKeyEntityHandler(void) {
    printf("[RUN ] HidingSlimeKeyEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* Already collected in room -> unloads */
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    HidingSlimeKeyEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    /* In Pothole Field outdoor room -> sets leaves to 5, increments to 6 (SLIME_KEY), holds above Link */
    gb_init(&gb);
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hMapRoom, ROOM_OW_POTHOLE_FIELD_SLIME_KEY);
    gb_write_hram(&gb, hRoomStatus, 0);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0x10);

    HidingSlimeKeyEntityHandler(&gb, 1);

    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x0F);
    assert(gb_read(&gb, wGoldenLeavesCount) == SLIME_KEY);
    assert(gb_read(&gb, (uint16_t)(wOverworldRoomStatus + ROOM_OW_POTHOLE_FIELD_SLIME_KEY)) & ROOM_STATUS_EVENT_1);
    assert((gb_read_hram(&gb, hRoomStatus) & (1 << OW_ROOM_STATUS_FLAG_CHANGED)) == 0);

    /* Countdown == 1 -> unloads */
    gb_init(&gb);
    gb_write(&gb, (uint16_t)(wEntitiesStatusTable + 1), ENTITY_STATUS_ACTIVE);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 1);
    HidingSlimeKeyEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    printf("[PASS] HidingSlimeKeyEntityHandler\n");
}

/* Test 25: DroppableFairyEntityHandler */
static void test_DroppableFairyEntityHandler(void) {
    printf("[RUN ] DroppableFairyEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* When Link is far (dist >= 0x20 in X and Y), fairy moves towards Link */
    setup_interactive_entity(&gb, 1);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), 0x10);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), 0x10);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x50);

    DroppableFairyEntityHandler(&gb, 1);

    /* Speed X & Y should be set towards Link */
    int8_t spd_x = (int8_t)gb_read(&gb, (uint16_t)(wEntitiesSpeedXTable + 1));
    int8_t spd_y = (int8_t)gb_read(&gb, (uint16_t)(wEntitiesSpeedYTable + 1));
    assert(spd_x > 0);
    assert(spd_y > 0);

    /* When Link is close and countdown == 0, sets countdown = 0x30 and picks random speed */
    gb_init(&gb);
    setup_interactive_entity(&gb, 1);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), 0x50);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), 0x50);
    gb_write_hram(&gb, hLinkPositionX, 0x55);
    gb_write_hram(&gb, hLinkPositionY, 0x55);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0);

    DroppableFairyEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x30);

    printf("[PASS] DroppableFairyEntityHandler\n");
}

/* Test: HeartPieceEntityHandler and states 0-8 */
static void test_HeartPieceEntityHandler(void) {
    printf("[RUN ] HeartPieceEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* 1. Pruning when room status event 1 is set */
    setup_interactive_entity(&gb, 2);
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);
    HeartPieceEntityHandler(&gb, 2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 2)) == ENTITY_STATUS_DISABLED);

    /* 2. State 0: dispatches to PickableHandler */
    gb_init(&gb);
    setup_interactive_entity(&gb, 1);
    gb_write_hram(&gb, hActiveEntityState, 0);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_HEART_PIECE);
    /* Link is far away -> no collision, entity stays active */
    gb_write_hram(&gb, hLinkPositionX, 0x10);
    gb_write_hram(&gb, hLinkPositionY, 0x10);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), 0x50);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), 0x50);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_ACTIVE);

    /* 3. State 1: holds above Link; returns if transition countdown != 0 */
    gb_init(&gb);
    setup_interactive_entity(&gb, 1);
    gb_write_hram(&gb, hActiveEntityState, 1);
    gb_write(&gb, (uint16_t)(wEntitiesStateTable + 1), 1);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 5);
    gb_write_hram(&gb, hLinkPositionX, 0x40);
    gb_write_hram(&gb, hLinkPositionY, 0x60);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 1)) == 0x40);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYTable + 1)) == 0x54); /* 0x60 - 0x0C */
    assert(gb_read_hram(&gb, hActiveEntityState) == 1);
    assert(gb_read(&gb, wC167) == 0);

    /* State 1 with countdown == 0 -> sets wC167 = 1 and increments state to 2 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, wC167) == 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 2);

    /* 4. State 2 -> sets hNeedsUpdatingBGTiles to TILESET_LOAD_PIECE_OF_HEART_1 and increments state to 3 */
    gb_write_hram(&gb, hActiveEntityState, 2);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_LOAD_PIECE_OF_HEART_1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 3);

    /* 5. State 3 -> sets hNeedsUpdatingBGTiles to TILESET_LOAD_PIECE_OF_HEART_2 and increments state to 4 */
    gb_write_hram(&gb, hActiveEntityState, 3);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_LOAD_PIECE_OF_HEART_2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 4);

    /* 6. State 4 -> opens Dialog04F, locks interaction, increments state to 5 */
    gb_write_hram(&gb, hActiveEntityState, 4);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, wDialogInteractionLocked) == 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 5);

    /* 7. State 5: increments inertia; at 0x38 increments heart pieces count */
    gb_write_hram(&gb, hActiveEntityState, 5);
    gb_write(&gb, (uint16_t)(wEntitiesInertiaTable + 1), 0x37);
    gb_write(&gb, wHeartPiecesCount, 1);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesInertiaTable + 1)) == 0x38);
    assert(gb_read(&gb, wHeartPiecesCount) == 2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 5);

    /* At inertia 0xA8, state increments to 6 */
    gb_write(&gb, (uint16_t)(wEntitiesInertiaTable + 1), 0xA7);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesInertiaTable + 1)) == 0xA8);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 6);

    /* 8. State 6: unlocks dialog; waits for wDialogState == 0 */
    gb_write_hram(&gb, hActiveEntityState, 6);
    gb_write(&gb, wDialogInteractionLocked, 1);
    gb_write(&gb, wDialogState, 1);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, wDialogInteractionLocked) == 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 6); /* still in state 6 */

    /* When dialog closes (wDialogState == 0) with < 4 heart pieces */
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wHeartPiecesCount, 2);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 7);
    assert(gb_read_hram(&gb, hJingle) == 0);

    /* Test 4-heart pieces completion branch in State 6 */
    gb_write(&gb, (uint16_t)(wEntitiesStateTable + 1), 6);
    gb_write_hram(&gb, hActiveEntityState, 6);
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wHeartPiecesCount, 4);
    gb_write(&gb, wMaxHearts, 3);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_NEW_HEART);
    assert(gb_read(&gb, wHeartPiecesCount) == 0);
    assert(gb_read(&gb, wAddHealthBuffer) == 0x40);
    assert(gb_read(&gb, wMaxHearts) == 4);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 7);

    /* 9. State 7: waits for dialog close, then requests TILESET_CLEAR_PIECE_OF_HEART_1 and state 8 */
    gb_write_hram(&gb, hActiveEntityState, 7);
    gb_write(&gb, wDialogState, 1);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 7);

    gb_write(&gb, wDialogState, 0);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_CLEAR_PIECE_OF_HEART_1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 8);

    /* 10. State 8: requests TILESET_CLEAR_PIECE_OF_HEART_2, unloads, replaces tiles, marks room completed */
    gb_write_hram(&gb, hActiveEntityState, 8);
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hMapRoom, 0x50);
    HeartPieceEntityHandler(&gb, 1);
    assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_CLEAR_PIECE_OF_HEART_2);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);
    assert(gb_read_hram(&gb, hReplaceTiles) == REPLACE_TILES_TRADING_ITEM);
    assert(gb_read(&gb, wC167) == 0);
    assert((gb_read(&gb, (uint16_t)(wOverworldRoomStatus + 0x50)) & ROOM_STATUS_EVENT_1) != 0);

    /* 11. DrawHeartPiecesInDialog checks */
    gb_init(&gb);
    /* Dialog inactive -> does nothing */
    gb_write(&gb, wDialogState, 0);
    DrawHeartPiecesInDialog(&gb, 0);
    assert(gb_read_hram(&gb, hActiveEntityVisualPosY) == 0);

    /* Dialog character index >= 0x21 -> does nothing */
    gb_write(&gb, wDialogState, 1);
    gb_write(&gb, wDialogCharacterIndex, 0x22);
    DrawHeartPiecesInDialog(&gb, 0);
    assert(gb_read_hram(&gb, hActiveEntityVisualPosY) == 0);

    /* Top dialog box (bit 7 clear) -> Y = 0x23, X = 0x8E, variant = wHeartPiecesCount */
    gb_write(&gb, wDialogCharacterIndex, 0x10);
    gb_write(&gb, wHeartPiecesCount, 3);
    DrawHeartPiecesInDialog(&gb, 0);
    assert(gb_read_hram(&gb, hActiveEntityVisualPosY) == 0x23);
    assert(gb_read_hram(&gb, hActiveEntityPosX) == 0x8E);
    assert(gb_read_hram(&gb, hActiveEntitySpriteVariant) == 3);

    /* Bottom dialog box (bit 7 set) -> Y = 0x6B */
    gb_write(&gb, wDialogState, DIALOG_BOX_BOTTOM_FLAG | 1);
    DrawHeartPiecesInDialog(&gb, 0);
    assert(gb_read_hram(&gb, hActiveEntityVisualPosY) == 0x6B);

    printf("[PASS] HeartPieceEntityHandler\n");
}

/* Test: SwordShieldPickableEntityHandler and states 0-3 */
static void test_SwordShieldPickableEntityHandler(void) {
    printf("[RUN ] SwordShieldPickableEntityHandler\n");

    GBState gb;
    gb_init(&gb);

    /* 1. Beach sword pruning when room event 1 is set and wSwordLevel == 0 */
    setup_interactive_entity(&gb, 1);
    gb_write(&gb, wSwordLevel, 0);
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    /* 2. Like-like dropped shield (wSwordLevel > 0) does not prune even with room event 1 */
    gb_init(&gb);
    setup_interactive_entity(&gb, 1);
    gb_write_hram(&gb, hLinkPositionX, 0x10);
    gb_write_hram(&gb, hLinkPositionY, 0x10);
    gb_write(&gb, (uint16_t)(wEntitiesPosXTable + 1), 0x60);
    gb_write(&gb, (uint16_t)(wEntitiesPosYTable + 1), 0x60);
    gb_write_hram(&gb, hFrameCounter, 0x01);
    gb_write(&gb, wSwordLevel, 1);
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);
    gb_write_hram(&gb, hActiveEntityState, 0);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_ACTIVE);

    /* 3. State 0:
          Countdown 0x10 -> decrements countdown to 0x0F, opens Dialog09B, holds above Link */
    gb_init(&gb);
    setup_interactive_entity(&gb, 1);
    gb_write_hram(&gb, hActiveEntityState, 0);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0x10);
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x0F);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 1)) == 0x30);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosYTable + 1)) == 0x34); /* 0x40 - 0x0C */
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 0); /* still state 0 */

    /* Countdown 1 -> plays MUSIC_OVERWORLD_INTRO, configures tracks, countdown = 0x52, state = 1 */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 1);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OVERWORLD_INTRO);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_OVERWORLD);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_OVERWORLD);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSlowTransitionCountdownTable + 1)) == 0x52);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 1);

    /* 4. State 1:
          Holds above Link; returns if slow transition countdown != 0 */
    gb_write_hram(&gb, hActiveEntityState, 1);
    gb_write(&gb, (uint16_t)(wEntitiesSlowTransitionCountdownTable + 1), 5);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 1);

    /* When slow countdown == 0: sets sprite variant 0xFF, countdown 0x20, spin attack, SFX, state = 2 */
    gb_write(&gb, (uint16_t)(wEntitiesSlowTransitionCountdownTable + 1), 0);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 1)) == 0xFF);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 0x20);
    assert(gb_read(&gb, wIsUsingSpinAttack) == USING_SPIN_ATTACK_MAX);
    assert(gb_read_hram(&gb, hNoiseSfx) == NOISE_SFX_SPIN_ATTACK);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 2);

    /* 5. State 2:
          Waits for countdown == 0, then sets countdown = 32, variant = 0, state = 3 */
    gb_write_hram(&gb, hActiveEntityState, 2);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 10);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 2);

    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1)) == 32);
    assert(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 1)) == 0x00);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStateTable + 1)) == 3);

    /* 6. State 3:
          Holds above Link, sets Link animation state 0x6B, sets PosX = LinkX - 4 */
    gb_write_hram(&gb, hActiveEntityState, 3);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 26);
    gb_write_hram(&gb, hActiveEntityPosY, 0x30);
    gb_write_hram(&gb, hActiveEntityPosX, 0x48);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read_hram(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_UNKNOWN_6B);
    assert(gb_read(&gb, (uint16_t)(wEntitiesPosXTable + 1)) == 0x4C); /* 0x50 - 4 */
    /* At countdown 26: triggers sword poke VFX and JINGLE_SWORD_POKING */
    assert(gb_read_hram(&gb, hJingle) == JINGLE_SWORD_POKING);
    assert(gb_read_hram(&gb, hMultiPurpose1) == 0x24); /* 0x30 - 0x0C */
    assert(gb_read_hram(&gb, hMultiPurpose0) == 0x48);

    /* At countdown 0: completes sequence, awards INVENTORY_SWORD, sword level = 1, room completed, unloads */
    gb_write(&gb, (uint16_t)(wEntitiesTransitionCountdownTable + 1), 0);
    gb_write(&gb, wIsIndoor, 0);
    gb_write_hram(&gb, hMapRoom, 0xF2);
    SwordShieldPickableEntityHandler(&gb, 1);
    assert(gb_read(&gb, wC167) == 0);
    assert(gb_read(&gb, wSwordLevel) == 1);
    assert(gb_read(&gb, wInventoryBButtonSlot) == INVENTORY_SWORD);
    assert((gb_read(&gb, (uint16_t)(wOverworldRoomStatus + 0xF2)) & ROOM_STATUS_EVENT_1) != 0);
    assert(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 1)) == ENTITY_STATUS_DISABLED);

    printf("[PASS] SwordShieldPickableEntityHandler\n");
}

void test_bank3_entities_droppable(void) {
    test_PickableCanBeCollectedBySwordTable();
    test_PickableHandleGrabbedByItemIfNeeded();
    test_PickableCollectIfNeeded();
    test_PickDroppableMagicPowder();
    test_PickSecretSeashell();
    test_PickDroppableArrows_and_Bombs();
    test_PickSirensInstrument();
    test_PickHeartContainer();
    test_PickToadstool_and_HeartPiece();
    test_PowerUps();
    test_PickSword();
    test_GiveInventoryItem();
    test_PickDroppableKey();
    test_PickDroppableHeart_Rupee_Fairy();
    test_SpawnNewEntity();
    test_MarkRoomCompleted_and_GetRoomStatusAddressInHL();
    test_HoldEntityAboveLink_and_func_003_5A2E();
    test_HeartContainerEntityHandler();
    test_HeartPieceEntityHandler();
    test_GuardianAcorn_PieceOfPower_IronMasksMask();
    test_SwordShieldPickableEntityHandler();
    test_HookshotDropEntityHandler();
    test_KeyDropPointEntityHandler();
    test_DroppableHeart_Bombs_Seashell();
    test_SleepyToadstoolEntityHandler();
    test_HidingSlimeKeyEntityHandler();
    test_DroppableFairyEntityHandler();
}
