#include "bank2/room_transition.h"
#include "constants/directions.h"
#include "constants/gameplay.h"
#include "constants/gfx.h"
#include "constants/hardware.h"
#include "constants/maps.h"
#include "constants/memory.h"
#include "constants/physics.h"
#include "constants/rooms.h"
#include "constants/sfx.h"
#include "constants/tilesets.h"
#include "constants/audio.h"
#include "home/bank.h"
#include "home/entities.h"
#include "home/gfx.h"
#include "home/link.h"
#include "home/room.h"

/* Bank 2 Debug Warp Tables (02:5471+) */
const uint8_t DebugWarpRooms[11] = {
    0x30, /* MAP_BOTTLE_GROTTO */
    0x33, /* MAP_BOTTLE_GROTTO */
    0x81, /* MAP_CATFISHS_MAW */
    0x01, /* MAP_TAIL_CAVE */
    0x28, /* MAP_BOTTLE_GROTTO */
    0x56, /* MAP_KEY_CAVERN */
    0x68, /* MAP_ANGLERS_TUNNEL */
    0x87, /* MAP_CATFISHS_MAW */
    0xB3, /* MAP_FACE_SHRINE */
    0xE6, /* MAP_EAGLES_TOWER */
    0x0A  /* MAP_TURTLE_ROCK */
};

const uint8_t DebugWarpMaps[11] = {
    MAP_BOTTLE_GROTTO,
    MAP_BOTTLE_GROTTO,
    MAP_CATFISHS_MAW,
    MAP_TAIL_CAVE,
    MAP_BOTTLE_GROTTO,
    MAP_KEY_CAVERN,
    MAP_ANGLERS_TUNNEL,
    MAP_CATFISHS_MAW,
    MAP_FACE_SHRINE,
    MAP_EAGLES_TOWER,
    MAP_TURTLE_ROCK
};

void ExecuteDebugWarp(GBState *gb) {
    if (!gb) return;

    gb_write(gb, wWarp0MapCategory, 0x01);
    uint8_t index = gb_read(gb, wDebugWarpIndex);
    uint8_t next_index = (uint8_t)(index + 1);
    if (next_index >= 0x0B) {
        next_index = 0;
    }
    gb_write(gb, wDebugWarpIndex, next_index);

    if (index >= 11) {
        index = 0;
    }
    gb_write(gb, wWarp0Room, DebugWarpRooms[index]);
    gb_write(gb, wWarp0Map, DebugWarpMaps[index]);
    gb_write(gb, wWarp0DestinationX, 0x50);
    gb_write(gb, wWarp0DestinationY, 0x70);
    gb_write_hram(gb, hJingle, JINGLE_PUZZLE_SOLVED);
    ApplyMapFadeOutTransitionWithNoise(gb);
}

void staircaseIsActive(GBState *gb) {
    if (!gb) return;

    /* Don't trigger staircase when jumping over it */
    if (gb_read_hram(gb, hLinkPositionZ) != 0) {
        return;
    }

    /* Don't trigger staircase if not over it */
    uint8_t diff_x = (uint8_t)(gb_read_hram(gb, hLinkPositionX) - gb_read_hram(gb, hStaircasePosX) + 0x05);
    if (diff_x >= 0x0A) {
        return;
    }

    uint8_t diff_y = (uint8_t)(gb_read_hram(gb, hLinkPositionY) - gb_read_hram(gb, hStaircasePosY) + 0x05);
    if (diff_y >= 0x0A) {
        return;
    }

    /* Don't trigger staircase when carrying an object */
    if (gb_read(gb, wIsCarryingLiftedObject) != 0) {
        return;
    }

    /* Don't trigger staircase when color dungeon entrance is not open */
    if (gb_read_hram(gb, hMapRoom) == ROOM_OW_COLOR_DUNGEON_ENTRANCE && gb_read(gb, wIsIndoor) == 0) {
        if (gb_read(gb, wColorDungonCorrectTombStones) != 0x80) {
            return;
        }
    }

    /* Trigger staircase warp */
    ApplyMapFadeOutTransitionWithSound(gb);
    gb_write_hram(gb, hStaircase, STAIRCASE_NONE);
}

