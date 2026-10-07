#include "bank3/entities_collision.h"
#include "bank3/entities_physics.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/rooms.h"
#include "constants/gameplay.h"
#include "constants/directions.h"
#include "constants/inventory.h"
#include "constants/joypad.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include "home/entities.h"
#include "home/room.h"
#include "home/bank.h"
#include "home/audio.h"
#include "home/gameplay.h"
#include "home/vfx.h"
#include "bank3/entities_droppable.h"
#include "constants/audio.h"
#include "constants/vfx.h"
#include "constants/dialog.h"
#include "home/dialog.h"
#include "bank3/entities_init_core.h"
#include "bank3/entities_handlers.h"
#include "bank3/entities_liftable_rock.h"

/* Amount of damages an entity deals when colliding with Link (03:47F1) */
const uint8_t EntityDamagesForGroup[53] = {
    0x04, 0x04, 0x08, 0x08, 0x18, 0x08, 0x04, 0x08,
    0x10, 0x08, 0x10, 0x08, 0x08, 0x04, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x0C, 0x00, 0x00,
    0x08, 0x08, 0x08, 0x0C, 0x0C, 0x14, 0x10, 0x20,
    0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00,
    0x14, 0x08, 0x04, 0x08, 0x04, 0x04, 0x08, 0x08,
    0x04, 0x04, 0x04, 0x08, 0x08
};

/* Directional speed tables for Spiked Beetle collision (03:6F65, 03:6F69) */
static const uint8_t Data_003_6F65[4] = {
    0x10, /* RIGHT:  16 */
    0xF0, /* LEFT:  -16 */
    0x00, /* UP:      0 */
    0x00  /* DOWN:    0 */
};

static const uint8_t Data_003_6F69[4] = {
    0x00, /* RIGHT:   0 */
    0x00, /* LEFT:    0 */
    0xF0, /* UP:    -16 */
    0x10  /* DOWN:   16 */
};

/* Direction table for Iron Mask (03:6FE4) */
const uint8_t Data_003_6FE4[4] = {
    0x00, /* RIGHT: 0 */
    0x01, /* LEFT:  1 */
    0x02, /* UP:    2 */
    0x03  /* DOWN:  3 */
};

/* Dropped item table for Ghini (03:73E7) */
const uint8_t Data_003_73E7[4] = {
    0x2D, 0x2E, 0x38, 0x37
};

const uint8_t Data_003_43EC[848] = {
    0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x02, 0x01, 0x02, 0x03, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x01, 0x02, 0x01, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x02, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x02, 0x04, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x04, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x04, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x05, 0x02, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x04, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x04, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t Data_003_473C[128] = {
    0x00, 0x01, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x02, 0x01, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x04, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x08, 0x04, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x10, 0x08, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x01, 0x04, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x01, 0x04, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x18, 0xFE, 0x02, 0xFD, 0xFF, 0x00,
    0x00, 0xFF, 0xFD, 0xFE, 0x00, 0x00, 0x02, 0x00,
    0x00, 0x01, 0x04, 0xFE, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x01, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x01, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x01, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0x01, 0x02, 0x40, 0x00, 0x00, 0xFF, 0x00
};


/* ===== CheckLinkCollisionWithEnemy (03:6C72) ===== */
bool CheckLinkCollisionWithEnemy(GBState *gb, uint16_t bc) {
    if (!gb) return false;

    /* If Link is in the air, skip the collision check */
    if (gb_read_hram(gb, hLinkPositionZ) != 0) {
        return false;
    }

    /* If Link is not interactive, return */
    if (gb_read(gb, wLinkMotionState) >= LINK_MOTION_TYPE_NON_INTERACTIVE) {
        return false;
    }

    /* Hitbox offset table: 4 bytes per entity:
       byte 0: X offset
       byte 1: X half-width radius
       byte 2: Y offset
       byte 3: Y half-height radius */
    uint16_t hitbox_addr = (uint16_t)(wEntitiesHitboxPositionTable + ((bc & 0xFF) << 2));

    /* Check X distance */
    uint8_t ent_x = (uint8_t)(gb_read_hram(gb, hActiveEntityPosX) + gb_read(gb, hitbox_addr + 0));
    uint8_t diff_x = (uint8_t)(ent_x - gb_read_hram(gb, hLinkPositionX) - 8);
    if ((diff_x & 0x80) != 0) {
        diff_x = (uint8_t)(~diff_x + 1);
    }
    uint8_t limit_x = (uint8_t)(gb_read(gb, hitbox_addr + 1) + 4);
    if (diff_x >= limit_x) {
        return false;
    }

    /* Check Y distance */
    uint8_t ent_y = (uint8_t)(gb_read_hram(gb, hActiveEntityVisualPosY) + gb_read(gb, hitbox_addr + 2));
    uint8_t diff_y = (uint8_t)(ent_y - gb_read_hram(gb, hLinkPositionY) - 8);
    if ((diff_y & 0x80) != 0) {
        diff_y = (uint8_t)(~diff_y + 1);
    }
    uint8_t limit_y = (uint8_t)(gb_read(gb, hitbox_addr + 3) + 4);
    if (diff_y >= limit_y) {
        return false;
    }

    /* Collision occurred! Check if harmless or Link in falling animation */
    if (func_003_6CC0(gb, bc)) {
        return true;
    }

    /* Apply damages to Link */
    ApplyLinkCollisionWithEnemy(gb, bc);
    return true;
}

/* ===== ApplyLinkCollisionWithEnemy (03:6CD5) ===== */
void ApplyLinkCollisionWithEnemy(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* Special case when a cheep-cheep hurts Link */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_CHEEP_CHEEP_JUMPING) {
        uint8_t dir = 0, dist = 0;
        GetEntityYDistanceToLink_03_idx(gb, bc, &dir, &dist);
        if (dir == DIRECTION_UP) {
            IncrementEntityState(gb, bc);
            gb_write(gb, wEntitiesStateTable + bc, ENTITY_STATUS_ACTIVE);
            gb_write(gb, wIsLinkInTheAir, 0x02);
            gb_write_hram(gb, hLinkSpeedY, 0xF0);
            ClearEntitySpeed(gb, bc);
            gb_write_hram(gb, hWaveSfx, WAVE_SFX_FLOOR_SWITCH);
            return;
        }
        goto goombaEnd;
    }

    /* Special case when a Goomba hurts Link */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_GOOMBA) {
        if (gb_read(gb, wIsLinkInTheAir) != 0) {
            bool squish = false;
            if (gb_read_hram(gb, hLinkCountdown) != 0) {
                squish = true;
            } else {
                uint8_t check_val;
                if (gb_read_hram(gb, hIsSideScrolling) != 0) {
                    check_val = gb_read_hram(gb, hLinkSpeedY);
                } else {
                    check_val = (uint8_t)(gb_read_hram(gb, hLinkVelocityZ) ^ 0x80);
                }
                if ((check_val & 0x80) == 0) {
                    squish = true;
                }
            }

            if (squish) {
                gb_write_hram(gb, hLinkCountdown, 0x02);
                gb_write(gb, wEntitiesStateTable + bc, 0x02);
                gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x30);
                gb_write_hram(gb, hWaveSfx, WAVE_SFX_FLOOR_SWITCH);
                if (gb_read_hram(gb, hIsSideScrolling) != 0) {
                    gb_write_hram(gb, hLinkSpeedY, 0xF0);
                } else {
                    gb_write_hram(gb, hLinkVelocityZ, 0x10);
                }
                return;
            }
        }
    }

goombaEnd:
    /* Special case when Link collides with a Gel */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_GEL) {
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x80);
        IncrementEntityState(gb, bc);
        gb_write(gb, wEntitiesStateTable + bc, 0x04);
        return;
    }

    if (gb_read_hram(gb, hActiveEntityType) != ENTITY_CUE_BALL &&
        gb_read_hram(gb, hActiveEntityType) != ENTITY_ROLLING_BONES_BAR) {
        if (gb_read(gb, wIgnoreLinkCollisionsCountdown) != 0) {
            return;
        }
    }

    /* Moblin King in state 4 */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_MOBLIN_KING) {
        if (gb_read_hram(gb, hActiveEntityState) == 0x04) {
            IncrementEntityState(gb, bc);
            gb_write(gb, wEntitiesStateTable + bc, 0x08);
            gb_write_hram(gb, hWaveSfx, WAVE_SFX_LINK_HURT);
            return;
        }
    }

    /* Link damage immunity check */
    if ((gb_read(gb, wInvincibilityCounter) |
         gb_read(gb, wIsLinkImmuneToCollisionDamage) |
         gb_read(gb, wLinkPlayingOcarinaCountdown) |
         gb_read(gb, wDialogGotItem)) != 0) {
        return;
    }

    gb_write_hram(gb, hWaveSfx, WAVE_SFX_LINK_HURT);

    /* Nominal damage by health group */
    uint8_t health_group = gb_read(gb, wEntitiesHealthGroup + bc);
    uint8_t damage = 0;
    if (health_group < sizeof(EntityDamagesForGroup)) {
        damage = EntityDamagesForGroup[health_group];
    }

    if (gb_read(gb, wTunicType) == TUNIC_BLUE) {
        damage >>= 1;
    } else if (gb_read(gb, wActivePowerUp) == ACTIVE_POWER_UP_GUARDIAN_ACORN) {
        if (damage == 4) {
            damage = 0;
        } else {
            damage >>= 1;
        }
    }

    gb_write(gb, wSubtractHealthBuffer, (uint8_t)(gb_read(gb, wSubtractHealthBuffer) + damage));
    gb_write(gb, wInvincibilityCounter, 0x50);
    gb_write(gb, wGuardianAcornCounter, 0x00);

    if (gb_read(gb, wActivePowerUp) != 0) {
        uint8_t hits = (uint8_t)(gb_read(gb, wPowerUpHits) + 1);
        gb_write(gb, wPowerUpHits, hits);
        if (hits >= 3) {
            gb_write(gb, wActivePowerUp, 0x00);
            if (gb_read(gb, wInBossBattle) == 0) {
                uint8_t def_music = gb_read_hram(gb, hDefaultMusicTrack);
                if (def_music != MUSIC_OWL) {
                    gb_write(gb, wMusicTrackToPlay, def_music);
                }
                gb_write_hram(gb, hNextDefaultMusicTrack, def_music);
            }
        }
    }

    func_003_6DDF(gb, bc);
}

