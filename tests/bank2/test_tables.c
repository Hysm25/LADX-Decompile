#include "test_bank2.h"

#include "bank2/audio.h"
#include "bank2/items.h"
#include "constants/audio.h"
#include "constants/directions.h"

#include <assert.h>

void test_bank2_tables(void) {
    /* Test 1: Tables size and constant values */
    assert(sizeof(OverworldMusicTracks) == 256);
    assert(sizeof(HouseMusicTracks) == 32);
    assert(sizeof(MusicOverridesPowerUpTrack) == 38);
    assert(sizeof(HookshotChainSpeedX) == 4);
    assert(sizeof(HookshotChainSpeedY) == 4);

    assert(OverworldMusicTracks[0x00] == MUSIC_TAL_TAL_RANGE);
    assert(OverworldMusicTracks[0x20] == MUSIC_OVERWORLD);
    assert(OverworldMusicTracks[0x40] == MUSIC_MYSTERIOUS_FOREST);
    assert(OverworldMusicTracks[0x4C] == MUSIC_RAFT_RIDE_RAPIDS);
    assert(OverworldMusicTracks[0x81] == MUSIC_MABE_VILLAGE);
    assert(OverworldMusicTracks[0xCC] == MUSIC_ANIMAL_VILLAGE);

    assert(HouseMusicTracks[0] == MUSIC_TAIL_CAVE);
    assert(HouseMusicTracks[1] == MUSIC_BOTTLE_GROTTO);
    assert(HouseMusicTracks[6] == MUSIC_EAGLES_TOWER);
    assert(HouseMusicTracks[9] == MUSIC_COLOR_DUNGEON);
    assert(HouseMusicTracks[14] == MUSIC_SHOP);
    assert(HouseMusicTracks[15] == MUSIC_MINIGAME);
    assert(HouseMusicTracks[16] == MUSIC_INSIDE_BUILDING);
    assert(HouseMusicTracks[30] == MUSIC_GHOST_HOUSE);

    assert(MusicOverridesPowerUpTrack[MUSIC_MINIGAME] == 1);
    assert(MusicOverridesPowerUpTrack[MUSIC_MABE_VILLAGE] == 1);
    assert(MusicOverridesPowerUpTrack[MUSIC_SHOP] == 1);
    assert(MusicOverridesPowerUpTrack[MUSIC_OVERWORLD] == 0);

    assert(HookshotChainSpeedX[DIRECTION_RIGHT] == 0x30);
    assert(HookshotChainSpeedX[DIRECTION_LEFT] == -0x30);
    assert(HookshotChainSpeedX[DIRECTION_UP] == 0);
    assert(HookshotChainSpeedX[DIRECTION_DOWN] == 0);

    assert(HookshotChainSpeedY[DIRECTION_RIGHT] == 0);
    assert(HookshotChainSpeedY[DIRECTION_LEFT] == 0);
    assert(HookshotChainSpeedY[DIRECTION_UP] == -0x30);
    assert(HookshotChainSpeedY[DIRECTION_DOWN] == 0x30);

    /* Test ThresholdLowHealthTable (02:6308) */
    assert(sizeof(ThresholdLowHealthTable) == 16);
    static const uint8_t expected_thresholds[16] = {
        0x00, 0x22, 0xC9, 0x05, 0x05, 0x05, 0x09, 0x09,
        0x09, 0x11, 0x11, 0x11, 0x19, 0x19, 0x19, 0x19
    };
    for (int i = 0; i < 16; i++) {
        assert(ThresholdLowHealthTable[i] == expected_thresholds[i]);
    }
}
