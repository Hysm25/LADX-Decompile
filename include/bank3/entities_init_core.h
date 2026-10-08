#ifndef LADX_BANK3_ENTITIES_INIT_CORE_H
#define LADX_BANK3_ENTITIES_INIT_CORE_H

#include "gb.h"

/* Entity Configuration (03:485B-03:4891) */
void ConfigureNewEntity(GBState *gb);
void ConfigureNewEntity_attributes(GBState *gb, uint16_t bc);

/* Entity Health Configuration (03:4895-03:48AC) */
void ConfigureEntityHealth(GBState *gb, uint16_t bc, uint8_t entity_type, uint8_t d);

/* Entity Initialization Handler (03:48B5-03:4923) */
void EntityInitHandler(GBState *gb);

/* Master Stalfos Defeated Handler (03:48AD-03:48BE) */
void MasterStalfosDefeated(GBState *gb);

#endif /* LADX_BANK3_ENTITIES_INIT_CORE_H */