/* ===== DefaultEnemyDamageCollisionHandler (03:6E28) ===== */
void DefaultEnemyDamageCollisionHandler(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* call func_003_6C6B: on odd parity frames, check Link collision */
    if (func_003_6C6B(gb, bc)) {
        CheckLinkCollisionWithEnemy(gb, bc);
    }

    /* func_003_6E2B: sword/item damage collision */
    func_003_6E2B(gb, bc);
}

/* ===== func_003_6E2B (03:6E2B) ===== */
void func_003_6E2B(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld a, [wC140]; cp $00; jp z, label_003_73E6 */
    if (gb_read(gb, wC140) == 0) {
        return;
    }

    /* ld hl, wEntitiesFlashCountdownTable; add hl, bc; ld a, [hl]; and a; jr z, .jr_6E40 */
    uint8_t flash = gb_read(gb, wEntitiesFlashCountdownTable + bc);
    if (flash != 0) {
        /* cp $18; jp c, label_003_73E6 */
        if (flash < 0x18) {
            return;
        }
    }

    /* .jr_6E40: ld a, [wC1AC]; and a; jr z, .jr_6E4B */
    uint8_t c1ac = gb_read(gb, wC1AC);
    if (c1ac != 0) {
        /* dec a; cp c; jp z, label_003_73E6 */
        if ((uint8_t)(c1ac - 1) == (uint8_t)(bc & 0xFF)) {
            return;
        }
    }

    /* .jr_6E4B: ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld a, [hl]; and a; jp nz, label_003_73E6 */
    if (gb_read(gb, wEntitiesIgnoreHitsCountdownTable + bc) != 0) {
        return;
    }

    /* Hitbox check: bounding box comparison against weapon at wC140..wC143 */
    uint16_t hitbox_addr = (uint16_t)(wEntitiesHitboxPositionTable + ((bc & 0xFF) << 2));

    uint8_t ent_x = (uint8_t)(gb_read_hram(gb, hActiveEntityPosX) + gb_read(gb, hitbox_addr + 0));
    uint8_t diff_x = (uint8_t)(ent_x - gb_read(gb, wC140));
    if ((diff_x & 0x80) != 0) {
        diff_x = (uint8_t)(~diff_x + 1);
    }
    uint8_t limit_x = (uint8_t)(gb_read(gb, wC141) + gb_read(gb, hitbox_addr + 1));
    if (diff_x >= limit_x) {
        return;
    }

    uint8_t ent_y = (uint8_t)(gb_read_hram(gb, hActiveEntityVisualPosY) + gb_read(gb, hitbox_addr + 2));
    uint8_t diff_y = (uint8_t)(ent_y - gb_read(gb, wC142));
    if ((diff_y & 0x80) != 0) {
        diff_y = (uint8_t)(~diff_y + 1);
    }
    uint8_t limit_y = (uint8_t)(gb_read(gb, wC143) + gb_read(gb, hitbox_addr + 3));
    if (diff_y >= limit_y) {
        return;
    }

    /* Grabbable entity check */
    if ((gb_read(gb, wEntitiesPhysicsFlagsTable + bc) & ENTITY_PHYSICS_GRABBABLE) != 0) {
        PickableCollectIfNeeded(gb, bc);
        return;
    }

    /* Sword collision enabled */
    if (gb_read(gb, wSwordCollisionEnabled) != 0) {
        EnemyCollidedWithSword(gb, bc);
        return;
    }

    /* Save pegasus boots object below Link, reset pegasus boots */
    gb_write_hram(gb, hIndexOfObjectBelowLink, gb_read(gb, wIsRunningWithPegasusBoots));
    ResetPegasusBoots(gb);

    uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);

    if (entity_type == ENTITY_FLAME_SHOOTER) {
        if (gb_read(gb, wShieldLevel) != 2 || gb_read_hram(gb, hLinkDirection) != DIRECTION_UP) {
            return;
        }
        gb_write_hram(gb, hLinkSpeedY, 0x04);
        gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x08);
        IncrementEntityState(gb, bc);
        return;
    }

    if (entity_type == ENTITY_BOUNCING_BOMBITE) {
        if (gb_read_hram(gb, hActiveEntityState) != 0x02) {
            func_003_6F93(gb);
            return;
        }
        uint8_t sx = gb_read(gb, wEntitiesSpeedXTable + bc);
        gb_write(gb, wEntitiesSpeedXTable + bc, (uint8_t)(~sx + 1));
        uint8_t sy = gb_read(gb, wEntitiesSpeedYTable + bc);
        gb_write(gb, wEntitiesSpeedYTable + bc, (uint8_t)(~sy + 1));
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x40);
        gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x08);
        return;
    }

    if (entity_type == ENTITY_KNIGHT) {
        if ((gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_SWORD_CLINK_OFF) == 0) {
            func_003_6F93(gb);
            return;
        }
        uint8_t ps1 = gb_read(gb, wEntitiesPrivateState1Table + bc);
        gb_write(gb, wEntitiesPrivateState1Table + bc, (uint8_t)(~ps1 + 1));
        func_003_6F5C(gb, bc);
        gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x0C);
        gb_write(gb, wC160, 0x01);
        gb_write(gb, wSwordCharge, 0x00);
        gb_write_hram(gb, hMultiPurpose0, gb_read_hram(gb, hActiveEntityPosX));
        gb_write_hram(gb, hMultiPurpose1, gb_read_hram(gb, hActiveEntityVisualPosY));
        label_D15(gb);
        return;
    }

    if (entity_type == ENTITY_PAIRODD_PROJECTILE) {
        func_003_6F93(gb);
        gb_write(gb, wEntitiesCollisionsTable + bc, 0xFF);
        return;
    }

    if (entity_type == ENTITY_SPIKED_BEETLE) {
        if (gb_read(gb, wEntitiesStateTable + bc) == 0x03) {
            func_003_6F5C(gb, bc);
            return;
        }
        gb_write(gb, wEntitiesStateTable + bc, 0x03);
        gb_write(gb, wEntitiesSpeedZTable + bc, 0x20);
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0xFF);
        uint8_t dir = (uint8_t)(gb_read_hram(gb, hLinkDirection) & 0x03);
        gb_write(gb, wEntitiesSpeedXTable + bc, Data_003_6F65[dir]);
        gb_write(gb, wEntitiesSpeedYTable + bc, Data_003_6F69[dir]);
        func_003_6F5C(gb, bc);
        return;
    }

    if (entity_type == ENTITY_STAR || entity_type == ENTITY_ANTI_FAIRY) {
        uint8_t dir = gb_read_hram(gb, hLinkDirection);
        if ((dir & DIRECTION_VERTICAL_MASK) != 0) {
            uint8_t sy = gb_read(gb, wEntitiesSpeedYTable + bc);
            gb_write(gb, wEntitiesSpeedYTable + bc, (uint8_t)(~sy + 1));
        } else {
            uint8_t sx = gb_read(gb, wEntitiesSpeedXTable + bc);
            gb_write(gb, wEntitiesSpeedXTable + bc, (uint8_t)(~sx + 1));
        }
        func_003_6F5C(gb, bc);
        return;
    }

    if (entity_type == ENTITY_FACADE) {
        func_003_6F93(gb);
        gb_write(gb, wEntitiesCollisionsTable + bc, 0xFF);
        return;
    }

    func_003_6F93(gb);
}

