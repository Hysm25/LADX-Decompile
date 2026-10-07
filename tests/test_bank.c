#include <stdio.h>
#include <assert.h>
#include "gb.h"
#include "home/bank.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/gfx.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static void test_adjust_bank_number_for_gbc(void) {
    GBState gb;
    gb_init(&gb);

    /* Test DMG mode (hIsGBC = 0) */
    gb_write(&gb, hIsGBC, 0);
    TEST_ASSERT(AdjustBankNumberForGBC(&gb, 0x05) == 0x05, "DMG bank 0x05 altered unexpectedly");
    TEST_ASSERT(AdjustBankNumberForGBC(&gb, 0x12) == 0x12, "DMG bank 0x12 altered unexpectedly");

    /* Test GBC mode (hIsGBC = 1) */
    gb_write(&gb, hIsGBC, 1);
    TEST_ASSERT(AdjustBankNumberForGBC(&gb, 0x05) == 0x25, "GBC bank 0x05 bit 5 not set");
    TEST_ASSERT(AdjustBankNumberForGBC(&gb, 0x12) == 0x32, "GBC bank 0x12 bit 5 not set");
}

static void test_switch_bank(void) {
    GBState gb;
    gb_init(&gb);

    SwitchBank(&gb, 0x07);
    TEST_ASSERT(gb.rom_bank == 0x07, "ROM bank not switched to 0x07");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x07, "wCurrentBank not updated to 0x07");
}

static void test_switch_adjusted_bank(void) {
    GBState gb;
    gb_init(&gb);

    /* DMG mode */
    gb_write(&gb, hIsGBC, 0);
    SwitchAdjustedBank(&gb, 0x03);
    TEST_ASSERT(gb.rom_bank == 0x03, "DMG SwitchAdjustedBank did not set bank 3");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x03, "DMG wCurrentBank not 3");

    /* GBC mode */
    gb_write(&gb, hIsGBC, 1);
    SwitchAdjustedBank(&gb, 0x03);
    TEST_ASSERT(gb.rom_bank == 0x23, "GBC SwitchAdjustedBank did not set bank 0x23");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x23, "GBC wCurrentBank not 0x23");
}

static void test_reload_saved_bank(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wCurrentBank, 0x14);
    gb.rom_bank = 0x01;

    ReloadSavedBank(&gb);
    TEST_ASSERT(gb.rom_bank == 0x14, "ReloadSavedBank did not restore wCurrentBank");
}

static void test_restore_bank_and_return(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wCurrentBank, 0x0D);
    gb.rom_bank = 0x02;

    RestoreBankAndReturn(&gb);
    TEST_ASSERT(gb.rom_bank == 0x0D, "RestoreBankAndReturn did not reload wCurrentBank");
}

static void test_load_bank1_and_return(void) {
    GBState gb;
    gb_init(&gb);

    gb.rom_bank = 0x18;
    LoadBank1AndReturn(&gb);
    TEST_ASSERT(gb.rom_bank == 0x01, "LoadBank1AndReturn did not switch to bank 1");
}

static void test_restore_stacked_bank_and_return(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wCurrentBank, 0x03);
    gb.rom_bank = 0x09;

    RestoreStackedBankAndReturn(&gb, 0x11);
    TEST_ASSERT(gb.rom_bank == 0x11, "RestoreStackedBankAndReturn did not set stacked bank");
    /* wCurrentBank should remain unchanged */
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x03, "wCurrentBank should not be changed");
}

static void test_restore_stacked_bank(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wCurrentBank, 0x04);
    gb.rom_bank = 0x09;

    RestoreStackedBank(&gb, 0x15);
    TEST_ASSERT(gb.rom_bank == 0x15, "RestoreStackedBank did not set stacked bank in rom_bank");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x15, "RestoreStackedBank did not update wCurrentBank");
}

static void dummy_farcall_target(GBState *gb) {
    TEST_ASSERT(gb->rom_bank == 0x17, "ROM bank not switched to target during Farcall");
    gb_write(gb, 0xC100, 0x55);
}

