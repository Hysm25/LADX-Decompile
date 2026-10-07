#ifndef LADX_CONSTANTS_ROOMS_H
#define LADX_CONSTANTS_ROOMS_H

/* Overworld and Indoor Room IDs */
#define ROOM_OW_TURTLE_ROCK_WARP_HOLE       0x01
#define UNKNOWN_ROOM_06              0x06
#define ROOM_OW_RIGHT_OF_EGG         0x07
#define ROOM_OW_MARIN_BRIDGE         0x08
#define ROOM_OW_EAGLES_TOWER         0x0E
#define UNKNOWN_ROOM_0C              0x0C
#define UNKNOWN_ROOM_1B              0x1B
#define UNKNOWN_ROOM_1E              0x1E
#define ROOM_OW_ANGLERS_TUNNEL_ENTRANCE 0x2B
#define ROOM_OW_WATERFALL_WARP_HOLE  0x2C
#define ROOM_OW_CAMERA_SHOP          0x37
#define ROOM_INDOOR_B_MOUNTAIN_CAVE_ROOM_1  0x7A
#define ROOM_INDOOR_B_MOUNTAIN_CAVE_ROOM_2  0x7B
#define ROOM_INDOOR_B_MOUNTAIN_CAVE_ROOM_3  0x7C
#define ROOM_INDOOR_B_MOUNTAIN_CAVE_ROOM_4  0x7D
#define UNKNOWN_ROOM_D2                     0xD2
#define ROOM_OW_UKUKU_PRAIRIE_WARP_HOLE     0x95
#define ROOM_OW_ANIMAL_VILLAGE_WARP_HOLE    0xEC
#define ROOM_OW_KANALET_GATE         0x79
#define ROOM_OW_FACE_SHRINE_ENTRANCE 0x8C
#define ROOM_OW_GIANT_SKULL          0x97
#define ROOM_OW_SIREN                0xC9
#define ROOM_OW_WALRUS               0xFD
#define ROOM_INDOOR_B_CAMERA_SHOP    0xB5
#define ROOM_INDOOR_A_GORIYA         0xF5
#define ROOM_INDOOR_A_WATER_FLOODED_GROTTO 0xF2
#define ROOM_OW_COLOR_DUNGEON_ENTRANCE     0x77
#define ROOM_OW_KANALET_MOAT_HEARTPIECE    0x78
#define UNKNOWN_ROOM_8D                    0x8D
#define TRADING_ITEM_MAGNIFYING_LENS 0x0E

#define ROOM_SECTION_OW_SECOND_HALF  0x80
#define ROOM_INDOOR_B_MANBO             0xFD
#define ROOM_INDOOR_B_FISHING_MINIGAME  0xB1
#define ROOM_INDOOR_B_MRS_MEOW_MEOW     0xA7
#define ROOM_SECTION_OW_GHOST_TRIGGER   0x40
#define UNKNOWN_ROOM_A4                 0xA4
#define UNKNOWN_ROOM_C7                 0xC7

/* Overworld Room Banks */
#define BANK_OverworldRoomsFirstHalf  0x09
#define BANK_OverworldRoomsSecondHalf 0x1A

/* Room Template & World Map Loader Addresses */
#define BANK_LoadRoomTemplate        0x14
#define LoadRoomTemplate             0x4880
#define BANK_LoadWorldMapBGMap       0x20
#define LoadWorldMapBGMap            0x588B

/* Room Pointer Tables Addresses */
#define OverworldRoomPointers        0x4000
#define IndoorsARoomPointers         0x4000
#define IndoorsBRoomPointers         0x4000
#define ColorDungeonRoomPointers     0x7B77

/* Alternate Overworld / Indoor Room Data Addresses */
#define Overworld0EAlt               0x47EC
#define Overworld8CAlt               0x434E
#define Overworld79Alt               0x6513
#define Overworld06Alt               0x4496
#define Overworld1BAlt               0x4C0F
#define Overworld2BAlt               0x509A
#define IndoorsAF5Alt                0x7855

/* Room Header & Object Stream Control Bytes */
#define ROOM_WARP                    0xE0
#define ROOM_END                     0xFE
#define ROOM_BORDER                  0xFF

/* Room Status Flags */
#define ROOM_STATUS_DOOR_OPEN_RIGHT  0x01
#define ROOM_STATUS_DOOR_OPEN_LEFT   0x02
#define ROOM_STATUS_DOOR_OPEN_UP     0x04
#define ROOM_STATUS_DOOR_OPEN_DOWN   0x08
#define OW_ROOM_STATUS_CHANGED       0x10
#define OW_ROOM_STATUS_OWL_TALKED    0x20
#define ROOM_STATUS_VISITED          0x80

