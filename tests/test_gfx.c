#include <stdio.h>
#include <string.h>
#include "gb.h"
#include "home/gfx.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/gameplay.h"

static int gfx_failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            gfx_failures++; \
        } \
    } while (0)

#define ROM_BANK_OFFSET(bank, addr) (((bank) * 0x4000) + ((addr) - 0x4000))

static uint8_t mock_rom[0x4000 * 0x39];

static void setup_mock_data(void) {
    memset(mock_rom, 0, sizeof(mock_rom));

    /* Populate test markers in banks */
    mock_rom[ROM_BANK_OFFSET(0x01, IntroRainTiles)] = 0x51;

    /* Bank $13: EndingTiles ($4000), EaglesTowerTop1Tiles ($6800), EaglesTowerTop2Tiles ($7C00) */
    mock_rom[ROM_BANK_OFFSET(0x13, EndingTiles + 0x2800)] = 0x11; /* Same as EaglesTowerTop1Tiles */
    mock_rom[ROM_BANK_OFFSET(0x13, EndingTiles + 0x3000)] = 0x22;
    mock_rom[ROM_BANK_OFFSET(0x13, EndingTiles)] = 0x33;
    mock_rom[ROM_BANK_OFFSET(0x13, EndingTiles + 0x800)] = 0x55;
    mock_rom[ROM_BANK_OFFSET(0x13, EndingTiles + 0x1800)] = 0x66;
    mock_rom[ROM_BANK_OFFSET(0x13, EndingTiles + 0x2000)] = 0x77;
    mock_rom[ROM_BANK_OFFSET(0x13, EndingTiles + 0x3800)] = 0x18;
    mock_rom[ROM_BANK_OFFSET(0x13, EaglesTowerTop2Tiles)] = 0xA2;

    mock_rom[ROM_BANK_OFFSET(0x0C, Overworld1Tiles + 0x8E0)] = 0x88;
    mock_rom[ROM_BANK_OFFSET(0x0C, Overworld1Tiles + 0x100)] = 0x99;
    mock_rom[ROM_BANK_OFFSET(0x0C, LinkCharacterTiles)] = 0x5A;
    mock_rom[ROM_BANK_OFFSET(0x0C, InventoryEquipmentItemsTiles)] = 0x6B;
    mock_rom[ROM_BANK_OFFSET(0x0C, Items1Tiles + 0x3A0)] = 0x7C;
    mock_rom[ROM_BANK_OFFSET(0x0C, WorldMapTiles)] = 0x91;

    mock_rom[ROM_BANK_OFFSET(0x12, Npc4Tiles + 0x100)] = 0xAA;
    mock_rom[ROM_BANK_OFFSET(0x12, EndingTiles + 0x3600)] = 0x44; /* Bank 12 active from LoadTileset15 */
    mock_rom[ROM_BANK_OFFSET(0x12, Npc3Tiles + 0x2000)] = 0xBB;
    mock_rom[ROM_BANK_OFFSET(0x12, Npc3Tiles + 0x2600)] = 0xBC;

    mock_rom[ROM_BANK_OFFSET(0x0F, Overworld2Tiles + 0x600)] = 0xCC;
    mock_rom[ROM_BANK_OFFSET(0x0F, MenuTiles)] = 0x8D;
    mock_rom[ROM_BANK_OFFSET(0x0F, FontTiles)] = 0x9E;
    mock_rom[ROM_BANK_OFFSET(0x0F, TitleLogoTiles)] = 0x71;
    mock_rom[ROM_BANK_OFFSET(0x0F, SaveMenuTiles)] = 0x72;

    
    /* Bank $20: DungeonFloorTilesPointers ($4589), DungeonWallsTilesPointers ($45A9), DungeonItemsTilesPointers ($45CA) */
    mock_rom[ROM_BANK_OFFSET(0x20, DungeonFloorTilesPointers + 0x01)] = 0x50; /* high byte for floor */
    mock_rom[ROM_BANK_OFFSET(0x20, DungeonWallsTilesPointers + 0x01)] = 0x58; /* high byte for walls */
    mock_rom[ROM_BANK_OFFSET(0x20, DungeonItemsTilesPointers + 0x01)] = 0x64; /* high byte for items */

    /* Bank $0D: DungeonsTiles */
    mock_rom[ROM_BANK_OFFSET(0x0D, 0x5000)] = 0x31; /* floor tile */
    mock_rom[ROM_BANK_OFFSET(0x0D, DungeonsTiles)] = 0x32; /* shared dungeon tile */
    mock_rom[ROM_BANK_OFFSET(0x0D, 0x5800)] = 0x33; /* wall tile */

    /* Bank $12: DungeonItemsTiles ($6000), InventoryIndoorItemsTiles ($7D00) */
    mock_rom[ROM_BANK_OFFSET(0x12, 0x6400)] = 0x41; /* indoor object */
    mock_rom[ROM_BANK_OFFSET(0x12, InventoryIndoorItemsTiles)] = 0x42; /* indoor inventory */

    /* Bank $0C: OverworldLandscapeTiles ($5200), InventoryOverworldItemsTiles ($4C00), Items1Tiles + 0x3C0 */
    mock_rom[ROM_BANK_OFFSET(0x0C, OverworldLandscapeTiles)] = 0x73;
    mock_rom[ROM_BANK_OFFSET(0x0C, InventoryOverworldItemsTiles)] = 0x74;
    mock_rom[ROM_BANK_OFFSET(0x0C, Items1Tiles + 0x3C0)] = 0x75;

    
    mock_rom[ROM_BANK_OFFSET(0x0C, 0x68C0)] = 0x2A;
    mock_rom[ROM_BANK_OFFSET(0x0C, 0x68E0)] = 0x2C;

    /* Bank $2F: GBC adjusted bank for $0F */
    mock_rom[ROM_BANK_OFFSET(0x2F, TitleLogoTiles)] = 0x71;

    mock_rom[ROM_BANK_OFFSET(0x10, Intro1Tiles)] = 0x52;
    mock_rom[ROM_BANK_OFFSET(0x10, Intro3Tiles)] = 0x53;
    mock_rom[ROM_BANK_OFFSET(0x10, ChristineTiles)] = 0x61;
    mock_rom[ROM_BANK_OFFSET(0x10, MarinBeachTiles)] = 0x62;
    mock_rom[ROM_BANK_OFFSET(0x10, FontLargeTiles)] = 0x63;
    mock_rom[ROM_BANK_OFFSET(0x10, FontLargeTiles + 0x100)] = 0x3E;
    mock_rom[ROM_BANK_OFFSET(0x10, FontLargeTiles + 0x200)] = 0x4F;
    mock_rom[ROM_BANK_OFFSET(0x10, ReliefTiles)] = 0x64;
    mock_rom[ROM_BANK_OFFSET(0x10, PaintingTiles)] = 0x65;

    mock_rom[ROM_BANK_OFFSET(0x35, PhotoAlbumTiles)] = 0xDD;
    mock_rom[ROM_BANK_OFFSET(0x35, PhotoAlbumTiles + 0x800)] = 0xDE;
    mock_rom[ROM_BANK_OFFSET(0x35, EndingCGBAltTiles)] = 0xDF;

    mock_rom[ROM_BANK_OFFSET(0x38, CreditsRollTiles)] = 0xEE;
    mock_rom[ROM_BANK_OFFSET(0x38, TitleDXTilesCGB)] = 0x81;
    mock_rom[ROM_BANK_OFFSET(0x38, TitleDXTilesDMG)] = 0x82;
    mock_rom[ROM_BANK_OFFSET(0x38, TitleDXOAMTiles)] = 0x83;
    mock_rom[ROM_BANK_OFFSET(0x38, TitleDXOAMTiles + 0x100)] = 0x84;

    /* Bank $0D: DungeonsTiles sideview and indoor top-view */
    mock_rom[ROM_BANK_OFFSET(0x0D, DungeonSideview1Tiles)] = 0xD1;
    mock_rom[ROM_BANK_OFFSET(0x0D, DungeonSideview2Tiles)] = 0xD2;
    mock_rom[ROM_BANK_OFFSET(0x0D, 0x5200)] = 0x52;
    mock_rom[ROM_BANK_OFFSET(0x2D, 0x5200)] = 0x52;

    /* Objects tilemaps */
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapCGB + 4 + 0)] = 0xA1;
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapCGB + 4 + 1)] = 0xA2;
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapCGB + 4 + 2)] = 0xA3;
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapCGB + 4 + 3)] = 0xA4;

    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapCGB + 4 + 0)] = 0xB1;
    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapCGB + 4 + 1)] = 0xB2;
    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapCGB + 4 + 2)] = 0xB3;
    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapCGB + 4 + 3)] = 0xB4;

    mock_rom[ROM_BANK_OFFSET(0x08, ColorDungeonObjectsTilemap + 4 + 0)] = 0xC1;
    mock_rom[ROM_BANK_OFFSET(0x08, ColorDungeonObjectsTilemap + 4 + 1)] = 0xC2;
    mock_rom[ROM_BANK_OFFSET(0x08, ColorDungeonObjectsTilemap + 4 + 2)] = 0xC3;
    mock_rom[ROM_BANK_OFFSET(0x08, ColorDungeonObjectsTilemap + 4 + 3)] = 0xC4;

    /* Object tile attributes in Bank 0x1B */
    mock_rom[ROM_BANK_OFFSET(0x1B, 0x5000 + 0)] = 0x05;
    mock_rom[ROM_BANK_OFFSET(0x1B, 0x5000 + 1)] = 0x06;
    mock_rom[ROM_BANK_OFFSET(0x1B, 0x5000 + 2)] = 0x07;
    mock_rom[ROM_BANK_OFFSET(0x1B, 0x5000 + 3)] = 0x08;


    /* Bank $0E: Npc1Tiles (BowWow follower 0xA4 -> offset 0x6400) */
    mock_rom[ROM_BANK_OFFSET(0x0E, 0x6400)] = 0x94;

    /* Bank $11: Npc2Tiles (spritesheet 0x45 -> offset 0x4500) */
    mock_rom[ROM_BANK_OFFSET(0x11, 0x4500)] = 0x75;

    /* Bank $0F: Overworld2Tiles at 0x4300 */
    mock_rom[ROM_BANK_OFFSET(0x0F, 0x4300)] = 0x43;

    /* Bank $08: IndoorObjectsTilemapDMG */
    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapDMG + 8 + 0)] = 0x21;
    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapDMG + 8 + 1)] = 0x22;
    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapDMG + 8 + 2)] = 0x23;
    mock_rom[ROM_BANK_OFFSET(0x08, IndoorObjectsTilemapDMG + 8 + 3)] = 0x24;

    /* Bank $1A: OverworldObjectsTilemapDMG */
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapDMG + 4 + 0)] = 0x11;
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapDMG + 4 + 1)] = 0x12;
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapDMG + 4 + 2)] = 0x13;
    mock_rom[ROM_BANK_OFFSET(0x1A, OverworldObjectsTilemapDMG + 4 + 3)] = 0x14;

    /* Bank $20: ColorDungeonObjectsTilemap */
    mock_rom[ROM_BANK_OFFSET(0x20, ColorDungeonObjectsTilemap + 12 + 0)] = 0x31;
    mock_rom[ROM_BANK_OFFSET(0x20, ColorDungeonObjectsTilemap + 12 + 1)] = 0x32;
    mock_rom[ROM_BANK_OFFSET(0x20, ColorDungeonObjectsTilemap + 12 + 2)] = 0x33;
    mock_rom[ROM_BANK_OFFSET(0x20, ColorDungeonObjectsTilemap + 12 + 3)] = 0x34;

    mock_rom[ROM_BANK_OFFSET(0x35, CameraShopIndoorTiles)] = 0xC5;
    mock_rom[ROM_BANK_OFFSET(0x35, PhotoAlbumTiles + 0x600)] = 0x6E;
    mock_rom[ROM_BANK_OFFSET(0x35, PhotoAlbumTiles + 0x610)] = 0x6F;
}