static void test_farcall(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wFarcallBank, 0x17);
    gb_write(&gb, wFarcallReturnBank, 0x05);
    gb.rom_bank = 0x02;

    Farcall(&gb, dummy_farcall_target);

    TEST_ASSERT(gb_read(&gb, 0xC100) == 0x55, "Farcall did not execute target function");
    TEST_ASSERT(gb.rom_bank == 0x05, "Farcall did not switch back to wFarcallReturnBank");

    /* Test Farcall_trampoline */
    gb_write(&gb, wFarcallAdressHigh, 0x4E);
    gb_write(&gb, wFarcallAdressLow, 0x2A);
    uint16_t addr = Farcall_trampoline(&gb);
    TEST_ASSERT(addr == 0x4E2A, "Farcall_trampoline did not return correct 16-bit address");
}

static void test_backup_object_in_ram2(void) {
    GBState gb;
    gb_init(&gb);

    /* 1. On DMG (hIsGBC = 0), returns immediately without copying or changing ROM bank */
    gb_write(&gb, hIsGBC, 0);
    gb.wram[1][0x500] = 0x03; /* Overworld object at $D500 */
    gb.wram[2][0x500] = 0x00;
    gb.rom_bank = 0x01;
    BackupObjectInRAM2(&gb, 0xD500, 0x04);
    TEST_ASSERT(gb.wram[2][0x500] == 0x00, "DMG should not backup object to RAM2");
    TEST_ASSERT(gb.rom_bank == 0x01, "DMG should return immediately without restoring bank");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose2) == 0x04, "hMultiPurpose2 not set on DMG");

    /* 2. GBC mode, object IS in ignore list (0x03), bit 7 clear -> copy performed */
    gb_write(&gb, hIsGBC, 1);
    gb.wram[1][0x500] = 0x03;
    gb.wram[2][0x500] = 0x00;
    BackupObjectInRAM2(&gb, 0xD500, 0x05);
    TEST_ASSERT(gb.wram[2][0x500] == 0x03, "GBC in-list object was not backed up to RAM2");
    TEST_ASSERT(gb.rom_bank == 0x05, "ROM bank not set to return_bank 0x05");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose2) == 0x05, "hMultiPurpose2 not set");

    /* 3. GBC mode, object NOT in ignore list (0x42), bit 7 clear -> skipped */
    gb.wram[1][0x501] = 0x42;
    gb.wram[2][0x501] = 0x00;
    BackupObjectInRAM2(&gb, 0xD501, 0x06);
    TEST_ASSERT(gb.wram[2][0x501] == 0x00, "Object not in ignore list should be skipped");
    TEST_ASSERT(gb.rom_bank == 0x06, "ROM bank not restored when skipped");

    /* 4. GBC mode, object NOT in ignore list (0x42), bit 7 set -> forced backup */
    gb.wram[1][0x502] = 0x42;
    gb.wram[2][0x502] = 0x00;
    BackupObjectInRAM2(&gb, 0xD502, 0x87);
    TEST_ASSERT(gb.wram[2][0x502] == 0x42, "Forced backup (bit 7 set) did not backup object");
    TEST_ASSERT(gb.rom_bank == 0x07, "ROM bank not set to return_bank & 0x7F");
}