/* ===== EnemyCollidedWithSword (03:6FE8) ===== */
void EnemyCollidedWithSword(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* Ignore collisions between the flame shooter and the player sword */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_FLAME_SHOOTER) {
        return;
    }

    /* Special cases for Final Nightmare */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_FINAL_NIGHTMARE) {
        uint8_t form = gb_read(gb, wFinalNightmareForm);
        if (form == 0x00) {
            return;
        } else if (form == 0x01) {
            IncrementEntityState(gb, bc);
            gb_write(gb, wEntitiesStateTable + bc, 0x06);
            return;
        } else if (form == 0x02) {
            if (gb_read(gb, wIsUsingSpinAttack) != 0) {
                IncrementEntityState(gb, bc);
                return;
            }
            if (gb_read(gb, wC16A) < 0x04) {
                IncrementEntityState(gb, bc);
                return;
            }
            return;
        } else if (form == 0x03 || form == 0x04) {
            /* Falls through to defaultSwordCollision */
        } else {
            /* Form 5: standard sword collision */
            goto standardSwordCollision;
        }
    }

    /* Special case for Buzz Blob */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BUZZ_BLOB) {
        if (gb_read_hram(gb, hActiveEntityStatus) == ENTITY_STATUS_ACTIVE) {
            IncrementEntityState(gb, bc);
            gb_write(gb, wEntitiesStateTable + bc, 0x01);
            gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x40);
            gb_write(gb, wD464, 0x40);
            gb_write(gb, wSwordAnimationState, 0x00);
            gb_write(gb, wC16A, 0x00);
            gb_write(gb, wIsUsingSpinAttack, 0x00);
            gb_write_hram(gb, hNoiseSfx, NOISE_SFX_BUZZ_BLOB_ELECTROCUTE);
            ApplyLinkCollisionWithEnemy(gb, bc);
            return;
        }
    }

standardSwordCollision:;
    uint8_t entity_type = gb_read_hram(gb, hActiveEntityType);

    /* Special case for Bouncing Bombite */
    if (entity_type == ENTITY_BOUNCING_BOMBITE) {
        GetVectorTowardsLink_with_length(gb, 0x30, NULL, NULL);
        uint8_t recoil_y = (uint8_t)(~gb_read_hram(gb, hMultiPurpose0) + 1);
        gb_write(gb, wEntitiesSpeedYTable + bc, recoil_y);
        uint8_t recoil_x = (uint8_t)(~gb_read_hram(gb, hMultiPurpose1) + 1);
        gb_write(gb, wEntitiesSpeedXTable + bc, recoil_x);
        IncrementEntityState(gb, bc);
        gb_write(gb, wEntitiesStateTable + bc, 0x02);
        gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x40);
        gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x08);
        return;
    }

    /* Special case for Angler Fish */
    if (entity_type == ENTITY_ANGLER_FISH) {
        func_003_6DDF(gb, bc);
        gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x08);
    } else if (entity_type == ENTITY_SLIME_EYE) {
        if (gb_read_hram(gb, hMultiPurposeG) != 0) {
            func_003_6DDF(gb, bc);
            return;
        }
        if (gb_read(gb, wEntitiesPrivateState1Table + bc) != 0x04) {
            if (gb_read(gb, wIsRunningWithPegasusBoots) != 0) {
                func_003_6DDF(gb, bc);
            } else {
                gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x04);
                func_003_7565_with_length(gb, 0x10);
            }
            goto continueDefaultCollision;
        } else {
            if (gb_read(gb, wIsRunningWithPegasusBoots) == 0) {
                goto continueDefaultCollision;
            }
            gb_write(gb, wEntitiesPrivateCountdown2Table + bc, 0x0C);
            return;
        }
    }

    /* If sword clink is disabled... */
    if ((gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_SWORD_CLINK_OFF) != 0) {
        if (entity_type == ENTITY_KNIGHT) {
            uint8_t ps1 = gb_read(gb, wEntitiesPrivateState1Table + bc);
            gb_write(gb, wEntitiesPrivateState1Table + bc, (uint8_t)(~ps1 + 1));
            func_003_6F5C(gb, bc);
            gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0x0C);
            gb_write(gb, wC160, 0x01);
            gb_write(gb, wSwordCharge, 0x00);
            gb_write_hram(gb, hMultiPurpose0, gb_read_hram(gb, hActiveEntityPosX));
            gb_write_hram(gb, hMultiPurpose1, gb_read_hram(gb, hActiveEntityVisualPosY));
            label_D15(gb);
            return;
        }

        if (entity_type == ENTITY_GENIE) {
            uint8_t recoil = SWORD_RECOIL_GENIE_JAR_DEFAULT;
            if (gb_read(gb, wTunicType) == TUNIC_RED || gb_read(gb, wActivePowerUp) == ACTIVE_POWER_UP_PIECE_OF_POWER) {
                recoil = SWORD_RECOIL_GENIE_JAR_STRONGER;
            }
            ConfigureEntityRecoil(gb, bc, recoil);
            gb_write(gb, wEntitiesFlashCountdownTable + bc, 0x00);
        }

        gb_write(gb, wC1AC, (uint8_t)((bc & 0xFF) + 1));
        label_D07(gb);
        gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x10);
        gb_write(gb, wEntitiesRecoilVelocityX + bc, 0x00);
        gb_write(gb, wEntitiesRecoilVelocityY + bc, 0x00);
        func_003_6DDF(gb, bc);
        return;
    }

continueDefaultCollision:
    entity_type = gb_read_hram(gb, hActiveEntityType);

    if (entity_type == ENTITY_CUE_BALL) {
        ResetPegasusBoots(gb);
        goto jr_003_714D;
    }

    if (entity_type == ENTITY_IRON_MASK) {
        if (gb_read(gb, wEntitiesPrivateState2Table + bc) == 0) {
            uint8_t link_dir = (uint8_t)(gb_read_hram(gb, hLinkDirection) & 0x03);
            if (Data_003_6FE4[link_dir] == gb_read(gb, wEntitiesDirectionTable + bc)) {
                goto jr_003_714D;
            }
            ResetPegasusBoots(gb);
            gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x10);
            func_003_7565_with_length(gb, 0x10);
            ConfigureEntityRecoil(gb, bc, 0x10);
            gb_write_hram(gb, hMultiPurpose0, gb_read_hram(gb, hActiveEntityPosX));
            gb_write_hram(gb, hMultiPurpose1, gb_read_hram(gb, hActiveEntityVisualPosY));
            label_D15(gb);
            return;
        }
    }

    if (entity_type == ENTITY_ANTI_FAIRY) {
        return;
    }

jr_003_714D:
    gb_write(gb, wC160, 0x01);
    if (gb_read(gb, wC16A) == 0x05) {
        gb_write(gb, wC16D, 0x0C);
    }
    gb_write(gb, wSwordCharge, 0x00);
    ConfigureEntityRecoil(gb, bc, SWORD_RECOIL_DEFAULT);
    gb_write_hram(gb, hJingle, JINGLE_BUMP);

    if (gb_read(gb, wTunicType) == TUNIC_RED || gb_read(gb, wActivePowerUp) == ACTIVE_POWER_UP_PIECE_OF_POWER) {
        ApplySwordDamagesToEnemy(gb, bc);
        gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x20);
        gb_write(gb, wEntitiesPowerRecoilingTable + bc, 0x01);
        gb_write_hram(gb, hWaveSfx, WAVE_SFX_POWER_HIT);
        if (gb_read(gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DYING) {
            gb_write(gb, wEntitiesPrivateCountdown3Table + bc, 0x40);
        }
        return;
    }

    ApplySwordDamagesToEnemy(gb, bc);
}

/* ===== ApplySwordDamagesToEnemy (03:719D) ===== */
void ApplySwordDamagesToEnemy(GBState *gb, uint16_t bc) {
    if (!gb) return;

    gb_write(gb, wC1AC, (uint8_t)((bc & 0xFF) + 1));

    /* Check power-up, tunic, spin attack, or pegasus boots to boost attack damage type */
    uint8_t sword_level = gb_read(gb, wSwordLevel);
    uint8_t tunic_red = (uint8_t)(gb_read(gb, wTunicType) & TUNIC_RED);
    uint8_t piece_of_power = (uint8_t)(gb_read(gb, wActivePowerUp) & ACTIVE_POWER_UP_PIECE_OF_POWER);
    uint8_t spin_attack = gb_read(gb, wIsUsingSpinAttack);
    uint8_t pegasus = gb_read(gb, wIsRunningWithPegasusBoots);

    if ((tunic_red | piece_of_power | spin_attack | pegasus) != 0) {
        sword_level++;
    }

    uint8_t damage_type = (uint8_t)(sword_level - 1);
    gb_write(gb, wAttackDamageType, damage_type);

    label_003_71C0(gb, bc);
}

