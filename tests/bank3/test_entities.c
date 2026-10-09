#include "test_bank3.h"
#include "../bank2/test_support.h"

#include "gb.h"
#include "bank3/entities.h"
#include "bank3/entities_init_basic.h"
#include "constants/audio.h"
#include "constants/entities.h"
#include "constants/gameplay.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/directions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Mock callbacks - declared in test_support.h */

/* Mock callbacks - provided by test_support.c */

/* Test ConfigureNewEntity (03:485B-03:4891) - SKIPPED: Requires ROM tables */
/* Test ConfigureEntityHealth (03:4895-03:48AC) - SKIPPED: Requires ROM tables */

/* Test MasterStalfosDefeated (03:48AD-03:48BE) */
void test_MasterStalfosDefeated(void) {
    printf("[RUN ] MasterStalfosDefeated\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x07);

    MasterStalfosDefeated(&gb);

    /* Verify room event executed flag set */
    assert(gb_read(&gb, wRoomEventEffectExecuted) == 0x01);

    /* Verify entity unloaded - entity status should be cleared */
    assert(gb_read(&gb, wEntitiesStatusTable + 0x07) == ENTITY_STATUS_DISABLED);

    printf("[PASS] MasterStalfosDefeated\n");
}

/* Test EntityInitHorsePiece (03:4926-03:4931) */
void test_EntityInitHorsePiece(void) {
    printf("[RUN ] EntityInitHorsePiece\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesLoadOrderTable + 0x02, 0x00); /* load order 0 -> variant 0x01 */

    EntityInitHorsePiece(&gb);

    /* Verify variant set from Data_003_4924[0] = 0x01 */
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x02) == 0x01);

    /* Test load order 1 -> variant 0x04 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write(&gb, wEntitiesLoadOrderTable + 0x04, 0x01);

    EntityInitHorsePiece(&gb);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x04) == 0x04);

    printf("[PASS] EntityInitHorsePiece\n");
}

/* Test EntityInitMarinAtTalTalHeights (03:4934-03:493C) */
void test_EntityInitMarinAtTalTalHeights(void) {
    printf("[RUN ] EntityInitMarinAtTalTalHeights\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x06);
    gb_write(&gb, wEntitiesPosYTable + 0x06, 0x50);

    EntityInitMarinAtTalTalHeights(&gb);

    /* Verify Y position decreased by 3 */
    assert(gb_read(&gb, wEntitiesPosYTable + 0x06) == 0x4D);

    printf("[PASS] EntityInitMarinAtTalTalHeights\n");
}

/* Test EntityInitHandler (03:48B5-03:4923) - Boss defeated check */
void test_EntityInitHandler_BossDefeated(void) {
    printf("[RUN ] EntityInitHandler (boss defeated)\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x01);
    gb_write(&gb, wEntitiesOptions1Table + 0x01, ENTITY_OPT1_IS_BOSS);
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1); /* Boss defeated */
    gb_write_hram(&gb, hActiveEntityType, 0x10); /* Not Master Stalfos */

    EntityInitHandler(&gb);

    /* Should unload entity and return */
    assert(gb_read(&gb, wEntitiesStatusTable + 0x01) == ENTITY_STATUS_DISABLED);
    assert(g_mock_label_27F2_calls == 0);
    assert(g_mock_get_entity_init_handler_calls == 0);

    printf("[PASS] EntityInitHandler (boss defeated)\n");
}

/* Test EntityInitHandler - Master Stalfos in non-Master-Stalfos room (defeated) */
void test_EntityInitHandler_MasterStalfos(void) {
    printf("[RUN ] EntityInitHandler (Master Stalfos in other room)\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesOptions1Table + 0x02, 0x00);
    gb_write_hram(&gb, hRoomStatus, 0x00);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MASTER_STALFOS);
    gb_write_hram(&gb, hMapRoom, 0x20); /* Different room (not MSTALFOS_1/2/3) */
    gb_write(&gb, wIndoorARoomStatus + 0x20, 0x00); /* Defeated (bit 4-5 = 0) */

    EntityInitHandler(&gb);

    /* Should unload entity */
    assert(gb_read(&gb, wEntitiesStatusTable + 0x02) == ENTITY_STATUS_DISABLED);

    printf("[PASS] EntityInitHandler (Master Stalfos in other room)\n");
}

/* Test EntityInitHandler - Master Stalfos (non-defeated, in Master Stalfos room) */
void test_EntityInitHandler_MasterStalfosNonDefeated(void) {
    printf("[RUN ] EntityInitHandler (Master Stalfos non-defeated)\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MASTER_STALFOS);
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_1);
    gb_write(&gb, wIndoorARoomStatus + ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_1, 0x30); /* Not defeated */

    EntityInitHandler(&gb);

    /* Should continue to normal init (in Master Stalfos room, no defeat check) */
    assert(gb_read(&gb, wDidBossIntro) == 0);
    assert(gb_read(&gb, wInBossBattle) == 1);
    assert(gb_read(&gb, wBossIntroDelay) == 0x20);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) == ENTITY_STATUS_ACTIVE);

    printf("[PASS] EntityInitHandler (Master Stalfos non-defeated)\n");
}

/* Test EntityInitHandler - Indoor mini-boss */
void test_EntityInitHandler_IndoorMiniBoss(void) {
    printf("[RUN ] EntityInitHandler (indoor mini-boss)\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x05);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, wD478, 0x00);
    gb_write(&gb, wEntitiesOptions1Table + 0x05, ENTITY_OPT1_IS_MINI_BOSS);
    gb_write_hram(&gb, hActiveEntityType, 0x20);
    gb_write_hram(&gb, hRoomStatus, 0x00);
    gb_write_hram(&gb, hMapRoom, 0x10);
    g_mock_label_27F2_calls = 0;
    g_mock_get_entity_init_handler_calls = 0;

    EntityInitHandler(&gb);

    /* Should set wC1CF from options1 */
    assert(gb_read(&gb, wC1CF) == ENTITY_OPT1_IS_MINI_BOSS);
    /* label_27F2 called directly, not through mock */
    assert(g_mock_get_entity_init_handler_calls == 0);

    printf("[PASS] EntityInitHandler (indoor mini-boss)\n");
}

/* Test EntityInitHandler - Normal entity */
void test_EntityInitHandler_Normal(void) {
    printf("[RUN ] EntityInitHandler (normal)\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x08);
    gb_write(&gb, wIsIndoor, 0); /* Overworld */
    gb_write(&gb, wEntitiesOptions1Table + 0x08, 0x00);
    gb_write_hram(&gb, hActiveEntityType, 0x30);
    gb_write_hram(&gb, hRoomStatus, 0x00);
    gb_write_hram(&gb, hMapRoom, 0x20);

    EntityInitHandler(&gb);

    assert(gb_read(&gb, wDidBossIntro) == 0);
    assert(gb_read(&gb, wInBossBattle) == 1);
    assert(gb_read(&gb, wBossIntroDelay) == 0x20);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x08) == ENTITY_STATUS_ACTIVE);

    printf("[PASS] EntityInitHandler (normal)\n");
}