void renderTranscientVFXs(GBState *gb, void (*render_transient_vfx)(GBState *, uint8_t)) {
    if (!gb) return;

    for (int i = 0x0F; i >= 0; i--) {
        gb_write(gb, wActiveEntityIndex, (uint8_t)i);
        uint8_t vfx_type = gb_read(gb, (uint16_t)(wTranscientVfxTypeTable + i));
        if (vfx_type != 0 && render_transient_vfx) {
            render_transient_vfx(gb, (uint8_t)i);
        }
    }

    if (gb_read(gb, wRoomTransitionState) != 0) {
        return;
    }

    uint8_t staircase = gb_read_hram(gb, hStaircase);
    if (staircase == STAIRCASE_NONE) {
        return;
    }

    if (staircase != STAIRCASE_INACTIVE) {
        staircaseIsActive(gb);
        return;
    }

    /* If the player left an inactive staircase, mark it as active */
    uint8_t diff_x = (uint8_t)(gb_read_hram(gb, hLinkPositionX) - gb_read_hram(gb, hStaircasePosX) + 0x06);
    if (diff_x >= 0x0C) {
        gb_write_hram(gb, hStaircase, STAIRCASE_ACTIVE);
        return;
    }

    uint8_t diff_y = (uint8_t)(gb_read_hram(gb, hLinkPositionY) - gb_read_hram(gb, hStaircasePosY) + 0x06);
    if (diff_y >= 0x0C) {
        gb_write_hram(gb, hStaircase, STAIRCASE_ACTIVE);
        return;
    }
}

void label_002_5487(GBState *gb, void (*render_transient_vfx)(GBState *, uint8_t)) {
    if (!gb) return;

    gb_write(gb, wIndoorARoomStatus, 0x00);
    gb_write(gb, wIndoorBRoomStatus, 0x00);

    uint8_t dialog_cd = gb_read(gb, wDialogCooldown);
    if (dialog_cd != 0) {
        gb_write(gb, wDialogCooldown, (uint8_t)(dialog_cd - 1));
    }

    uint8_t photo_cd = gb_read(gb, wPhotoAlbumCooldown);
    if (photo_cd != 0) {
        gb_write(gb, wPhotoAlbumCooldown, (uint8_t)(photo_cd - 1));
    }

    renderTranscientVFXs(gb, render_transient_vfx);
}

/* Room Transition Data Tables (02:78D8+) */

const int8_t RoomTransitionLinkXIncrement[4] = {
    (int8_t)0xC6,  /* right */
    (int8_t)0x3A,  /* left */
    (int8_t)0x00,  /* top */
    (int8_t)0x00   /* bottom */
};

const int8_t RoomTransitionLinkYIncrement[4] = {
    (int8_t)0x00,  /* right */
    (int8_t)0x00,  /* left */
    (int8_t)0x3A,  /* top */
    (int8_t)0xC6   /* bottom */
};

const int8_t RoomTransitionXIncrement[4] = {
    (int8_t)0x04,  /* right */
    (int8_t)0xFC,  /* left */
    (int8_t)0x00,  /* top */
    (int8_t)0x00   /* bottom */
};

const int8_t RoomTransitionYIncrement[4] = {
    (int8_t)0x00,  /* right */
    (int8_t)0x00,  /* left */
    (int8_t)0xFC,  /* top */
    (int8_t)0x04   /* bottom */
};

const uint8_t WindFishEggMazeSequence[32] = {
    DIRECTION_LEFT,  DIRECTION_LEFT,  DIRECTION_UP,    DIRECTION_RIGHT,
    DIRECTION_RIGHT, DIRECTION_UP,    DIRECTION_LEFT,  DIRECTION_UP,
    DIRECTION_RIGHT, DIRECTION_UP,    DIRECTION_UP,    DIRECTION_RIGHT,
    DIRECTION_UP,    DIRECTION_UP,    DIRECTION_RIGHT, DIRECTION_UP,
    DIRECTION_LEFT,  DIRECTION_UP,    DIRECTION_RIGHT, DIRECTION_UP,
    DIRECTION_LEFT,  DIRECTION_UP,    DIRECTION_RIGHT, DIRECTION_UP,
    DIRECTION_RIGHT, DIRECTION_RIGHT, DIRECTION_RIGHT, DIRECTION_RIGHT,
    DIRECTION_UP,    DIRECTION_UP,    DIRECTION_UP,    DIRECTION_UP
};

const uint8_t OverworldRoomIncrement[4] = {
    0x01,  /* right */
    0xFF,  /* left */
    0xF0,  /* top */
    0x10   /* bottom */
};

const uint8_t IndoorRoomIncrement[4] = {
    0x01,  /* right */
    0xFF,  /* left */
    0xF8,  /* top */
    0x08   /* bottom */
};

const uint8_t RoomTransitionBGOriginHigh[4] = {
    0x00,  /* right */
    0x00,  /* left */
    0x02,  /* top */
    0x02   /* bottom */
};

const uint8_t RoomTransitionBGOriginLow[4] = {
    0x14,  /* right */
    0x0C,  /* left */
    0x00,  /* top */
    0x00   /* bottom */
};