static void test_load_credits_koholint_disappearing_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadCreditsKoholintDisappearingTiles(&gb);

    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x11, "vTiles2 marker not copied");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x22, "vTiles1 marker not copied");
}

static void test_load_tileset_15(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadTileset15(&gb);

    TEST_ASSERT(gb_read(&gb, vTiles0) == 0xAA, "vTiles0 Npc4 marker not copied");
    TEST_ASSERT(gb_read(&gb, vTiles2 + 0x7F0) == 0x88, "vTiles2+0x7F0 Overworld1 marker not copied");
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x500) == 0xAA, "vTiles1+0x500 Npc4 marker not copied");
}

static void test_load_credits_stairs_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadCreditsStairsTiles(&gb);

    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x400) == 0x44, "vTiles0+0x400 EndingTiles marker not copied");
}

static void test_load_credits_koholint_views_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadCreditsKoholintViewsTiles(&gb);

    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x99, "vTiles2 Overworld1 marker not copied");
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0xBB, "vTiles0 Npc3 marker not copied");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0xCC, "vTiles1 Overworld2 marker not copied");
}

static void test_load_credits_link_on_sea_dmg_and_cgb(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));

    /* 1. DMG Mode: Link on sea close */
    gb_write(&gb, hIsGBC, 0);
    LoadCreditsLinkOnSeaCloseTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x33, "DMG Close sea tiles not from EndingTiles");
    TEST_ASSERT(gb_read(&gb, vTiles0 + TILE_SIZE * 0x80) == 0x66, "DMG Link on sea part 2 mismatch");

    /* 2. CGB Mode: Link on sea close uses PhotoAlbumTiles in bank 0x35 */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    /* Also set ending tiles in GBC bank $33 */
    mock_rom[ROM_BANK_OFFSET(0x33, EndingTiles + 0x1800)] = 0x67;
    gb_write(&gb, hIsGBC, 1);
    LoadCreditsLinkOnSeaCloseTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0xDD, "CGB Close sea tiles not from PhotoAlbumTiles");
    TEST_ASSERT(gb_read(&gb, vTiles0 + TILE_SIZE * 0x80) == 0x67, "CGB Link on sea part 2 mismatch");

    /* 3. Sun above tiles */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    LoadCreditsSunAboveTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x55, "Sun above tiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles0 + TILE_SIZE * 0x80) == 0x66, "Sun above part 2 mismatch");

    /* 4. Link on sea large tiles */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    LoadCreditsLinkOnSeaLargeTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x77, "Link on sea large tiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles0 + TILE_SIZE * 0x80) == 0x66, "Link on sea large part 2 mismatch");
}