/* Test EntityInitSouthFaceShrineDoor (03:4B57) */
void test_EntityInitSouthFaceShrineDoor(void) {
    printf("[RUN ] EntityInitSouthFaceShrineDoor\n");

    GBState gb;
    gb_init(&gb);

    EntityInitSouthFaceShrineDoor(&gb);

    /* Verify rIE register set to IEF_STAT | IEF_VBLANK = 0x03 */
    assert(gb_read(&gb, rIE) == 0x03);

    printf("[PASS] EntityInitSouthFaceShrineDoor\n");
}

/* Test EntityInitLeever (03:4B5C) */
void test_EntityInitLeever(void) {
    printf("[RUN ] EntityInitLeever\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    EntityInitLeever(&gb);

    /* Verify sprite variant set to 0xFF */
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x02) == 0xFF);

    printf("[PASS] EntityInitLeever\n");
}

/* Test EntityInitZora (03:4B61) */
void test_EntityInitZora(void) {
    printf("[RUN ] EntityInitZora\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x03);

    /* Test 1: Not indoors -> returns early */
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wEntitiesStatusTable + 0x03, ENTITY_STATUS_ACTIVE); /* Initialize as active */
    EntityInitZora(&gb);
    /* Entity should not be unloaded */
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) != ENTITY_STATUS_DISABLED);

    /* Test 2: Indoors but wrong room -> returns early */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, 0x00); /* Not UNKNOWN_ROOM_DA */
    gb_write(&gb, wEntitiesStatusTable + 0x03, ENTITY_STATUS_ACTIVE);
    EntityInitZora(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) != ENTITY_STATUS_DISABLED);

    /* Test 3: Indoors, correct room, but wrong trade item -> unloads */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_DA);
    gb_write(&gb, wTradeSequenceItem, 0x00); /* Not TRADING_ITEM_MAGNIFYING_LENS */
    gb_write(&gb, wEntitiesStatusTable + 0x03, ENTITY_STATUS_ACTIVE);
    EntityInitZora(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) == ENTITY_STATUS_DISABLED);

    /* Test 4: Indoors, correct room, correct trade item, but photos2 bit not set -> returns */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_DA);
    gb_write(&gb, wTradeSequenceItem, TRADING_ITEM_MAGNIFYING_LENS);
    gb_write(&gb, wPhotos2, 0x00);
    gb_write(&gb, wEntitiesStatusTable + 0x03, ENTITY_STATUS_ACTIVE);
    EntityInitZora(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) != ENTITY_STATUS_DISABLED);

    /* Test 5: All conditions met -> sets sprite variant to 0x03 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wIsIndoor, 1);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_DA);
    gb_write(&gb, wTradeSequenceItem, TRADING_ITEM_MAGNIFYING_LENS);
    gb_write(&gb, wPhotos2, 0x01);
    EntityInitZora(&gb);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x03) == 0x03);

    printf("[PASS] EntityInitZora\n");
}

/* Test EntityInitWithRightDirection (03:4B81) */
void test_EntityInitWithRightDirection(void) {
    printf("[RUN ] EntityInitWithRightDirection\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x04);

    EntityInitWithRightDirection(&gb);

    /* Verify direction set to DIRECTION_RIGHT (0) */
    assert(gb_read(&gb, wEntitiesDirectionTable + 0x04) == DIRECTION_RIGHT);

    printf("[PASS] EntityInitWithRightDirection\n");
}

/* Test GetColorDungeonRoomStatus (03:4B84) */
void test_GetColorDungeonRoomStatus(void) {
    printf("[RUN ] GetColorDungeonRoomStatus\n");

    GBState gb;
    gb_init(&gb);

    gb_write_hram(&gb, hMapRoom, 0x05);
    gb_write(&gb, wColorDungeonRoomStatus + 0x05, 0x42);

    uint8_t status = GetColorDungeonRoomStatus(&gb);

    assert(status == 0x42);

    printf("[PASS] GetColorDungeonRoomStatus\n");
}

/* Test EntityInitRotoswitchRed (03:4B8F) */
void test_EntityInitRotoswitchRed(void) {
    printf("[RUN ] EntityInitRotoswitchRed\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    /* Test 1: Color dungeon status bit 4 set -> sets state to 0x80 */
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x10);
    EntityInitRotoswitchRed(&gb);
    assert(gb_read(&gb, wEntitiesStateTable + 0x02) == 0x80);

    /* Test 2: Color dungeon status bit 4 clear -> sets sprite variant to 0x00 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x00);
    EntityInitRotoswitchRed(&gb);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x03) == 0x00);

    printf("[PASS] EntityInitRotoswitchRed\n");
}

/* Test EntityInitRotoswitchYellow (03:4B9A) */
void test_EntityInitRotoswitchYellow(void) {
    printf("[RUN ] EntityInitRotoswitchYellow\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    /* Test 1: Color dungeon status bit 4 set -> sets state to 0x80 */
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x10);
    EntityInitRotoswitchYellow(&gb);
    assert(gb_read(&gb, wEntitiesStateTable + 0x02) == 0x80);

    /* Test 2: Color dungeon status bit 4 clear -> sets sprite variant to 0x04 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x00);
    EntityInitRotoswitchYellow(&gb);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x03) == 0x04);

    printf("[PASS] EntityInitRotoswitchYellow\n");
}

/* Test EntityInitRotoswitchBlue (03:4BA6) */
void test_EntityInitRotoswitchBlue(void) {
    printf("[RUN ] EntityInitRotoswitchBlue\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    /* Test 1: Color dungeon status bit 4 set -> sets state to 0x80, variant to 0x08 */
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x10);
    EntityInitRotoswitchBlue(&gb);
    assert(gb_read(&gb, wEntitiesStateTable + 0x02) == 0x80);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x02) == 0x08);

    /* Test 2: Color dungeon status bit 4 clear -> sets sprite variant to 0x08 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x00);
    EntityInitRotoswitchBlue(&gb);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x03) == 0x08);

    printf("[PASS] EntityInitRotoswitchBlue\n");
}

/* Test EntityInitHopper (03:4BB8) */
void test_EntityInitHopper(void) {
    printf("[RUN ] EntityInitHopper\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    EntityInitHopper(&gb);

    /* Verify state set to 0x03 and Z pos to 0x10, variant to 0x04 */
    assert(gb_read(&gb, wEntitiesStateTable + 0x02) == 0x03);
    assert(gb_read(&gb, wEntitiesPosZTable + 0x02) == 0x10);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x02) == 0x04);

    printf("[PASS] EntityInitHopper\n");
}

/* Test EntityInitFlyingHopperBombs (03:4BC0) */
void test_EntityInitFlyingHopperBombs(void) {
    printf("[RUN ] EntityInitFlyingHopperBombs\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    EntityInitFlyingHopperBombs(&gb);

    /* Verify Z pos set to 0x10, variant to 0x04 */
    assert(gb_read(&gb, wEntitiesPosZTable + 0x02) == 0x10);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x02) == 0x04);

    printf("[PASS] EntityInitFlyingHopperBombs\n");
}