static void test_copy_objects_attributes_to_wram2(void) {
    GBState gb;
    gb_init(&gb);

    /* Allocate simulated ROM buffer with 4 banks */
    uint8_t test_rom[0x10000];
    for (int i = 0; i < 0x10000; i++) {
        test_rom[i] = 0;
    }
    /* Bank 3 is at offset 0xC000 in ROM */
    test_rom[0xC000] = 0xDE;
    test_rom[0xC001] = 0xAD;
    test_rom[0xC002] = 0xBE;
    test_rom[0xC003] = 0xEF;

    gb_attach_rom(&gb, test_rom, sizeof(test_rom));

    /* Set source bank in hMultiPurpose0 to 3 */
    gb_write(&gb, hMultiPurpose0, 3);

    /* Copy 4 bytes from 0x4000 to WRAM2 (0xD100) */
    CopyObjectsAttributesToWRAM2(&gb, 0xD100, 0x4000, 4);

    /* Check WRAM bank 2 contains copied data */
    TEST_ASSERT(gb.wram[2][0x100] == 0xDE, "WRAM2 byte 0 incorrect");
    TEST_ASSERT(gb.wram[2][0x101] == 0xAD, "WRAM2 byte 1 incorrect");
    TEST_ASSERT(gb.wram[2][0x102] == 0xBE, "WRAM2 byte 2 incorrect");
    TEST_ASSERT(gb.wram[2][0x103] == 0xEF, "WRAM2 byte 3 incorrect");

    /* Verify WRAM bank was restored to 0 (or 1) and ROM bank to 0x20 */
    TEST_ASSERT(gb.wram_bank == 1 || gb.wram_bank == 0, "WRAM bank not restored");
    TEST_ASSERT(gb.rom_bank == 0x20, "ROM bank not restored to 0x20");
}

static int tramp_called = 0;
static void hook_bank20(GBState *gb) {
    tramp_called++;
    TEST_ASSERT(gb->rom_bank == 0x20, "Target callback not executed in bank 0x20");
}

static void hook_generic(GBState *gb) {
    tramp_called++;
    (void)gb;
}