static void test_ending_scene_tiles_and_credits_roll(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));

    /* 1. func_2B92 */
    gb_write(&gb, hIsGBC, 0);
    func_2B92(&gb, FontLargeTiles + 0x100, BANK_FontLargeTiles);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x3E, "func_2B92 vTiles0 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x22, "func_2B92 vTiles1 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x11, "func_2B92 vTiles2 mismatch");

    /* 2. LoadCreditsLinkFaceCloseUpTiles DMG */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    LoadCreditsLinkFaceCloseUpTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x18, "DMG LinkFaceCloseUp vTiles0 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x22, "DMG LinkFaceCloseUp vTiles1 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x11, "DMG LinkFaceCloseUp vTiles2 mismatch");

    /* 3. LoadCreditsLinkFaceCloseUpTiles CGB */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    mock_rom[ROM_BANK_OFFSET(0x33, EndingTiles + 0x3000)] = 0x23;
    mock_rom[ROM_BANK_OFFSET(0x33, EndingTiles + 0x2800)] = 0x12;
    LoadCreditsLinkFaceCloseUpTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0xDF, "CGB LinkFaceCloseUp vTiles0 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x23, "CGB LinkFaceCloseUp vTiles1 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x12, "CGB LinkFaceCloseUp vTiles2 mismatch");

    /* 4. LoadCreditsLinkSeatedOnLogTiles DMG */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    LoadCreditsLinkSeatedOnLogTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x55, "DMG LinkSeatedOnLog vTiles0 mismatch");

    /* 5. LoadCreditsLinkSeatedOnLogTiles CGB */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    LoadCreditsLinkSeatedOnLogTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0xDE, "CGB LinkSeatedOnLog vTiles0 mismatch");

    /* 6. LoadCreditsRollTiles DMG */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    LoadCreditsRollTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0xBC, "DMG CreditsRoll Npc3 tiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x100) == 0x4F, "DMG CreditsRoll FontLarge tiles mismatch");

    /* 7. LoadCreditsRollTiles CGB */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    mock_rom[ROM_BANK_OFFSET(0x32, Npc3Tiles + 0x2600)] = 0xBD;
    LoadCreditsRollTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0xEE, "CGB CreditsRoll tiles mismatch");
}

