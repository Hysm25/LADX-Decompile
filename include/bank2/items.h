#ifndef LADX_BANK2_ITEMS_H
#define LADX_BANK2_ITEMS_H

#include "gb.h"

extern const int8_t HookshotChainSpeedX[4];
extern const int8_t HookshotChainSpeedY[4];
/**
 * Fires the hookshot chain projectile in Link's facing direction. (02:4254)
 *
 * @param gb Pointer to Game Boy system state.
 * @return true if hookshot projectile was successfully spawned, false otherwise.
 */
bool FireHookshot(GBState *gb);

/**
 * Link got item sequence dispatcher: triggers power-up jingle on countdown 0x2E and updates sprite (02:51BC).
 *
 * @param gb Pointer to Game Boy system state.
 * @param apply_motion_state Optional callback for ApplyLinkMotionState.
 * @param func_020_4ab3 Optional callback for bank 20 helper func_020_4AB3.
 */
void HandleGotItemA(GBState *gb,
                    void (*apply_motion_state)(GBState *),
                    void (*func_020_4ab3)(GBState *));

/**
 * Link got item state handler: resets spin attack, applies air physics, updates OAM sprite buffer (02:51C7).
 *
 * @param gb Pointer to Game Boy system state.
 * @param apply_motion_state Optional callback for ApplyLinkMotionState.
 * @param func_020_4ab3 Optional callback for bank 20 helper func_020_4AB3.
 */
void HandleGotItemB(GBState *gb,
                    void (*apply_motion_state)(GBState *),
                    void (*func_020_4ab3)(GBState *));

/**
 * Writes Guardian Acorn tile and returns OAM flags (02:523A).
 *
 * @param gb Pointer to Game Boy system state.
 * @param bc Destination address in OAM buffer.
 * @param e Tile ID.
 * @return OAM attributes byte ($14).
 */
uint8_t func_002_523A(GBState *gb, uint16_t bc, uint8_t e);

/**
 * Writes default got-item tile and returns OAM flags (02:523F).
 *
 * @param gb Pointer to Game Boy system state.
 * @param bc Destination address in OAM buffer.
 * @param e Tile ID.
 * @return OAM attributes byte ($14).
 */
uint8_t func_002_523F(GBState *gb, uint16_t bc, uint8_t e);

/**
 * Writes Magic Rod tile and returns OAM flags (02:524A).
 *
 * @param gb Pointer to Game Boy system state.
 * @param bc Destination address in OAM buffer.
 * @param e Tile ID.
 * @return OAM attributes byte ($10).
 */
uint8_t func_002_524A(GBState *gb, uint16_t bc, uint8_t e);

/* Bank 2 Magic Rod Sprite Data Tables */
extern const int8_t LinkDirectionToMagicRodXOffset[8];
extern const int8_t LinkDirectionToMagicRodYOffset[8];
extern const uint8_t LinkDirectionToMagicRodTiles[16];
extern const uint8_t LinkDirectionToMagicRodOAMAttributes[16];
extern const int8_t LinkDirectionToEntitiesPositionX[4];
extern const int8_t LinkDirectionToEntitiesPositionY[4];

/**
 * Emits Magic Rod attack OAM sprites according to direction and swing phase (02:5310).
 *
 * @param gb Pointer to Game Boy system state.
 */
void label_002_5310(GBState *gb);

/**
 * Initializes projectile entity position, variant, and velocity based on Link direction (02:538B).
 *
 * @param gb Pointer to Game Boy system state.
 * @param de Target entity slot index.
 */
void label_002_538B_entity(GBState *gb, uint8_t de);

/**
 * Initializes spawned Magic Rod fireball entity position, variant, and velocity (02:538B).
 *
 * @param gb Pointer to Game Boy system state.
 */
void label_002_538B(GBState *gb);

/**
 * Clamps an item count to its maximum value (02:60D8).
 *
 * If the current count at `de` is greater than or equal to the maximum at `hl`,
 * the current count is set to the maximum. The `hl` pointer is incremented
 * (per the assembly's `inc hl` at 02:60DE) but not returned.
 *
 * @param gb Pointer to Game Boy system state.
 * @param hl Address of the maximum item count.
 * @param de Address of the current item count.
 */
void ClampItemCount(GBState *gb, uint16_t hl, uint16_t de);

/**
 * Inventory and subscreen handler (02:60E0-02:61E7).
 *
 * Clamps item counts (magic powder, bombs, arrows), checks if Link is in an
 * interactive state, handles subscreen opening/closing, inventory scrolling,
 * and calls UpdateRupeesCount/UpdateHealth when appropriate.
 *
 * @param gb Pointer to Game Boy system state.
 */
void func_002_60E0(GBState *gb);

/* Forward declarations for functions in other bank 2 modules */
void LoadMinimap(GBState *gb);
void func_002_753A(GBState *gb);
void func_002_754F(GBState *gb);
void func_002_755B(GBState *gb);

/* Forward declarations for functions in this file */
void UpdateRupeesCount(GBState *gb);
void UpdateHealth(GBState *gb);
void LoadRupeesDigits(GBState *gb);
void LoadHeartsCount(GBState *gb);
void func_002_61BA(GBState *gb);
extern const uint8_t ThresholdLowHealthTable[16];

#endif /* LADX_BANK2_ITEMS_H */
