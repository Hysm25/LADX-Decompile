#ifndef LADX_CONSTANTS_DIALOG_H
#define LADX_CONSTANTS_DIALOG_H

#include "common.h"

/* Dialog states */
#define DIALOG_CLOSED                  0x00
#define DIALOG_OPENING_1               0x01
#define DIALOG_OPENING_2               0x02
#define DIALOG_OPENING_3               0x03
#define DIALOG_OPENING_4               0x04
#define DIALOG_OPENING_5               0x05
#define DIALOG_LETTER_IN_1             0x06
#define DIALOG_LETTER_IN_2             0x07
#define DIALOG_LETTER_IN_3             0x08
#define DIALOG_BREAK                   0x09
#define DIALOG_SCROLLING_1             0x0A
#define DIALOG_SCROLLING_2             0x0B
#define DIALOG_END                     0x0C
#define DIALOG_CHOICE                  0x0D
#define DIALOG_CLOSING_1               0x0E
#define DIALOG_CLOSING_2               0x0F

/* Got item dialog IDs (wDialogGotItem) */
#define DIALOG_GOT_PIECE_OF_POWER      0x01
#define DIALOG_GOT_TOADSTOOL           0x02
#define DIALOG_GOT_MAGIC_POWDER        0x03
#define DIALOG_GOT_ROD                 0x04
#define DIALOG_GOT_GUARDIAN_ACORN      0x05

#define DIALOG_BOX_BOTTOM_FLAG         0x80
#define DIALOG_BOX_BOTTOM_BIT          7
#define DIALOG_COOLDOWN                0x18

/* Dialog background tile constants */
#define Dialog026                  0x26
#define Dialog028                  0x28
#define Dialog0DA                  0xDA
#define DIALOG_BG_TILE_DARK        0x7E
#define DIALOG_BG_TILE_LIGHT       0x7F

/* Dialog Codepoint Conversion Tables in ROM Bank $1C */
#define BANK_CodepointToTileMap    0x1C
#define ADDR_CodepointToTileMap    0x4641
#define ADDR_CodepointToDiacritic  0x4741

#define Dialog18E                       0x8E
#define Dialog1A9                       0xA9
#define Dialog1AF                       0xAF
#define Dialog083                       0x83
#define Dialog22D                       0x2D
#define Dialog278                       0x78
#define Dialog0FF                       0xFF
#define Dialog0FC                       0xFC
#define Dialog0FD                       0xFD
#define Dialog04F                       0x4F
#define Dialog199                       0xC7
#define Dialog0B7                       0xB7
#define Dialog0B9                       0xB9
#define Dialog0BD                       0xBD
#define Dialog0EF                       0xEF

#endif /* LADX_CONSTANTS_DIALOG_H */