const uint8_t RoomTransitionBGInitialUpdateRegionHigh[4] = {
    0x00,  /* right */
    0x00,  /* left */
    0x03,  /* top */
    0x02   /* bottom */
};

const uint8_t RoomTransitionBGInitialUpdateRegionLow[4] = {
    0x14,  /* right */
    0x1F,  /* left */
    0xE0,  /* top */
    0x00   /* bottom */
};

const uint8_t RoomUpdateTileAmount[4] = {
    0x0F,  /* right (TILES_PER_COLUMN - 1) */
    0x0F,  /* left (TILES_PER_COLUMN - 1) */
    0x10,  /* up (TILES_PER_ROW) */
    0x10   /* down (TILES_PER_ROW) */
};

const uint8_t RoomTransitionFramesToMidScreen[4] = {
    0x14,  /* right */
    0x14,  /* left */
    0x10,  /* top */
    0x10   /* bottom */
};

const uint8_t RoomTransitionOffset[8] = {
    0x00,  /* right */
    0x09,  /* left */
    0x70,  /* top */
    0x00,  /* bottom */
    0x40,
    0x40,
    0x02,
    0x02
};

const uint8_t RoomTransitionTargetScrollX[4] = {
    0xA0,  /* right */
    0x60,  /* left */
    0x00,  /* top */
    0x00   /* bottom */
};

const uint8_t RoomTransitionTargetScrollY[4] = {
    0x00,  /* right */
    0x00,  /* left */
    0x80,  /* top */
    0x80   /* bottom */
};

const int8_t Data_002_7C04[8] = {
    0x00, 0x00, (int8_t)0xFF, 0x01, 0x01, (int8_t)0xFF, 0x01, (int8_t)0xFF
};

const int8_t Data_002_7C0C[8] = {
    0x01, (int8_t)0xFF, 0x00, 0x00, 0x01, 0x01, (int8_t)0xFF, (int8_t)0xFF
};

const int8_t Data_002_7C40[8] = {
    (int8_t)0xFC, (int8_t)0xFA, (int8_t)0xF8, (int8_t)0xF6, 0x0C, 0x00, 0x00, (int8_t)0xF4
};

const int8_t Data_002_7C48[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, (int8_t)0xF4, 0x0C, 0x00
};

/* Room Transition Functions (02:78E8+) */

/**
 * Applies room transition state machine (02:78E8-02:79D9).
 * Handles sliding transitions between rooms of the same map.
 *
 * @param gb Pointer to Game Boy system state.
 * @param bg_collision_handler Optional callback for background collision handling.
 * @param create_following_npc Optional callback for creating following NPC.
 * @param set_world_music_track Optional callback for setting world music track.
 * @param clear_link_position_increment Optional callback for clearing link position increment.
 */
