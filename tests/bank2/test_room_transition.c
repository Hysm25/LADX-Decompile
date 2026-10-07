#include "test_bank2.h"
#include "test_support.h"

#include "gb.h"
#include "bank2/room_transition.h"
#include "constants/directions.h"
#include "constants/entities.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/memory.h"
#include "constants/physics.h"
#include "constants/rooms.h"
#include "constants/sfx.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/* Mock callbacks - provided by test_support.c */



/* Test ApplyRoomTransition (02:78E8-02:79D9) - Early return when NONE */
void test_ApplyRoomTransition_None(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, ROOM_TRANSITION_NONE);

    ApplyRoomTransition(&gb, mock_bg_collision, mock_create_following_npc,
                        mock_set_world_music, mock_clear_link_pos);

    /* Should return early, no callbacks called */
    assert(g_mock_bg_collision_calls == 0);
    assert(g_mock_create_following_npc_calls == 0);
    assert(g_mock_set_world_music_calls == 0);
    assert(g_mock_clear_link_pos_calls == 0);

}

/* Test ApplyRoomTransition - Dispatch to state 1 (Prepare) */
void test_ApplyRoomTransition_DispatchPrepare(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, ROOM_TRANSITION_LOAD_ROOM); /* state 1 */

    ApplyRoomTransition(&gb, mock_bg_collision, mock_create_following_npc,
                        mock_set_world_music, mock_clear_link_pos);

    /* Should dispatch to Prepare handler, which increments state to 2 */
    assert(gb_read(&gb, wRoomTransitionState) == ROOM_TRANSITION_LOAD_SPRITES);

}

/* Test ApplyRoomTransition - Scroll offset application */
void test_ApplyRoomTransition_ScrollOffset(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, ROOM_TRANSITION_FIRST_HALF); /* state 4 */
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_RIGHT);
    gb_write_hram(&gb, hBaseScrollX, 0x10);
    gb_write_hram(&gb, hBaseScrollY, 0x20);
    gb_write(&gb, wRoomTransitionTargetScrollX, 0xA0);
    gb_write(&gb, wRoomTransitionTargetScrollY, 0x00);

    ApplyRoomTransition(&gb, mock_bg_collision, mock_create_following_npc,
                        mock_set_world_music, mock_clear_link_pos);

    /* Scroll should be incremented: X += 4, Y += 0 */
    assert(gb_read_hram(&gb, hBaseScrollX) == 0x14);
    assert(gb_read_hram(&gb, hBaseScrollY) == 0x20);

    /* Link speed should be set */
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0xC6); /* RoomTransitionLinkXIncrement[RIGHT] */
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);

}

/* Test ApplyRoomTransition - Target reached completion */
void test_ApplyRoomTransition_TargetReached(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, ROOM_TRANSITION_FIRST_HALF);
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_RIGHT);
    gb_write_hram(&gb, hBaseScrollX, 0x9C); /* One step before target (0xA0 - 0x04) */
    gb_write_hram(&gb, hBaseScrollY, 0x00);
    gb_write(&gb, wRoomTransitionTargetScrollX, 0xA0);
    gb_write(&gb, wRoomTransitionTargetScrollY, 0x00);
    gb_write_hram(&gb, hNextMusicTrackToFadeInto, 0x15);
    gb_write(&gb, wNextJingle, JINGLE_PUZZLE_SOLVED);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wFreeMovementMode, 0);
    gb_write(&gb, wCollisionType, 0);
    gb_write_hram(&gb, hObjectUnderEntity, 0x00);

ApplyRoomTransition(&gb, mock_bg_collision, mock_create_following_npc,
                        mock_set_world_music, mock_clear_link_pos);

    /* State should be cleared */
    assert(gb_read(&gb, wRoomTransitionState) == ROOM_TRANSITION_NONE);

    /* Velocity Z cleared */
    assert(gb_read_hram(&gb, hLinkVelocityZ) == 0);

    /* Position saved - direct WRAM access since gb_read doesn't support WRAM */
    /* Position is updated by UpdateFinalLinkPosition before saving.
     * For RIGHT: speed = 0xC6 (-4), position decreases by 4: 0x50 -> 0x4C
     * For Y: speed = 0x00, no change: 0x40 -> 0x40 */
    assert(gb.wram[gb.wram_bank][wLinkMapEntryPositionX - 0xD000] == 0x4C);
    assert(gb.wram[gb.wram_bank][wLinkMapEntryPositionY - 0xD000] == 0x40);

    /* Music track changed */
    assert(g_mock_set_world_music_calls == 1);
    assert(gb_read_hram(&gb, hNextMusicTrackToFadeInto) == 0);

    /* Jingle played */
    assert(gb_read_hram(&gb, hJingle) == JINGLE_PUZZLE_SOLVED);
    assert(gb_read(&gb, wNextJingle) == 0);

    /* Following NPC created */
    assert(g_mock_create_following_npc_calls == 1);

    /* Animated tiles frame reset */
    assert(gb_read_hram(&gb, hAnimatedTilesFrameCount) == 0xFF);

    /* Clear link position increment called */
    assert(g_mock_clear_link_pos_calls == 1);

}