/* ===== label_003_71C0 (03:71C0) ===== */
void label_003_71C0(GBState *gb, uint16_t bc) {
    if (!gb) return;

    uint8_t damage_type = gb_read(gb, wAttackDamageType);
    uint8_t health_group = gb_read(gb, wEntitiesHealthGroup + bc);
    if (health_group >= 53) {
        health_group = 0;
    }

    uint8_t table_entry = Data_003_43EC[health_group * 16 + (damage_type & 0x0F)];
    uint8_t damage_index = (uint8_t)(((damage_type & 0x1F) << 3) + (table_entry & 0x07));
    uint8_t damage = Data_003_473C[damage_index];

    if (damage == 0) {
        return;
    }

    /* Special case: Final Nightmare form 4 */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_FINAL_NIGHTMARE) {
        if (gb_read(gb, wFinalNightmareForm) == 0x04) {
            func_003_6DDF(gb, bc);
            if (gb_read(gb, wIsRunningWithPegasusBoots) == 0 && gb_read(gb, wIsUsingSpinAttack) == 0) {
                return;
            }
        }
    }

    gb_write_hram(gb, hJingle, JINGLE_ENEMY_HIT);

    if ((gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_IS_BOSS) != 0) {
        gb_write_hram(gb, hWaveSfx, WAVE_SFX_BOSS_HURT);
    }

    if (gb_read(gb, wEntitiesTypeTable + bc) == ENTITY_CUCCO) {
        gb_write_hram(gb, hWaveSfx, WAVE_SFX_CUCCO_HURT);
    }

    /* Special effect damages: burn, stun, morph/fairy */
    if (damage >= 0xF0) {
        if (damage == 0xFE) {
            /* Burn */
            gb_write_hram(gb, hNoiseSfx, NOISE_SFX_BURSTING_FLAME);
            StartIgnoringHitsForEntity_idx(gb, bc);
            gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_BURNING);
            gb_write(gb, wEntitiesTransitionCountdownTable + bc, 0x60);
            uint8_t phys = gb_read(gb, wEntitiesPhysicsFlagsTable + bc);
            gb_write(gb, wEntitiesPhysicsFlagsTable + bc, (uint8_t)(phys + 2));
            uint8_t opt1 = gb_read(gb, wEntitiesOptions1Table + bc);
            gb_write(gb, wEntitiesOptions1Table + bc, (uint8_t)(opt1 & (ENTITY_OPT1_EXCLUDED_FROM_KILL_ALL | ENTITY_OPT1_SWORD_CLINK_OFF | ENTITY_OPT1_IS_BOSS)));
            return;
        }

        if (damage == 0xFF) {
            /* Stun */
            StartIgnoringHitsForEntity_idx(gb, bc);
            EntityBecomeStunned(gb, bc);
            return;
        }

        if (damage == 0xFD) {
            /* Morph / turn into fairy */
            uint8_t type = gb_read(gb, wEntitiesTypeTable + bc);
            if (type == ENTITY_GIANT_BUZZ_BLOB || type == ENTITY_BUZZ_BLOB) {
                if (gb_read(gb, wEntitiesPrivateState1Table + bc) != 0) {
                    return;
                }
                uint8_t ps1 = gb_read(gb, wEntitiesPrivateState1Table + bc);
                gb_write(gb, wEntitiesPrivateState1Table + bc, (uint8_t)(ps1 + 1));
            } else {
                gb_write(gb, wEntitiesTypeTable + bc, 0x2F);
                uint8_t prev_active = gb_read(gb, wActiveEntityIndex);
                gb_write(gb, wActiveEntityIndex, (uint8_t)(bc & 0xFF));
                ConfigureNewEntity(gb);
                gb_write(gb, wActiveEntityIndex, prev_active);
                gb_write(gb, wEntitiesSlowTransitionCountdownTable + bc, 0x80);
            }
            gb_write_hram(gb, hMultiPurpose0, gb_read(gb, wEntitiesPosXTable + bc));
            gb_write_hram(gb, hMultiPurpose1, (uint8_t)(gb_read(gb, wEntitiesPosYTable + bc) - gb_read(gb, wEntitiesPosZTable + bc)));
            AddTranscientVfx(gb, TRANSCIENT_VFX_POOF);
            return;
        }

        /* Unhandled special effect codes (0xF0-0xFC) return without doing damage */
        return;
    }

    /* Standard damage calculation */
    uint8_t health = gb_read(gb, wEntitiesHealthTable + bc);
    if (health > damage) {
        gb_write(gb, wEntitiesHealthTable + bc, (uint8_t)(health - damage));
    } else {
        /* Enemy died */
        gb_write(gb, wEntitiesHealthTable + bc, 0x00);
        gb_write(gb, wEntitiesStatusTable + bc, ENTITY_STATUS_DYING);

        uint8_t opt1 = gb_read(gb, wEntitiesOptions1Table + bc);
        if ((opt1 & ENTITY_OPT1_IS_BOSS) != 0) {
            if ((opt1 & ENTITY_OPT1_IS_MINI_BOSS) == 0) {
                bool another_boss_active = false;
                for (int e = 0x0F; e >= 0; e--) {
                    if (e == (int)(bc & 0xFF)) continue;
                    if (gb_read(gb, wEntitiesStatusTable + e) == ENTITY_STATUS_ACTIVE) {
                        if ((gb_read(gb, wEntitiesOptions1Table + e) & ENTITY_OPT1_IS_BOSS) != 0) {
                            another_boss_active = true;
                            break;
                        }
                    }
                }
                if (!another_boss_active) {
                    label_27F2(gb);
                }
            }

            gb_write(gb, wBossAgonySFXCountdown, 0x03);
            gb_write(gb, wEntitiesPrivateState2Table + bc, 0x00);

            uint8_t type = gb_read(gb, wEntitiesTypeTable + bc);
            if (type == ENTITY_FACADE) {
                OpenDialogInTable0(gb, Dialog0B7);
                gb_write(gb, wMusicTrackToPlay, MUSIC_BOSS_DEFEAT);
            } else if (type == ENTITY_EVIL_EAGLE) {
                uint8_t old_y = gb_read_hram(gb, hLinkPositionY);
                gb_write_hram(gb, hLinkPositionY, 0x10);
                OpenDialogInTable0(gb, Dialog0B9);
                gb_write_hram(gb, hLinkPositionY, old_y);
            }
        }

        IncrementEntityState(gb, bc);
        gb_write(gb, wEntitiesStateTable + bc, 0x00);
        gb_write(gb, wEntitiesPrivateCountdown3Table + bc, 0x2F);
        gb_write(gb, wEntitiesFlashCountdownTable + bc, 0x00);

        if ((gb_read(gb, wEntitiesOptions1Table + bc) & ENTITY_OPT1_IS_BOSS) == 0) {
            uint8_t phys = gb_read(gb, wEntitiesPhysicsFlagsTable + bc);
            gb_write(gb, wEntitiesPhysicsFlagsTable + bc, (uint8_t)((phys & 0xF0) | 0x04));
        }

        if (gb_read(gb, wEntitiesTypeTable + bc) == ENTITY_GHINI) {
            for (int e = 0x0F; e >= 0; e--) {
                if (e == (int)(bc & 0xFF)) continue;
                uint8_t other_type = gb_read(gb, wEntitiesTypeTable + e);
                if (other_type == ENTITY_HIDING_GHINI || other_type == ENTITY_GIANT_GHINI) {
                    if (gb_read(gb, wEntitiesStateTable + e) == 0 &&
                        gb_read(gb, wEntitiesStatusTable + e) != 0) {
                        gb_write(gb, wEntitiesStatusTable + e, ENTITY_STATUS_DYING);
                        gb_write(gb, wEntitiesPrivateCountdown3Table + e, 0x1F);
                        uint8_t rnd = (uint8_t)(GetRandomByte(gb) & 0x03);
                        gb_write(gb, wEntitiesDroppedItemTable + e, Data_003_73E7[rnd]);
                    }
                }
            }
            gb_write(gb, wEntitiesDroppedItemTable + bc, ENTITY_DROPPABLE_RUPEE);
        }
    }

    /* jr_003_73B6 */
    uint8_t ent_type = gb_read(gb, wEntitiesTypeTable + bc);
    if ((ent_type == ENTITY_FINAL_NIGHTMARE && gb_read(gb, wFinalNightmareForm) == 0x03) ||
        (ent_type == ENTITY_MOLDORM)) {
        gb_write(gb, wEntitiesFlashCountdownTable + bc, 0x28);
        gb_write(gb, wEntitiesPrivateCountdown2Table + bc, 0xC8);
        return;
    }

    gb_write(gb, wEntitiesFlashCountdownTable + bc, 0x18);
    StartIgnoringHitsForEntity_idx(gb, bc);
}

/* Tables for non-Blaino sword collision clink spark position offsets (03:74E4, 03:74E8) */
const uint8_t Data_003_74E4[4] = { 0x00, 0xF0, 0xF8, 0xFC };
const uint8_t Data_003_74E8[4] = { 0xFC, 0xFC, 0xF0, 0x00 };

/* Helper for Blaino powerful knockback / knockout (03:7571-03:7598) */
static void BlainoKnockoutPunch(GBState *gb, uint16_t bc) {
    /* ld hl, wEntitiesInertiaTable; add hl, bc; ld a, [hl]; cp $22; jr c, ret_003_7570 */
    if (gb_read(gb, wEntitiesInertiaTable + bc) < 0x22) {
        return;
    }

    /* ld a, LINK_MOTION_UNKNOWN_0A; ld [wLinkMotionState], a */
    gb_write(gb, wLinkMotionState, LINK_MOTION_UNKNOWN_0A);

    /* ld hl, wEntitiesDirectionTable; add hl, bc; ld a, [hl]; and a; ld a, $30; jr z, .jr_758B; ld a, $D0 */
    uint8_t dir = gb_read(gb, wEntitiesDirectionTable + bc);
    if (dir == 0) {
        gb_write_hram(gb, hLinkSpeedX, 0x30);
    } else {
        gb_write_hram(gb, hLinkSpeedX, 0xD0);
    }

    /* .jr_758B: ldh [hLinkSpeedX], a; xor a; ldh [hLinkSpeedY], a */
    gb_write_hram(gb, hLinkSpeedY, 0x00);

    /* ld a, $30; ldh [hLinkVelocityZ], a */
    gb_write_hram(gb, hLinkVelocityZ, 0x30);

    /* ld a, JINGLE_STRONG_BUMP; ldh [hJingle], a; ret */
    gb_write_hram(gb, hJingle, JINGLE_STRONG_BUMP);
}