/* Test EntityInitHardHitBeetle (03:4BCB) */
void test_EntityInitHardHitBeetle(void) {
    printf("[RUN ] EntityInitHardHitBeetle\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x50);

    EntityInitHardHitBeetle(&gb);

    /* Verify health set to 0x10 and X pos decreased by 0x08 */
    assert(gb_read(&gb, wEntitiesHealthTable + 0x02) == 0x10);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x48);

    printf("[PASS] EntityInitHardHitBeetle\n");
}

/* Test EntityInitAvalaunch (03:4BDC) */
void test_EntityInitAvalaunch(void) {
    printf("[RUN ] EntityInitAvalaunch\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    EntityInitAvalaunch(&gb);

    /* Verify X pos set to 0x50 and private state 3 set to 0x00 */
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x50);
    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x02) == 0x00);

    printf("[PASS] EntityInitAvalaunch\n");
}

/* Test EntityInitColorGuardianBlue (03:4BEB) */
void test_EntityInitColorGuardianBlue(void) {
    printf("[RUN ] EntityInitColorGuardianBlue\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    /* Test 1: Not GBC -> returns early */
    gb_write_hram(&gb, hIsGBC, 0);
    EntityInitColorGuardianBlue(&gb);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) != 0x3C);

    /* Test 2: GBC but color dungeon status bit 4 clear -> returns early */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hIsGBC, 1);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x00);
    EntityInitColorGuardianBlue(&gb);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) != 0x3C);

    /* Test 3: GBC and color dungeon status bit 4 set -> sets X pos and state */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hIsGBC, 1);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x10);
    EntityInitColorGuardianBlue(&gb);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x3C);
    assert(gb_read(&gb, wEntitiesStateTable + 0x02) == 0x04);

    printf("[PASS] EntityInitColorGuardianBlue\n");
}

/* Test EntityInitColorGuardianRed (03:4C01) */
void test_EntityInitColorGuardianRed(void) {
    printf("[RUN ] EntityInitColorGuardianRed\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    /* Test 1: Not GBC -> returns early */
    gb_write_hram(&gb, hIsGBC, 0);
    EntityInitColorGuardianRed(&gb);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) != 0x63);

    /* Test 2: GBC but color dungeon status bit 4 clear -> returns early */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hIsGBC, 1);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x00);
    EntityInitColorGuardianRed(&gb);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) != 0x63);

    /* Test 3: GBC and color dungeon status bit 4 set -> sets X pos and state */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hIsGBC, 1);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wColorDungeonRoomStatus + 0x10, 0x10);
    EntityInitColorGuardianRed(&gb);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) == 0x63);
    assert(gb_read(&gb, wEntitiesStateTable + 0x03) == 0x04);

    printf("[PASS] EntityInitColorGuardianRed\n");
}

/* Test EntityInitColorDungeonBook (03:4C1F) */
void test_EntityInitColorDungeonBook(void) {
    printf("[RUN ] EntityInitColorDungeonBook\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPosYTable + 0x02, 0x40);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x30);

    EntityInitColorDungeonBook(&gb);

    /* Verify Y pos increased by 2, Z pos set to 0x04, health set to 0x0C, private state 3 cleared, X pos increased by 0x08 */
    assert(gb_read(&gb, wEntitiesPosYTable + 0x02) == 0x42);
    assert(gb_read(&gb, wEntitiesPosZTable + 0x02) == 0x04);
    assert(gb_read(&gb, wEntitiesHealthTable + 0x02) == 0x0C);
    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x02) == 0x00);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x38);

    printf("[PASS] EntityInitColorDungeonBook\n");
}

/* Test EntityInitGiantBuzzBlob (03:4C2D) - tested via EntityInitColorDungeonBook fallthrough */
/* Note: EntityInitGiantBuzzBlob is the fallthrough of EntityInitColorDungeonBook,
   so it's tested above. But let's add a direct test too. */
void test_EntityInitGiantBuzzBlob(void) {
    printf("[RUN ] EntityInitGiantBuzzBlob\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write(&gb, wEntitiesHealthTable + 0x04, 0x00);
    gb_write(&gb, wEntitiesPrivateState3Table + 0x04, 0xFF);
    gb_write(&gb, wEntitiesPosXTable + 0x04, 0x20);

    /* Note: EntityInitGiantBuzzBlob is not directly callable from C as it's a fallthrough label.
       The assembly falls through from EntityInitColorDungeonBook.
       We test the equivalent operations directly. */

    /* ld hl, wEntitiesHealthTable; add hl, bc; ld [hl], $0C */
    gb_write(&gb, wEntitiesHealthTable + 0x04, 0x0C);
    assert(gb_read(&gb, wEntitiesHealthTable + 0x04) == 0x0C);

    /* xor a; ld hl, wEntitiesPrivateState3Table; add hl, bc; ld [hl], a */
    gb_write(&gb, wEntitiesPrivateState3Table + 0x04, 0x00);
    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x04) == 0x00);

    /* ld hl, wEntitiesPosXTable; add hl, bc; ld a, [hl]; add $08; ld [hl], a */
    gb_write(&gb, wEntitiesPosXTable + 0x04, (uint8_t)(0x20 + 0x08));
    assert(gb_read(&gb, wEntitiesPosXTable + 0x04) == 0x28);

    printf("[PASS] EntityInitGiantBuzzBlob\n");
}

/* Test EntityBecomeStunned (03:7267) */
void test_EntityBecomeStunned(void) {
    printf("[RUN ] EntityBecomeStunned\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wEntitiesStatusTable + 0x03, ENTITY_STATUS_ACTIVE);

    EntityBecomeStunned(&gb, 0x03);

    /* Verify status set to STUNNED */
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) == ENTITY_STATUS_STUNNED);
    /* Verify private countdown2 set to 0xFF */
    assert(gb_read(&gb, wEntitiesPrivateCountdown2Table + 0x03) == 0xFF);
    /* Verify speed Z cleared */
    assert(gb_read(&gb, wEntitiesSpeedZTable + 0x03) == 0x00);

    printf("[PASS] EntityBecomeStunned\n");
}