void ApplyRoomTransition(GBState *gb,
                         void (*bg_collision_handler)(GBState *),
                         void (*create_following_npc)(GBState *),
                         void (*set_world_music_track)(GBState *),
                         void (*clear_link_position_increment)(GBState *)) {
    if (!gb) return;

    /* ld a, [wRoomTransitionState]; cp ROOM_TRANSITION_NONE; jp z, .return */
    if (gb_read(gb, wRoomTransitionState) == ROOM_TRANSITION_NONE) {
        return;
    }

    /* push af; cp ROOM_TRANSITION_FIRST_HALF; jp c, .dispatchTransition */
    uint8_t state = gb_read(gb, wRoomTransitionState);
    if (state < ROOM_TRANSITION_FIRST_HALF) {
        /* .dispatchTransition: pop af; dec a; JP_TABLE */
        goto dispatchTransition;
    }

    /* Apply the scroll offset */
    /* hLinkSpeedX = RoomTransitionLinkXIncrement[wRoomTransitionDirection] */
    uint8_t dir = gb_read(gb, wRoomTransitionDirection);
    gb_write_hram(gb, hLinkSpeedX, (uint8_t)RoomTransitionLinkXIncrement[dir & 3]);

    /* hLinkSpeedY = RoomTransitionLinkYIncrement[wRoomTransitionDirection] */
    gb_write_hram(gb, hLinkSpeedY, (uint8_t)RoomTransitionLinkYIncrement[dir & 3]);

    /* call UpdateFinalLinkPosition */
    UpdateFinalLinkPosition(gb);

    /* hBaseScrollX += RoomTransitionXIncrement[wRoomTransitionDirection] */
    uint8_t scroll_x = gb_read_hram(gb, hBaseScrollX);
    scroll_x = (uint8_t)(scroll_x + RoomTransitionXIncrement[dir & 3]);
    gb_write_hram(gb, hBaseScrollX, scroll_x);

    /* hBaseScrollY += RoomTransitionYIncrement[wRoomTransitionDirection] */
    uint8_t scroll_y = gb_read_hram(gb, hBaseScrollY);
    scroll_y = (uint8_t)(scroll_y + RoomTransitionYIncrement[dir & 3]);
    gb_write_hram(gb, hBaseScrollY, scroll_y);

    /* If the target scroll position is not reached yet, go to the jump table */
    if (scroll_y != gb_read(gb, wRoomTransitionTargetScrollY)) {
        goto dispatchTransition;
    }

    if (scroll_x != gb_read(gb, wRoomTransitionTargetScrollX)) {
        goto dispatchTransition;
    }

    /* Scroll reached the target scroll position: finish the transition. */

    /* Change the music track if needed */
    /* pop af; ldh a, [hNextMusicTrackToFadeInto]; and a; jr z, .noMusicTrackChange */
    uint8_t next_music = gb_read_hram(gb, hNextMusicTrackToFadeInto);
    if (next_music != 0) {
        /* call SetWorldMusicTrack; xor a; ldh [hNextMusicTrackToFadeInto], a */
        if (set_world_music_track) {
            set_world_music_track(gb);
        }
        gb_write_hram(gb, hNextMusicTrackToFadeInto, 0);
    }
    /* .noMusicTrackChange */

    /* Clear variables */
    if (clear_link_position_increment) {
        clear_link_position_increment(gb);
    }
    gb_write_hram(gb, hLinkVelocityZ, 0);
    gb_write(gb, wRoomTransitionState, 0);

    /* Save Link's initial position on the new map */
    gb_write(gb, wLinkMapEntryPositionX, gb_read_hram(gb, hLinkPositionX));
    gb_write(gb, wLinkMapEntryPositionY, gb_read_hram(gb, hLinkPositionY));

    /* If the transition direction was to the bottom... */
    if (dir == DIRECTION_DOWN) {
        /* Initiate a jump if Link landed on a ledge */
        gb_write_hram(gb, hLinkSpeedY, 0x01);
        if (bg_collision_handler) {
            bg_collision_handler(gb);
        }

        /* If transitioning over an object with collisions, unstuck the player */
        uint8_t obj_under = gb_read_hram(gb, hObjectUnderEntity);
        if (obj_under != 0xDB && obj_under != 0xDC) {
            if (obj_under == OBJECT_ROCKY_CAVE_DOOR || gb_read(gb, wCollisionType) != 0) {
                if (gb_read(gb, wFreeMovementMode) == 0) {
                    /* call func_002_6EAD - stub for now */
                }
            }
        }
    }

    /* If a jingle has been configured... */
    if (gb_read(gb, wNextJingle) != 0) {
        /* ... play the configured jingle immediately */
        gb_write_hram(gb, hJingle, gb_read(gb, wNextJingle));
        gb_write(gb, wNextJingle, 0);
    }

    if (create_following_npc) {
        create_following_npc(gb);
    }

    /* Reset animated tiles frame */
    gb_write_hram(gb, hAnimatedTilesFrameCount, 0xFF);

    /* Play compass SFX if needed */
    /* If wIsIndoor == 0, return */
    if (gb_read(gb, wIsIndoor) == 0) {
        return;
    }

    /* call GetChestsStatusForRoom_trampoline - stub for now */
    /* If chest status is not some key, and room trigger != EFFECT_DROP_KEY, return. */
    /* For now, we skip this check */

    return;

dispatchTransition:
    /* pop af; dec a; JP_TABLE */
    switch (state - 1) {
        case 0: RoomTransitionPrepareHandler(gb, NULL, NULL, NULL, NULL, NULL, NULL, NULL); break;
        case 1: RoomTransitionLoadTiles(gb, NULL); break;
        case 2: RoomTransitionConfigureScrollTargets(gb); break;
        case 3: RoomTransitionFirstHalfHandler(gb, NULL, NULL, NULL, NULL, NULL, NULL); break;
        case 4: RoomTransitionSecondHalfHandler(gb); break;
        default: break;
    }
}

/**
 * Room transition prepare handler (02:79FA-02:7ADB).
 * Handles Wind Fish's Egg maze, indoor/overworld room increment, and loads room.
 *
 * @param gb Pointer to Game Boy system state.
 * @param load_room Optional callback for loading room.
 * @param replace_objects56and57 Optional callback for replacing objects 56 and 57.
 * @param load_room_entities Optional callback for loading room entities.
 * @param draw_link_sprite Optional callback for drawing Link sprite.
 * @param apply_link_motion_state Optional callback for applying link motion state.
 * @param select_room_tilesets Optional callback for selecting room tilesets.
 * @param reset_music_fade_timer Optional callback for resetting music fade timer.
 */