static void test_load_base_and_menu_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    /* Test LoadBaseTiles */
    LoadBaseTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x5A, "LoadBaseTiles LinkCharacterTiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x6B, "LoadBaseTiles InventoryEquipmentItemsTiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x600) == 0x7C, "LoadBaseTiles Items1Tiles mismatch");
    TEST_ASSERT(gb.rom_bank == 0x01, "LoadBaseTiles bank not restored to 0x01");

    /* Test LoadMenuTiles */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    LoadMenuTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x5A, "LoadMenuTiles LinkCharacterTiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x8D, "LoadMenuTiles MenuTiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x9E, "LoadMenuTiles FontTiles mismatch");
}

static void test_load_intro_sequence_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadIntroSequenceTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x700) == 0x51, "IntroRainTiles not copied to vTiles0+0x700");
    TEST_ASSERT(gb_read(&gb, vTiles0) == 0x53, "Intro3Tiles not copied to vTiles0");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x52, "Intro1Tiles not copied to vTiles1");
}

static void test_load_title_screen_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));

    /* DMG mode */
    gb_write(&gb, hIsGBC, 0);
    LoadTitleScreenTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x71, "TitleLogoTiles not copied to vTiles1");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x400) == 0x82, "TitleDXTilesDMG not copied to vTiles0+0x400");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x200) == 0x84, "TitleDXOAMTiles+0x100 not copied to vTiles0+0x200");

    /* CGB mode */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    LoadTitleScreenTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x71, "TitleLogoTiles not copied to vTiles1 on CGB");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x400) == 0x81, "TitleDXTilesCGB not copied to vTiles0+0x400");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x200) == 0x83, "TitleDXOAMTiles not copied to vTiles0+0x200");
}

static void test_load_world_map_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadWorldMapTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x700) == 0x91, "WorldMapTiles not copied to vTiles1+0x700");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x200) == 0x99, "Overworld1Tiles+0x100 not copied to vTiles0+0x200");
}

static void test_load_static_pictures_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    /* Relief */
    LoadFaceShrineReliefTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x64, "ReliefTiles not copied to vTiles2");

    /* Painting */
    gb_write(&gb, vTiles2, 0);
    LoadSchulePaintingTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x65, "PaintingTiles not copied to vTiles2");

    /* Christine */
    gb_write(&gb, vTiles2, 0);
    LoadChristinePortraitTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x61, "ChristineTiles not copied to vTiles2");
}

static void test_load_eagles_tower_top_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadEaglesTowerTopTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x400) == 0xA2, "EaglesTowerTop2Tiles not copied to vTiles1+0x400");
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x11, "EaglesTowerTop1Tiles not copied to vTiles2");
}

static void test_load_marin_beach_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadMarinBeachTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x400) == 0x63, "FontLargeTiles not copied to vTiles0+0x400");
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x62, "MarinBeachTiles not copied to vTiles2");
}

static void test_load_save_menu_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);

    LoadSaveMenuTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x72, "SaveMenuTiles not copied to vTiles1");
    TEST_ASSERT(gb.rom_bank == BANK_SaveMenuTiles, "Bank not set to BANK_SaveMenuTiles");
}


static void test_copy_word(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, 0xC000, 0x42);
    gb_write(&gb, 0xC001, 0x43);

    CopyWord(&gb, 0x8000, 0xC000);

    TEST_ASSERT(gb_read(&gb, 0x8000) == 0x42, "CopyWord byte 1 mismatch");
    TEST_ASSERT(gb_read(&gb, 0x8001) == 0x43, "CopyWord byte 2 mismatch");
}

