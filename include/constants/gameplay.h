#ifndef LADX_CONSTANTS_GAMEPLAY_H
#define LADX_CONSTANTS_GAMEPLAY_H

#include "common.h"

#define NAME_LENGTH                     5

/* Values for wGameplayType */
#define GAMEPLAY_INTRO                  0x00
#define GAMEPLAY_CREDITS                0x01
#define GAMEPLAY_FILE_SELECT            0x02
#define GAMEPLAY_FILE_NEW               0x03
#define GAMEPLAY_FILE_DELETE            0x04
#define GAMEPLAY_FILE_COPY              0x05
#define GAMEPLAY_FILE_SAVE              0x06
#define GAMEPLAY_WORLD_MAP              0x07
#define GAMEPLAY_CUTSCENE               0x08
#define GAMEPLAY_MARIN_BEACH            0x09
#define GAMEPLAY_WF_MURAL               0x0A
#define GAMEPLAY_WORLD                  0x0B
#define GAMEPLAY_INVENTORY              0x0C
#define GAMEPLAY_PHOTO_ALBUM            0x0D
#define GAMEPLAY_PHOTO_DIZZY_LINK       0x0E
#define GAMEPLAY_PHOTO_NICE_LINK        0x0F
#define GAMEPLAY_PHOTO_MARIN_CLIFF      0x10
#define GAMEPLAY_PHOTO_MARIN_WELL       0x11
#define GAMEPLAY_PHOTO_MABE             0x12
#define GAMEPLAY_PHOTO_ULRIRA           0x13
#define GAMEPLAY_PHOTO_BOW_WOW          0x14
#define GAMEPLAY_PHOTO_THIEF            0x15
#define GAMEPLAY_PHOTO_FISHERMAN        0x16
#define GAMEPLAY_PHOTO_ZORA             0x17
#define GAMEPLAY_PHOTO_KANALET          0x18
#define GAMEPLAY_PHOTO_GHOST            0x19
#define GAMEPLAY_PHOTO_BRIDGE           0x1A

/* Values for wGameplaySubtype */
#define GAMEPLAY_INVENTORY_INITIAL      0x00
#define GAMEPLAY_INTRO_SEA              0x03
#define GAMEPLAY_INTRO_LINK_FACE        0x04
#define GAMEPLAY_INTRO_LIGHTNING        0x05
#define GAMEPLAY_INTRO_BEACH            0x07
#define GAMEPLAY_INTRO_TITLE            0x0B
#define GAMEPLAY_WORLD_LOAD_2           0x02
#define GAMEPLAY_WORLD_INTERACTIVE      0x07

/* Values for wLinkMotionState */
#define LINK_MOTION_DEFAULT             0x00
#define LINK_MOTION_SWIMMING            0x01
#define LINK_MOTION_UNSTUCKING          0x02
#define LINK_MOTION_TYPE_NON_INTERACTIVE LINK_MOTION_UNSTUCKING
#define LINK_MOTION_MAP_FADE_OUT        0x03
#define LINK_MOTION_MAP_FADE_IN         0x04
#define LINK_MOTION_REVOLVING_DOOR      0x05
#define LINK_MOTION_FALLING_DOWN        0x06
#define LINK_MOTION_PASS_OUT            0x07
#define LINK_MOTION_RECOVER             0x08
#define LINK_MOTION_TELEPORT            0x09
#define LINK_MOTION_UNKNOWN_0A          0x0A

#define GOLDEN_LEAVES_5                 0x05
#define SLIME_KEY                       0x06
#define TRADING_ITEM_BANANAS            0x04
#define TRADING_ITEM_MAGNIFYING_LENS    0x0E
#define BOW_WOW_KIDNAPPED               0x80
#define BOW_WOW_FOLLOWING 0x01


/* Event trigger / effect constants */
#define TRIGGER_KILL_ALL_ENEMIES        0x01
#define TRIGGER_KILL_SPECIALS           0x08
#define EVENT_TRIGGER_MASK              0x1F
#define EVENT_EFFECT_MASK               0xE0
#define EFFECT_NONE                     0x00
#define EFFECT_OPEN_SHUTTER_DOORS       0x20
#define EFFECT_KILL_ALL_ENEMIES         0x40
#define EFFECT_REVEAL_CHEST             0x60
#define EFFECT_DROP_KEY                 0x80
#define EFFECT_REVEAL_STAIRCASE         0xA0
#define EFFECT_CLEAR_MIDBOSS            0xC0