void RoomTransitionPrepareHandler(GBState *gb,
                                  void (*load_room)(GBState *, const void *),
                                  void (*replace_objects56and57)(GBState *, uint8_t, void (*)(GBState *)),
                                  void (*load_room_entities)(GBState *,
                                                             void (*)(GBState *),
                                                             uint16_t (*)(GBState *, uint8_t),
                                                             void (*)(GBState *, uint8_t),
                                                             void (*)(GBState *, uint8_t)),
                                  void (*draw_link_sprite)(GBState *),
                                  void (*apply_link_motion_state)(GBState *,
                                                                  void (*)(GBState *),
                                                                  void (*)(GBState *),
                                                                  void (*)(GBState *)),
                                  void (*select_room_tilesets)(GBState *),
                                  void (*reset_music_fade_timer)(GBState *)) {
    if (!gb) return;

    uint8_t dir = gb_read(gb, wRoomTransitionDirection);

    /* If map in indoor... */
    if (gb_read(gb, wIsIndoor) != 0) {
        /* Wind Fish's Egg */
        if (gb_read_hram(gb, hMapId) != MAP_COLOR_DUNGEON) {
            if (gb_read_hram(gb, hMapId) < MAP_DUNGEON_G1) {
                if (gb_read_hram(gb, hMapId) == MAP_WINDFISHS_EGG) {
                    if (gb_read_hram(gb, hMapRoom) == UNKNOWN_ROOM_71) {
                        if (dir != DIRECTION_DOWN) {
                            /* hl = WindFishEggMazeSequence + wWindFishEggMazeSequenceOffset */
                            uint8_t offset = gb_read(gb, wWindFishEggMazeSequenceOffset);
                            uint8_t progress = gb_read(gb, wEggMazeProgress);
                            uint8_t expected_dir = WindFishEggMazeSequence[offset + progress];

                            /* wEggMazeProgress += 1 */
                            gb_write(gb, wEggMazeProgress, (uint8_t)(progress + 1));

                            /* If direction != hl[wEggMazeProgress]... clear progress */
                            if (dir != expected_dir) {
                                gb_write(gb, wEggMazeProgress, 0);
                                goto loadRoom;
                            }

                            /* If maze progress >= 7, mark maze as solved */
                            if (progress >= 7) {
                                gb_write(gb, wNextJingle, JINGLE_PUZZLE_SOLVED);
                            }
                        }
                    }
                }
            }
        }

        /* .noWindFishEggMaze: Reset maze progress */
        gb_write(gb, wEggMazeProgress, 0);

        /* Compute the next IndoorRoom according to the direction */
        /* If wRoomTransitionDirection == Top... */
        if (dir == DIRECTION_UP) {
            /* ... and dungeon is Face Shrine... */
            if (gb_read_hram(gb, hMapId) == MAP_FACE_SHRINE) {
                /* ... and dungeon room is $1D... */
                if (gb_read(gb, wIndoorRoom) == 0x1D) {
                    /* ... actually pretend we are on map $35. */
                    gb_write(gb, wIndoorRoom, 0x35);
                }
            }
        }

        /* a = IndoorRoomIncrement[direction]; hl = wIndoorRoom */
        uint8_t inc = IndoorRoomIncrement[dir & 3];
        uint8_t new_room = (uint8_t)(gb_read(gb, wIndoorRoom) + inc);
        gb_write(gb, wIndoorRoom, new_room);
        goto afterRoomSet;
    }

    /* .indoorEnd: Overworld */
    /* If wShouldGetLostInMysteriousWoods != 0... */
    if (gb_read(gb, wShouldGetLostInMysteriousWoods) != 0) {
        /* ... and direction == top... */
        if (dir == DIRECTION_UP) {
            /* ... Link got lost in the Mysterious Woods */
            gb_write(gb, wNextJingle, JINGLE_FOREST_LOST);
            /* a = $63; hl = hMapRoom */
            gb_write_hram(gb, hMapRoom, UNKNOWN_ROOM_63);
            goto afterRoomSet;
        }
    }

    /* .mysteriousWoodsEnd: a = OverworldRoomIncrement[direction]; hl = hMapRoom */
    uint8_t inc = OverworldRoomIncrement[dir & 3];
    gb_write_hram(gb, hMapRoom, (uint8_t)(gb_read_hram(gb, hMapRoom) + inc));

afterRoomSet:;
    uint8_t new_room = gb_read_hram(gb, hMapRoom);
    /* If room == $41 (Tail Cave Key on Mysterious Forest)... */
    if (new_room == UNKNOWN_ROOM_41) {
        /* ... and direction == top... */
        if (dir == DIRECTION_UP) {
            /* ... and this room has not been visited yet... */
            uint8_t status = gb_read(gb, wOverworldRoomStatus + UNKNOWN_ROOM_41);
            if ((status & 0x40) == 0) {  /* bit 6 = visited */
                /* ... mark the room as discovered... */
                gb_write(gb, wOverworldRoomStatus + UNKNOWN_ROOM_41, status | 0x40);
                /* ... and play a success jingle. */
                gb_write(gb, wNextJingle, JINGLE_PUZZLE_SOLVED);
            }
        }
    }

loadRoom:
    /* call LoadRoom */
    if (load_room) {
        load_room(gb, NULL);
    }

    /* If in Color Dungeon... */
    if (gb_read(gb, wIsIndoor) != 0 && gb_read_hram(gb, hMapId) == MAP_COLOR_DUNGEON) {
        /* force update the background tiles */
        gb_write_hram(gb, hNeedsUpdatingBGTiles, TILESET_LOAD_WORLD);

        /* Replace objects $56 and $57 by object $0D */
        if (replace_objects56and57) {
            replace_objects56and57(gb, 0x02, NULL);
        }
    }

    if (load_room_entities) {
        load_room_entities(gb, NULL, NULL, NULL, NULL);
    }
    if (draw_link_sprite) {
        draw_link_sprite(gb);
    }
    if (apply_link_motion_state) {
        apply_link_motion_state(gb, NULL, NULL, NULL);
    }

/* Get the music track to apply later */
    if (gb_read(gb, wC1CF) != 0) {
        gb_write(gb, wC1CF, 0);

#ifndef PATCH_0
        if (gb_read(gb, wTunicType) != 0) {
            uint8_t music = gb_read_hram(gb, hDefaultMusicTrack);
            gb_write_hram(gb, hNextMusicTrackToFadeInto, music);
            goto setMusicTrackDone;
        }
#endif

        if (gb_read(gb, wActivePowerUp) != 0) {
            uint8_t music = gb_read_hram(gb, hDefaultMusicTrack);
            gb_write_hram(gb, hNextMusicTrackToFadeInto, music);
            goto setMusicTrackDone;
        }

        gb_write_hram(gb, hNextMusicTrackToFadeInto, MUSIC_ACTIVE_POWER_UP);
    setMusicTrackDone:
        if (reset_music_fade_timer) {
            reset_music_fade_timer(gb);
        }
        goto IncrementRoomTransitionStateAndReturn;
    }

    /* .C1CFIsZero: If indoors, or sword wasn't found yet... */
    if (gb_read(gb, wIsIndoor) != 0 || gb_read(gb, wSwordLevel) == 0) {
        goto IncrementRoomTransitionStateAndReturn;
    }

    /* Load the music track from the Overworld tracks array */
    /* OverworldMusicTracks table lookup - stub for now */
    /* For now we skip the music track selection */

IncrementRoomTransitionStateAndReturn:
    /* Increment wRoomTransitionState */
    gb_write(gb, wRoomTransitionState, (uint8_t)(gb_read(gb, wRoomTransitionState) + 1));
}