/* ===== label_003_74EC (03:74EC-03:7598) - Enemy Body Collision Handler for Link ===== */
void label_003_74EC(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ldh a, [hFrameCounter]; xor c; rra; jr nc, ret_003_7570 */
    uint8_t frame = gb_read_hram(gb, hFrameCounter);
    uint8_t c = (uint8_t)(bc & 0xFF);
    if (((frame ^ c) & 0x01) == 0) {
        return;
    }

    /* ldh a, [hLinkPositionX]; add $08; ldh [hMultiPurpose0], a */
    uint8_t link_cx = (uint8_t)(gb_read_hram(gb, hLinkPositionX) + 0x08);
    gb_write_hram(gb, hMultiPurpose0, link_cx);

    /* ldh a, [hLinkPositionY]; add $08; ldh [hMultiPurpose2], a */
    uint8_t link_cy = (uint8_t)(gb_read_hram(gb, hLinkPositionY) + 0x08);
    gb_write_hram(gb, hMultiPurpose2, link_cy);

    /* ld de, hActiveEntityPosX; ld hl, wD5C0; ld a, [de]; add [hl] */
    /* push hl; ld hl, hMultiPurpose0; sub [hl]; cp $80; jr c, .jr_7511; cpl; inc a */
    uint8_t diff_x = (uint8_t)((uint8_t)(gb_read_hram(gb, hActiveEntityPosX) + gb_read(gb, wD5C0)) - link_cx);
    if (diff_x >= 0x80) {
        diff_x = (uint8_t)(~diff_x + 1);
    }

    /* pop hl; push af; inc hl; ld a, $04; add [hl]; ld e, a; pop af; cp e; jr nc, ret_003_7570 */
    uint8_t bound_x = (uint8_t)(0x04 + gb_read(gb, wD5C1));
    if (diff_x >= bound_x) {
        return;
    }

    /* inc hl; ld de, hActiveEntityVisualPosY; ld a, [de]; add [hl] */
    /* push hl; ld hl, hMultiPurpose2; sub [hl]; cp $80; jr c, .jr_752D; cpl; inc a */
    uint8_t diff_y = (uint8_t)((uint8_t)(gb_read_hram(gb, hActiveEntityVisualPosY) + gb_read(gb, wD5C2)) - link_cy);
    if (diff_y >= 0x80) {
        diff_y = (uint8_t)(~diff_y + 1);
    }

    /* pop hl; push af; inc hl; ld a, $05; add [hl]; ld e, a; pop af; cp e; jr nc, ret_003_7570 */
    uint8_t bound_y = (uint8_t)(0x05 + gb_read(gb, wD5C3));
    if (diff_y >= bound_y) {
        return;
    }

    /* ld a, [wInvincibilityCounter]; and a; jr nz, ret_003_7570 */
    if (gb_read(gb, wInvincibilityCounter) != 0) {
        return;
    }

    /* call ApplyLinkCollisionWithEnemy */
    ApplyLinkCollisionWithEnemy(gb, bc);

    /* ldh a, [hActiveEntityType]; cp ENTITY_BLAINO; jr nz, ret_003_7570 */
    if (gb_read_hram(gb, hActiveEntityType) != ENTITY_BLAINO) {
        return;
    }

    /* ld a, [wD205]; and a; jr z, ret_003_7570 */
    /* cp $01; jr z, ret_003_7570 */
    /* cp $04; jr z, ret_003_7570 */
    uint8_t d205 = gb_read(gb, wD205);
    if (d205 == 0x00 || d205 == 0x01 || d205 == 0x04) {
        return;
    }

    /* cp $02; jr nz, jr_003_7571 */
    if (d205 == 0x02) {
        /* call GetEntityPrivateCountdown1; ld [hl], $A0 */
        gb_write(gb, wEntitiesPrivateCountdown1Table + bc, 0xA0);
        /* ld a, $20; ld [wIgnoreLinkCollisionsCountdown], a */
        gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x20);
        /* ld a, $30; call func_003_7565 */
        func_003_7565_with_length(gb, 0x30);
        return;
    }

    /* jr_003_7571 */
    BlainoKnockoutPunch(gb, bc);
}

/* ===== func_003_73EB (03:73EB-03:74E0) - Enemy Collision Handler for Link ===== */
void func_003_73EB(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wC1AC; ld a, [wIgnoreLinkCollisionsCountdown]; or [hl] */
    /* ld hl, hLinkPunchedAwayCountdown; or [hl]; ld hl, wIsUsingSpinAttack; or [hl]; jp nz, label_003_74E1 */
    if ((gb_read(gb, wIgnoreLinkCollisionsCountdown) |
         gb_read(gb, wC1AC) |
         gb_read_hram(gb, hLinkPunchedAwayCountdown) |
         gb_read(gb, wIsUsingSpinAttack)) != 0) {
        label_003_74EC(gb, bc);
        return;
    }

    /* ld a, [wC140]; cp $00; jp z, label_003_74E1 */
    uint8_t c140 = gb_read(gb, wC140);
    if (c140 == 0) {
        label_003_74EC(gb, bc);
        return;
    }

    /* ld hl, wEntitiesDirectionTable; add hl, bc; ldh a, [hLinkDirection]; cp [hl]; jp z, label_003_74E1 */
    if (gb_read_hram(gb, hLinkDirection) == gb_read(gb, wEntitiesDirectionTable + bc)) {
        label_003_74EC(gb, bc);
        return;
    }

    /* ld de, hActiveEntityPosX; ld hl, wD5C0; ld a, [de]; add [hl] */
    /* push hl; ld hl, wC140; sub [hl]; cp $80; jr c, .jr_7422; cpl; inc a */
    uint8_t diff_x = (uint8_t)((uint8_t)(gb_read_hram(gb, hActiveEntityPosX) + gb_read(gb, wD5C0)) - c140);
    if (diff_x >= 0x80) {
        diff_x = (uint8_t)(~diff_x + 1);
    }

    /* pop hl; push af; inc hl; ld a, [wC141]; add [hl]; ld e, a; pop af; cp e; jp nc, label_003_74E1 */
    uint8_t bound_x = (uint8_t)(gb_read(gb, wC141) + gb_read(gb, wD5C1));
    if (diff_x >= bound_x) {
        label_003_74EC(gb, bc);
        return;
    }

    /* inc hl; ld de, hActiveEntityVisualPosY; ld a, [de]; add [hl] */
    /* push hl; ld hl, wC142; sub [hl]; cp $80; jr c, .jr_7440; cpl; inc a */
    uint8_t diff_y = (uint8_t)((uint8_t)(gb_read_hram(gb, hActiveEntityVisualPosY) + gb_read(gb, wD5C2)) - gb_read(gb, wC142));
    if (diff_y >= 0x80) {
        diff_y = (uint8_t)(~diff_y + 1);
    }

    /* pop hl; push af; inc hl; ld a, [wC143]; add [hl]; ld e, a; pop af; cp e; jp nc, label_003_74E1 */
    uint8_t bound_y = (uint8_t)(gb_read(gb, wC143) + gb_read(gb, wD5C3));
    if (diff_y >= bound_y) {
        label_003_74EC(gb, bc);
        return;
    }

    /* call ResetPegasusBoots */
    ResetPegasusBoots(gb);

    /* ld a, $08; ld [wIgnoreLinkCollisionsCountdown], a */
    gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x08);

    /* ld a, $12; call func_003_7565 */
    func_003_7565_with_length(gb, 0x12);

    /* ld a, $18; call GetVectorTowardsLink */
    GetVectorTowardsLink_with_length(gb, 0x18, NULL, NULL);

    /* ldh a, [hMultiPurpose0]; cpl; inc a; ld hl, wEntitiesRecoilVelocityY; add hl, bc; ld [hl], a */
    uint8_t recoil_y = (uint8_t)(~gb_read_hram(gb, hMultiPurpose0) + 1);
    gb_write(gb, wEntitiesRecoilVelocityY + bc, recoil_y);

    /* ldh a, [hMultiPurpose1]; cpl; inc a; ld hl, wEntitiesRecoilVelocityX; add hl, bc; ld [hl], a */
    uint8_t recoil_x = (uint8_t)(~gb_read_hram(gb, hMultiPurpose1) + 1);
    gb_write(gb, wEntitiesRecoilVelocityX + bc, recoil_x);

    /* call StartIgnoringHitsForEntity; ld [hl], $08 */
    StartIgnoringHitsForEntity_idx(gb, bc);
    gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x08);

    /* xor a; ld [wSwordCharge], a */
    gb_write(gb, wSwordCharge, 0x00);

    /* call AlertSwordMoblins */
    AlertSwordMoblins(gb);

    /* ld hl, wIsUsingSpinAttack; ld a, [wC16A]; or [hl]; jr z, .jr_748B */
    if ((gb_read(gb, wIsUsingSpinAttack) | gb_read(gb, wC16A)) != 0) {
        /* ld a, $0C; ld [wC16D], a */
        gb_write(gb, wC16D, 0x0C);
    }

    /* ldh a, [hActiveEntityType]; cp ENTITY_BLAINO; jr nz, jr_003_74C1 */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BLAINO) {
        /* ld a, JINGLE_BUMP; ldh [hJingle], a */
        gb_write_hram(gb, hJingle, JINGLE_BUMP);

        uint8_t d205 = gb_read(gb, wD205);
        if (d205 == 0x00) {
            /* jr_003_74BF -> jr_003_74DC */
        } else if (d205 == 0x01 || d205 == 0x04) {
            /* .jr_74B5: ld a, $10; ld [wIgnoreLinkCollisionsCountdown], a; ld a, $20; call func_003_7565 */
            gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x10);
            func_003_7565_with_length(gb, 0x20);
        } else if (d205 == 0x03) {
            /* jp z, jr_003_7571 */
            BlainoKnockoutPunch(gb, bc);
            return;
        } else {
            /* ld a, $20; ld [wIgnoreLinkCollisionsCountdown], a; ld a, $20; call func_003_7565 */
            gb_write(gb, wIgnoreLinkCollisionsCountdown, 0x20);
            func_003_7565_with_length(gb, 0x20);
        }

        /* jr_003_74DC: ld a, $0C; ldh [hLinkPunchedAwayCountdown], a; ret */
        gb_write_hram(gb, hLinkPunchedAwayCountdown, 0x0C);
        return;
    }

    /* jr_003_74C1: ldh a, [hLinkDirection]; ld e, a; ld d, b */
    uint8_t link_dir = (uint8_t)(gb_read_hram(gb, hLinkDirection) & 0x03);
    /* ld hl, Data_003_74E4; add hl, de; ld a, [wC140]; add [hl]; ldh [hMultiPurpose0], a */
    gb_write_hram(gb, hMultiPurpose0, (uint8_t)(gb_read(gb, wC140) + Data_003_74E4[link_dir]));
    /* ld hl, Data_003_74E8; add hl, de; ld a, [wC142]; add [hl]; ldh [hMultiPurpose1], a */
    gb_write_hram(gb, hMultiPurpose1, (uint8_t)(gb_read(gb, wC142) + Data_003_74E8[link_dir]));
    /* call label_D15 */
    label_D15(gb);

    /* jr_003_74DC: ld a, $0C; ldh [hLinkPunchedAwayCountdown], a; ret */
    gb_write_hram(gb, hLinkPunchedAwayCountdown, 0x0C);
}