/* Test ApplyRoomTransition - Bottom direction ledge jump */
void test_ApplyRoomTransition_BottomLedge(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, ROOM_TRANSITION_FIRST_HALF);
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_DOWN);
    gb_write_hram(&gb, hBaseScrollX, 0x00);
    gb_write_hram(&gb, hBaseScrollY, 0x7C); /* One step before target (0x80 - 0x04) */
    gb_write(&gb, wRoomTransitionTargetScrollX, 0x00);
    gb_write(&gb, wRoomTransitionTargetScrollY, 0x80);
    gb_write(&gb, wFreeMovementMode, 0);
    gb_write_hram(&gb, hObjectUnderEntity, 0x00);
    gb_write(&gb, wCollisionType, 0);

    ApplyRoomTransition(&gb, mock_bg_collision, NULL, NULL, NULL);

    /* Should set hLinkSpeedY = 1 for ledge jump */
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x01);
    /* Should call bg collision */
    assert(g_mock_bg_collision_calls == 1);

}

/* Test RoomTransitionPrepareHandler (02:79FA-02:7ADB) - Indoor room increment */
void test_RoomTransitionPrepareHandler_Indoor(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 2); /* state 1 = prepare */
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_RIGHT);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, wIndoorRoom, 0x10);
    gb_write_hram(&gb, hMapId, MAP_TAIL_CAVE);
    gb_write_hram(&gb, hMapRoom, 0x10);
    gb_write(&gb, wEggMazeProgress, 0);
    gb_write(&gb, wWindFishEggMazeSequenceOffset, 0);

    RoomTransitionPrepareHandler(&gb, mock_load_room, mock_replace_objects,
                                 mock_load_entities, mock_draw_link,
                                 mock_apply_motion, mock_select_tilesets,
                                 mock_reset_music_fade);

    /* Indoor room should increment by 1 (right) */
    assert(gb_read(&gb, wIndoorRoom) == 0x11);

    /* State should increment */
    assert(gb_read(&gb, wRoomTransitionState) == 3);

    /* Room load called */
    assert(g_mock_load_room_calls == 1);

}

/* Test RoomTransitionPrepareHandler - Face Shrine room $1D hack */
void test_RoomTransitionPrepareHandler_FaceShrineHack(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 2);
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_UP);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, wIndoorRoom, 0x1D);
    gb_write_hram(&gb, hMapId, MAP_FACE_SHRINE);
    gb_write_hram(&gb, hMapRoom, 0x1D);

    RoomTransitionPrepareHandler(&gb, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    /* Face Shrine hack: pretends room is 0x35, then applies increment (0xF8 for UP)
     * 0x35 + 0xF8 = 0x2D */
    assert(gb_read(&gb, wIndoorRoom) == 0x2D);

}

/* Test RoomTransitionPrepareHandler - Overworld Mysterious Woods lost */
void test_RoomTransitionPrepareHandler_MysteriousWoods(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 2);
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_UP);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wShouldGetLostInMysteriousWoods, 1);
    gb_write_hram(&gb, hMapRoom, 0x20);

    RoomTransitionPrepareHandler(&gb, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    /* Should force room $63 */
    assert(gb_read_hram(&gb, hMapRoom) == UNKNOWN_ROOM_63);
    /* Should set forest lost jingle */
    assert(gb_read(&gb, wNextJingle) == JINGLE_FOREST_LOST);

}