/**
 * Room transition load tiles handler (02:7B3E-02:7B4B).
 * Selects room tilesets and handles switch blocks.
 *
 * @param gb Pointer to Game Boy system state.
 * @param select_room_tilesets Optional callback for selecting room tilesets.
 */
void RoomTransitionLoadTiles(GBState *gb, void (*select_room_tilesets)(GBState *)) {
    if (!gb) return;

    /* call SelectRoomTilesets */
    if (select_room_tilesets) {
        select_room_tilesets(gb);
    }

    /* If room has switch blocks... */
    if (gb_read(gb, wRoomSwitchableObject) == ROOM_SWITCHABLE_OBJECT_MOBILE_BLOCK) {
        /* ... mark both kind of blocks as needing their tiles to be updated */
        gb_write_hram(gb, hSwitchBlockNeedingUpdate, 0x02);
    }

    /* jp IncrementRoomTransitionStateAndReturn */
    gb_write(gb, wRoomTransitionState, (uint8_t)(gb_read(gb, wRoomTransitionState) + 1));
}

/**
 * Room transition configure scroll targets handler (02:7B7F-02:7BFC).
 * Configures scroll targets, BG update region, and transition timing.
 *
 * @param gb Pointer to Game Boy system state.
 */
void RoomTransitionConfigureScrollTargets(GBState *gb) {
    if (!gb) return;

    /* If hSwitchBlockNeedingUpdate != 0, return */
    if (gb_read_hram(gb, hSwitchBlockNeedingUpdate) != 0) {
        return;
    }

    uint8_t dir = gb_read(gb, wRoomTransitionDirection);

    /* e = (direction horizontal ? $DF : $FF) */
    uint8_t e = (dir & 0x02) ? 0xDF : 0xFF;

    /* Configure the target scrollX position */
    uint8_t target_scroll_x = (uint8_t)(gb_read(gb, wRoomTransitionTargetScrollX) + RoomTransitionTargetScrollX[dir & 3]);
    gb_write(gb, wRoomTransitionTargetScrollX, target_scroll_x);

    /* Configure the target scrollY position */
    uint8_t target_scroll_y = (uint8_t)(gb_read(gb, wRoomTransitionTargetScrollY) + RoomTransitionTargetScrollY[dir & 3]);
    gb_write(gb, wRoomTransitionTargetScrollY, target_scroll_y);

    /* Configure the initial position of the Background region to be updated */
    uint8_t bg_origin_low = gb_read(gb, wBGOriginLow);
    uint8_t d = 0;
    uint8_t val_low = (uint8_t)(bg_origin_low + RoomTransitionBGInitialUpdateRegionLow[dir & 3]);
    d = (val_low & 0x80) ? 1 : 0;  /* rl d equivalent */
    val_low &= e;  /* and e */
    gb_write(gb, wBGUpdateRegionOriginLow, val_low);

    uint8_t bg_origin_high = gb_read(gb, wBGOriginHigh);
    d = (d << 1) | ((val_low & 0x80) ? 1 : 0);  /* rr d equivalent - simplified */
    uint8_t val_high = (uint8_t)(bg_origin_high + RoomTransitionBGInitialUpdateRegionHigh[dir & 3] + d);
    val_high &= 0x03;
    gb_write(gb, wBGUpdateRegionOriginHigh, val_high);

    /* Save the background origin position after the transition */
    val_low = (uint8_t)(bg_origin_low + RoomTransitionBGOriginLow[dir & 3]);
    d = (val_low & 0x80) ? 1 : 0;
    val_low &= e;
    gb_write(gb, wBGOriginLow, val_low);

    val_high = (uint8_t)(bg_origin_high + RoomTransitionBGOriginHigh[dir & 3] + d);
    val_high &= 0x03;
    gb_write(gb, wBGOriginHigh, val_high);

    /* Configure wBGUpdateRegionTilesCount */
    gb_write(gb, wBGUpdateRegionTilesCount, RoomUpdateTileAmount[dir & 3]);

    /* Set number of frames to elapse before reaching the mid-transition point */
    gb_write(gb, wRoomTransitionFramesBeforeMidScreen, RoomTransitionFramesToMidScreen[dir & 3]);

    gb_write(gb, wTransitionOffset, RoomTransitionOffset[dir & 3]);
    gb_write(gb, wTransitionZeroNeverUsed, 0);

    /* jp IncrementRoomTransitionStateAndReturn */
    gb_write(gb, wRoomTransitionState, (uint8_t)(gb_read(gb, wRoomTransitionState) + 1));
}

