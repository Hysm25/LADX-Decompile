#include <stdio.h>
#include <assert.h>
#include "gb.h"
#include "home/audio.h"
#include "constants/audio.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/sfx.h"

void test_play_wrong_answer_jingle(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, hJingle, 0x00);
    PlayWrongAnswerJingle(&gb);
    assert(gb_read(&gb, hJingle) == JINGLE_WRONG_ANSWER);
}

void test_alert_sword_moblins(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wSwordMoblinAlertingSoundCounter, 0x00);
    AlertSwordMoblins(&gb);
    assert(gb_read(&gb, wSwordMoblinAlertingSoundCounter) == 4);
}

void test_play_bomb_explosion_sfx(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, hNoiseSfx, 0x00);
    gb_write(&gb, wSwordMoblinAlertingSoundCounter, 0x00);

    PlayBombExplosionSfx(&gb);

    assert(gb_read(&gb, hNoiseSfx) == NOISE_SFX_EXPLOSION);
    assert(gb_read(&gb, wSwordMoblinAlertingSoundCounter) == 4);
}

static int sfx_calls = 0;
static int m1b_calls = 0;
static int m1e_calls = 0;

static void mock_play_sfx(GBState *gb) {
    sfx_calls++;
    assert(gb->rom_bank == 0x1F);
}

static void mock_play_music_1b(GBState *gb) {
    m1b_calls++;
    assert(gb->rom_bank == 0x1B);
}

static void mock_play_music_1e(GBState *gb) {
    m1e_calls++;
    assert(gb->rom_bank == 0x1E);
}

void test_play_audio_step(void) {
    GBState gb;
    gb_init(&gb);

    /* 1. Wave SFX active -> skips music handlers */
    sfx_calls = m1b_calls = m1e_calls = 0;
    gb_write(&gb, hWaveSfx, 0x01);
    PlayAudioStepWithHooks(&gb, mock_play_sfx, mock_play_music_1b, mock_play_music_1e);
    assert(sfx_calls == 1);
    assert(m1b_calls == 0);
    assert(m1e_calls == 0);

    /* 2. Wave SFX inactive, normal timing (0) -> 1 music step */
    sfx_calls = m1b_calls = m1e_calls = 0;
    gb_write(&gb, hWaveSfx, 0x00);
    gb_write(&gb, wMusicTrackTiming, 0x00);
    PlayAudioStepWithHooks(&gb, mock_play_sfx, mock_play_music_1b, mock_play_music_1e);
    assert(sfx_calls == 1);
    assert(m1b_calls == 1);
    assert(m1e_calls == 1);

    /* 3. Half-speed timing (2), odd frame -> music skipped */
    sfx_calls = m1b_calls = m1e_calls = 0;
    gb_write(&gb, wMusicTrackTiming, 0x02);
    gb_write(&gb, hFrameCounter, 0x01); /* odd */
    PlayAudioStepWithHooks(&gb, mock_play_sfx, mock_play_music_1b, mock_play_music_1e);
    assert(sfx_calls == 1);
    assert(m1b_calls == 0);
    assert(m1e_calls == 0);

    /* 4. Half-speed timing (2), even frame -> 1 music step */
    sfx_calls = m1b_calls = m1e_calls = 0;
    gb_write(&gb, wMusicTrackTiming, 0x02);
    gb_write(&gb, hFrameCounter, 0x02); /* even */
    PlayAudioStepWithHooks(&gb, mock_play_sfx, mock_play_music_1b, mock_play_music_1e);
    assert(sfx_calls == 1);
    assert(m1b_calls == 1);
    assert(m1e_calls == 1);

    /* 5. Double-speed timing (1) -> 2 music steps */
    sfx_calls = m1b_calls = m1e_calls = 0;
    gb_write(&gb, wMusicTrackTiming, 0x01);
    PlayAudioStepWithHooks(&gb, mock_play_sfx, mock_play_music_1b, mock_play_music_1e);
    assert(sfx_calls == 1);
    assert(m1b_calls == 2);
    assert(m1e_calls == 2);
}