/* Test RoomTransitionPrepareHandler - Tail Cave key room visited */
void test_RoomTransitionPrepareHandler_TailCaveKey(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 2);
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_UP);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wShouldGetLostInMysteriousWoods, 0);
    gb_write_hram(&gb, hMapRoom, 0x51); /* Room $51, will increment to $41 (0x51 + 0xF0 = 0x141 -> 0x41) */
    gb_write(&gb, wOverworldRoomStatus + UNKNOWN_ROOM_41, 0x00); /* Not visited */

    RoomTransitionPrepareHandler(&gb, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    /* Room should increment to $41 */
    assert(gb_read_hram(&gb, hMapRoom) == UNKNOWN_ROOM_41);
    /* Should mark visited (bit 6) */
    assert((gb_read(&gb, wOverworldRoomStatus + UNKNOWN_ROOM_41) & 0x40) != 0);
    /* Should set puzzle solved jingle */
    assert(gb_read(&gb, wNextJingle) == JINGLE_PUZZLE_SOLVED);

}

/* Test RoomTransitionLoadTiles (02:7B3E-02:7B4B) */
void test_RoomTransitionLoadTiles(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 3);
    gb_write(&gb, wRoomSwitchableObject, ROOM_SWITCHABLE_OBJECT_MOBILE_BLOCK);

    RoomTransitionLoadTiles(&gb, mock_select_tilesets);

    /* Should call SelectRoomTilesets */
    assert(g_mock_select_tilesets_calls == 1);

    /* Should set hSwitchBlockNeedingUpdate = 2 */
    assert(gb_read_hram(&gb, hSwitchBlockNeedingUpdate) == 0x02);

    /* State should increment */
    assert(gb_read(&gb, wRoomTransitionState) == 4);

}

/* Test RoomTransitionConfigureScrollTargets (02:7B7F-02:7BFC) */
void test_RoomTransitionConfigureScrollTargets(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 4);
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_RIGHT);
    gb_write(&gb, wRoomTransitionTargetScrollX, 0x00);
    gb_write(&gb, wRoomTransitionTargetScrollY, 0x00);
    gb_write(&gb, wBGOriginLow, 0x10);
    gb_write(&gb, wBGOriginHigh, 0x01);
    gb_write_hram(&gb, hSwitchBlockNeedingUpdate, 0x00);

    RoomTransitionConfigureScrollTargets(&gb);

    /* Target scroll X = 0x00 + 0xA0 = 0xA0 */
    assert(gb_read(&gb, wRoomTransitionTargetScrollX) == 0xA0);
    /* Target scroll Y = 0x00 + 0x00 = 0x00 */
    assert(gb_read(&gb, wRoomTransitionTargetScrollY) == 0x00);

    /* BG update region origin */
    assert(gb_read(&gb, wBGUpdateRegionOriginLow) == 0x24); /* 0x10 + 0x14 */
    assert(gb_read(&gb, wBGUpdateRegionOriginHigh) == 0x01); /* 0x01 + 0x00 */

    /* BG origin after transition */
    assert(gb_read(&gb, wBGOriginLow) == 0x24);
    assert(gb_read(&gb, wBGOriginHigh) == 0x01);

    /* Tiles count */
    assert(gb_read(&gb, wBGUpdateRegionTilesCount) == 0x0F); /* TILES_PER_COLUMN - 1 */

    /* Frames to mid-screen */
    assert(gb_read(&gb, wRoomTransitionFramesBeforeMidScreen) == 0x14);

    /* Transition offset */
    assert(gb_read(&gb, wTransitionOffset) == 0x00);

    /* wTransitionZeroNeverUsed cleared */
    assert(gb_read(&gb, wTransitionZeroNeverUsed) == 0x00);

    /* State incremented */
    assert(gb_read(&gb, wRoomTransitionState) == 5);

}

/* Test RoomTransitionConfigureScrollTargets - Early return when switch block needs update */
void test_RoomTransitionConfigureScrollTargets_SwitchBlock(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 4);
    gb_write(&gb, wRoomTransitionDirection, DIRECTION_RIGHT);
    gb_write_hram(&gb, hSwitchBlockNeedingUpdate, 0x02);

    RoomTransitionConfigureScrollTargets(&gb);

    /* Should return early, state NOT incremented */
    assert(gb_read(&gb, wRoomTransitionState) == 4);

}

/* Test RoomTransitionFirstHalfHandler (02:7C00-02:7C02) */
void test_RoomTransitionFirstHalfHandler(void) {

    GBState gb;
    gb_init(&gb);

    RoomTransitionFirstHalfHandler(&gb, mock_update_bg_region,
                                   NULL, NULL, NULL, NULL, NULL);

    /* Should call update_bg_region */
    assert(g_mock_update_bg_region_calls == 1);

}