/**
 * Room transition first half handler (02:7C00-02:7C02).
 * Updates BG region.
 *
 * @param gb Pointer to Game Boy system state.
 * @param update_bg_region Optional callback for updating BG region.
 * @param func_020_4a76 Optional callback for func_020_4a76.
 * @param get_bg_attr_addr Optional callback for getting BG attribute address.
 * @param switch_to_tilemap_bank Optional callback for switching to tilemap bank.
 * @param func_020_49d9 Optional callback for func_020_49d9.
 * @param update_origin Optional callback for updating origin.
 */
void RoomTransitionFirstHalfHandler(GBState *gb,
                                    void (*update_bg_region)(GBState *,
                                                             void (*)(GBState *),
                                                             void (*)(GBState *),
                                                             void (*)(GBState *),
                                                             void (*)(GBState *),
                                                             void (*)(GBState *)),
                                    void (*func_020_4a76)(GBState *),
                                    void (*get_bg_attr_addr)(GBState *),
                                    void (*switch_to_tilemap_bank)(GBState *),
                                    void (*func_020_49d9)(GBState *),
                                    void (*update_origin)(GBState *)) {
    if (!gb) return;

    /* jp UpdateBGRegion */
    if (update_bg_region) {
        update_bg_region(gb, func_020_4a76, get_bg_attr_addr, switch_to_tilemap_bank, func_020_49d9, update_origin);
    }
}

/**
 * Room transition second half handler (02:7C03-02:7C03).
 * The scroll increment has already been done earlier: nothing more to do.
 *
 * @param gb Pointer to Game Boy system state.
 */
void RoomTransitionSecondHalfHandler(GBState *gb) {
    if (!gb) return;

    /* ret - nothing to do */
}

