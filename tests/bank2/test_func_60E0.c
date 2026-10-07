#include "test_bank2.h"
#include "test_support.h"

#include "gb.h"
#include "bank2/items.h"
#include "constants/dialog.h"
#include "constants/gameplay.h"
#include "constants/hardware.h"
#include "constants/joypad.h"
#include "constants/memory.h"
#include "constants/sfx.h"
#include "constants/maps.h"
#include "constants/tilesets.h"

#include <assert.h>
#include <stdint.h>

void test_bank2_func_002_60E0(void) {
    /* Test: func_002_60E0 (02:60E0-02:6206) */

    /* 1. Clamps item counts - magic powder */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wMaxMagicPowder, 0x20);  /* max = 32 */
        gb_write(&gb, wMagicPowderCount, 0x30); /* current = 48 */

        func_002_60E0(&gb);

        assert(gb_read(&gb, wMagicPowderCount) == 0x20);
    }

    /* 2. Clamps item counts - bombs */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wMaxMagicPowder, 0x3C);  /* max = 60 */
        gb_write(&gb, wBombCount, 0x50);       /* current = 80 */

        func_002_60E0(&gb);

        assert(gb_read(&gb, wBombCount) == 0x3C);
    }

    /* 3. Clamps item counts - arrows */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wMaxMagicPowder, 0x46);  /* max = 70 */
        gb_write(&gb, wArrowCount, 0x46);      /* current = 70 */

        func_002_60E0(&gb);

        assert(gb_read(&gb, wArrowCount) == 0x46);
    }

    /* 4. Returns early if Link is not interactive */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wLinkMotionState, LINK_MOTION_TYPE_NON_INTERACTIVE);

        func_002_60E0(&gb);

        /* Should not crash */
    }

    /* 5. Returns early if dialog state is non-zero */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wDialogState, 0x01);

        func_002_60E0(&gb);

        /* Should not crash */
    }

    /* 6. Returns early if room transition state is non-zero */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wRoomTransitionState, 0x01);

        func_002_60E0(&gb);

        /* Should not crash */
    }

    /* 7. Handles subscreen transition - inventory appearing */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wInventoryAppearing, 0x01);
        gb_write(&gb, wDrawCommand, 0x00);
        gb_write(&gb, wInventoryShouldScroll, 0x00);

        func_002_60E0(&gb);

        assert(gb_read(&gb, wInventoryShouldScroll) == 0x01);
    }

    /* 8. Returns early if SELECT not pressed */
    {
        GBState gb;
        gb_init(&gb);
        gb_write_hram(&gb, hPressedButtonsMask, 0x00);

        func_002_60E0(&gb);

        /* Should not crash - returns early */
    }

    /* 9. Returns early if START not pressed */
    {
        GBState gb;
        gb_init(&gb);
        gb_write_hram(&gb, hPressedButtonsMask, J_SELECT);
        gb_write_hram(&gb, hJoypadState, 0x00);

        func_002_60E0(&gb);

        /* Should not crash - returns early */
    }

    /* 10. NULL state handling */
    {
        func_002_60E0(NULL);
        /* Should not crash */
    }

    /* 11. func_002_61BA helper */
    {
        GBState gb;
        gb_init(&gb);
        func_002_61BA(&gb);
        /* Should not crash with stubs */
    }

    /* 12. UpdateRupeesCount - basic */
    {
        GBState gb;
        gb_init(&gb);
        UpdateRupeesCount(&gb);
        /* Should not crash */
    }

    /* 13. UpdateHealth - basic */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wMaxHearts, 0x03);
        gb_write(&gb, wHealth, 0x18);
        UpdateHealth(&gb);
        /* Should not crash */
    }

    /* 14. LoadRupeesDigits */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wDrawCommandsSize, 0x00);
        gb_write(&gb, wRupeeCountHigh, 0x01);
        gb_write(&gb, wRupeeCountLow, 0x23);
        gb_write(&gb, wDrawCommand + 6, 0xEE); /* Guard byte */
        LoadRupeesDigits(&gb);
        assert(gb_read(&gb, wDrawCommandsSize) == 0x06);
        assert(gb_read(&gb, wDrawCommand + 0) == 0x9C);
        assert(gb_read(&gb, wDrawCommand + 1) == 0x2A);
        assert(gb_read(&gb, wDrawCommand + 2) == 0x02);
        assert(gb_read(&gb, wDrawCommand + 3) == 0xB1); /* BCD '1' */
        assert(gb_read(&gb, wDrawCommand + 4) == 0xB2); /* BCD '2' */
        assert(gb_read(&gb, wDrawCommand + 5) == 0xB3); /* BCD '3' */
        assert(gb_read(&gb, wDrawCommand + 6) == 0xEE); /* Guard byte untouched */

        /* Zero rupees */
        gb_write(&gb, wDrawCommandsSize, 0x00);
        gb_write(&gb, wRupeeCountHigh, 0x00);
        gb_write(&gb, wRupeeCountLow, 0x00);
        LoadRupeesDigits(&gb);
        assert(gb_read(&gb, wDrawCommandsSize) == 0x06);
        assert(gb_read(&gb, wDrawCommand + 3) == 0xB0);
        assert(gb_read(&gb, wDrawCommand + 4) == 0xB0);
        assert(gb_read(&gb, wDrawCommand + 5) == 0xB0);
    }

    /* 15. LoadHeartsCount */
    {
        /* Case A: 3 full hearts */
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wDrawCommandsSize, 0x00);
        gb_write(&gb, wMaxHearts, 0x03);
        gb_write(&gb, wHealth, 0x18); /* 3 * 8 = 24 */
        LoadHeartsCount(&gb);
        assert(gb_read(&gb, wDrawCommandsSize) == 0x14);
        assert(gb_read(&gb, wDrawCommand + 0) == 0x9C);
        assert(gb_read(&gb, wDrawCommand + 1) == 0x0D);
        assert(gb_read(&gb, wDrawCommand + 2) == 0x06);
        assert(gb_read(&gb, wDrawCommand + 3) == 0xA9); /* Full heart */
        assert(gb_read(&gb, wDrawCommand + 4) == 0xA9); /* Full heart */
        assert(gb_read(&gb, wDrawCommand + 5) == 0xA9); /* Full heart */
        assert(gb_read(&gb, wDrawCommand + 6) == 0x7F); /* Empty slot */
        assert(gb_read(&gb, wDrawCommand + 10) == 0x9C); /* Row 2 header */
        assert(gb_read(&gb, wDrawCommand + 11) == 0x2D);
        assert(gb_read(&gb, wDrawCommand + 12) == 0x06);
        assert(gb_read(&gb, wDrawCommand + 20) == 0x00); /* Terminator */

        /* Case B: 1 full heart, 1 half heart, 1 empty heart */
        gb_init(&gb);
        gb_write(&gb, wDrawCommandsSize, 0x00);
        gb_write(&gb, wMaxHearts, 0x03);
        gb_write(&gb, wHealth, 0x0C); /* 8 + 4 = 12 */
        LoadHeartsCount(&gb);
        assert(gb_read(&gb, wDrawCommand + 3) == 0xA9); /* Full */
        assert(gb_read(&gb, wDrawCommand + 4) == 0xCE); /* Half */
        assert(gb_read(&gb, wDrawCommand + 5) == 0xCD); /* Empty */
        assert(gb_read(&gb, wDrawCommand + 6) == 0x7F); /* Empty slot */

        /* Case C: 0 health -> 3 empty hearts */
        gb_init(&gb);
        gb_write(&gb, wDrawCommandsSize, 0x00);
        gb_write(&gb, wMaxHearts, 0x03);
        gb_write(&gb, wHealth, 0x00);
        LoadHeartsCount(&gb);
        assert(gb_read(&gb, wDrawCommand + 3) == 0xCD);
        assert(gb_read(&gb, wDrawCommand + 4) == 0xCD);
        assert(gb_read(&gb, wDrawCommand + 5) == 0xCD);

        /* Case D: 10 max hearts, 8 full hearts (spans row 1 and row 2) */
        gb_init(&gb);
        gb_write(&gb, wDrawCommandsSize, 0x00);
        gb_write(&gb, wMaxHearts, 10);
        gb_write(&gb, wHealth, 64); /* 8 * 8 */
        LoadHeartsCount(&gb);
        /* 7 full hearts in row 1 */
        for (int i = 0; i < 7; i++) {
            assert(gb_read(&gb, (uint16_t)(wDrawCommand + 3 + i)) == 0xA9);
        }
        /* Row 2 header preserved */
        assert(gb_read(&gb, wDrawCommand + 10) == 0x9C);
        assert(gb_read(&gb, wDrawCommand + 11) == 0x2D);
        assert(gb_read(&gb, wDrawCommand + 12) == 0x06);
        /* 8th full heart in row 2 */
        assert(gb_read(&gb, wDrawCommand + 13) == 0xA9);
        /* 9th and 10th empty hearts */
        assert(gb_read(&gb, wDrawCommand + 14) == 0xCD);
        assert(gb_read(&gb, wDrawCommand + 15) == 0xCD);
        /* Slots 11..14 empty space */
        assert(gb_read(&gb, wDrawCommand + 16) == 0x7F);
        assert(gb_read(&gb, wDrawCommand + 17) == 0x7F);
        assert(gb_read(&gb, wDrawCommand + 18) == 0x7F);
        assert(gb_read(&gb, wDrawCommand + 19) == 0x7F);
        assert(gb_read(&gb, wDrawCommand + 20) == 0x00);
    }

    /* 16. ClampItemCount with zero max */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wMaxMagicPowder, 0x00);
        gb_write(&gb, wMagicPowderCount, 0x10);

        func_002_60E0(&gb);

        assert(gb_read(&gb, wMagicPowderCount) == 0x00);
    }

    /* 17. Subscreen opening logic (START pressed, SELECT not pressed) */
    {
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wSubscreenScrollIncrement, 0x08);  /* Initial value */
        gb_write_hram(&gb, hPressedButtonsMask, 0x00);
        gb_write_hram(&gb, hJoypadState, J_START);
        gb_write(&gb, wWindowY, 0x00);
        gb_write(&gb, wD464, 0x00);
        gb_write(&gb, wC167, 0x00);
        gb_write_hram(&gb, hLinkInteractiveMotionBlocked, 0x00);
        gb_write_hram(&gb, hLinkAnimationState, 0x00);
        gb_write(&gb, wOcarinaMenuOpen, 0x00);
        gb_write(&gb, wOcarinaMenuOpening, 0x00);
        gb_write(&gb, wOcarinaMenuClosing, 0x00);
        gb_write(&gb, wIsIndoor, 0x00);
        gb_write_hram(&gb, hMapId, 0x00);

        func_002_60E0(&gb);

        assert(gb_read(&gb, wInventoryAppearing) == 0x01);
        assert(gb_read(&gb, wGameplayType) == GAMEPLAY_INVENTORY);
        assert(gb_read(&gb, wGameplaySubtype) == GAMEPLAY_INVENTORY_INITIAL);
    }

    /* 18. Map opening logic (SELECT pressed) */
    {
        GBState gb;
        gb_init(&gb);
        gb_write_hram(&gb, hPressedButtonsMask, J_SELECT);
        gb_write_hram(&gb, hJoypadState, 0x00);
        gb_write(&gb, wDialogState, 0x00);

        func_002_60E0(&gb);

        /* Should reach inventory_fully_closed2 and call UpdateRupeesCount/UpdateHealth */
        /* Just verify it doesn't crash */
    }

    /* 19. Indoor tileset selection: minimap vs inventory */
    {
        /* Case A: Dungeon map (< MAP_WINDFISHS_EGG) -> TILESET_LOAD_DUNGEON_MINIMAP */
        GBState gb;
        gb_init(&gb);
        gb_write(&gb, wSubscreenScrollIncrement, 0x08);
        gb_write_hram(&gb, hPressedButtonsMask, 0x00);
        gb_write_hram(&gb, hJoypadState, J_START);
        gb_write(&gb, wIsIndoor, 0x01);
        gb_write_hram(&gb, hMapId, 0x00); /* Tail Cave */
        func_002_60E0(&gb);
        assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_LOAD_DUNGEON_MINIMAP);

        /* Case B: Color dungeon -> TILESET_LOAD_DUNGEON_MINIMAP */
        gb_init(&gb);
        gb_write(&gb, wSubscreenScrollIncrement, 0x08);
        gb_write_hram(&gb, hPressedButtonsMask, 0x00);
        gb_write_hram(&gb, hJoypadState, J_START);
        gb_write(&gb, wIsIndoor, 0x01);
        gb_write_hram(&gb, hMapId, MAP_COLOR_DUNGEON);
        func_002_60E0(&gb);
        assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_LOAD_DUNGEON_MINIMAP);

        /* Case C: Windfish's Egg (MAP_WINDFISHS_EGG) -> TILESET_LOAD_INVENTORY */
        gb_init(&gb);
        gb_write(&gb, wSubscreenScrollIncrement, 0x08);
        gb_write_hram(&gb, hPressedButtonsMask, 0x00);
        gb_write_hram(&gb, hJoypadState, J_START);
        gb_write(&gb, wIsIndoor, 0x01);
        gb_write_hram(&gb, hMapId, MAP_WINDFISHS_EGG);
        func_002_60E0(&gb);
        assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_LOAD_INVENTORY);

        /* Case D: House (>= MAP_WINDFISHS_EGG) -> TILESET_LOAD_INVENTORY */
        gb_init(&gb);
        gb_write(&gb, wSubscreenScrollIncrement, 0x08);
        gb_write_hram(&gb, hPressedButtonsMask, 0x00);
        gb_write_hram(&gb, hJoypadState, J_START);
        gb_write(&gb, wIsIndoor, 0x01);
        gb_write_hram(&gb, hMapId, 0x1A);
        func_002_60E0(&gb);
        assert(gb_read_hram(&gb, hNeedsUpdatingBGTiles) == TILESET_LOAD_INVENTORY);
    }
}