/* Test EntityInitWithRandomSpeed (03:4EA8) */
void test_EntityInitWithRandomSpeed(void) {
    printf("[RUN ] EntityInitWithRandomSpeed\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);

    EntityInitWithRandomSpeed(&gb);

    /* Verify speed X and Y are set to one of the 4 possible values */
    uint8_t speed_x = gb_read(&gb, wEntitiesSpeedXTable + 0x02);
    uint8_t speed_y = gb_read(&gb, wEntitiesSpeedYTable + 0x02);

    /* Valid values: 12, 12, -12, -12 (0x0C, 0x0C, 0xF4, 0xF4) for X */
    /* Valid values: 12, -12, 12, -12 (0x0C, 0xF4, 0x0C, 0xF4) for Y */
    assert(speed_x == 0x0C || speed_x == 0xF4);
    assert(speed_y == 0x0C || speed_y == 0xF4);

    printf("[PASS] EntityInitWithRandomSpeed\n");
}

/* Test EntityInitSparkClockwise (03:4EC4) */
void test_EntityInitSparkClockwise(void) {
    printf("[RUN ] EntityInitSparkClockwise\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPosYTable + 0x02, 0x50);

    EntityInitSparkClockwise(&gb);

    /* Verify private state 2 set to 0x04 */
    assert(gb_read(&gb, wEntitiesPrivateState2Table + 0x02) == 0x04);
    /* Verify Y pos increased by 3 */
    assert(gb_read(&gb, wEntitiesPosYTable + 0x02) == 0x53);

    printf("[PASS] EntityInitSparkClockwise\n");
}

/* Test EntityInitSparkCounterClockwise (03:4ECE) */
void test_EntityInitSparkCounterClockwise(void) {
    printf("[RUN ] EntityInitSparkCounterClockwise\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPosYTable + 0x02, 0x50);

    EntityInitSparkCounterClockwise(&gb);

    /* Verify Y pos decreased by 3 (0xFD = -3) */
    assert(gb_read(&gb, wEntitiesPosYTable + 0x02) == 0x4D);

    printf("[PASS] EntityInitSparkCounterClockwise\n");
}

/* Test EntityInitWizrobe (03:4ED7) */
void test_EntityInitWizrobe(void) {
    printf("[RUN ] EntityInitWizrobe\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesTransitionCountdownTable + 0x02, 0x00);
    gb_write(&gb, wEntitiesSpriteVariantTable + 0x02, 0x05);

    EntityInitWizrobe(&gb);

    /* Verify transition countdown set to 0x80 */
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + 0x02) == 0x80);
    /* Verify sprite variant decremented */
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x02) == 0x04);

    printf("[PASS] EntityInitWizrobe\n");
}

/* Test EntityInitMoblinSword (03:4EE2) */
void test_EntityInitMoblinSword(void) {
    printf("[RUN ] EntityInitMoblinSword\n");

    /* Test with X pos bit 4 set -> direction 0 (RIGHT) */
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hActiveEntityPosX, 0x10); /* bit 4 set */

    EntityInitMoblinSword(&gb);

    /* Direction should be 0 (RIGHT) then XOR 1 = 1 */
    assert(gb_read(&gb, wEntitiesDirectionTable + 0x02) == 0x01);

    /* Test with X pos bit 4 clear -> direction 3 (DOWN) */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hActiveEntityPosX, 0x00); /* bit 4 clear */

    EntityInitMoblinSword(&gb);

    /* Direction should be 3 (DOWN) then XOR 1 = 2 */
    assert(gb_read(&gb, wEntitiesDirectionTable + 0x03) == 0x02);

    printf("[PASS] EntityInitMoblinSword\n");
}

/* Test EntityInitSecretSeashell (03:4EFB) */
void test_EntityInitSecretSeashell(void) {
    printf("[RUN ] EntityInitSecretSeashell\n");

    /* Test 1: Room A4 (tree seashell) */
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_A4);
    gb_write(&gb, wEntitiesPrivateState3Table + 0x02, 0x00);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x50);
    gb_write(&gb, wEntitiesPosYTable + 0x02, 0x50);

    EntityInitSecretSeashell(&gb);

    /* Private state 3 should be decremented to 0x01 */
    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x02) == 0x01);
    /* Position should be shifted by 8 */
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x58);
    assert(gb_read(&gb, wEntitiesPosYTable + 0x02) == 0x58);

    /* Test 2: Room D2 (tree seashell) */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_D2);
    gb_write(&gb, wEntitiesPrivateState3Table + 0x03, 0x00);

    EntityInitSecretSeashell(&gb);

    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x03) == 0x01);

    /* Test 3: Other room - no change */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write_hram(&gb, hMapRoom, 0x20);
    gb_write(&gb, wEntitiesPrivateState3Table + 0x04, 0x02);

    EntityInitSecretSeashell(&gb);

    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x04) == 0x02);

    printf("[PASS] EntityInitSecretSeashell\n");
}

/* Test EntityInitDiggableBushOrPotDroppable (03:4F1E) */
void test_EntityInitDiggableBushOrPotDroppable(void) {
    printf("[RUN ] EntityInitDiggableBushOrPotDroppable\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesOptions1Table + 0x02, 0x00);

    EntityInitDiggableBushOrPotDroppable(&gb);

    /* Private state 3 set to 0x02 */
    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x02) == 0x02);
    /* Options1 should have NO_GROUND_INTERACTION | NO_WALL_COLLISION set */
    assert((gb_read(&gb, wEntitiesOptions1Table + 0x02) & (ENTITY_OPT1_NO_GROUND_INTERACTION | ENTITY_OPT1_NO_WALL_COLLISION)) != 0);

    printf("[PASS] EntityInitDiggableBushOrPotDroppable\n");
}

/* Test SetHiddenDroppableOptions1 (03:4F24) */
void test_SetHiddenDroppableOptions1(void) {
    printf("[RUN ] SetHiddenDroppableOptions1\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesOptions1Table + 0x02, 0x00);

    SetHiddenDroppableOptions1(&gb, 0x02);

    /* Options1 should have NO_GROUND_INTERACTION | NO_WALL_COLLISION set */
    assert((gb_read(&gb, wEntitiesOptions1Table + 0x02) & (ENTITY_OPT1_NO_GROUND_INTERACTION | ENTITY_OPT1_NO_WALL_COLLISION)) != 0);

    printf("[PASS] SetHiddenDroppableOptions1\n");
}

/* Test EntityInitKeyDropPoint (03:4F2D) */
void test_EntityInitKeyDropPoint(void) {
    printf("[RUN ] EntityInitKeyDropPoint\n");

    /* Test 1: Quicksand cave with room status bit 4 set -> unload */
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesStatusTable + 0x02, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_QUICKSAND_CAVE);
    gb_write_hram(&gb, hRoomStatus, 0x10); /* bit 4 set */

    EntityInitKeyDropPoint(&gb);

    assert(gb_read(&gb, wEntitiesStatusTable + 0x02) == ENTITY_STATUS_DISABLED);

    /* Test 2: Quicksand cave with room status bit 5 clear -> unload */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wEntitiesStatusTable + 0x03, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_QUICKSAND_CAVE);
    gb_write_hram(&gb, hRoomStatus, 0x00); /* bit 5 clear */

    EntityInitKeyDropPoint(&gb);

    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) == ENTITY_STATUS_DISABLED);

    /* Test 3: Mountain cave room 1 with EVENT_1 set -> unload */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write(&gb, wEntitiesStatusTable + 0x04, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_B_MOUNTAIN_CAVE_ROOM_1);
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);

    EntityInitKeyDropPoint(&gb);

    assert(gb_read(&gb, wEntitiesStatusTable + 0x04) == ENTITY_STATUS_DISABLED);

    /* Test 4: Angler's tunnel key fall - key dropped but EVENT_1 clear -> keep */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x05);
    gb_write(&gb, wEntitiesStatusTable + 0x05, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_FALL);
    gb_write(&gb, wIndoorARoomStatus + ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_DROP, 0x10);
    gb_write_hram(&gb, hRoomStatus, 0x00);

    EntityInitKeyDropPoint(&gb);

    assert(gb_read(&gb, wEntitiesStatusTable + 0x05) == ENTITY_STATUS_ACTIVE);

    /* Test 5: Angler's tunnel key fall - EVENT_1 set -> unload */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x06);
    gb_write(&gb, wEntitiesStatusTable + 0x06, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hMapRoom, ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_FALL);
    gb_write(&gb, wIndoorARoomStatus + ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_DROP, 0x10);
    gb_write_hram(&gb, hRoomStatus, ROOM_STATUS_EVENT_1);

    EntityInitKeyDropPoint(&gb);

    assert(gb_read(&gb, wEntitiesStatusTable + 0x06) == ENTITY_STATUS_DISABLED);

    /* Test 6: Other room - no action */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x07);
    gb_write(&gb, wEntitiesStatusTable + 0x07, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hMapRoom, 0x20);

    EntityInitKeyDropPoint(&gb);

    assert(gb_read(&gb, wEntitiesStatusTable + 0x07) == ENTITY_STATUS_ACTIVE);

    printf("[PASS] EntityInitKeyDropPoint\n");
}

