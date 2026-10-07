#ifndef LADX_CONSTANTS_PHYSICS_H
#define LADX_CONSTANTS_PHYSICS_H

/* Values for wCollisionType */
#define COLLISION_TYPE_NONE         0x00
#define COLLISION_TYPE_UP           0x01
#define COLLISION_TYPE_DOWN         0x02
#define COLLISION_TYPE_VERTICAL     0x03
#define COLLISION_TYPE_LEFT         0x04
#define COLLISION_TYPE_RIGHT        0x08
#define COLLISION_TYPE_HORIZONTAL   0x0C
#define COLLISION_TYPE_UNKNOWN_10   0x10

/* Values for objects physic flags (See GetObjectPhysicsFlags) */
#define OBJ_PHYSICS_NONE               0x00 /* pass-through */
#define OBJ_PHYSICS_SOLID              0x01
#define OBJ_PHYSICS_STAIRS             0x02
#define OBJ_PHYSICS_DOOR               0x03
#define OBJ_PHYSICS_OCEAN_SWITCH_BLOCK 0x04
#define OBJ_PHYSICS_SHALLOW_WATER      0x05
#define OBJ_PHYSICS_GRASS              0x06 /* cuttable */
#define OBJ_PHYSICS_DEEP_WATER         0x07
#define OBJ_PHYSICS_RAISED             0x08
#define OBJ_PHYSICS_LOWERED            0x09
#define OBJ_PHYSICS_WIDE_STAIRS        0x0A
#define OBJ_PHYSICS_LAVA               0x0B
#define OBJ_PHYSICS_CONVEYOR           0xF0
#define OBJ_PHYSICS_SPIKES             0xE0
#define OBJ_PHYSICS_LEDGE              0xD0 /* jumpable, 0-3: direction */
#define OBJ_PHYSICS_LEDGE_OVERWORLD    0x10 /* jumpable, only downwards */
#define OBJ_PHYSICS_REMOVABLE_OBSTACLE 0x30 /* bush/rock/keyblock/cracked block/sword-crystals */
#define OBJ_PHYSICS_PIT                0x50
#define OBJ_PHYSICS_PIT_WARP           0x51 /* pit with warp to other room */
#define OBJ_PHYSICS_HOOKSHOTABLE       0x60 /* solid or dash-crystal */
#define OBJ_PHYSICS_DOOR_OPEN          0x70 /* open door/flip door */
#define OBJ_PHYSICS_FINE_COLLISION     0x80 /* for 8x8 tile collision */
#define OBJ_PHYSICS_DOOR_CLOSED        0x90 /* key door / boss door / bombable door */
#define OBJ_PHYSICS_WATER_SIDESCROLL   0xB0
#define OBJ_PHYSICS_LADDER_SIDESCROLL  0xB1
#define OBJ_PHYSICS_TRACTOR_DEVICE     0xFF

/* Ground status values */
#define GROUND_STATUS_NONE          0x00
#define GROUND_STATUS_SLOW          0x03
#define GROUND_STATUS_PIT           0x07

#endif /* LADX_CONSTANTS_PHYSICS_H */