/* Test RoomTransitionSecondHalfHandler (02:7C03) - No-op */
void test_RoomTransitionSecondHalfHandler(void) {

    GBState gb;
    gb_init(&gb);

    /* Should do nothing */
    RoomTransitionSecondHalfHandler(&gb);

}

/* Test label_002_7C14 - Conveyor belt physics (02:7C14-02:7C3F) */
void test_label_002_7C14(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wLinkObjectPhysics, OBJ_PHYSICS_CONVEYOR + 0); /* Right conveyor */
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write_hram(&gb, hFrameCounter, 0x00); /* Frame counter & 3 == 0 */
    gb_write(&gb, wC167, 0x00);
    gb_write_hram(&gb, hLinkInteractiveMotionBlocked, 0x00);
    gb_write(&gb, wDialogGotItem, 0x00);

    label_002_7C14(&gb);

    /* Data_002_7C04[0] = 0x00, Data_002_7C0C[0] = 0x01 */
    assert(gb_read_hram(&gb, hLinkPositionX) == 0x50);
    assert(gb_read_hram(&gb, hLinkPositionY) == 0x41);

    /* Test left conveyor (index 1) */
    gb_init(&gb);
    gb_write(&gb, wLinkObjectPhysics, OBJ_PHYSICS_CONVEYOR + 1);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write_hram(&gb, hFrameCounter, 0x00);
    gb_write(&gb, wC167, 0x00);
    gb_write_hram(&gb, hLinkInteractiveMotionBlocked, 0x00);
    gb_write(&gb, wDialogGotItem, 0x00);

    label_002_7C14(&gb);

    /* Data_002_7C04[1] = 0x00, Data_002_7C0C[1] = 0xFF (-1) */
    assert(gb_read_hram(&gb, hLinkPositionX) == 0x50);
    assert(gb_read_hram(&gb, hLinkPositionY) == 0x3F);

    /* Test early return when frame counter & 3 != 0 */
    gb_init(&gb);
    gb_write(&gb, wLinkObjectPhysics, OBJ_PHYSICS_CONVEYOR + 0);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write_hram(&gb, hFrameCounter, 0x01); /* Frame counter & 3 != 0 */

    label_002_7C14(&gb);

    /* Position should not change */
    assert(gb_read_hram(&gb, hLinkPositionX) == 0x50);
    assert(gb_read_hram(&gb, hLinkPositionY) == 0x40);

}

/* Test label_002_7C50 - Lava/deep water/river rapids (02:7C50-02:7C9E) */
void test_label_002_7C50(void) {

    /* Reset mock counter */
    g_mock_bg_collision_calls = 0;

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 0x00);
    gb_write(&gb, wDialogGotItem, 0x00);
    gb_write_hram(&gb, hLinkInteractiveMotionBlocked, 0x00);
    gb_write(&gb, wDialogState, 0x00);
    gb_write(&gb, wInventoryAppearing, 0x00);
    gb_write_hram(&gb, hObjectUnderEntity, 0xE7); /* Index 0 */
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x40);

    label_002_7C50(&gb, mock_bg_collision);

    /* Data_002_7C40[0] = 0xFC (-4), Data_002_7C48[0] = 0x00 */
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0xFC);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);
    assert(g_mock_bg_collision_calls == 1);

    /* Test river rapids object $0E in room $3E */
    g_mock_bg_collision_calls = 0;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 0x00);
    gb_write(&gb, wDialogGotItem, 0x00);
    gb_write_hram(&gb, hLinkInteractiveMotionBlocked, 0x00);
    gb_write(&gb, wDialogState, 0x00);
    gb_write(&gb, wInventoryAppearing, 0x00);
    gb_write_hram(&gb, hObjectUnderEntity, 0x0E);
    gb_write_hram(&gb, hMapRoom, UNKNOWN_ROOM_3E);
    gb_write_hram(&gb, hLinkPositionX, 0x50);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    g_mock_bg_collision_calls = 0;

    label_002_7C50(&gb, mock_bg_collision);

    /* Room $3E -> e = 1 -> Data_002_7C40[1] = 0xFA (-6), Data_002_7C48[1] = 0x00 */
    assert(gb_read_hram(&gb, hLinkSpeedX) == 0xFA);
    assert(gb_read_hram(&gb, hLinkSpeedY) == 0x00);
    assert(g_mock_bg_collision_calls == 1);

    /* Test early return when transition active */
    g_mock_bg_collision_calls = 0;
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 0x04); /* Active transition */
    gb_write_hram(&gb, hObjectUnderEntity, 0xE7);

    label_002_7C50(&gb, mock_bg_collision);

    /* Should return early, no collision call */
    assert(g_mock_bg_collision_calls == 0);

}