/* ===== StartIgnoringHitsForEntity (03:73DB) ===== */
void StartIgnoringHitsForEntity_idx(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld hl, wEntitiesPowerRecoilingTable; add hl, bc; ld [hl], b */
    gb_write(gb, wEntitiesPowerRecoilingTable + bc, 0x00);

    /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, bc; ld [hl], $0A */
    gb_write(gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x0A);
}

void StartIgnoringHitsForEntity(GBState *gb) {
    if (!gb) return;
    uint16_t bc = gb_read(gb, wActiveEntityIndex);
    StartIgnoringHitsForEntity_idx(gb, bc);
}

/* Helper for spawning entity into last available slot (03:64CA) */
static uint16_t SpawnNewEntity_internal(GBState *gb, uint8_t entity_type, uint16_t bc) {
    for (int8_t e = 15; e >= 0; e--) {
        if (gb_read(gb, (uint16_t)(wEntitiesStatusTable + e)) == 0) {
            gb_write(gb, (uint16_t)(wEntitiesStatusTable + e), ENTITY_STATUS_ACTIVE);
            gb_write(gb, (uint16_t)(wEntitiesTypeTable + e), entity_type);
            gb_write_hram(gb, hMultiPurpose0, gb_read(gb, (uint16_t)(wEntitiesPosXTable + bc)));
            gb_write_hram(gb, hMultiPurpose1, gb_read(gb, (uint16_t)(wEntitiesPosYTable + bc)));
            gb_write_hram(gb, hMultiPurpose2, gb_read(gb, (uint16_t)(wEntitiesDirectionTable + bc)));
            gb_write_hram(gb, hMultiPurpose3, gb_read(gb, (uint16_t)(wEntitiesPosZTable + bc)));
            gb_write(gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + e), 0x01);
            return (uint16_t)e;
        }
    }
    return 0xFFFF;
}

/* ===== func_003_75A2 (03:75A2) ===== */
void func_003_75A2(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld e, $0F; ld d, $00 */
    for (int8_t e = 0x0F; e >= 0; e--) {
        uint16_t de = (uint16_t)e;

        /* ld a, e; cp c; jp z, checkNextEntity */
        if (de == (bc & 0xFF)) {
            continue;
        }

        /* ldh a, [hFrameCounter]; xor e; and $01; jp nz, checkNextEntity */
        if (((gb_read_hram(gb, hFrameCounter) ^ (uint8_t)e) & 0x01) != 0) {
            continue;
        }

        /* ld hl, wEntitiesStatusTable; add hl, de; ld a, [hl]; cp ENTITY_STATUS_ACTIVE; jp c, checkNextEntity */
        if (gb_read(gb, (uint16_t)(wEntitiesStatusTable + de)) < ENTITY_STATUS_ACTIVE) {
            continue;
        }

        /* ld hl, wEntitiesPhysicsFlagsTable; add hl, de; ld a, [hl]; and ENTITY_PHYSICS_PROJECTILE_NOCLIP; jp nz, checkNextEntity */
        if ((gb_read(gb, (uint16_t)(wEntitiesPhysicsFlagsTable + de)) & ENTITY_PHYSICS_PROJECTILE_NOCLIP) != 0) {
            continue;
        }

        /* ld hl, wEntitiesPosXTable; add hl, de; ldh a, [hActiveEntityPosX]; sub [hl]; add $0C; cp $18; jp nc, checkNextEntity */
        uint8_t dx = (uint8_t)(gb_read_hram(gb, hActiveEntityPosX) - gb_read(gb, (uint16_t)(wEntitiesPosXTable + de)) + 0x0C);
        if (dx >= 0x18) {
            continue;
        }

        /* ld hl, wEntitiesPosYTable; add hl, de; ld a, [hl] */
        /* ld hl, wEntitiesPosZTable; add hl, de; sub [hl] */
        /* ld hl, hActiveEntityVisualPosY; sub [hl]; add $0C; cp $18; jp nc, checkNextEntity */
        uint8_t target_visual_y = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesPosYTable + de)) - gb_read(gb, (uint16_t)(wEntitiesPosZTable + de)));
        uint8_t dy = (uint8_t)(target_visual_y - gb_read_hram(gb, hActiveEntityVisualPosY) + 0x0C);
        if (dy >= 0x18) {
            continue;
        }

        /* ld hl, wEntitiesSpriteVariantTable; add hl, de; ld a, [hl]; cp $FF; jp z, checkNextEntity */
        if (gb_read(gb, (uint16_t)(wEntitiesSpriteVariantTable + de)) == 0xFF) {
            continue;
        }

        /* ldh a, [hActiveEntityType]; cp ENTITY_BOUNCING_BOMBITE; jr nz, .selfBombiteEnd */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BOUNCING_BOMBITE) {
            /* call GetEntityTransitionCountdown; ld [hl], b */
            gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0x00);
        }

        /* ld hl, wEntitiesTypeTable; add hl, de; ld a, [hl]; cp ENTITY_BOUNCING_BOMBITE; jr nz, .bombiteEnd */
        if (gb_read(gb, (uint16_t)(wEntitiesTypeTable + de)) == ENTITY_BOUNCING_BOMBITE) {
            gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + de), gb_read(gb, (uint16_t)(wEntitiesSpeedXTable + bc)));
            gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + de), gb_read(gb, (uint16_t)(wEntitiesSpeedYTable + bc)));
            gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + de), 0x40);
            gb_write(gb, (uint16_t)(wEntitiesStateTable + de), 0x02);
            gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown1Table + de), 0x08);
            continue;
        }

        /* ld hl, wEntitiesPhysicsFlagsTable; add hl, de; ld a, [hl]; and ENTITY_PHYSICS_GRABBABLE; jp nz, label_003_7715 */
        if ((gb_read(gb, (uint16_t)(wEntitiesPhysicsFlagsTable + de)) & ENTITY_PHYSICS_GRABBABLE) != 0) {
            goto label_003_7715;
        }

        /* ldh a, [hActiveEntityType]; cp ENTITY_MAGIC_POWDER_SPRINKLE; jr z, forceCollisionEnd */
        if (gb_read_hram(gb, hActiveEntityType) != ENTITY_MAGIC_POWDER_SPRINKLE) {
            /* ld hl, wEntitiesTypeTable; add hl, de; ld a, [hl]; cp ENTITY_FINAL_NIGHTMARE; jr nz, .finalNightmareEnd */
            if (gb_read(gb, (uint16_t)(wEntitiesTypeTable + de)) == ENTITY_FINAL_NIGHTMARE &&
                gb_read(gb, wFinalNightmareForm) == 0x05 &&
                gb_read_hram(gb, hActiveEntitySpriteVariant) != 0x02) {
                goto forceCollision;
            }

            /* ld hl, wEntitiesHitboxFlagsTable; add hl, de; ld a, [hl]; and $80; jr z, forceCollisionEnd */
            if ((gb_read(gb, (uint16_t)(wEntitiesHitboxFlagsTable + de)) & 0x80) != 0) {
                goto forceCollision;
            }
        }
        goto forceCollisionEnd;