/* Door Types */
#define DOOR_TYPE_KEY_TOP            0x00
#define DOOR_TYPE_KEY_BOTTOM         0x01
#define DOOR_TYPE_KEY_LEFT           0x02
#define DOOR_TYPE_KEY_RIGHT          0x03
#define DOOR_TYPE_SHUTTER_TOP        0x04
#define DOOR_TYPE_SHUTTER_BOTTOM     0x05
#define DOOR_TYPE_SHUTTER_LEFT       0x06
#define DOOR_TYPE_SHUTTER_RIGHT      0x07
#define DOOR_TYPE_BOSS_TOP           0x08

/* Shutter Door Mask Bits */
#define DOOR_TYPE_SHUTTER_TOP_BIT    0x01
#define DOOR_TYPE_SHUTTER_BOTTOM_BIT 0x02
#define DOOR_TYPE_SHUTTER_LEFT_BIT   0x04
#define DOOR_TYPE_SHUTTER_RIGHT_BIT  0x08

/* Additional Room Constants */
#define ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_DROP 0x69
#define ROOM_INDOOR_A_QUICKSAND_CAVE 0xF8
#define ROOM_INDOOR_A_ANGLERS_TUNNEL_KEY_FALL 0x7C
#define ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_4 0x80
#define ROOM_INDOOR_B_KANALET_MAIN_ENTRANCE 0xD3
#define UNKNOWN_ROOM_0A              0x0A
#define UNKNOWN_ROOM_4A              0x4A
#define UNKNOWN_ROOM_75              0x75
#define UNKNOWN_ROOM_AA              0xAA
#define UNKNOWN_ROOM_C4              0xC4
#define UNKNOWN_ROOM_32              0x32
#define ROOM_INDOOR_B_KANALET_GATE_SWITCH 0xC3
#define UNKNOWN_ROOM_3C                   0x3C
#define UNKNOWN_ROOM_3D                   0x3D
#define UNKNOWN_ROOM_3E                   0x3E
#define UNKNOWN_ROOM_3F                   0x3F
#define UNKNOWN_ROOM_41                   0x41
#define UNKNOWN_ROOM_63                   0x63
#define UNKNOWN_ROOM_71                   0x71

/* Bank 0 Macro Tables ROM Addresses */
#define KeyDoorTopObjectIds_Addr             0x35F8
#define KeyDoorBottomObjectIds_Addr          0x3613
#define KeyDoorLeftObjectIds_Addr            0x362E
#define KeyDoorRightObjectIds_Addr           0x3649
#define OpenDoorTopObjectIds_Addr            0x36B0
#define OpenDoorBottomObjectIds_Addr         0x36E8
#define OpenDoorLeftObjectIds_Addr           0x36FC
#define OpenDoorRightObjectIds_Addr          0x3710
#define BossDoorObjectIds_Addr               0x3724
#define StairsDoorObjectIds_Addr             0x375C
#define RevolvingDoorObjectIds_Addr          0x376B
#define OneWayArrowObjectIds_Addr            0x377A
#define DungeonEntranceObjectOffsets_Addr    0x3789
#define DungeonEntranceObjectIds_Addr        0x3796
#define EntranceObjectIds_Addr               0x37B4
#define HorizontalObjectOffsets_Addr         0x37E1
#define VerticalObjectOffsets_Addr           0x37E4

/* Static Object Physics Flags ROM tables in Bank $08 */
#define BANK_ObjectPhysicFlags       0x08
#define OverworldObjectPhysicFlags   0x4AD4
#define Indoors1ObjectPhysicFlags    0x4BD4

#define ROOM_INDOOR_B_SCHULE_HOUSE            0xDD
#define ROOM_INDOOR_B_EAGLES_TOWER_BOSS       0xE8
#define ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_1 0x95
#define ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_2 0x92
#define ROOM_INDOOR_A_CATFISHS_MAW_MSTALFOS_3 0x84
#define UNKNOWN_ROOM_65                     0x65
#define UNKNOWN_ROOM_C0                     0xC0
#define UNKNOWN_ROOM_DA                     0xDA
#define UNKNOWN_ROOM_E2                     0xE2
#define ROOM_INDOOR_B_CHRISTINE_HOUSE       0xD9
#define ROOM_OW_YARNA_LANMOLA               0xCE
#define OW_ROOM_STATUS_OPENED               0x04

#endif /* LADX_CONSTANTS_ROOMS_H */
