#include "test_bank1.h"

#include "gb.h"
#include "bank1/game_over.h"
#include "constants/audio.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/link.h"
#include "constants/memory.h"

#include <assert.h>
#include <stdio.h>

void test_game_over_subsystem(void) {
    GBState gb;

    /* Test 1: LinkPassOutHandler passing out animation (countdown != 0) */
    gb_init(&gb);
    gb_write(&gb, hLinkCountdown, 0x10); /* 16 */
    gb_write(&gb, wBGPalette, 0xE4);
    LinkPassOutHandler(&gb);
    assert(gb_read(&gb, wScreenShakeHorizontal) == 0);
    assert(gb_read(&gb, wScreenShakeVertical) == 0);
    assert(gb_read(&gb, wObjectAffectingBGPalette) == 1);
    assert(gb_read(&gb, wOBJ0Palette) == 0x1C);
    assert(gb_read(&gb, wOBJ1Palette) == 0xE4);
    assert(gb_read(&gb, hLinkAnimationState) == Data_001_41CF[0x10 >> 3]);
    assert(gb_read(&gb, wC3CD) == Data_001_41E7[0x10 >> 3]);

    /* Test 2: LinkPassOutHandler animation finish (countdown == 0) and BCD death counter */
    gb_init(&gb);
    gb_write(&gb, hLinkCountdown, 0);
    gb_write(&gb, wDeathCount, 0x09);
    gb_write(&gb, wDeathCount + 1, 0x00);
    LinkPassOutHandler(&gb);
    assert(gb_read(&gb, hLinkCountdown) == 16);
    assert(gb_read(&gb, hGameOverStage) == 1);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_0F);
    assert(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_HIDDEN);
    assert(gb_read(&gb, wDeathCount) == 0x10); /* 9 + 1 = 10 in BCD */
    assert(gb_read(&gb, wDeathCount + 1) == 0x00);

    /* Test BCD max cap at 999 */
    gb_init(&gb);
    gb_write(&gb, hLinkCountdown, 0);
    gb_write(&gb, wDeathCount, 0x99);
    gb_write(&gb, wDeathCount + 1, 0x09);
    LinkPassOutHandler(&gb);
    assert(gb_read(&gb, wDeathCount) == 0x99);
    assert(gb_read(&gb, wDeathCount + 1) == 0x09);

    /* Test 3: LoadGameOverStage1Handler */
    gb_init(&gb);
    gb_write(&gb, hGameOverStage, 1);
    LoadGameOverStage1Handler(&gb);
    assert(gb_read(&gb, wTilesetToLoad) == TILESET_SAVE_MENU);
    assert(gb_read(&gb, hGameOverStage) == 2);

    /* Test 4: LoadGameOverStage2Handler */
    gb_init(&gb);
    gb_write(&gb, hGameOverStage, 2);
    LoadGameOverStage2Handler(&gb);
    assert(gb_read(&gb, wBGPalette) == 0xE4);
    assert(gb_read(&gb, wBGMapToLoad) == TILEMAP_GAME_OVER);
    assert(gb_read(&gb, wWindowY) == 0xFF);
    assert(gb_read(&gb, hGameOverStage) == 3);

    /* Test 5: LoadGameOverStage3Handler */
    gb_init(&gb);
    gb_write(&gb, hGameOverStage, 3);
    gb_write(&gb, hLinkCountdown, 5);
    LoadGameOverStage3Handler(&gb);
    assert(gb_read(&gb, hGameOverStage) == 3); /* Still waiting */

    gb_write(&gb, hLinkCountdown, 0);
    LoadGameOverStage3Handler(&gb);
    assert(gb_read(&gb, hGameOverStage) == 4);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_GAME_OVER);

    /* Test 6: func_001_4339 navigation */
    gb_init(&gb);
    gb_write(&gb, wC13F, 0);
    gb_write(&gb, hJoypadState, J_DOWN);
    func_001_4339(&gb);
    assert(gb_read(&gb, wC13F) == 1);
    assert(gb_read(&gb, wOAMBuffer + 0x18) == Data_001_4336[1]);
    assert(gb_read(&gb, wOAMBuffer + 0x19) == 0x24);
    assert(gb_read(&gb, wOAMBuffer + 0x1A) == 0xBE);

    gb_write(&gb, hJoypadState, J_UP);
    func_001_4339(&gb);
    assert(gb_read(&gb, wC13F) == 0);
    assert(gb_read(&gb, wOAMBuffer + 0x18) == Data_001_4336[0]);

    /* Test 7: GameOverInteractiveHandler option 1 (Save and Quit) */
    gb_init(&gb);
    gb_write(&gb, wC13F, 1);
    gb_write(&gb, hJoypadState, J_A);
    GameOverInteractiveHandler(&gb);
    assert(gb_read(&gb, hActiveEntityTilesOffset) == 0);

    /* Test 8: GameOverInteractiveHandler option 0 (Save and Continue) */
    gb_init(&gb);
    gb_write(&gb, wC13F, 0);
    gb_write(&gb, hJoypadState, J_A);
    gb_write(&gb, wEntitiesStatusTable, 0x55);
    GameOverInteractiveHandler(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable) == 0);
    assert(gb_read(&gb, wInvincibilityCounter) == 0x80);

    /* Test 9: GameOverInteractiveHandler option 2 (Continue without saving) */
    gb_init(&gb);
    gb_write(&gb, wC13F, 2);
    gb_write(&gb, hJoypadState, J_START);
    gb_write(&gb, wEntitiesStatusTable, 0xAA);
    GameOverInteractiveHandler(&gb);
    assert(gb_read(&gb, wEntitiesStatusTable) == 0);
    assert(gb_read(&gb, wInvincibilityCounter) == 0x80);

    /* Test 10: LinkPassOut dispatcher */
    gb_init(&gb);
    gb_write(&gb, hGameOverStage, 1);
    LinkPassOut(&gb);
    assert(gb_read(&gb, hGameOverStage) == 2);
}