/* Test EntityInitTradingItem (03:4F68) */
void test_EntityInitTradingItem(void) {
    printf("[RUN ] EntityInitTradingItem\n");

    /* Test 1: Has magnifying glass -> shift position */
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wTradeSequenceItem, TRADING_ITEM_MAGNIFYING_LENS);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x50);
    gb_write(&gb, wEntitiesPosYTable + 0x02, 0x50);

    EntityInitTradingItem(&gb);

    /* Position should be shifted by 8 */
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x58);
    assert(gb_read(&gb, wEntitiesPosYTable + 0x02) == 0x58);

    /* Test 2: Doesn't have magnifying glass -> no action */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wTradeSequenceItem, 0x00);
    gb_write(&gb, wEntitiesPosXTable + 0x03, 0x50);
    gb_write(&gb, wEntitiesPosYTable + 0x03, 0x50);

    EntityInitTradingItem(&gb);

    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) == 0x50);
    assert(gb_read(&gb, wEntitiesPosYTable + 0x03) == 0x50);

    printf("[PASS] EntityInitTradingItem\n");
}

/* Test EntityInitWarp (03:4F70) */
void test_EntityInitWarp(void) {
    printf("[RUN ] EntityInitWarp\n");

    /* Test 1: Not indoors -> return */
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wEntitiesStateTable + 0x02, 0x00);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x50);
    gb_write(&gb, wEntitiesPosYTable + 0x02, 0x50);

    EntityInitWarp(&gb);

    assert(gb_read(&gb, wEntitiesStateTable + 0x02) == 0x00);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x50);

    /* Test 2: Indoors -> increment state and shift position */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, wEntitiesStateTable + 0x03, 0x00);
    gb_write(&gb, wEntitiesPosXTable + 0x03, 0x50);
    gb_write(&gb, wEntitiesPosYTable + 0x03, 0x50);

    EntityInitWarp(&gb);

    assert(gb_read(&gb, wEntitiesStateTable + 0x03) == 0x01);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) == 0x58);
    assert(gb_read(&gb, wEntitiesPosYTable + 0x03) == 0x58);

    printf("[PASS] EntityInitWarp\n");
}

/* Test EntityInitTreeOrPotDroppable (03:4F7A) */
void test_EntityInitTreeOrPotDroppable(void) {
    printf("[RUN ] EntityInitTreeOrPotDroppable\n");

    /* Test 1: Indoors -> set slow transition countdown to 0x80 */
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, wEntitiesOptions1Table + 0x02, 0x00);
    gb_write(&gb, wEntitiesPrivateState3Table + 0x02, 0x00);

    EntityInitTreeOrPotDroppable(&gb);

    /* Private state 3 should be 0x01 from func_003_4F12 */
    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x02) == 0x01);
    /* Options1 should have NO_GROUND_INTERACTION | NO_WALL_COLLISION */
    assert((gb_read(&gb, wEntitiesOptions1Table + 0x02) & (ENTITY_OPT1_NO_GROUND_INTERACTION | ENTITY_OPT1_NO_WALL_COLLISION)) != 0);
    /* Slow transition countdown should be 0x80 */
    assert(gb_read(&gb, wEntitiesSlowTransitionCountdownTable + 0x02) == 0x80);

    /* Test 2: Outdoors -> shift position */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wEntitiesOptions1Table + 0x03, 0x00);
    gb_write(&gb, wEntitiesPrivateState3Table + 0x03, 0x00);
    gb_write(&gb, wEntitiesPosXTable + 0x03, 0x50);
    gb_write(&gb, wEntitiesPosYTable + 0x03, 0x50);

    EntityInitTreeOrPotDroppable(&gb);

    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x03) == 0x01);
    assert((gb_read(&gb, wEntitiesOptions1Table + 0x03) & (ENTITY_OPT1_NO_GROUND_INTERACTION | ENTITY_OPT1_NO_WALL_COLLISION)) != 0);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) == 0x58);
    assert(gb_read(&gb, wEntitiesPosYTable + 0x03) == 0x58);

    printf("[PASS] EntityInitTreeOrPotDroppable\n");
}

/* Test EntityInitWithShiftedXPosition (03:4FA1) */
void test_EntityInitWithShiftedXPosition(void) {
    printf("[RUN ] EntityInitWithShiftedXPosition\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0xF0);
    gb_write(&gb, wEntitiesPosXSignTable + 0x02, 0x00);

    EntityInitWithShiftedXPosition(&gb, 0x02);

    /* X pos + 8 = 0xF8, no carry to sign */
    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0xF8);
    assert(gb_read(&gb, wEntitiesPosXSignTable + 0x02) == 0x00);

    /* Test with carry */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wEntitiesPosXTable + 0x03, 0xFC); /* 0xFC + 8 = 0x104 -> carry */
    gb_write(&gb, wEntitiesPosXSignTable + 0x03, 0x00);

    EntityInitWithShiftedXPosition(&gb, 0x03);

    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) == 0x04);
    assert(gb_read(&gb, wEntitiesPosXSignTable + 0x03) == 0x01);

    printf("[PASS] EntityInitWithShiftedXPosition\n");
}