static void test_switch_to_objects_tilemap_bank(void) {
    GBState gb;
    gb_init(&gb);

    /* Outdoors */
    gb_write(&gb, wIsIndoor, 0);
    SwitchToObjectsTilemapBank(&gb);
    TEST_ASSERT(gb.rom_bank == BANK_OverworldObjectsTilemapDMG, "Outdoors tilemap bank mismatch");

    /* Indoors */
    gb_write(&gb, wIsIndoor, 1);
    SwitchToObjectsTilemapBank(&gb);
    TEST_ASSERT(gb.rom_bank == BANK_IndoorObjectsTilemapDMG, "Indoors tilemap bank mismatch");
}

static void test_write_object_to_bg_dmg(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));

    /* 1. Overworld object 1 */
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hMapId, 0);
    gb_write(&gb, 0xC000, 0x01); /* object id = 1 */
    gb.rom_bank = BANK_OverworldObjectsTilemapDMG;
    WriteObjectToBG_DMG(&gb, 0x9800, 0xC000);

    TEST_ASSERT(gb_read(&gb, 0x9800) == 0x11, "Overworld object top-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9801) == 0x12, "Overworld object top-right tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9820) == 0x13, "Overworld object bottom-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9821) == 0x14, "Overworld object bottom-right tile mismatch");

    /* 2. Indoor object 2 */
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, 0);
    gb_write(&gb, 0xC000, 0x02); /* object id = 2 */
    gb.rom_bank = BANK_IndoorObjectsTilemapDMG;
    WriteObjectToBG_DMG(&gb, 0x9840, 0xC000);

    TEST_ASSERT(gb_read(&gb, 0x9840) == 0x21, "Indoor object top-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9841) == 0x22, "Indoor object top-right tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9860) == 0x23, "Indoor object bottom-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9861) == 0x24, "Indoor object bottom-right tile mismatch");

    /* 3. Color Dungeon object 3 */
    gb_write(&gb, hMapId, MAP_COLOR_DUNGEON);
    gb_write(&gb, 0xC000, 0x03); /* object id = 3 */
    gb.rom_bank = 0x20;
    WriteObjectToBG_DMG(&gb, 0x9880, 0xC000);

    TEST_ASSERT(gb_read(&gb, 0x9880) == 0x31, "Color dungeon object top-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x9881) == 0x32, "Color dungeon object top-right tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x98A0) == 0x33, "Color dungeon object bottom-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, 0x98A1) == 0x34, "Color dungeon object bottom-right tile mismatch");
}

static bool dummy_color_dungeon_called = false;
static uint8_t color_dungeon_callback_bank = 0;
static void dummy_color_dungeon_callback(GBState *gb) {
    dummy_color_dungeon_called = true;
    color_dungeon_callback_bank = gb->rom_bank;
}

static void test_load_room_specific_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));

    /* 1. Color Dungeon callback */
    dummy_color_dungeon_called = false;
    color_dungeon_callback_bank = 0;
    gb_write(&gb, hMapId, MAP_COLOR_DUNGEON);
    LoadRoomSpecificTiles(&gb, dummy_color_dungeon_callback);
    TEST_ASSERT(dummy_color_dungeon_called, "Color dungeon callback was not invoked");
    TEST_ASSERT(color_dungeon_callback_bank == 0x20, "Color dungeon callback bank was not 0x20");

    /* 2. Overworld with BowWow follower override and entity row 1 */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wIsBowWowFollowingLink, BOW_WOW_FOLLOWING);
    gb_write(&gb, wLoadedEntitySpritesheets + 0, 0x10); /* overridden to 0xA4 -> bank 0x0E, 0x6400 */
    gb_write(&gb, wLoadedEntitySpritesheets + 1, 0x45); /* bank index 1 -> bank 0x11, 0x4500 */
    gb_write(&gb, wLoadedEntitySpritesheets + 2, 0x00);
    gb_write(&gb, wLoadedEntitySpritesheets + 3, 0x00);
    gb_write(&gb, hWorldTileset, 3); /* (0x40 + 3) << 8 = 0x4300 in bank 0x0F */

    LoadRoomSpecificTiles(&gb, NULL);

    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x400) == 0x94, "BowWow follower tile not loaded to vTiles0+0x400");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x500) == 0x75, "Row 1 NPC tile not loaded to vTiles0+0x500");
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x43, "Overworld BG tiles not loaded to vTiles2");

    /* 3. Indoors side-scrolling: Eagles Tower */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hIsSideScrolling, 1);
    gb_write(&gb, hMapId, MAP_EAGLES_TOWER);

    LoadRoomSpecificTiles(&gb, NULL);
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0xD1, "Eagles Tower sideview tiles not copied to vTiles2");

    /* 4. Indoors side-scrolling: Seashell Mansion */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hIsSideScrolling, 1);
    gb_write(&gb, hMapId, MAP_CAVE_B);
    gb_write(&gb, hMapRoom, ROOM_INDOOR_B_SEASHELL_MANSION);

    LoadRoomSpecificTiles(&gb, NULL);
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0xD2, "Seashell mansion sideview tiles not copied to vTiles2");

    /* 5. Indoors top-view with GBC extra photo album tiles */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hIsSideScrolling, 0);
    gb_write(&gb, hMapId, 0);
    gb_write(&gb, hWorldTileset, 2); /* 0x50 + 2 = 0x5200 in bank 0x0D */

    LoadRoomSpecificTiles(&gb, NULL);
    TEST_ASSERT(gb_read(&gb, vTiles2) == 0x52, "Indoors top-view BG tiles mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles2 + 0x690) == 0x6E, "GBC extra photo album tile 1 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles2 + 0x790) == 0x6F, "GBC extra photo album tile 2 mismatch");

    /* 6. Camera shop indoor special tiles */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hIsSideScrolling, 0);
    gb_write(&gb, hMapId, MAP_HOUSE);
    gb_write(&gb, hMapRoom, ROOM_INDOOR_B_CAMERA_SHOP);
    gb_write(&gb, hWorldTileset, W_TILESET_NO_UPDATE);

    LoadRoomSpecificTiles(&gb, NULL);
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x700) == 0xC5, "Camera shop indoor tiles not copied to vTiles1+0x700");
}