forceCollision:
        /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld [hl], $01; jp jr_003_7737 */
        gb_write(gb, (uint16_t)(wEntitiesCollisionsTable + bc), 0x01);
        goto jr_003_7737;

forceCollisionEnd:
        /* ldh a, [hActiveEntityType]; cp ENTITY_MAGIC_POWDER_SPRINKLE; jr nz, jr_003_76AC */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_MAGIC_POWDER_SPRINKLE) {
            uint8_t target_type = gb_read(gb, (uint16_t)(wEntitiesTypeTable + de));
            if (target_type == ENTITY_MAD_BATTER) {
                if (gb_read(gb, (uint16_t)(wEntitiesStateTable + de)) == 0) {
                    uint8_t state = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesStateTable + de)) + 1);
                    gb_write(gb, (uint16_t)(wEntitiesStateTable + de), state);
                } else {
                    goto jr_003_76AC;
                }
            } else if (target_type == ENTITY_TARIN) {
                if (gb_read(gb, wIsIndoor) == 0 &&
                    gb_read(gb, (uint16_t)(wEntitiesStateTable + de)) == 0) {
                    uint8_t state = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesStateTable + de)) + 1);
                    gb_write(gb, (uint16_t)(wEntitiesStateTable + de), state);
                    gb_write(gb, (uint16_t)(wEntitiesSlowTransitionCountdownTable + de), 0x7F);
                    gb_write(gb, (uint16_t)(wEntitiesFlashCountdownTable + de), 0x10);
                    gb_write(gb, wCurrentBank, 0x03);
                    label_27F2(gb);
                    gb_write(gb, wCurrentBank, 0x18);
                } else {
                    goto jr_003_76AC;
                }
            } else {
                goto jr_003_76AC;
            }
        }

jr_003_76AC:
        /* ld hl, wEntitiesHitboxFlagsTable; add hl, de; ld a, [hl]; and $80; jp nz, checkNextEntity */
        if ((gb_read(gb, (uint16_t)(wEntitiesHitboxFlagsTable + de)) & 0x80) != 0) {
            continue;
        }

        /* ld hl, wEntitiesIgnoreHitsCountdownTable; add hl, de; ld a, [hl]; and a; jp nz, checkNextEntity */
        if (gb_read(gb, (uint16_t)(wEntitiesIgnoreHitsCountdownTable + de)) != 0) {
            continue;
        }

        /* ld hl, wEntitiesTypeTable; add hl, de; ld a, [hl]; cp ENTITY_IRON_MASK; jr nz, jr_003_7710 */
        if (gb_read(gb, (uint16_t)(wEntitiesTypeTable + de)) == ENTITY_IRON_MASK) {
            /* ld hl, wEntitiesDirectionTable; add hl, de; ld a, [hl]; xor $01 */
            /* ld hl, wEntitiesDirectionTable; add hl, bc; cp [hl]; jr nz, jr_003_7710 */
            uint8_t dir_de_opposite = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesDirectionTable + de)) ^ 0x01);
            if (dir_de_opposite == gb_read(gb, (uint16_t)(wEntitiesDirectionTable + bc))) {
                /* ld hl, wEntitiesPrivateState2Table; add hl, de; ld a, [hl]; and a; jr nz, jr_003_7710 */
                if (gb_read(gb, (uint16_t)(wEntitiesPrivateState2Table + de)) == 0) {
                    /* ldh a, [hActiveEntityType]; cp ENTITY_HOOKSHOT_CHAIN; jp nz, forceCollision */
                    if (gb_read_hram(gb, hActiveEntityType) != ENTITY_HOOKSHOT_CHAIN) {
                        goto forceCollision;
                    }

                    /* ld [hl], $01; push de */
                    gb_write(gb, (uint16_t)(wEntitiesPrivateState2Table + de), 0x01);

                    /* ld a, ENTITY_IRON_MASKS_MASK; call SpawnNewEntity; jr c, .jr_770D */
                    uint16_t new_slot = SpawnNewEntity_internal(gb, ENTITY_IRON_MASKS_MASK, bc);
                    if (new_slot != 0xFFFF) {
                        /* ldh a, [hMultiPurpose0]; ld hl, wEntitiesPosXTable; add hl, de; ld [hl], a */
                        gb_write(gb, (uint16_t)(wEntitiesPosXTable + new_slot), gb_read_hram(gb, hMultiPurpose0));
                        /* ldh a, [hMultiPurpose1]; ld hl, wEntitiesPosYTable; add hl, de; ld [hl], a */
                        gb_write(gb, (uint16_t)(wEntitiesPosYTable + new_slot), gb_read_hram(gb, hMultiPurpose1));
                        /* ld hl, wEntitiesPrivateState5Table; add hl, de; ld a, c; inc a; ld [hl], a */
                        gb_write(gb, (uint16_t)(wEntitiesPrivateState5Table + new_slot), (uint8_t)((bc & 0xFF) + 1));
                        /* ldh a, [hMultiPurpose2]; and $01; ld hl, wEntitiesSpriteVariantTable; add hl, de; ld [hl], a */
                        gb_write(gb, (uint16_t)(wEntitiesSpriteVariantTable + new_slot), (uint8_t)(gb_read_hram(gb, hMultiPurpose2) & 0x01));
                    }
                    /* .jr_770D: pop de; jr jr_003_7737 */
                    goto jr_003_7737;
                }
            }
        }

        /* jr_003_7710: call func_003_77A7; jr jr_003_7737 */
        func_003_77A7(gb, bc, de);
        goto jr_003_7737;

label_003_7715:
        /* ldh a, [hActiveEntityType]; cp ENTITY_BOOMERANG; jr z, .jr_771F */
        /* cp ENTITY_HOOKSHOT_CHAIN; jr nz, jr_003_7734 */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BOOMERANG ||
            gb_read_hram(gb, hActiveEntityType) == ENTITY_HOOKSHOT_CHAIN) {
            /* .jr_771F: call GetEntityTransitionCountdown; xor a; ld [hl], a */
            gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0x00);

            /* ld hl, wEntitiesPhysicsFlagsTable; add hl, de; ld a, [hl]; and ENTITY_PHYSICS_GRABBABLE; jr z, jr_003_7737 */
            if ((gb_read(gb, (uint16_t)(wEntitiesPhysicsFlagsTable + de)) & ENTITY_PHYSICS_GRABBABLE) == 0) {
                goto jr_003_7737;
            }

            /* ld a, c; inc a; ld hl, wEntitiesPrivateState5Table; add hl, de; ld [hl], a */
            gb_write(gb, (uint16_t)(wEntitiesPrivateState5Table + de), (uint8_t)((bc & 0xFF) + 1));
        }
        /* jr_003_7734: jp checkNextEntity */
        continue;

jr_003_7737:
        /* ldh a, [hActiveEntityType]; cp ENTITY_WRECKING_BALL; jr z, jr_003_775A */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_WRECKING_BALL) {
            goto jr_003_775A;
        }

        /* cp ENTITY_BOOMERANG; jr z, jr_003_779A */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_BOOMERANG) {
            goto jr_003_779A;
        }

        /* cp ENTITY_HOOKSHOT_CHAIN; jr z, jr_003_779A */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_HOOKSHOT_CHAIN) {
            goto jr_003_779A;
        }

        /* cp ENTITY_LIFTABLE_ROCK; jr nz, .jr_7751 */
        if (gb_read_hram(gb, hActiveEntityType) == ENTITY_LIFTABLE_ROCK) {
            LiftableRockStartSmashingAnimation(gb, bc);
            continue;
        }

        /* .jr_7751: ld hl, wEntitiesStatusTable; add hl, bc; ld a, [hl]; cp $08; jr nz, jr_003_7782 */
        if (gb_read(gb, (uint16_t)(wEntitiesStatusTable + bc)) == ENTITY_STATUS_THROWN) {
            goto jr_003_775A;
        }
        goto jr_003_7782;