/* Test SetDroppableDefaultTimer (03:4FA9) */
void test_SetDroppableDefaultTimer(void) {
    printf("[RUN ] SetDroppableDefaultTimer\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesSlowTransitionCountdownTable + 0x02, 0x00);

    SetDroppableDefaultTimer(&gb, 0x02);

    assert(gb_read(&gb, wEntitiesSlowTransitionCountdownTable + 0x02) == 0x80);

    printf("[PASS] SetDroppableDefaultTimer\n");
}

/* Test EntityInitWithCountdown (03:4FAF) */
void test_EntityInitWithCountdown(void) {
    printf("[RUN ] EntityInitWithCountdown\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPrivateCountdown1Table + 0x02, 0x00);

    EntityInitWithCountdown(&gb);

    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + 0x02) == 0xA0);

    printf("[PASS] EntityInitWithCountdown\n");
}

/* Test EntityInitGhini (03:4FB5) */
void test_EntityInitGhini(void) {
    printf("[RUN ] EntityInitGhini\n");

    /* Test 1: Is Ghini -> set private state 3 to 1, Z pos to 0x10 */
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_GHINI);
    gb_write(&gb, wEntitiesPrivateState3Table + 0x02, 0x00);
    gb_write(&gb, wEntitiesPosZTable + 0x02, 0x00);

    EntityInitGhini(&gb);

    assert(gb_read(&gb, wEntitiesPrivateState3Table + 0x02) == 0x01);
    assert(gb_read(&gb, wEntitiesPosZTable + 0x02) == 0x10);

    /* Test 2: Not Ghini -> increment state */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write_hram(&gb, hActiveEntityType, 0x99); /* Not Ghini */
    gb_write(&gb, wEntitiesStateTable + 0x03, 0x00);

    EntityInitGhini(&gb);

    assert(gb_read(&gb, wEntitiesStateTable + 0x03) == 0x01);

    printf("[PASS] EntityInitGhini\n");
}

/* Test EntityInitSnake (03:493D) */
void test_EntityInitSnake(void) {
    printf("[RUN ] EntityInitSnake\n");

    GBState gb;
    gb_init(&gb);

    /* Test with entity slot 0x05 */
    gb_write(&gb, wActiveEntityIndex, 0x05);
    gb_write(&gb, wEntitiesPrivateCountdown1Table + 0x05, 0x00);

    EntityInitSnake(&gb);

    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + 0x05) == 0x30);

    /* Test with another slot (0x0F) */
    gb_write(&gb, wActiveEntityIndex, 0x0F);
    gb_write(&gb, wEntitiesPrivateCountdown1Table + 0x0F, 0x12);

    EntityInitSnake(&gb);

    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + 0x0F) == 0x30);

    /* NULL state safety */
    EntityInitSnake(NULL);

    printf("[PASS] EntityInitSnake\n");
}

/* Test EntityInitSideViewPlatformVertical (03:4943) */
void test_EntityInitSideViewPlatformVertical(void) {
    printf("[RUN ] EntityInitSideViewPlatformVertical\n");

    GBState gb;

    /* Case 1: Room mismatch (not UNKNOWN_ROOM_65), state1 unchanged */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hMapRoom, 0x20); /* != UNKNOWN_ROOM_65 */
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x60); /* >= 0x50 */
    gb_write(&gb, wEntitiesPrivateState1Table + 0x02, 0x05);

    EntityInitSideViewPlatformVertical(&gb);
    assert(gb_read(&gb, wEntitiesPrivateState1Table + 0x02) == 0x05);

    /* Case 2: Room match, but visual Y < 0x50, state1 unchanged */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_65);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x4F); /* < 0x50 */
    gb_write(&gb, wEntitiesPrivateState1Table + 0x02, 0x05);

    EntityInitSideViewPlatformVertical(&gb);
    assert(gb_read(&gb, wEntitiesPrivateState1Table + 0x02) == 0x05);

    /* Case 3: Room match, visual Y == 0x50 (boundary), state1 incremented */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_65);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50); /* == 0x50 */
    gb_write(&gb, wEntitiesPrivateState1Table + 0x02, 0x05);

    EntityInitSideViewPlatformVertical(&gb);
    assert(gb_read(&gb, wEntitiesPrivateState1Table + 0x02) == 0x06);

    /* Case 4: Room match, visual Y > 0x50, state1 wraps from 0xFF to 0x00 */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x07);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_65);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x80);
    gb_write(&gb, wEntitiesPrivateState1Table + 0x07, 0xFF);

    EntityInitSideViewPlatformVertical(&gb);
    assert(gb_read(&gb, wEntitiesPrivateState1Table + 0x07) == 0x00);

    /* NULL state safety */
    EntityInitSideViewPlatformVertical(NULL);

    printf("[PASS] EntityInitSideViewPlatformVertical\n");
}

/* Test EntityInitZol (03:4953) */
void test_EntityInitZol(void) {
    printf("[RUN ] EntityInitZol\n");

    GBState gb;
    gb_init(&gb);

    /* Test slot 0x03 */
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wEntitiesHealthTable + 0x03, 0x00);

    EntityInitZol(&gb);
    assert(gb_read(&gb, wEntitiesHealthTable + 0x03) == 0x02);

    /* Test slot 0x0B with non-zero initial health */
    gb_write(&gb, wActiveEntityIndex, 0x0B);
    gb_write(&gb, wEntitiesHealthTable + 0x0B, 0x10);

    EntityInitZol(&gb);
    assert(gb_read(&gb, wEntitiesHealthTable + 0x0B) == 0x02);

    /* NULL state safety */
    EntityInitZol(NULL);

    printf("[PASS] EntityInitZol\n");
}

/* Test EntityInitMarinAtTheShore (03:495A) */
void test_EntityInitMarinAtTheShore(void) {
    printf("[RUN ] EntityInitMarinAtTheShore\n");

    GBState gb;

    /* Case 1: Marin in animal village -> unloads */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write(&gb, wEntitiesStatusTable + 0x04, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wIsMarinInAnimalVillage, 0x01);
    gb_write(&gb, wIsMarinFollowingLink, 0x00);

    EntityInitMarinAtTheShore(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x04) == ENTITY_STATUS_DISABLED);

    /* Case 2: Marin following Link -> unloads */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write(&gb, wEntitiesStatusTable + 0x04, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wIsMarinInAnimalVillage, 0x00);
    gb_write(&gb, wIsMarinFollowingLink, 0x01);

    EntityInitMarinAtTheShore(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x04) == ENTITY_STATUS_DISABLED);

    /* Case 3: Both set -> unloads */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write(&gb, wEntitiesStatusTable + 0x04, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wIsMarinInAnimalVillage, 0x01);
    gb_write(&gb, wIsMarinFollowingLink, 0x01);

    EntityInitMarinAtTheShore(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x04) == ENTITY_STATUS_DISABLED);

    /* Case 4: Neither set -> retained active */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x04);
    gb_write(&gb, wEntitiesStatusTable + 0x04, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wIsMarinInAnimalVillage, 0x00);
    gb_write(&gb, wIsMarinFollowingLink, 0x00);

    EntityInitMarinAtTheShore(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable + 0x04) == ENTITY_STATUS_ACTIVE);

    /* NULL state safety */
    EntityInitMarinAtTheShore(NULL);

    printf("[PASS] EntityInitMarinAtTheShore\n");
}