static void mock_get_bg_attributes(GBState *gb, uint16_t hl, uint16_t bc) {
    (void)hl;
    (void)bc;
    gb_write(gb, hMultiPurpose8, 0x1B);
    gb_write(gb, hMultiPurpose9, 0x50);
    gb_write(gb, hMultiPurposeA, 0x00);
}

static bool minimap_arrow_called = false;
static void mock_update_minimap_arrow(GBState *gb) {
    (void)gb;
    minimap_arrow_called = true;
}

static void test_write_overworld_and_indoor_object_to_bg(void) {
    GBState gb;

    /* 1. WriteOverworldObjectToBG */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hMapId, 0);

    /* Write object ID 1 into WRAM bank 2 at wRoomObjects ($D711) */
    gb_write(&gb, rSVBK, 2);
    gb_write(&gb, wRoomObjects, 1);
    /* Write dummy value into WRAM bank 1 to verify bank 2 was read */
    gb_write(&gb, rSVBK, 1);
    gb_write(&gb, wRoomObjects, 99);

    WriteOverworldObjectToBG(&gb, vBGMap0 + 4, wRoomObjects, mock_get_bg_attributes);

    /* Verify tiles copied to vBGMap0 upper and lower rows in VRAM bank 0 */
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 4) == 0xA1, "Overworld object top-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 5) == 0xA2, "Overworld object top-right tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 4 + 0x20) == 0xA3, "Overworld object bottom-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 5 + 0x20) == 0xA4, "Overworld object bottom-right tile mismatch");

    /* Verify attributes copied to VRAM bank 1 */
    gb_write(&gb, rVBK, 1);
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 4) == 0x05, "Overworld object top-left attribute mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 5) == 0x06, "Overworld object top-right attribute mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 4 + 0x20) == 0x07, "Overworld object bottom-left attribute mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 5 + 0x20) == 0x08, "Overworld object bottom-right attribute mismatch");
    gb_write(&gb, rVBK, 0);

    TEST_ASSERT(gb.wram_bank == 1, "WRAM bank not restored to 1");

    /* 2. WriteIndoorObjectToBG - default indoor */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, 1);
    gb_write(&gb, hMapRoom, 0);
    gb_write(&gb, wRoomObjects, 1);

    WriteIndoorObjectToBG(&gb, vBGMap0 + 8, wRoomObjects, mock_get_bg_attributes);

    TEST_ASSERT(gb_read(&gb, vBGMap0 + 8) == 0xB1, "Indoor object top-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 9) == 0xB2, "Indoor object top-right tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 8 + 0x20) == 0xB3, "Indoor object bottom-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 9 + 0x20) == 0xB4, "Indoor object bottom-right tile mismatch");

    gb_write(&gb, rVBK, 1);
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 8) == 0x05, "Indoor object top-left attribute mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 8 + 0x20) == 0x07, "Indoor object bottom-left attribute mismatch");
    gb_write(&gb, rVBK, 0);

    /* 3. WriteIndoorObjectToBG - Color Dungeon */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_COLOR_DUNGEON);
    gb_write(&gb, wRoomObjects, 1);

    WriteIndoorObjectToBG(&gb, vBGMap0 + 12, wRoomObjects, mock_get_bg_attributes);

    TEST_ASSERT(gb_read(&gb, vBGMap0 + 12) == 0xC1, "Color Dungeon object top-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 13) == 0xC2, "Color Dungeon object top-right tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 12 + 0x20) == 0xC3, "Color Dungeon object bottom-left tile mismatch");
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 13 + 0x20) == 0xC4, "Color Dungeon object bottom-right tile mismatch");

    /* 4. WriteIndoorObjectToBG - Camera shop */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_HOUSE);
    gb_write(&gb, hMapRoom, ROOM_INDOOR_B_CAMERA_SHOP);
    gb_write(&gb, wRoomObjects, 1);

    WriteIndoorObjectToBG(&gb, vBGMap0 + 16, wRoomObjects, mock_get_bg_attributes);

    TEST_ASSERT(gb_read(&gb, vBGMap0 + 16) == 0xC1, "Camera shop object top-left tile mismatch");
}