jr_003_775A:
        /* ld hl, wEntitiesPrivateCountdown3Table; add hl, bc; ld a, [hl]; and a; jr nz, checkNextEntity */
        if (gb_read(gb, (uint16_t)(wEntitiesPrivateCountdown3Table + bc)) != 0) {
            continue;
        }

        /* ld [hl], $0C */
        gb_write(gb, (uint16_t)(wEntitiesPrivateCountdown3Table + bc), 0x0C);

        /* ld hl, wEntitiesSpeedXTable; add hl, bc; sra [hl]; sra [hl]; ld a, [hl]; cpl; ld [hl], a */
        int8_t speed_x = (int8_t)gb_read(gb, (uint16_t)(wEntitiesSpeedXTable + bc));
        speed_x = (int8_t)(speed_x >> 2);
        gb_write(gb, (uint16_t)(wEntitiesSpeedXTable + bc), (uint8_t)(~((uint8_t)speed_x)));

        /* ld hl, wEntitiesSpeedYTable; add hl, bc; sra [hl]; sra [hl]; ld a, [hl]; cpl; ld [hl], a */
        int8_t speed_y = (int8_t)gb_read(gb, (uint16_t)(wEntitiesSpeedYTable + bc));
        speed_y = (int8_t)(speed_y >> 2);
        gb_write(gb, (uint16_t)(wEntitiesSpeedYTable + bc), (uint8_t)(~((uint8_t)speed_y)));

        /* ld hl, wEntitiesThrownDirectionTable; add hl, bc; ld [hl], $FF; jr jr_003_779A */
        gb_write(gb, (uint16_t)(wEntitiesThrownDirectionTable + bc), 0xFF);
        goto jr_003_779A;

jr_003_7782:
        /* ld hl, wEntitiesCollisionsTable; add hl, bc; ld a, [hl]; and a; jr nz, jr_003_779A */
        if (gb_read(gb, (uint16_t)(wEntitiesCollisionsTable + bc)) == 0) {
            /* ldh a, [hActiveEntityType]; cp ENTITY_ARROW; jr nz, .jr_7795 */
            /* ldh a, [hActiveEntityState]; and a; jr nz, jr_003_7798 */
            if (gb_read_hram(gb, hActiveEntityType) == ENTITY_ARROW &&
                gb_read_hram(gb, hActiveEntityState) != 0) {
                /* jr_003_7798: jr checkNextEntity */
                continue;
            }

            /* .jr_7795: call UnloadEntity */
            UnloadEntity(gb, bc);
            continue;
        }

jr_003_779A:
        /* call GetEntityTransitionCountdown; xor a; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0x00);
    }
}

/* ===== func_003_77A7 (03:77A7) ===== */
void func_003_77A7(GBState *gb, uint16_t bc, uint16_t de) {
    if (!gb) return;

    /* ldh a, [hActiveEntityType]; cp ENTITY_ARROW; jr nz, .jr_77B8 */
    if (gb_read_hram(gb, hActiveEntityType) == ENTITY_ARROW &&
        gb_read_hram(gb, hActiveEntityState) != 0) {
        /* ldh a, [hActiveEntityState]; and a; jr z, .jr_77B8 */
        /* call GetEntityTransitionCountdown; ld [hl], $03; ret */
        gb_write(gb, (uint16_t)(wEntitiesTransitionCountdownTable + bc), 0x03);
        return;
    }

    /* .jr_77B8: */
    /* ld hl, wEntitiesSpeedXTable; add hl, bc; ld a, [hl] */
    /* ld hl, wEntitiesRecoilVelocityX; add hl, de; ld [hl], a */
    uint8_t speed_x = gb_read(gb, (uint16_t)(wEntitiesSpeedXTable + bc));
    gb_write(gb, (uint16_t)(wEntitiesRecoilVelocityX + de), speed_x);

    /* ld hl, wEntitiesSpeedYTable; add hl, bc; ld a, [hl] */
    /* ld hl, wEntitiesRecoilVelocityY; add hl, de; ld [hl], a */
    uint8_t speed_y = gb_read(gb, (uint16_t)(wEntitiesSpeedYTable + bc));
    gb_write(gb, (uint16_t)(wEntitiesRecoilVelocityY + de), speed_y);

    /* push bc; ld c, e; ld b, d; push de; call func_003_77D6; pop de; pop bc; ret */
    func_003_77D6(gb, de);
}

/* ===== func_003_77D6 (03:77D6) ===== */
void func_003_77D6(GBState *gb, uint16_t bc) {
    label_003_71C0(gb, bc);
}

/* ===== CheckExplosionInteractionWithEntities (03:77D9) ===== */
void CheckExplosionInteractionWithEntities(GBState *gb, uint16_t bc) {
    if (!gb) return;

    /* ld e, $0F; ld d, $00 */
    for (int8_t e = 0x0F; e >= 0; e--) {
        uint16_t de = (uint16_t)e;

        /* ld hl, wEntitiesStatusTable; add hl, de; ld a, [hl]; cp ENTITY_STATUS_ACTIVE; jr c, .noDamage */
        if (gb_read(gb, (uint16_t)(wEntitiesStatusTable + de)) < ENTITY_STATUS_ACTIVE) {
            continue;
        }

        /* ld hl, wEntitiesPhysicsFlagsTable; add hl, de; ld a, [hl] */
        /* and ENTITY_PHYSICS_PROJECTILE_NOCLIP | ENTITY_PHYSICS_GRABBABLE; jr nz, .noDamage */
        if ((gb_read(gb, (uint16_t)(wEntitiesPhysicsFlagsTable + de)) &
             (ENTITY_PHYSICS_PROJECTILE_NOCLIP | ENTITY_PHYSICS_GRABBABLE)) != 0) {
            continue;
        }

        /* ld hl, wEntitiesHitboxFlagsTable; add hl, de; ld a, [hl]; and HITFLAGS_IGNORE_HITS; jr nz, .noDamage */
        if ((gb_read(gb, (uint16_t)(wEntitiesHitboxFlagsTable + de)) & HITFLAGS_IGNORE_HITS) != 0) {
            continue;
        }

        /* ld hl, wEntitiesPosXTable; add hl, de; ldh a, [hActiveEntityPosX]; sub [hl]; add $18; cp $30; jr nc, .noDamage */
        uint8_t dx = (uint8_t)(gb_read_hram(gb, hActiveEntityPosX) - gb_read(gb, (uint16_t)(wEntitiesPosXTable + de)) + 0x18);
        if (dx >= 0x30) {
            continue;
        }

        /* ld hl, wEntitiesPosYTable; add hl, de; ld a, [hl] */
        /* ld hl, wEntitiesPosZTable; add hl, de; sub [hl] */
        /* ld hl, hActiveEntityVisualPosY; sub [hl]; add $18; cp $30; jr nc, .noDamage */
        uint8_t visual_y_de = (uint8_t)(gb_read(gb, (uint16_t)(wEntitiesPosYTable + de)) - gb_read(gb, (uint16_t)(wEntitiesPosZTable + de)));
        uint8_t dy = (uint8_t)(visual_y_de - gb_read_hram(gb, hActiveEntityVisualPosY) + 0x18);
        if (dy >= 0x30) {
            continue;
        }

        /* ld a, DAMAGE_TYPE_BOMB; ld [wAttackDamageType], a */
        gb_write(gb, wAttackDamageType, DAMAGE_TYPE_BOMB);

        /* call func_003_77A7 */
        func_003_77A7(gb, bc, de);

        /* ld a, $30; call GetVectorTowardsOtherEntity */
        GetVectorTowardsOtherEntity(gb, 0x30, de);

        /* ld hl, wEntitiesRecoilVelocityY; add hl, de; ldh a, [hMultiPurpose0]; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesRecoilVelocityY + de), gb_read_hram(gb, hMultiPurpose0));

        /* ld hl, wEntitiesRecoilVelocityX; add hl, de; ldh a, [hMultiPurpose1]; ld [hl], a */
        gb_write(gb, (uint16_t)(wEntitiesRecoilVelocityX + de), gb_read_hram(gb, hMultiPurpose1));
    }
}

/* ===== GetVectorTowardsOtherEntity (03:783B) ===== */
void GetVectorTowardsOtherEntity(GBState *gb, uint8_t length, uint16_t de) {
    if (!gb) return;

    /* ldh [hMultiPurpose0], a */
    /* ldh a, [hLinkPositionX]; push af */
    uint8_t saved_link_x = gb_read_hram(gb, hLinkPositionX);

    /* ld hl, wEntitiesPosXTable; add hl, de; ld a, [hl]; ldh [hLinkPositionX], a */
    gb_write_hram(gb, hLinkPositionX, gb_read(gb, (uint16_t)(wEntitiesPosXTable + de)));

    /* ldh a, [hLinkPositionY]; push af */
    uint8_t saved_link_y = gb_read_hram(gb, hLinkPositionY);

    /* ld hl, wEntitiesPosYTable; add hl, de; ld a, [hl]; ldh [hLinkPositionY], a */
    gb_write_hram(gb, hLinkPositionY, gb_read(gb, (uint16_t)(wEntitiesPosYTable + de)));

    /* push de; ldh a, [hMultiPurpose0]; call GetVectorTowardsLink; pop de */
    GetVectorTowardsLink_with_length(gb, length, NULL, NULL);

    /* pop af; ldh [hLinkPositionY], a */
    gb_write_hram(gb, hLinkPositionY, saved_link_y);

    /* pop af; ldh [hLinkPositionX], a; ret */
    gb_write_hram(gb, hLinkPositionX, saved_link_x);
}