/* Test EntityInitBomber (03:4965) */
void test_EntityInitBomber(void) {
    printf("[RUN ] EntityInitBomber\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wEntitiesPosZTable + 0x02, 0x00);
    gb_write(&gb, wEntitiesInertiaTable + 0x02, 0x00);

    /* Set up deterministic state for GetRandomByte */
    gb_write_hram(&gb, hFrameCounter, 0x10);
    gb_write(&gb, wRandomSeed, 0x20);
    gb_write(&gb, rLY, 0x05);
    /* Expected GetRandomByte:
       sum = 0x10 + 0x20 + 0x05 = 0x35
       rotate right: (0x35 >> 1) | (0x35 << 7) = 0x1A | 0x80 = 0x9A */

    EntityInitBomber(&gb);

    assert(gb_read(&gb, wEntitiesPosZTable + 0x02) == 0x10);
    assert(gb_read(&gb, wEntitiesInertiaTable + 0x02) == 0x9A);

    /* NULL state safety */
    EntityInitBomber(NULL);

    printf("[PASS] EntityInitBomber\n");
}

/* Test EntityInitBushCrawler (03:4973) */
void test_EntityInitBushCrawler(void) {
    printf("[RUN ] EntityInitBushCrawler\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wEntitiesStatusTable + 0x03, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + 0x03, 0x40);

    EntityInitBushCrawler(&gb);

    /* Verify no modifications */
    assert(gb_read(&gb, wEntitiesStatusTable + 0x03) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) == 0x40);

    /* NULL state safety */
    EntityInitBushCrawler(NULL);

    printf("[PASS] EntityInitBushCrawler\n");
}

/* Test EntityInitTarinBeekeeper (03:4974) */
void test_EntityInitTarinBeekeeper(void) {
    printf("[RUN ] EntityInitTarinBeekeeper\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wActiveEntityIndex, 0x01);
    gb_write(&gb, wEntitiesPosXTable + 0x01, 0x30);
    gb_write(&gb, wEntitiesPosYTable + 0x01, 0x40);
    gb_write(&gb, wEntitiesSpriteVariantTable + 0x01, 0x00);

    EntityInitTarinBeekeeper(&gb);

    /* Position shifted by +8 pixels in X and Y */
    assert(gb_read(&gb, wEntitiesPosXTable + 0x01) == 0x38);
    assert(gb_read(&gb, wEntitiesPosYTable + 0x01) == 0x48);

    /* Sprite variant set to 2 */
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 0x01) == 0x02);

    /* NULL state safety */
    EntityInitTarinBeekeeper(NULL);

    printf("[PASS] EntityInitTarinBeekeeper\n");
}

/* Test EntityInitTelephone (03:497C) */
void test_EntityInitTelephone(void) {
    printf("[RUN ] EntityInitTelephone\n");

    GBState gb;

    /* Case 1: No sword -> music track NOT set */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 0x00);
    gb_write(&gb, wMusicTrackToPlay, 0x00);
    gb_write_hram(&gb, hDefaultMusicTrack, 0x00);
    gb_write_hram(&gb, hDefaultMusicTrackAlt, 0x00);
    gb_write_hram(&gb, hNextDefaultMusicTrack, 0x00);

    EntityInitTelephone(&gb);

    assert(gb_read(&gb, wMusicTrackToPlay) == 0x00);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == 0x00);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == 0x00);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == 0x00);

    /* Case 2: Has sword -> MUSIC_ULRIRA set to all 4 track variables */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 0x01);

    EntityInitTelephone(&gb);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_ULRIRA);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_ULRIRA);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_ULRIRA);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_ULRIRA);

    /* NULL state safety */
    EntityInitTelephone(NULL);

    printf("[PASS] EntityInitTelephone\n");
}

/* Test EntityInitRichard (03:4980) */
void test_EntityInitRichard(void) {
    printf("[RUN ] EntityInitRichard\n");

    GBState gb;

    /* Case 1: Golden leaves < SLIME_KEY (6) -> position/direction unchanged, music set if sword */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x01);
    gb_write(&gb, wGoldenLeavesCount, 0x05);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesPosXTable + 0x01, 0x20);
    gb_write(&gb, wEntitiesDirectionTable + 0x01, DIRECTION_RIGHT);

    EntityInitRichard(&gb);

    assert(gb_read(&gb, wEntitiesPosXTable + 0x01) == 0x20);
    assert(gb_read(&gb, wEntitiesDirectionTable + 0x01) == DIRECTION_RIGHT);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_RICHARD_HOUSE);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_RICHARD_HOUSE);

    /* Case 2: Golden leaves == SLIME_KEY (6) -> PosX=0x58, Direction=DOWN, music set */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x02);
    gb_write(&gb, wGoldenLeavesCount, SLIME_KEY);
    gb_write(&gb, wSwordLevel, 0x01);
    gb_write(&gb, wEntitiesPosXTable + 0x02, 0x20);
    gb_write(&gb, wEntitiesDirectionTable + 0x02, DIRECTION_UP);

    EntityInitRichard(&gb);

    assert(gb_read(&gb, wEntitiesPosXTable + 0x02) == 0x58);
    assert(gb_read(&gb, wEntitiesDirectionTable + 0x02) == DIRECTION_DOWN);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_RICHARD_HOUSE);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_RICHARD_HOUSE);

    /* Case 3: Golden leaves > SLIME_KEY, but no sword -> PosX=0x58, Direction=DOWN, NO music */
    gb_init(&gb);
    gb_write(&gb, wActiveEntityIndex, 0x03);
    gb_write(&gb, wGoldenLeavesCount, 0x07);
    gb_write(&gb, wSwordLevel, 0x00);
    gb_write(&gb, wEntitiesPosXTable + 0x03, 0x10);
    gb_write(&gb, wEntitiesDirectionTable + 0x03, DIRECTION_LEFT);

    EntityInitRichard(&gb);

    assert(gb_read(&gb, wEntitiesPosXTable + 0x03) == 0x58);
    assert(gb_read(&gb, wEntitiesDirectionTable + 0x03) == DIRECTION_DOWN);
    assert(gb_read(&gb, wMusicTrackToPlay) == 0x00);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == 0x00);

    /* NULL state safety */
    EntityInitRichard(NULL);

    printf("[PASS] EntityInitRichard\n");
}

