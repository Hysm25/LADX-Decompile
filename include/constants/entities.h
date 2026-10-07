#ifndef LADX_CONSTANTS_ENTITIES_H
#define LADX_CONSTANTS_ENTITIES_H

#define ENTITY_TRADING_ITEM 0xAF
#define BowWowEatableEntitiesTable 0x5218
#define HitboxPositions 0x3AAA
#define BossIntroDialogTable         0x3EDF
#define ENTITY_DESERT_LANMOLA        0x87
#define ENTITY_GRIM_CREEPER          0xBC
#define ENTITY_BLAINO                0xBE
#define ENTITY_LEEVER                0x0E
#define ENTITY_OCTOROK               0x09
#define ENTITY_MOBLIN                0x0B
#define ENTITY_MOBLIN_SWORD          0x14
#define ENTITY_ZORA                  0xCB
#define ENTITY_HOPPER                0xF3
#define ENTITY_FLYING_HOPPER_BOMBS   0xF2
#define ENTITY_HARDHIT_BEETLE        0xF9
#define ENTITY_AVALAUNCH             0xF4
#define ENTITY_COLOR_GUARDIAN_BLUE   0xF6
#define ENTITY_COLOR_GUARDIAN_RED    0xF7
#define ENTITY_COLOR_DUNGEON_BOOK    0x4A
#define ENTITY_GIANT_BUZZ_BLOB       0xF8
#define ENTITY_SOUTH_FACE_SHRINE_DOOR 0x64
#define ENTITY_ROTOSWITCH_RED        0xEF
#define ENTITY_ROTOSWITCH_YELLOW     0xF0
#define ENTITY_ROTOSWITCH_BLUE       0xF1
#define ENTITY_COLOR_SHELL_RED       0xE9
#define ENTITY_COLOR_SHELL_GREEN     0xEA
#define ENTITY_COLOR_SHELL_BLUE      0xEB
#define ENTITY_GIBDO                 0x1F
#define ENTITY_WRECKING_BALL         0xA8
#define ENTITY_KEY_DROP_POINT        0x30
#define ENTITY_BOMB                  0x02
#define ENTITY_STALFOS_EVASIVE       0x1E
#define ENTITY_DROPPABLE_ARROWS      0x2C
#define ENTITY_DROPPABLE_BOMBS       0x28
#define ENTITY_DROPPABLE_MAGIC_POWDER 0x0D
#define ENTITY_SWORD_SHIELD_PICKUP   0x31
#define ENTITY_GUARDIAN_ACORN        0x34
#define ENTITY_PIECE_OF_POWER        0x33
#define ENTITY_HIDING_SLIME_KEY      0x3C
#define ENTITY_KEY_DROP_POINT        0x30
#define ENTITY_ARMOS_KNIGHT          0x88
#define ENTITY_SWORD_BEAM            0xDF
#define ENTITY_INSTRUMENT_OF_THE_SIRENS 0x39
#define ENTITY_GOOMBA                0x9F
#define ENTITY_FISH                  0xCC
#define ENTITY_PEAHAT                0xA0
#define ENTITY_ROOSTER               0xD5
#define ENTITY_BOW_WOW               0x6D
#define ENTITY_MARIN_AT_THE_SHORE    0xC1
#define ENTITY_HEART_CONTAINER       0x36
#define ENTITY_SPARK_COUNTER_CLOCKWISE 0x16
#define ENTITY_SPARK_CLOCKWISE       0x17
#define ENTITY_MOLDORM               0x59
#define ENTITY_WATER_TEKTITE         0x99
#define ENTITY_MAGIC_ROD_FIREBALL    0x04
#define ENTITY_MAGIC_POWDER_SPRINKLE 0x08
#define ENTITY_IRON_MASK             0x24
#define ENTITY_IRON_MASKS_MASK       0x32
#define ENTITY_BOOMERANG             0x01
#define ENTITY_HOOKSHOT_CHAIN        0x03
#define ENTITY_LIFTABLE_ROCK         0x05
#define ENTITY_FISH                  0xCC
#define ENTITY_PEAHAT                0xA0
#define ENTITY_ROOSTER               0xD5
#define ENTITY_BOW_WOW               0x6D
#define ENTITY_MARIN_AT_THE_SHORE    0xC1
#define ENTITY_HEART_CONTAINER       0x36
#define ENTITY_SPARK_COUNTER_CLOCKWISE 0x16
#define ENTITY_SPARK_CLOCKWISE       0x17
#define ENTITY_MOLDORM               0x59
#define ENTITY_WATER_TEKTITE         0x99
#define ENTITY_MAGIC_ROD_FIREBALL    0x04
#define ENTITY_MAGIC_POWDER_SPRINKLE 0x08
#define ENTITY_IRON_MASK             0x24
#define ENTITY_IRON_MASKS_MASK       0x32
#define ENTITY_BOOMERANG             0x01
#define ENTITY_HOOKSHOT_CHAIN        0x03
#define ENTITY_LIFTABLE_ROCK         0x05
#define ENTITY_FISH                  0xCC
#define ENTITY_PEAHAT                0xA0
#define ENTITY_ROOSTER               0xD5
#define ENTITY_BOW_WOW               0x6D
#define ENTITY_MARIN_AT_THE_SHORE    0xC1
#define ENTITY_HEART_CONTAINER       0x36
#define ENTITY_SPARK_COUNTER_CLOCKWISE 0x16
#define ENTITY_SPARK_CLOCKWISE       0x17
#define ENTITY_MOLDORM               0x59
#define ENTITY_WATER_TEKTITE         0x99

