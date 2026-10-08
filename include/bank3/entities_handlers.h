#ifndef LADX_BANK3_ENTITIES_HANDLERS_H
#define LADX_BANK3_ENTITIES_HANDLERS_H

#include <stdint.h>
#include "gb.h"

/* Data Tables (03:4C44-03:5721) */
extern const uint8_t FireSpriteVariants[8];
extern const uint8_t Unknown020SpriteVariants[4];
extern const uint8_t Data_003_4CA4[4];
extern const uint8_t Data_003_4CAC[6];
extern const uint8_t Data_003_4CA8[4];
extern const uint8_t Data_003_4E05[2];
extern const uint8_t Data_003_56EA[4];
extern const uint8_t Data_003_56EE[3];
extern const uint8_t Data_003_56F1[17];
extern const uint8_t Data_003_5701[17];
extern const uint8_t Data_003_5711[17];
extern const uint8_t Data_003_5721[17];

/* Entity 25/26 Stubs (03:4C44) */
void EntityInitEntity25(GBState *gb, uint16_t bc);
void EntityInitEntity26(GBState *gb, uint16_t bc);
void Entity25Handler(GBState *gb, uint16_t bc);
void Entity26Handler(GBState *gb, uint16_t bc);

/* Entity Handlers (03:4C4C-03:57E6, 03:7267) */
void EntityBurningHandler(GBState *gb, uint16_t bc);
void EntityFallHandler(GBState *gb, uint16_t bc);
void EntityThrownHandler(GBState *gb, uint16_t bc);
void EntityStunnedHandler(GBState *gb, uint16_t bc);
void EntityGetLiftedUp(GBState *gb, uint16_t bc);
void EntityLiftedHandler(GBState *gb, uint16_t bc);
void func_003_5795(GBState *gb, uint16_t bc, uint8_t e);
void EntityBecomeStunned(GBState *gb, uint16_t bc);

#endif /* LADX_BANK3_ENTITIES_HANDLERS_H */