static void test_load_room_tilemap(void) {
    GBState gb;

    /* 1. DMG path */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hMapId, 0);
    gb_write(&gb, wRoomObjects, 1); /* First object has ID 1 */

    minimap_arrow_called = false;
    LoadRoomTilemap(&gb, NULL, mock_update_minimap_arrow);

    TEST_ASSERT(minimap_arrow_called, "UpdateMinimapEntranceArrow callback was not called in DMG");
    TEST_ASSERT(gb.rom_bank == 1, "ROM bank not restored to 1");
    TEST_ASSERT(gb_read(&gb, vBGMap0) == 0x11, "DMG tile 0 mismatch at vBGMap0");

    /* 2. GBC Overworld path with row line wrap */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hMapId, 0);

    /* Setup object at row 0, col 0 ($D711) */
    gb_write(&gb, rSVBK, 2);
    gb_write(&gb, wRoomObjects, 1);
    /* Setup object at row 1, col 0 ($D721) */
    gb_write(&gb, wRoomObjects + 0x10, 1);
    gb_write(&gb, rSVBK, 0);

    minimap_arrow_called = false;
    LoadRoomTilemap(&gb, mock_get_bg_attributes, mock_update_minimap_arrow);

    TEST_ASSERT(minimap_arrow_called, "UpdateMinimapEntranceArrow callback was not called in GBC");
    /* Verify first object at (0, 0) -> vBGMap0 */
    TEST_ASSERT(gb_read(&gb, vBGMap0) == 0xA1, "GBC row 0 col 0 tile mismatch");
    gb_write(&gb, rVBK, 1);
    TEST_ASSERT(gb_read(&gb, vBGMap0) == 0x05, "GBC row 0 col 0 attribute mismatch");
    gb_write(&gb, rVBK, 0);

    /* Verify second row first object at (row 1, col 0) -> vBGMap0 + 0x40 */
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 0x40) == 0xA1, "GBC row 1 col 0 tile mismatch (row jump failed)");
    gb_write(&gb, rVBK, 1);
    TEST_ASSERT(gb_read(&gb, vBGMap0 + 0x40) == 0x05, "GBC row 1 col 0 attribute mismatch");
    gb_write(&gb, rVBK, 0);
}


static bool mock_marin_portrait_called = false;
static void mock_load_marin_portrait_cb(GBState *gb) {
    (void)gb;
    mock_marin_portrait_called = true;
}

static void test_load_credits_marin_portrait_tiles_trampoline(void) {
    GBState gb;
    gb_init(&gb);
    mock_marin_portrait_called = false;
    LoadCreditsMarinPortraitTiles_trampoline(&gb, mock_load_marin_portrait_cb);
    TEST_ASSERT(mock_marin_portrait_called, "LoadCreditsMarinPortraitTiles callback not invoked");
    TEST_ASSERT(gb.rom_bank == 0x27, "Bank not switched to 0x27 for Marin portrait");
}

static bool mock_thanks_called = false;
static void mock_load_thanks_cb(GBState *gb) {
    (void)gb;
    mock_thanks_called = true;
}

static void test_load_thanks_for_playing_tiles_trampoline(void) {
    GBState gb;
    gb_init(&gb);
    mock_thanks_called = false;
    LoadThanksForPlayingTiles_trampoline(&gb, mock_load_thanks_cb);
    TEST_ASSERT(mock_thanks_called, "LoadThanksForPlayingTiles callback not invoked");
    TEST_ASSERT(gb.rom_bank == 0x20, "Bank not switched to 0x20 for Thanks for playing tiles");
}


static void test_indoor_and_base_overworld_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));

    /* 1. func_2D50 */
    gb_write(&gb, hIsGBC, 0);
    func_2D50(&gb);
    TEST_ASSERT(gb_read(&gb, hAnimatedTilesFrameCount) == 0, "hAnimatedTilesFrameCount not cleared");
    TEST_ASSERT(gb_read(&gb, hAnimatedTilesDataOffset) == 0, "hAnimatedTilesDataOffset not cleared");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x6B, "func_2D50 InventoryEquipmentItemsTiles not copied");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x200) == 0x00, "func_2D50 LinkCharacterTiles offset check");

    /* 2. PatchInventoryTiles */
    /* Case A: Toadstool present */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, wHasToadstool, 1);
    PatchInventoryTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x0E0) == 0x2A, "Toadstool tile not patched to 0x2A");

    /* Case B: Golden leaf present and Overworld */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, wGoldenLeavesCount, SLIME_KEY);
    PatchInventoryTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x4A0) == 0x2C, "Golden leaf tile not patched to 0x2C");

    /* Case C: Trading sequence item */
    gb_init(&gb);
    gb_write(&gb, wTradeSequenceItem, TRADING_ITEM_RIBBON);
    PatchInventoryTiles(&gb);
    TEST_ASSERT(gb_read(&gb, hReplaceTiles) == REPLACE_TILES_TRADING_ITEM, "hReplaceTiles not set to REPLACE_TILES_TRADING_ITEM");

    /* 3. LoadBaseOverworldTiles */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    LoadBaseOverworldTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles2 + 0x200) == 0x73, "OverworldLandscapeTiles not copied to vTiles2+0x200");
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x400) == 0x74, "InventoryOverworldItemsTiles not copied to vTiles1+0x400");
    TEST_ASSERT(gb_read(&gb, vTiles1) == 0x6B, "func_2D50 not called in LoadBaseOverworldTiles");

    /* 4. LoadIndoorTiles (standard dungeon, Map 1) */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, 0x01);
    LoadIndoorTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles2 + 0x100) == 0x31, "Floor tiles not copied to vTiles2+0x100");
    TEST_ASSERT(gb_read(&gb, vTiles2 + 0x200) == 0x33, "Walls tiles not copied to vTiles2+0x200");
    TEST_ASSERT(gb_read(&gb, wAnimatedScrollingTilesStorage) == 0x75, "Animated scrolling storage not copied");
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x700) == 0x41, "Indoor objects not copied to vTiles1+0x700");
    TEST_ASSERT(gb_read(&gb, vTiles1 + 0x400) == 0x42, "Indoor items not copied to vTiles1+0x400");
}