static void test_bank_trampolines(void) {
    GBState gb;

    /* 1. func_020_6A30_trampoline -> executes in 0x20, restores wCurrentBank */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x05);
    gb.rom_bank = 0x05;
    tramp_called = 0;
    func_020_6A30_trampoline(&gb, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_020_6A30_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x05, "func_020_6A30_trampoline did not restore wCurrentBank");

    /* func_020_4B4A_trampoline -> executes in 0x20, restores wCurrentBank */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x02);
    gb.rom_bank = 0x02;
    tramp_called = 0;
    func_020_4B4A_trampoline(&gb, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_020_4B4A_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x02, "func_020_4B4A_trampoline did not restore wCurrentBank");

    /* 2. func_020_6AC1_trampoline -> executes in 0x20, restores wCurrentBank */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x08);
    gb.rom_bank = 0x08;
    tramp_called = 0;
    func_020_6AC1_trampoline(&gb, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_020_6AC1_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x08, "func_020_6AC1_trampoline did not restore wCurrentBank");

    /* 3. UpdateIntroSeaBGPalettes_trampoline -> executes in 0x20, restores wCurrentBank */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x0A);
    gb.rom_bank = 0x0A;
    tramp_called = 0;
    UpdateIntroSeaBGPalettes_trampoline(&gb, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "UpdateIntroSeaBGPalettes_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x0A, "UpdateIntroSeaBGPalettes_trampoline did not restore wCurrentBank");

    /* 4. ClearFileMenuBG_trampoline -> executes in 0x20, restores stacked bank */
    gb_init(&gb);
    tramp_called = 0;
    ClearFileMenuBG_trampoline(&gb, 0x14, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "ClearFileMenuBG_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x14, "ClearFileMenuBG_trampoline did not restore stacked bank (0x14)");

    /* 5. LoadFileMenuBG_trampoline -> executes in 0x20, loads bank 1 */
    gb_init(&gb);
    tramp_called = 0;
    LoadFileMenuBG_trampoline(&gb, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "LoadFileMenuBG_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x01, "LoadFileMenuBG_trampoline did not load bank 1");

    /* 6. CopyLinkTunicPalette_trampoline -> executes in 0x20, loads bank 1 */
    gb_init(&gb);
    tramp_called = 0;
    CopyLinkTunicPalette_trampoline(&gb, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "CopyLinkTunicPalette_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x01, "CopyLinkTunicPalette_trampoline did not load bank 1");

    /* 7. LoadBaseTiles_trampoline -> executes callback, restores stacked bank */
    gb_init(&gb);
    tramp_called = 0;
    LoadBaseTiles_trampoline(&gb, 0x25, hook_generic);
    TEST_ASSERT(tramp_called == 1, "LoadBaseTiles_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x25, "LoadBaseTiles_trampoline did not restore stacked bank (0x25)");
}

static void hook_bank24(GBState *gb) {
    tramp_called++;
    TEST_ASSERT(gb->rom_bank == 0x24, "Target callback not executed in bank 0x24");
}

static void hook_bank0f(GBState *gb) {
    tramp_called++;
    TEST_ASSERT(gb->rom_bank == BANK_FontTiles, "Target callback not executed in BANK(FontTiles)");
    TEST_ASSERT(gb_read(gb, wCurrentBank) == BANK_FontTiles, "wCurrentBank not set to BANK(FontTiles)");
}

static void hook_bank36_switch(GBState *gb) {
    tramp_called++;
    TEST_ASSERT(gb->rom_bank == 0x36, "Target callback not executed in bank 0x36");
    TEST_ASSERT(gb_read(gb, wCurrentBank) == 0x36, "wCurrentBank not set to 0x36");
}

static void hook_bank36_select(GBState *gb) {
    tramp_called++;
    TEST_ASSERT(gb->rom_bank == 0x36, "Target callback not executed in bank 0x36");
}

static void hook_bank3d(GBState *gb) {
    tramp_called++;
    TEST_ASSERT(gb->rom_bank == 0x3D, "Target callback not executed in bank 0x3D");
}

static void hook_bank03(GBState *gb) {
    tramp_called++;
    TEST_ASSERT(gb->rom_bank == 0x03, "Target callback not executed in bank 0x03");
}

static void test_bank_trampolines_batch2(void) {
    GBState gb;

    /* 1. ChangeBGColumnPaletteAndExecuteDrawCommands */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x02);
    /* Setup draw command at wDrawCommand (0xD601) */
    gb_write(&gb, wDrawCommand + 0, 0x98);
    gb_write(&gb, wDrawCommand + 1, 0x20);
    gb_write(&gb, wDrawCommand + 2, 0x01); /* copy row, count = 2 (0x01 + 1) */
    gb_write(&gb, wDrawCommand + 3, 0x44);
    gb_write(&gb, wDrawCommand + 4, 0x55);
    gb_write(&gb, wDrawCommand + 5, 0x00); /* end */
    tramp_called = 0;
    ChangeBGColumnPaletteAndExecuteDrawCommands(&gb, 0x12, hook_bank24);
    TEST_ASSERT(tramp_called == 1, "ChangeBGColumnPaletteAndExecuteDrawCommands callback not called");
    TEST_ASSERT(gb_read(&gb, 0x9820) == 0x44, "Draw commands not executed (byte 0)");
    TEST_ASSERT(gb_read(&gb, 0x9821) == 0x55, "Draw commands not executed (byte 1)");
    TEST_ASSERT(gb.rom_bank == 0x12, "ROM bank not restored to stacked bank (0x12)");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x12, "wCurrentBank not restored to stacked bank (0x12)");

    /* 2. func_A9B */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x01);
    tramp_called = 0;
    func_A9B(&gb, 0x07, hook_bank0f);
    TEST_ASSERT(tramp_called == 1, "func_A9B callback not called");
    TEST_ASSERT(gb.rom_bank == 0x07, "ROM bank not restored to stacked bank (0x07)");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == BANK_FontTiles, "wCurrentBank should remain at BANK(FontTiles)");

    /* 3. Spawn2x2RubbleEntities_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x01);
    tramp_called = 0;
    Spawn2x2RubbleEntities_trampoline(&gb, 0x04, hook_bank36_switch);
    TEST_ASSERT(tramp_called == 1, "Spawn2x2RubbleEntities_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x04, "ROM bank not restored to stacked bank (0x04)");
    TEST_ASSERT(gb_read(&gb, wCurrentBank) == 0x04, "wCurrentBank not restored to stacked bank (0x04)");

    /* 4. func_A5F */
    gb_init(&gb);
    tramp_called = 0;
    func_A5F(&gb, 0x08, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_A5F callback not called");
    TEST_ASSERT(gb.rom_bank == 0x08, "func_A5F did not restore stacked bank (0x08)");

    /* 5. func_036_703E_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_036_703E_trampoline(&gb, 0x09, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "func_036_703E_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x09, "func_036_703E_trampoline did not restore stacked bank (0x09)");

    /* 6. cycleInstrumentItemColor_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    cycleInstrumentItemColor_trampoline(&gb, 0x0A, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "cycleInstrumentItemColor_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x0A, "cycleInstrumentItemColor_trampoline did not restore stacked bank (0x0A)");

    /* 7. func_036_4A77_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x02);
    tramp_called = 0;
    func_036_4A77_trampoline(&gb, 0x0B, hook_bank36_switch);
    TEST_ASSERT(tramp_called == 1, "func_036_4A77_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x0B, "func_036_4A77_trampoline did not restore stacked bank (0x0B)");

    /* 8. GetOwlStatueDialogId_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    GetOwlStatueDialogId_trampoline(&gb, 0x0C, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "GetOwlStatueDialogId_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x0C, "GetOwlStatueDialogId_trampoline did not restore stacked bank (0x0C)");

    /* 9. SpawnPhotographer_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    SpawnPhotographer_trampoline(&gb, 0x0D, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "SpawnPhotographer_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x0D, "SpawnPhotographer_trampoline did not restore stacked bank (0x0D)");

    /* 10. LoadPhotoBgMap_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    LoadPhotoBgMap_trampoline(&gb, hook_bank3d);
    TEST_ASSERT(tramp_called == 1, "LoadPhotoBgMap_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x3D, "LoadPhotoBgMap_trampoline ROM bank should be 0x3D");
}

static void test_bank_trampolines_batch3(void) {
    GBState gb;

    /* 1. func_020_6D0E_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_020_6D0E_trampoline(&gb, 0x10, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_020_6D0E_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x10, "func_020_6D0E_trampoline did not restore stacked bank");

    /* 2. CheckPushedTombStone_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    CheckPushedTombStone_trampoline(&gb, 0x11, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "CheckPushedTombStone_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x11, "CheckPushedTombStone_trampoline did not restore stacked bank");

    /* 3. GetEntityInitHandler_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    GetEntityInitHandler_trampoline(&gb, 0x12, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "GetEntityInitHandler_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x12, "GetEntityInitHandler_trampoline did not restore stacked bank");

    /* 4. func_020_4874_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_020_4874_trampoline(&gb, 0x13, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_020_4874_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x13, "func_020_4874_trampoline did not restore stacked bank");

    /* 5. func_020_4954_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_020_4954_trampoline(&gb, 0x14, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_020_4954_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x14, "func_020_4954_trampoline did not restore stacked bank");

    /* 6. ReplaceObjects56and57_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    ReplaceObjects56and57_trampoline(&gb, 0x15, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "ReplaceObjects56and57_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x15, "ReplaceObjects56and57_trampoline did not restore stacked bank");

    /* 7. func_036_505F_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_036_505F_trampoline(&gb, 0x16, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "func_036_505F_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x16, "func_036_505F_trampoline did not restore stacked bank");

    /* 8. func_036_4F9B_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_036_4F9B_trampoline(&gb, 0x17, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "func_036_4F9B_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x17, "func_036_4F9B_trampoline did not restore stacked bank");

    /* 9. func_003_5A2E_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_003_5A2E_trampoline(&gb, 0x18, hook_bank03);
    TEST_ASSERT(tramp_called == 1, "func_003_5A2E_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x18, "func_003_5A2E_trampoline did not restore stacked bank");

    /* 10. func_036_4F68_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_036_4F68_trampoline(&gb, 0x19, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "func_036_4F68_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x19, "func_036_4F68_trampoline did not restore stacked bank");

    /* 11. func_020_6D52_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_020_6D52_trampoline(&gb, 0x1A, hook_bank20);
    TEST_ASSERT(tramp_called == 1, "func_020_6D52_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x1A, "func_020_6D52_trampoline did not restore stacked bank");

    /* 12. func_036_4BE8_trampoline */
    gb_init(&gb);
    tramp_called = 0;
    func_036_4BE8_trampoline(&gb, 0x1B, hook_bank36_select);
    TEST_ASSERT(tramp_called == 1, "func_036_4BE8_trampoline callback not called");
    TEST_ASSERT(gb.rom_bank == 0x1B, "func_036_4BE8_trampoline did not restore stacked bank");
}

static void mock_get_bg_attr_addr(GBState *gb) {
    TEST_ASSERT(gb->rom_bank == 0x1A, "GetBGAttributesAddressForObject must be called in bank $1A");
    /* Mock returning bank 0x02 and ROM address 0x5010 */
    gb_write(gb, hMultiPurpose8, 0x02);
    gb_write(gb, hMultiPurpose9, 0x50);
    gb_write(gb, hMultiPurposeA, 0x10);
}

static uint8_t mock_palette_counter = 0;
static void mock_01a_6710(GBState *gb) {
    TEST_ASSERT(gb->rom_bank == 0x1A, "func_01A_6710 must be called in bank $1A");
    gb_write(gb, hMultiPurpose8, 0x03);
    gb_write(gb, hMultiPurpose9, 0x51);
    gb_write(gb, hMultiPurposeA, (uint8_t)(0x20 + (mock_palette_counter++)));
}

static void test_bg_attributes_draw_commands(void) {
    GBState gb;
    static uint8_t mock_test_rom[0x10000];

    /* Initialize mock ROM bytes */
    /* Bank 2 at 0x5010 -> offset 2 * 0x4000 + 0x1010 = 0x9010 */
    mock_test_rom[0x9010] = 0xA1; /* TL */
    mock_test_rom[0x9011] = 0xB2; /* TR */
    mock_test_rom[0x9012] = 0xC3; /* BL */
    mock_test_rom[0x9013] = 0xD4; /* BR */

    /* Bank 3 at 0x5120 -> offset 3 * 0x4000 + 0x1120 = 0xD120 */
    mock_test_rom[0xD120] = 0x7E; /* for test 3 */
    mock_test_rom[0xD121] = 0x11; /* for test 4 val0 */
    mock_test_rom[0xD122] = 0x22; /* for test 4 val1 */

    /* 1. Test func_91D */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_test_rom, sizeof(mock_test_rom));
    gb_write(&gb, wDDD8, 0x04); /* bc = 4 << 2 = 16 */
    gb_write(&gb, hIntersectedObjectBGAddressHigh, 0x98);
    gb_write(&gb, hIntersectedObjectBGAddressLow, 0x24);
    gb_write(&gb, wDrawCommandsVRAM1Size, 0x00);

    func_91D(&gb, 0x07, mock_get_bg_attr_addr);

    TEST_ASSERT(gb.rom_bank == 0x07, "func_91D did not restore stacked bank");
    TEST_ASSERT(gb_read(&gb, wDrawCommandsVRAM1Size) == 0x0A, "func_91D did not add 10 to wDrawCommandsVRAM1Size");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 0) == 0x98, "func_91D byte 0 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 1) == 0x24, "func_91D byte 1 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 2) == 0x81, "func_91D byte 2 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 3) == 0xA1, "func_91D byte 3 (TL) wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 4) == 0xC3, "func_91D byte 4 (BL) wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 5) == 0x98, "func_91D byte 5 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 6) == 0x25, "func_91D byte 6 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 7) == 0x81, "func_91D byte 7 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 8) == 0xB2, "func_91D byte 8 (TR) wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 9) == 0xD4, "func_91D byte 9 (BR) wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 10) == 0x00, "func_91D terminator wrong");

    /* 2. Test func_91D_jp_92E appending to existing list */
    gb_write(&gb, hIntersectedObjectBGAddressHigh, 0x99);
    gb_write(&gb, hIntersectedObjectBGAddressLow, 0x30);
    func_91D_jp_92E(&gb, 0x0020, 0x09, mock_get_bg_attr_addr);

    TEST_ASSERT(gb.rom_bank == 0x09, "func_91D_jp_92E did not restore stacked bank");
    TEST_ASSERT(gb_read(&gb, wDrawCommandsVRAM1Size) == 0x14, "func_91D_jp_92E size not 20");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 10) == 0x99, "func_91D_jp_92E 2nd entry byte 0 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 11) == 0x30, "func_91D_jp_92E 2nd entry byte 1 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 20) == 0x00, "func_91D_jp_92E terminator wrong");

    /* 3. Test func_983 */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_test_rom, sizeof(mock_test_rom));
    mock_palette_counter = 0;
    uint16_t de = 0x2000;
    uint8_t val = func_983(&gb, &de, mock_01a_6710);
    TEST_ASSERT(val == 0x7E, "func_983 returned incorrect byte");
    TEST_ASSERT(de == 0x2001, "func_983 did not increment de");
    TEST_ASSERT(gb.rom_bank == 0x03, "func_983 did not switch to hMultiPurpose8");

    /* 4. Test func_999 */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_test_rom, sizeof(mock_test_rom));
    mock_palette_counter = 1; /* will read 0xD121 then 0xD122 */
    gb_write(&gb, hIntersectedObjectBGAddressHigh, 0x9A);
    gb_write(&gb, hIntersectedObjectBGAddressLow, 0x15);
    gb_write(&gb, wDrawCommandsVRAM1Size, 0x00);
    de = 0x3000;
    func_999(&gb, &de, 0x08, mock_01a_6710);

    TEST_ASSERT(gb.rom_bank == 0x08, "func_999 did not restore stacked bank");
    TEST_ASSERT(gb_read(&gb, wDrawCommandsVRAM1Size) == 0x05, "func_999 size not 5");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose0) == 0x11, "func_999 hMultiPurpose0 wrong");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose1) == 0x22, "func_999 hMultiPurpose1 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 0) == 0x9A, "func_999 byte 0 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 1) == 0x15, "func_999 byte 1 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 2) == 0x01, "func_999 byte 2 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 3) == 0x11, "func_999 byte 3 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 4) == 0x22, "func_999 byte 4 wrong");
    TEST_ASSERT(gb_read(&gb, wDrawCommandVRAM1 + 5) == 0x00, "func_999 terminator wrong");
}