static int mock_select_music_calls = 0;
static void mock_select_music(GBState *gb) {
    mock_select_music_calls++;
    assert(gb->rom_bank == 0x02);
}

static int mock_func_1f_calls = 0;
static void mock_func_1f(GBState *gb) {
    mock_func_1f_calls++;
    assert(gb->rom_bank == 0x1F);
}

void test_music_fade_and_track_routines(void) {

    GBState gb;
    gb_init(&gb);

    /* Test SetWorldMusicTrack */
    SetWorldMusicTrack(&gb, 0x2A);
    assert(gb_read(&gb, wMusicTrackToPlay) == 0x2A);
    assert(gb_read(&gb, hNextDefaultMusicTrack) == 0x2A);
    assert(gb_read(&gb, hMusicFadeInTimer) == 0x38);
    assert(gb_read(&gb, hMusicFadeOutTimer) == 0x00);

    /* Test ResetMusicFadeTimer */
    ResetMusicFadeTimer(&gb);
    assert(gb_read(&gb, hMusicFadeOutTimer) == MUSIC_FADE_OUT_TIMER_MAX);
    assert(gb_read(&gb, hMusicFadeInTimer) == 0x00);

    /* Test SelectMusicTrackAfterTransition_trampoline */
    gb_write(&gb, wCurrentBank, 0x05);
    gb.rom_bank = 0x05;
    mock_select_music_calls = 0;
    SelectMusicTrackAfterTransition_trampoline(&gb, mock_select_music);
    assert(mock_select_music_calls == 1);
    assert(gb.rom_bank == 0x05);

    /* Test func_27F2 with hContinueMusicAfterWarp == 0 */
    gb_write(&gb, wCurrentBank, 0x03);
    gb.rom_bank = 0x03;
    gb_write(&gb, hContinueMusicAfterWarp, 0x00);
    mock_func_1f_calls = 0;
    func_27F2(&gb, mock_func_1f);
    assert(mock_func_1f_calls == 1);
    assert(gb.rom_bank == 0x03);

    /* Test func_27F2 with hContinueMusicAfterWarp != 0 */
    gb_write(&gb, hContinueMusicAfterWarp, 0x01);
    mock_func_1f_calls = 0;
    func_27F2(&gb, mock_func_1f);
    assert(mock_func_1f_calls == 0);
    assert(gb.rom_bank == 0x03);
}

static int mock_boomerang_calls = 0;
static void mock_boomerang_sfx(GBState *gb) {
    mock_boomerang_calls++;
    assert(gb->rom_bank == 0x20);
}

void test_play_boomerang_sfx_trampoline(void) {

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wCurrentBank, 0x07);
    gb.rom_bank = 0x07;

    mock_boomerang_calls = 0;
    PlayBoomerangSfx_trampoline(&gb, mock_boomerang_sfx);
    assert(mock_boomerang_calls == 1);
    assert(gb.rom_bank == 0x07);
}

#define RUN_AUDIO_TEST(fn, name) \
    do { \
        printf("[RUN ] %s\n", name); \
        fn(); \
        printf("[PASS] %s\n", name); \
    } while (0)

void run_audio_tests(void) {
    printf("[TEST] Audio\n");
    RUN_AUDIO_TEST(test_play_wrong_answer_jingle, "PlayWrongAnswerJingle");
    RUN_AUDIO_TEST(test_alert_sword_moblins, "AlertSwordMoblins");
    RUN_AUDIO_TEST(test_play_bomb_explosion_sfx, "PlayBombExplosionSfx");
    RUN_AUDIO_TEST(test_play_audio_step, "PlayAudioStep");
    RUN_AUDIO_TEST(test_music_fade_and_track_routines, "SetWorldMusicTrackAndFade");
    RUN_AUDIO_TEST(test_play_boomerang_sfx_trampoline, "PlayBoomerangSfxTrampoline");
    printf("[PASS] Audio\n\n");
}