/* Room status flags */
#define ROOM_STATUS_NONE                0x00
#define ROOM_STATUS_DOOR_OPEN_RIGHT     0x01
#define ROOM_STATUS_DOOR_OPEN_LEFT      0x02
#define ROOM_STATUS_DOOR_OPEN_UP        0x04
#define ROOM_STATUS_DOOR_OPEN_DOWN      0x08
#define ROOM_STATUS_EVENT_1             0x10
#define ROOM_STATUS_EVENT_2             0x20
#define ROOM_STATUS_EVENT_3             0x40
#define ROOM_STATUS_VISITED             0x80

#define TRANSITION_GFX_MANBO_OUT         0x03
#define ATTACK_STEP_DURATION_MASK        0x7F
#define MAX_SWORD_CHARGE                 0x28
#define MAP_COLOR_DUNGEON                0xFF
#define MAP_EAGLES_TOWER                 0x06
#define MAP_CAVE_B                       0x0A
#define TRADING_ITEM_RIBBON             0x02
#define REPLACE_TILES_NONE              0x00
#define REPLACE_TILES_EE_RIDER_VISIBLE  0x01
#define REPLACE_TILES_EE_RIDER_HIDDEN   0x02
#define REPLACE_TILES_BUTTON_PRESSED    0x03
#define REPLACE_TILES_UNKNOWN_04        0x04
#define REPLACE_TILES_UNKNOWN_08        0x08
#define REPLACE_TILES_GOLDEN_LEAF       0x09
#define REPLACE_TILES_TOADSTOOL         0x0A
#define REPLACE_TILES_MAGIC_POWDER      0x0B
#define REPLACE_TILES_SIRENS_INSTRUMENT 0x0C
#define REPLACE_TILES_TRADING_ITEM      0x0D
#define REPLACE_TILES_ISLAND_FADE       0x0E
#define REPLACE_TILES_MARIN_SITTING     0x0F
#define REPLACE_TILES_MARIN_STANDING    0x10
#define ACTIVE_POWER_UP_PIECE_OF_POWER  0x01
#define POWER_UP_PIECE_OF_POWER         ACTIVE_POWER_UP_PIECE_OF_POWER
#define TRIGGER_THROW_POT_AT_CHEST      0x0D
#define EVENT_TRIGGER_MASK              0x1F
#define OBJECT_WEATHER_VANE_BASE        0x5E
#define OBJECT_OWL_STATUE               0x6F
#define OBJECT_SIGNPOST                 0xD4
#define OBJECT_CHEST_CLOSED             0xA0
#define ITEM_USAGE_READING_TEXT         0x02
#define ROOM_INDOOR_B_MARIN_HOUSE       0xA3
#define UNKNOWN_ROOM_FA                 0xFA

/* Intro entity types */
#define ENTITY_INTRO_SHIP               0x05
#define ENTITY_INTRO_MARIN              0x06
#define ENTITY_INTRO_INERT_LINK         0x07
#define ENTITY_INTRO_SPARKLE            0x08

#define SWORD_ANIMATION_STATE_NONE      0x00
#define USING_SPIN_ATTACK_MAX           0x20
#define PIT_MAX_SLIPPING                24
#define TRANSITION_GFX_MANBO_IN         0x02

#define GUARDIAN_ACORN_COUNTER_MAX      0x0C

/* Values for wRoomTransitionState */
#define ROOM_TRANSITION_NONE             0x00
#define ROOM_TRANSITION_LOAD_ROOM        0x01
#define ROOM_TRANSITION_LOAD_SPRITES     0x02
#define ROOM_TRANSITION_CONFIGURE_SCROLL 0x03
#define ROOM_TRANSITION_FIRST_HALF       0x04
#define ROOM_TRANSITION_SECOND_HALF      0x05

#define ONE_HEART                       0x08
#define MAX_HEARTS                      0x0E
#define DAMAGE_COOLDOWN_TIME            0xA0

/* Values for wSwordDirection */
#define SWORD_DIRECTION_RIGHT           0x00
#define SWORD_DIRECTION_RIGHT_BOTTOM    0x01
#define SWORD_DIRECTION_BOTTOM          0x02
#define SWORD_DIRECTION_LEFT_BOTTOM     0x03
#define SWORD_DIRECTION_LEFT            0x04
#define SWORD_DIRECTION_LEFT_TOP        0x05
#define SWORD_DIRECTION_TOP             0x06
#define SWORD_DIRECTION_RIGHT_TOP       0x07

/* Sword recoil amounts */
#define SWORD_RECOIL_GENIE_JAR_DEFAULT  0x20
#define SWORD_RECOIL_GENIE_JAR_STRONGER 0x30
#define SWORD_RECOIL_DEFAULT            0x30

#endif /* LADX_CONSTANTS_GAMEPLAY_H */