/* Chest types */
#define CHEST_POWER_BRACELET         0x00
#define CHEST_SHIELD                 0x01
#define CHEST_BOMB                   0x0A
#define CHEST_TAIL_KEY               0x11
#define CHEST_MAP                    0x16
#define CHEST_RUPEES_50              0x1B
#define CHEST_SEASHELL               0x20
#define CHEST_MESSAGE                0x21
#define CHEST_FLIPPERS               0x0C
#define CHEST_NIGHTMARE_KEY          0x19

#define MAX_ENTITIES 16

#define ENTITY_STATUS_DISABLED   0
#define ENTITY_STATUS_DYING      1
#define ENTITY_STATUS_FALLING    2
#define ENTITY_STATUS_BURNING    3
#define ENTITY_STATUS_INIT       4
#define ENTITY_STATUS_ACTIVE     5
#define ENTITY_STATUS_STUNNED    6
#define ENTITY_STATUS_LIFTED     7
#define ENTITY_STATUS_THROWN     8

/* Values for wEntitiesGroundStatus */
#define ENTITY_GROUND_STATUS_NORMAL        0
#define ENTITY_GROUND_STATUS_DEEP_WATER    1
#define ENTITY_GROUND_STATUS_SHALLOW_WATER 2
#define ENTITY_GROUND_STATUS_TALL_GRASS    3

/* Values for wEntitiesOptions1Table. */
#define ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL 0x02
#define ENTITY_OPT1_ALLOW_OUT_OF_BOUNDS    0x20
#define ENTITY_OPT1_NO_GROUND_INTERACTION  0x01
#define ENTITY_OPT1_NO_WALL_COLLISION      0x10
#define ENTITY_OPT1_SPLASH_IN_WATER        0x08
#define ENTITY_OPT1_SWORD_CLINK_OFF        0x40

/* Values for wEntitiesPhysicsFlagsTable. */
#define ENTITY_PHYSICS_HARMLESS       0x80
#define ENTITY_PHYSICS_MASK           0xF0
#define ENTITY_PHYSICS_PROJECTILE_NOCLIP 0x40
#define ENTITY_PHYSICS_GRABBABLE      0x20

#define ENTITY_RAFT_RAFT_OWNER   0x6A

#define PhysicsFlagsForEntity      0x4000
#define HitboxFlagsForEntity       0x40FB
#define HealthGroupForEntity       0x41F6
#define InitialHealthForGroup      0x47BC
#define Options1ForEntity          0x42F1

#define EntityHandlersTable      0x4000

#define ENTITY_ARROW                 0x00
#define ENTITY_BOOMERANG             0x01
#define ENTITY_BOMB                  0x02
#define ENTITY_HOOKSHOT_CHAIN        0x03
#define ENTITY_MAGIC_ROD_FIREBALL    0x04
#define ENTITY_LIFTABLE_ROCK         0x05
#define ENTITY_PUSHED_BLOCK          0x06
#define ENTITY_CHEST_WITH_ITEM       0x07
#define ENTITY_MAGIC_POWDER_SPRINKLE 0x08
#define ENTITY_DROPPABLE_HEART       0x2D
#define ENTITY_DROPPABLE_RUPEE       0x2E
#define ENTITY_DROPPABLE_FAIRY       0x2F
#define ENTITY_KEY_DROP_POINT        0x30
#define ENTITY_HEART_PIECE           0x35
#define ENTITY_HIDING_SLIME_KEY      0x3C
#define ENTITY_BOW_WOW               0x6D
#define ENTITY_WRECKING_BALL         0xA8
#define ENTITY_MARIN_AT_THE_SHORE    0xC1
#define ENTITY_GHOST                 0xD4
#define ENTITY_ROOSTER               0xD5
#define ENTITY_SWORD_BEAM            0xDF
#define ENTITY_MUSICAL_NOTE          0xC9
#define ENTITY_MASTER_STALFOS        0x5F
#define ENTITY_GENIE                 0x5C
#define ENTITY_FACADE                0x5A
#define ENTITY_MOLDORM               0x59
#define ENTITY_HOT_HEAD              0x62
#define ENTITY_EVIL_EAGLE            0x63
#define ENTITY_CUCCO                 0x6C
#define ENTITY_TEXT_DEBUGGER         0x6B
#define ENTITY_HIDING_GHINI          0x10
#define ENTITY_GIANT_GHINI           0x11
#define ENTITY_GHINI                 0x12
#define ENTITY_LIKE_LIKE             0x23
#define ENTITY_ARMOS_KNIGHT          0x88
#define ENTITY_DROPPABLE_SECRET_SEASHELL 0x3D
#define ENTITY_LASER_BEAM            0x2B
#define ENTITY_MOBLIN_ARROW          0x0C
#define ENTITY_CHEEP_CHEEP_JUMPING   0xAC
#define ENTITY_GOOMBA                0x9F
#define ENTITY_GEL                   0x1C
#define ENTITY_CUE_BALL              0x8E
#define ENTITY_ROLLING_BONES_BAR     0x82
#define ENTITY_MOBLIN_KING           0xE4
#define ENTITY_BUZZ_BLOB             0xB9
#define ENTITY_BOUNCING_BOMBITE      0x55
#define ENTITY_ANGLER_FISH           0x65
#define ENTITY_SLIME_EYE             0x5B
#define ENTITY_KNIGHT                0x51
#define ENTITY_FINAL_NIGHTMARE       0xE6
#define ENTITY_ANTI_FAIRY            0x15
#define ENTITY_SPIKED_BEETLE         0x2C
#define ENTITY_PAIRODD_PROJECTILE    0x58
#define ENTITY_STAR                  0x9C
#define ENTITY_FLAME_SHOOTER         0xE2
#define ENTITY_NONE                  0xFF