/* Test Data tables */
void test_RoomTransitionDataTables(void) {

    /* RoomTransitionLinkXIncrement */
    assert(RoomTransitionLinkXIncrement[0] == (int8_t)0xC6); /* right */
    assert(RoomTransitionLinkXIncrement[1] == (int8_t)0x3A); /* left */
    assert(RoomTransitionLinkXIncrement[2] == 0x00); /* top */
    assert(RoomTransitionLinkXIncrement[3] == 0x00); /* bottom */

    /* RoomTransitionLinkYIncrement */
    assert(RoomTransitionLinkYIncrement[0] == 0x00); /* right */
    assert(RoomTransitionLinkYIncrement[1] == 0x00); /* left */
    assert(RoomTransitionLinkYIncrement[2] == (int8_t)0x3A); /* top */
    assert(RoomTransitionLinkYIncrement[3] == (int8_t)0xC6); /* bottom */

    /* RoomTransitionXIncrement */
    assert(RoomTransitionXIncrement[0] == 0x04); /* right */
    assert(RoomTransitionXIncrement[1] == (int8_t)0xFC); /* left */
    assert(RoomTransitionXIncrement[2] == 0x00); /* top */
    assert(RoomTransitionXIncrement[3] == 0x00); /* bottom */

    /* RoomTransitionYIncrement */
    assert(RoomTransitionYIncrement[0] == 0x00); /* right */
    assert(RoomTransitionYIncrement[1] == 0x00); /* left */
    assert(RoomTransitionYIncrement[2] == (int8_t)0xFC); /* top */
    assert(RoomTransitionYIncrement[3] == 0x04); /* bottom */

    /* WindFishEggMazeSequence (32 entries) */
    assert(sizeof(WindFishEggMazeSequence) == 32);
    assert(WindFishEggMazeSequence[0] == DIRECTION_LEFT);
    assert(WindFishEggMazeSequence[31] == DIRECTION_UP);

    /* OverworldRoomIncrement */
    assert(OverworldRoomIncrement[0] == 0x01); /* right */
    assert(OverworldRoomIncrement[1] == 0xFF); /* left */
    assert(OverworldRoomIncrement[2] == 0xF0); /* top */
    assert(OverworldRoomIncrement[3] == 0x10); /* bottom */

    /* IndoorRoomIncrement */
    assert(IndoorRoomIncrement[0] == 0x01); /* right */
    assert(IndoorRoomIncrement[1] == 0xFF); /* left */
    assert(IndoorRoomIncrement[2] == 0xF8); /* top */
    assert(IndoorRoomIncrement[3] == 0x08); /* bottom */

    /* Data_002_7C04 */
    assert(Data_002_7C04[0] == 0x00);
    assert(Data_002_7C04[2] == (int8_t)0xFF);
    assert(Data_002_7C04[3] == 0x01);

    /* Data_002_7C0C */
    assert(Data_002_7C0C[0] == 0x01);
    assert(Data_002_7C0C[1] == (int8_t)0xFF);

    /* Data_002_7C40 */
    assert(Data_002_7C40[0] == (int8_t)0xFC);
    assert(Data_002_7C40[4] == 0x0C);

    /* Data_002_7C48 */
    assert(Data_002_7C48[5] == (int8_t)0xF4);
    assert(Data_002_7C48[6] == 0x0C);

}

void test_bank2_room_transition(void) {
    test_ApplyRoomTransition_None();
    test_ApplyRoomTransition_DispatchPrepare();
    test_ApplyRoomTransition_ScrollOffset();
    test_ApplyRoomTransition_TargetReached();
    test_ApplyRoomTransition_BottomLedge();
    test_RoomTransitionPrepareHandler_Indoor();
    test_RoomTransitionPrepareHandler_FaceShrineHack();
    test_RoomTransitionPrepareHandler_MysteriousWoods();
    test_RoomTransitionPrepareHandler_TailCaveKey();
    test_RoomTransitionLoadTiles();
    test_RoomTransitionConfigureScrollTargets();
    test_RoomTransitionConfigureScrollTargets_SwitchBlock();
    test_RoomTransitionFirstHalfHandler();
    test_RoomTransitionSecondHalfHandler();
    test_label_002_7C14();
    test_label_002_7C50();
    test_RoomTransitionDataTables();

}