#define RUN_BANK_TEST(name, code) \
    do { \
        int _prev = failures; \
        printf("[RUN ] %s\n", name); \
        code; \
        if (failures == _prev) { \
            printf("[PASS] %s\n", name); \
        } else { \
            printf("[FAIL] %s\n", name); \
        } \
    } while (0)

int run_bank_tests(void) {
    printf("[TEST] Bank Switching\n");
    RUN_BANK_TEST("AdjustBankNumberForGBC", test_adjust_bank_number_for_gbc());
    RUN_BANK_TEST("SwitchBank", test_switch_bank());
    RUN_BANK_TEST("SwitchAdjustedBank", test_switch_adjusted_bank());
    RUN_BANK_TEST("ReloadSavedBank", test_reload_saved_bank());
    RUN_BANK_TEST("RestoreBankAndReturn", test_restore_bank_and_return());
    RUN_BANK_TEST("LoadBank1AndReturn", test_load_bank1_and_return());
    RUN_BANK_TEST("RestoreStackedBankAndReturn", test_restore_stacked_bank_and_return());
    RUN_BANK_TEST("RestoreStackedBank", test_restore_stacked_bank());
    RUN_BANK_TEST("Farcall", test_farcall());
    RUN_BANK_TEST("BackupObjectInRAM2", test_backup_object_in_ram2());
    RUN_BANK_TEST("CopyObjectsAttributesToWRAM2", test_copy_objects_attributes_to_wram2());
    RUN_BANK_TEST("BankTrampolines", {
        test_bank_trampolines();
        test_bank_trampolines_batch2();
        test_bank_trampolines_batch3();
    });
    RUN_BANK_TEST("BGAttributesDrawCommands", test_bg_attributes_draw_commands());

    if (failures == 0) {
        printf("[PASS] Bank Switching\n\n");
        return 0;
    } else {
        printf("[FAIL] Bank Switching (%d failures)\n\n", failures);
        return 1;
    }
}