#define ENTITY_OPT1_IS_BOSS          0x80
#define ENTITY_OPT1_IS_MINI_BOSS     0x04
#define ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL 0x02

#define HITFLAGS_IGNORE_HITS         0x80

#define ENTITY_IRON_MASK             0x24
#define ENTITY_OCTOROK_ROCK          0x0A
#define ENTITY_TARIN                 0x3F
#define ENTITY_MAD_BATTER            0xCA

/* Active power-up types */
#define ACTIVE_POWER_UP_PIECE_OF_POWER 0x01
#define ACTIVE_POWER_UP_GUARDIAN_ACORN 0x02

#define ENTITIES_END                 0xFF

/* Trigger types */
#define TRIGGER_THROW_POT_AT_CHEST   0x0D
#define TRIGGER_THROW_AT_DOOR        0x0B
#define TRIGGER_PUSH_SINGLE_BLOCK    0x02
#define TRIGGER_PUSH_BLOCKS          0x07
#define TRIGGER_SHOOT_STATUE_EYE     0x0F

/* Damage types */
#define DAMAGE_TYPE_BOMB               0x07
#define DAMAGE_TYPE_THROW_AT           0x0B
#define DAMAGE_TYPE_ARROW              0x05
#define DAMAGE_TYPE_MAGIC_ROD          0x0A
#define DAMAGE_TYPE_BOMB_ARROW         0x0C

/* Object types */
#define OBJECT_CHEST_CLOSED          0xA0
#define OBJECT_ONE_EYED_STATUE       0xC0
#define OBJECT_FROZEN_BLOCK          0x8A
#define OBJECT_BUSH_GROUND_STAIRS    0xD3
#define OBJECT_BUSH                  0x5C
#define OBJECT_TALL_GRASS            0x0A
#define OBJECT_BOMBABLE_BLOCK        0xA9
#define OBJECT_ROCKY_GROUND          0x09
#define OBJECT_ROCKY_CAVE_DOOR       0xE1
#define OBJECT_GIANT_SKULL_TL        0xBB
#define OBJECT_GIANT_SKULL_BR        0xBE
#define OBJECT_SHORT_GRASS           0x04
#define OBJECT_SHOVEL_HOLE           0xCC




/* Room constants */
#define UNKNOWN_ROOM_74              0x74
#define UNKNOWN_ROOM_C7              0xC7
#define ROOM_OW_COLOR_DUNGEON_ENTRANCE 0x77
#define ROOM_INDOOR_B_KANALET_MAIN_ENTRANCE 0xD3
#define MAP_GHOST_HOUSE              0x1E
#define MAP_HOUSE                    0x10
#define MAP_BOWWOW_HIDEOUT           0x15
#define MAP_UNKNOWN_1A               0x1A
#define UNKNOWN_ROOM_DA              0xDA
#define UNKNOWN_ROOM_A5              0xA5
#define UNKNOWN_ROOM_3A              0x3A
#define UNKNOWN_ROOM_A8              0xA8
#define UNKNOWN_ROOM_B2              0xB2

#define BANK_OverworldEntitiesPointersTable     0x16
#define OverworldEntitiesPointersTable          0x4000
#define IndoorsAEntitiesPointersTable           0x4200
#define IndoorsBEntitiesPointersTable           0x4400
#define ColorDungeonEntitiesPointersTable       0x4600

#endif /* LADX_CONSTANTS_ENTITIES_H */