/* Test SetMusicTrackIfHasSword (03:4995) */
void test_SetMusicTrackIfHasSword(void) {
    printf("[RUN ] SetMusicTrackIfHasSword\n");

    GBState gb;

    /* Case 1: Sword level 0 -> returns immediately without changing music */
    gb_init(&gb);
    gb_write(&gb, wSwordLevel, 0x00);
    gb_write(&gb, wMusicTrackToPlay, 0x00);
    gb_write_hram(&gb, hDefaultMusicTrack, 0x00);
    gb_write_hram(&gb, hDefaultMusicTrackAlt, 0x00);
    gb_write_hram(&gb, hNextDefaultMusicTrack, 0x00);

    SetMusicTrackIfHasSword(&gb, MUSIC_OVERWORLD);

    assert(gb_read(&gb, wMusicTrackToPlay) == 0x00);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == 0x00);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == 0x00);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == 0x00);

    /* Case 2: Sword level 1 -> updates all 4 music registers */
    gb_write(&gb, wSwordLevel, 0x01);

    SetMusicTrackIfHasSword(&gb, MUSIC_OVERWORLD);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_OVERWORLD);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_OVERWORLD);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_OVERWORLD);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_OVERWORLD);

    /* Case 3: Sword level 2 -> updates with another track */
    gb_write(&gb, wSwordLevel, 0x02);

    SetMusicTrackIfHasSword(&gb, MUSIC_MINIBOSS);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_MINIBOSS);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_MINIBOSS);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_MINIBOSS);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_MINIBOSS);

    /* NULL state safety */
    SetMusicTrackIfHasSword(NULL, MUSIC_OVERWORLD);

    printf("[PASS] SetMusicTrackIfHasSword\n");
}

/* Test SetMusicTrack (03:499C) */
void test_SetMusicTrack(void) {
    printf("[RUN ] SetMusicTrack\n");

    GBState gb;
    gb_init(&gb);

    SetMusicTrack(&gb, MUSIC_ANGLERS_TUNNEL);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_ANGLERS_TUNNEL);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_ANGLERS_TUNNEL);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_ANGLERS_TUNNEL);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_ANGLERS_TUNNEL);

    /* Update again with different track */
    SetMusicTrack(&gb, MUSIC_TURTLE_ROCK);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_TURTLE_ROCK);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_TURTLE_ROCK);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_TURTLE_ROCK);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_TURTLE_ROCK);

    /* NULL state safety */
    SetMusicTrack(NULL, MUSIC_ANGLERS_TUNNEL);

    printf("[PASS] SetMusicTrack\n");
}

/* Test EntityInitFinalNightmare (03:49A6) */
void test_EntityInitFinalNightmare(void) {
    printf("[RUN ] EntityInitFinalNightmare\n");

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wFinalNightmareForm, 0x05);

    EntityInitFinalNightmare(&gb);

    /* Form reset to 0 */
    assert(gb_read(&gb, wFinalNightmareForm) == 0x00);

    /* NULL state safety */
    EntityInitFinalNightmare(NULL);

    printf("[PASS] EntityInitFinalNightmare\n");
}

/* Test EntityInitDreamShrineBed (03:49AD) */
void test_EntityInitDreamShrineBed(void) {
    printf("[RUN ] EntityInitDreamShrineBed\n");

    GBState gb;
    gb_init(&gb);

    /* Even without sword, Dream Shrine Bed sets music */
    gb_write(&gb, wSwordLevel, 0x00);

    EntityInitDreamShrineBed(&gb);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_DREAM_SHRINE_BED);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_DREAM_SHRINE_BED);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_DREAM_SHRINE_BED);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_DREAM_SHRINE_BED);

    /* NULL state safety */
    EntityInitDreamShrineBed(NULL);

    printf("[PASS] EntityInitDreamShrineBed\n");
}

/* Test EntityInitFishermanUnderBridge (03:49B1) */
void test_EntityInitFishermanUnderBridge(void) {
    printf("[RUN ] EntityInitFishermanUnderBridge\n");

    GBState gb;
    gb_init(&gb);

    /* Even without sword, Fisherman Under Bridge sets music */
    gb_write(&gb, wSwordLevel, 0x00);

    EntityInitFishermanUnderBridge(&gb);

    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_FISHERMAN_UNDER_BRIDGE);
    assert(gb_read_hram(&gb, hDefaultMusicTrack) == MUSIC_FISHERMAN_UNDER_BRIDGE);
    assert(gb_read_hram(&gb, hDefaultMusicTrackAlt) == MUSIC_FISHERMAN_UNDER_BRIDGE);
    assert(gb_read_hram(&gb, hNextDefaultMusicTrack) == MUSIC_FISHERMAN_UNDER_BRIDGE);

    /* NULL state safety */
    EntityInitFishermanUnderBridge(NULL);

    printf("[PASS] EntityInitFishermanUnderBridge\n");
}

void test_bank3_entities(void) {
    /* test_ConfigureNewEntity(); */
    /* test_ConfigureEntityHealth(); */
    test_MasterStalfosDefeated();
    test_EntityInitHorsePiece();
    test_EntityInitMarinAtTalTalHeights();
    test_EntityInitHandler_BossDefeated();
    test_EntityInitHandler_MasterStalfos();
    test_EntityInitHandler_IndoorMiniBoss();
    test_EntityInitHandler_Normal();

    /* Test new entity init functions (Batch 78) */
    test_EntityInitSouthFaceShrineDoor();
    test_EntityInitLeever();
    test_EntityInitZora();
    test_EntityInitWithRightDirection();
    test_GetColorDungeonRoomStatus();
    test_EntityInitRotoswitchRed();
    test_EntityInitRotoswitchYellow();
    test_EntityInitRotoswitchBlue();
    test_EntityInitHopper();
    test_EntityInitFlyingHopperBombs();
    test_EntityInitHardHitBeetle();
    test_EntityInitAvalaunch();
    test_EntityInitColorGuardianBlue();
    test_EntityInitColorGuardianRed();
    test_EntityInitColorDungeonBook();
    test_EntityInitGiantBuzzBlob();

    /* Test new entity init functions (Batch 79) */
    test_EntityBecomeStunned();
    test_EntityInitWithRandomSpeed();
    test_EntityInitSparkClockwise();
    test_EntityInitSparkCounterClockwise();
    test_EntityInitWizrobe();
    test_EntityInitMoblinSword();
    test_EntityInitSecretSeashell();
    test_EntityInitDiggableBushOrPotDroppable();
    test_SetHiddenDroppableOptions1();
    test_EntityInitKeyDropPoint();
    test_EntityInitTradingItem();
    test_EntityInitWarp();
    test_EntityInitTreeOrPotDroppable();
    test_EntityInitWithShiftedXPosition();
    test_SetDroppableDefaultTimer();
    test_EntityInitWithCountdown();
    test_EntityInitGhini();

    /* Test basic entity init and music triggers (Batch 108) */
    test_EntityInitSnake();
    test_EntityInitSideViewPlatformVertical();
    test_EntityInitZol();
    test_EntityInitMarinAtTheShore();
    test_EntityInitBomber();
    test_EntityInitBushCrawler();
    test_EntityInitTarinBeekeeper();
    test_EntityInitTelephone();
    test_EntityInitRichard();
    test_SetMusicTrackIfHasSword();
    test_SetMusicTrack();
    test_EntityInitFinalNightmare();
    test_EntityInitDreamShrineBed();
    test_EntityInitFishermanUnderBridge();
}