static void test_reload_color_dungeon_npc_tiles(void) {
    GBState gb;
    gb_init(&gb);
    gb_attach_rom(&gb, mock_rom, sizeof(mock_rom));

    /* DMG (bank 0x34) */
    mock_rom[ROM_BANK_OFFSET(0x34, ColorDungeonNpcTiles)] = 0xC4;
    gb_write(&gb, hIsGBC, 0);
    ReloadColorDungeonNpcTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x400) == 0xC4, "DMG Color dungeon NPC tile mismatch");
    TEST_ASSERT(gb.rom_bank == 0x20, "ROM bank not restored to 0x20");

    /* CGB (bank 0x35) */
    mock_rom[ROM_BANK_OFFSET(0x35, ColorDungeonNpcTiles)] = 0xC5;
    gb_write(&gb, hIsGBC, 1);
    ReloadColorDungeonNpcTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x400) == 0xC5, "CGB Color dungeon NPC tile mismatch");
    TEST_ASSERT(gb.rom_bank == 0x20, "ROM bank not restored to 0x20");
}

#define RUN_GFX_TEST(name, code) \
    do { \
        int _prev = gfx_failures; \
        printf("[RUN ] %s\n", name); \
        code; \
        if (gfx_failures == _prev) { \
            printf("[PASS] %s\n", name); \
        } else { \
            printf("[FAIL] %s\n", name); \
        } \
    } while (0)

int run_gfx_tests(void) {
    printf("[TEST] GFX\n");
    RUN_GFX_TEST("ReloadColorDungeonNpcTiles", test_reload_color_dungeon_npc_tiles());
    RUN_GFX_TEST("IndoorAndBaseOverworldTiles", {
        setup_mock_data();
        test_indoor_and_base_overworld_tiles();
    });
    RUN_GFX_TEST("LoadCreditsKoholintDisappearingTiles", test_load_credits_koholint_disappearing_tiles());
    RUN_GFX_TEST("LoadTileset15", test_load_tileset_15());
    RUN_GFX_TEST("LoadCreditsStairsTiles", test_load_credits_stairs_tiles());
    RUN_GFX_TEST("LoadCreditsKoholintViewsTiles", test_load_credits_koholint_views_tiles());
    RUN_GFX_TEST("LoadCreditsLinkOnSea", test_load_credits_link_on_sea_dmg_and_cgb());
    RUN_GFX_TEST("EndingSceneTilesAndCreditsRoll", test_ending_scene_tiles_and_credits_roll());
    RUN_GFX_TEST("LoadBaseAndMenuTiles", test_load_base_and_menu_tiles());
    RUN_GFX_TEST("LoadIntroSequenceTiles", test_load_intro_sequence_tiles());
    RUN_GFX_TEST("LoadTitleScreenTiles", test_load_title_screen_tiles());
    RUN_GFX_TEST("LoadWorldMapTiles", test_load_world_map_tiles());
    RUN_GFX_TEST("LoadStaticPicturesTiles", test_load_static_pictures_tiles());
    RUN_GFX_TEST("LoadEaglesTowerTopTiles", test_load_eagles_tower_top_tiles());
    RUN_GFX_TEST("LoadMarinBeachTiles", test_load_marin_beach_tiles());
    RUN_GFX_TEST("LoadSaveMenuTiles", test_load_save_menu_tiles());
    RUN_GFX_TEST("CopyWord", test_copy_word());
    RUN_GFX_TEST("SwitchToObjectsTilemapBank", test_switch_to_objects_tilemap_bank());
    RUN_GFX_TEST("WriteObjectToBGDmg", test_write_object_to_bg_dmg());
    RUN_GFX_TEST("LoadRoomSpecificTiles", test_load_room_specific_tiles());
    RUN_GFX_TEST("WriteOverworldAndIndoorObjectToBG", test_write_overworld_and_indoor_object_to_bg());
    RUN_GFX_TEST("LoadRoomTilemap", test_load_room_tilemap());
    RUN_GFX_TEST("LoadCreditsMarinPortraitTilesTrampoline", test_load_credits_marin_portrait_tiles_trampoline());
    RUN_GFX_TEST("LoadThanksForPlayingTilesTrampoline", test_load_thanks_for_playing_tiles_trampoline());

    if (gfx_failures == 0) {
        printf("[PASS] GFX\n\n");
    } else {
        printf("[FAIL] GFX (%d failures)\n\n", gfx_failures);
    }
    return gfx_failures;
}