/**
 * Conveyor belt physics handler (02:7C14-02:7C3F).
 * Moves Link according to conveyor belt direction.
 *
 * @param gb Pointer to Game Boy system state.
 */
void label_002_7C14(GBState *gb) {
    if (!gb) return;

    /* ldh a, [hFrameCounter]; and $03; ld hl, wC167; or [hl] */
    /* ld hl, hLinkInteractiveMotionBlocked; or [hl] */
    /* ld hl, wDialogGotItem; or [hl]; ret nz */
    if ((gb_read_hram(gb, hFrameCounter) & 0x03) != 0 ||
        gb_read(gb, wC167) != 0 ||
        gb_read_hram(gb, hLinkInteractiveMotionBlocked) != 0 ||
        gb_read(gb, wDialogGotItem) != 0) {
        return;
    }

    /* ld a, [wLinkObjectPhysics]; sub OBJ_PHYSICS_CONVEYOR; ld e, a; ld d, $00 */
    uint8_t obj_physics = gb_read(gb, wLinkObjectPhysics);
    uint8_t idx = (uint8_t)(obj_physics - OBJ_PHYSICS_CONVEYOR);

    /* ldh a, [hLinkPositionX]; add [hl]; ldh [hLinkPositionX], a */
    int8_t dx = Data_002_7C04[idx & 7];
    uint8_t pos_x = (uint8_t)(gb_read_hram(gb, hLinkPositionX) + dx);
    gb_write_hram(gb, hLinkPositionX, pos_x);

    /* ldh a, [hLinkPositionY]; add [hl]; ldh [hLinkPositionY], a */
    int8_t dy = Data_002_7C0C[idx & 7];
    uint8_t pos_y = (uint8_t)(gb_read_hram(gb, hLinkPositionY) + dy);
    gb_write_hram(gb, hLinkPositionY, pos_y);
}

/**
 * Lava/Deep water physics handler (02:7C50-02:7C9E).
 * Handles Link movement in lava, deep water, and river rapids.
 *
 * @param gb Pointer to Game Boy system state.
 * @param bg_collision_handler Optional callback for background collision handling.
 */
void label_002_7C50(GBState *gb, void (*bg_collision_handler)(GBState *)) {
    if (!gb) return;

    /* ldh a, [hFrameCounter]; and $00; ... ret nz */
    /* Note: and $00 always results in 0, so this check is effectively:
     * if (wRoomTransitionState != 0 || wDialogGotItem != 0 || hLinkInteractiveMotionBlocked != 0 || wDialogState != 0 || wInventoryAppearing != 0) return;
     */
    if (gb_read(gb, wRoomTransitionState) != 0 ||
        gb_read(gb, wDialogGotItem) != 0 ||
        gb_read_hram(gb, hLinkInteractiveMotionBlocked) != 0 ||
        gb_read(gb, wDialogState) != 0 ||
        gb_read(gb, wInventoryAppearing) != 0) {
        return;
    }

    uint8_t e = 0x01;
    uint8_t obj_under = gb_read_hram(gb, hObjectUnderEntity);

    /* cp $0E; jr nz, .jr_7C88 */
    if (obj_under == 0x0E) {
        /* River rapids area - check specific rooms */
        uint8_t map_room = gb_read_hram(gb, hMapRoom);
        if (map_room == UNKNOWN_ROOM_3E) {
            goto jr_002_7C8B;
        }
        e++;
        if (map_room == UNKNOWN_ROOM_3D) {
            goto jr_002_7C8B;
        }
        e++;
        if (map_room == UNKNOWN_ROOM_3C) {
            goto jr_002_7C8B;
        }
        if (map_room != UNKNOWN_ROOM_3F) {
            return;
        }
        e = 0x00;
        goto jr_002_7C8B;
    }

    /* .jr_7C88: sub $E7; ld e, a */
    e = (uint8_t)(obj_under - 0xE7);

jr_002_7C8B: ;
    /* ld hl, Data_002_7C40; add hl, de; ld a, [hl]; ldh [hLinkSpeedX], a */
    int8_t speed_x = Data_002_7C40[e & 7];
    gb_write_hram(gb, hLinkSpeedX, (uint8_t)speed_x);

    /* ld hl, Data_002_7C48; add hl, de; ld a, [hl]; ldh [hLinkSpeedY], a */
    int8_t speed_y = Data_002_7C48[e & 7];
    gb_write_hram(gb, hLinkSpeedY, (uint8_t)speed_y);

    /* call UpdateFinalLinkPosition; jp BackgroundCollisionHandler */
    UpdateFinalLinkPosition(gb);
    if (bg_collision_handler) {
        bg_collision_handler(gb);
    }
}
