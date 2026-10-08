#ifndef LADX_HOME_ENTITIES_H
#define LADX_HOME_ENTITIES_H

#include "gb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IsZero (00:0C08)
 * Tests if the byte at (hl + bc) is equal to zero.
 * Returns the byte read from memory at (hl + bc).
 */
uint8_t IsZero(GBState *gb, uint16_t hl, uint16_t bc);

/**
 * GetEntitySlowTransitionCountdown (00:0BFB)
 * Retrieves the slow transition countdown for the entity at index bc.
 * Lookups value in wEntitiesSlowTransitionCountdownTable (0xC450).
 */
uint8_t GetEntitySlowTransitionCountdown(GBState *gb, uint16_t entity_index);

/**
 * GetEntityPrivateCountdown1 (00:0C00)
 * Retrieves private countdown 1 for the entity at index bc.
 * Lookups value in wEntitiesPrivateCountdown1Table (0xC2F0).
 */
uint8_t GetEntityPrivateCountdown1(GBState *gb, uint16_t entity_index);

/**
 * GetEntityTransitionCountdown (00:0C05)
 * Retrieves the transition countdown for the entity at index bc.
 * Lookups value in wEntitiesTransitionCountdownTable (0xC2E0).
 */
uint8_t GetEntityTransitionCountdown(GBState *gb, uint16_t entity_index);

/**
 * DecrementEntityIgnoreHitsCountdown (00:0C56)
 * Decrements the ignore-hits countdown for entity at index bc if it is > 0.
 * Operates on wEntitiesIgnoreHitsCountdownTable (0xC410).
 */
void DecrementEntityIgnoreHitsCountdown(GBState *gb, uint16_t entity_index);

/**
 * CreateTradingItemEntity (00:0C0C)
 * Spawns a temporary entity with ENTITY_TRADING_ITEM (0xAF) at Link's current position.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param spawn_func Callback to spawn entity (e.g. SpawnNewEntity_trampoline)
 */
void CreateTradingItemEntity(GBState *gb, uint16_t (*spawn_func)(GBState *, uint8_t entity_type));

/**
 * SpawnNewEntity_trampoline (00:3B86)
 * Switches to BANK(SpawnNewEntity) ($03), calls SpawnNewEntity, and restores saved bank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_type Type ID of entity to spawn
 * @param spawn_new_entity Target callback
 * @return Entity slot offset (de)
 */
uint16_t SpawnNewEntity_trampoline(GBState *gb, uint8_t entity_type, uint16_t (*spawn_new_entity)(GBState *, uint8_t));

/**
 * SpawnNewEntityInRange_trampoline (00:3B98)
 * Switches to BANK(SpawnNewEntityInRange) ($03), calls SpawnNewEntityInRange, and restores saved bank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_type Type ID of entity to spawn
 * @param spawn_in_range Target callback
 * @return Entity slot offset (de)
 */
uint16_t SpawnNewEntityInRange_trampoline(GBState *gb, uint8_t entity_type, uint16_t (*spawn_in_range)(GBState *, uint8_t));

/**
 * AnimateEntitiesAndRestoreBank (00:0EF4)
 * Calls AnimateEntities, then switches to bank given in argument via SwitchBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param bank Bank number to restore/switch to
 * @param animate_entities Callback to AnimateEntities handler
 */
void AnimateEntitiesAndRestoreBank(GBState *gb, uint8_t bank, void (*animate_entities)(GBState *));

/**
 * AnimateEntitiesAndRestoreBank17 (00:0EED)
 * Selects bank $03 in rSelectROMBank, then calls AnimateEntitiesAndRestoreBank with bank $17.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param animate_entities Callback to AnimateEntities handler
 */
void AnimateEntitiesAndRestoreBank17(GBState *gb, void (*animate_entities)(GBState *));

/**
 * AnimateEntitiesAndRestoreBank01 (00:0EFC)
 * Selects bank $03 in rSelectROMBank, then calls AnimateEntitiesAndRestoreBank with bank $01.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param animate_entities Callback to AnimateEntities handler
 */
void AnimateEntitiesAndRestoreBank01(GBState *gb, void (*animate_entities)(GBState *));

/**
 * AnimateEntitiesAndRestoreBank02 (00:0F05)
 * Selects bank $03 in rSelectROMBank, then calls AnimateEntitiesAndRestoreBank with bank $02.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param animate_entities Callback to AnimateEntities handler
 */
void AnimateEntitiesAndRestoreBank02(GBState *gb, void (*animate_entities)(GBState *));

/**
 * CanBowWowEatEntity (00:3925)
 * Checks whether Bow-Wow can eat an entity of the given type.
 * Switches to Bank $14, reads from BowWowEatableEntitiesTable, switches to Bank $05, and returns the byte.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_type Type ID of entity
 * @return Value read from table (nonzero if eatable)
 */
uint8_t CanBowWowEatEntity(GBState *gb, uint8_t entity_type);

/**
 * label_3935 (00:3935)
 * Switches to Bank $19 via SwitchBank, calls func_019_7c50, and then switches to Bank $03 via SwitchBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_019_7c50 Target callback in Bank $19
 */
void label_3935(GBState *gb, void (*func_019_7c50)(GBState *));

/**
 * LiftableRockStartSmashingAnimation_trampoline (00:3942)
 * Switches to Bank $03, calls LiftableRockStartSmashingAnimation, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param start_smashing Target callback in Bank $03
 */
void LiftableRockStartSmashingAnimation_trampoline(GBState *gb, void (*start_smashing)(GBState *));

/**
 * label_394D (00:394D)
 * Switches to Bank $14, calls func_014_54ac, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_014_54ac Target callback in Bank $14
 */
void label_394D(GBState *gb, void (*func_014_54ac)(GBState *));

/**
 * CreateFollowingNpcEntity_trampoline (00:3958)
 * Switches to Bank $01 via SwitchBank, calls create_following_npc, and switches to Bank $02 via SwitchBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param create_following_npc Target callback in Bank $01
 */
void CreateFollowingNpcEntity_trampoline(GBState *gb, void (*create_following_npc)(GBState *));

/**
 * ConfigureNewEntity_trampoline (00:3965)
 * Switches to Bank $03, calls configure_new_entity, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param configure_new_entity Target callback in Bank $03
 */
void ConfigureNewEntity_trampoline(GBState *gb, void (*configure_new_entity)(GBState *));

/**
 * GetEntityDirectionToLink_trampoline (00:3970)
 * Switches to Bank $03, calls get_direction, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param get_direction Target callback in Bank $03
 */
void GetEntityDirectionToLink_trampoline(GBState *gb, void (*get_direction)(GBState *));

/**
 * label_397B (00:397B)
 * Switches to Bank $14, calls func_014_5347, and sets rSelectROMBank to $03.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_014_5347 Target callback in Bank $14
 */
void label_397B(GBState *gb, void (*func_014_5347)(GBState *));

/**
 * ConfigureEntityHitbox (00:3AEA)
 * Copies 4 hitbox position bytes from HitboxPositions to wEntitiesHitboxPositionTable for entity index.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void ConfigureEntityHitbox(GBState *gb, uint16_t entity_index);

/**
 * SetEntitySpriteVariant (00:3B0C)
 * Sets the entity sprite variant in wEntitiesSpriteVariantTable for entity index.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 * @param variant Variant value to assign
 */
void SetEntitySpriteVariant(GBState *gb, uint16_t entity_index, uint8_t variant);

/**
 * IncrementEntityState (00:3B12)
 * Increments the entity state in wEntitiesStateTable for entity index.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void IncrementEntityState(GBState *gb, uint16_t entity_index);

/**
 * HurtBySpikes_trampoline (00:3B18)
 * Switches to Bank $02, calls hurt_by_spikes, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param hurt_by_spikes Target callback in Bank $02
 */
void HurtBySpikes_trampoline(GBState *gb, void (*hurt_by_spikes)(GBState *));

/**
 * ApplyEntityInteractionWithBackground_trampoline (00:3B23)
 * Switches to Bank $03, calls apply_interaction, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param apply_interaction Target callback in Bank $03
 */
void ApplyEntityInteractionWithBackground_trampoline(GBState *gb, void (*apply_interaction)(GBState *));

/**
 * label_3B2E (00:3B2E)
 * Switches to Bank $03, calls apply_sword_intersection, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param apply_sword_intersection Target callback in Bank $03
 */
void label_3B2E(GBState *gb, void (*apply_sword_intersection)(GBState *));

/**
 * DefaultEnemyDamageCollisionHandler_trampoline (00:3B39)
 * Switches to Bank $03, calls default_handler, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param default_handler Target callback in Bank $03
 */
void DefaultEnemyDamageCollisionHandler_trampoline(GBState *gb, void (*default_handler)(GBState *));

/**
 * label_3B44 (00:3B44)
 * Switches to Bank $03, calls func_003_6c6b, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_003_6c6b Target callback in Bank $03
 */
void label_3B44(GBState *gb, void (*func_003_6c6b)(GBState *));

/**
 * CheckLinkCollisionWithProjectile_trampoline (00:3B4F)
 * Switches to Bank $03, calls check_collision, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param check_collision Target callback in Bank $03
 */
void CheckLinkCollisionWithProjectile_trampoline(GBState *gb, void (*check_collision)(GBState *));

/**
 * CheckLinkCollisionWithEnemy (03:6C72)
 * Checks collision between Link and an enemy entity.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 * @return true if collision detected, false otherwise
 */
bool CheckLinkCollisionWithEnemy(GBState *gb, uint16_t entity_index);

/**
 * CheckLinkCollisionWithEnemy_trampoline (00:3B5A)
 * Switches to Bank $03, calls check_collision, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param check_collision Target callback in Bank $03
 */
void CheckLinkCollisionWithEnemy_trampoline(GBState *gb, void (*check_collision)(GBState *));

/**
 * label_3B65 (00:3B65)
 * Switches to Bank $03, calls func_003_73eb, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_003_73eb Target callback in Bank $03
 */
void label_3B65(GBState *gb, void (*func_003_73eb)(GBState *));

/**
 * label_3B70 (00:3B70)
 * Switches to Bank $03, calls func_003_6e2b, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_003_6e2b Target callback in Bank $03
 */
void label_3B70(GBState *gb, void (*func_003_6e2b)(GBState *));

/**
 * label_3B7B (00:3B7B)
 * Switches to Bank $03, calls func_003_75a2, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_003_75a2 Target callback in Bank $03
 */
void label_3B7B(GBState *gb, void (*func_003_75a2)(GBState *));

/**
 * ApplyVectorTowardsLink_trampoline (00:3BAA)
 * Switches to Bank $03, calls apply_vector, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param apply_vector Target callback in Bank $03
 */
void ApplyVectorTowardsLink_trampoline(GBState *gb, void (*apply_vector)(GBState *));

/**
 * GetVectorTowardsLink_trampoline (00:3BB5)
 * Switches to Bank $03, calls get_vector, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param get_vector Target callback in Bank $03
 */
void GetVectorTowardsLink_trampoline(GBState *gb, void (*get_vector)(GBState *));

typedef struct {
    void (*func_020_4303)(GBState *gb);
    void (*func_020_6352)(GBState *gb);
    void (*AnimateEntity)(GBState *gb, uint16_t entity_index);
} AnimateEntitiesCallbacks;

/**
 * AnimateEntities (00:398D)
 * Main loop iterating through active entity slots and animating each entity.
 * Handles boss agony countdown, dialog state, and OAM slot cycling.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param callbacks Callbacks for bank 0x20 routines and entity animation
 */
void AnimateEntities(GBState *gb, const AnimateEntitiesCallbacks *callbacks);

/**
 * ResetEntity_trampoline (00:3A0A)
 * Switches to Bank $15, calls reset_entity, and sets rSelectROMBank to $03.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param reset_entity Target callback in Bank $15
 */
void ResetEntity_trampoline(GBState *gb, void (*reset_entity)(GBState *));

typedef struct {
    void (*UpdateEntityPositionForRoomTransition)(GBState *gb);
    void (*CopyEntityPositionToActivePosition)(GBState *gb, uint16_t entity_index);
    void (*UpdateEntityTimers)(GBState *gb);
    void (*EntityDeathHandler)(GBState *gb);
    void (*EntityFallHandler)(GBState *gb);
    void (*EntityBurningHandler)(GBState *gb);
    void (*EntityInitHandler)(GBState *gb);
    void (*ExecuteActiveEntityHandler)(GBState *gb);
    void (*EntityStunnedHandler)(GBState *gb);
    void (*EntityLiftedHandler)(GBState *gb);
    void (*EntityThrownHandler)(GBState *gb);
} AnimateEntityCallbacks;

/**
 * AnimateEntity (00:3A18)
 * Dispatches active entity logic based on its state and status jump table.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 * @param callbacks Handlers for transitions, timers, and status states
 */
void AnimateEntity(GBState *gb, uint16_t entity_index, const AnimateEntityCallbacks *callbacks);

/**
 * ExecuteActiveEntityHandler_trampoline (00:3A81)
 * Calls execute_active_handler, then restores Bank $03 in wCurrentBank and rSelectROMBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param execute_active_handler Target callback
 */
void ExecuteActiveEntityHandler_trampoline(GBState *gb, void (*execute_active_handler)(GBState *));

/**
 * ExecuteActiveEntityHandler (00:3A8D)
 * Reads entity handler pointer from EntityHandlersTable in Bank $20 and jumps to target.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param handler_dispatch Callback to execute the target handler
 */
void ExecuteActiveEntityHandler(GBState *gb, void (*handler_dispatch)(GBState *, uint8_t bank, uint16_t addr));

/**
 * ClearEntitySpeed (00:3D7F)
 * Clears X and Y speed components for the entity slot.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void ClearEntitySpeed(GBState *gb, uint16_t entity_index);

/**
 * CopyEntityPositionToActivePosition (00:3D8A)
 * Copies entity X and Y to hActiveEntityPosX and hActiveEntityPosY,
 * and computes visual Y = posY - posZ into hActiveEntityVisualPosY.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void CopyEntityPositionToActivePosition(GBState *gb, uint16_t entity_index);

/**
 * SkipDisabledEntityDuringRoomTransition (00:3D57)
 * Returns true if active entity rendering should be skipped during a room transition.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity slot index
 * @return true if entity should be skipped
 */
bool SkipDisabledEntityDuringRoomTransition(GBState *gb, uint16_t entity_index);

typedef struct {
    void (*func_015_795D)(GBState *gb, uint16_t entity_index);
    void (*func_015_7995)(GBState *gb, uint16_t entity_index);
} EntityRenderCallbacks;

/**
 * RenderActiveEntitySpritesPair (00:3BC0)
 * Renders a pair of 8x16 sprites for the active entity into wDynamicOAMBuffer.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param display_list Pointer to OAM display list (4 bytes per variant)
 * @param callbacks Bank $15 rendering callbacks
 */
void RenderActiveEntitySpritesPair(GBState *gb, const uint8_t *display_list, const EntityRenderCallbacks *callbacks);

/**
 * label_3C71 (00:3C71)
 * Calls func_015_7995 in Bank $15 and reloads saved bank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 * @param func_015_7995 Target callback in Bank $15
 */
void label_3C71(GBState *gb, uint16_t entity_index, void (*func_015_7995)(GBState *, uint16_t));

/**
 * RenderActiveEntitySprite (00:3C77)
 * Renders a single 8x16 sprite for the active entity into wDynamicOAMBuffer.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param display_list Pointer to OAM display list (2 bytes per variant)
 * @param callbacks Bank $15 rendering callbacks
 */
void RenderActiveEntitySprite(GBState *gb, const uint8_t *display_list, const EntityRenderCallbacks *callbacks);

/**
 * label_3CD9 (00:3CD9)
 * Sets rSelectROMBank to $15 and jumps to label_3C71.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 * @param func_015_7995 Target callback in Bank $15
 */
void label_3CD9(GBState *gb, uint16_t entity_index, void (*func_015_7995)(GBState *, uint16_t));

/**
 * RenderActiveEntitySpritesRect (00:3CE6)
 * Renders a rectangle of sprites starting at wDynamicOAMBuffer + wOAMNextAvailableSlot.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param display_list Pointer to OAM display list (4 bytes per sprite)
 * @param sprite_count Number of sprites to render
 * @param func_015_795D Callback for post-rendering logic in Bank $15
 */
void RenderActiveEntitySpritesRect(GBState *gb, const uint8_t *display_list, uint8_t sprite_count, void (*func_015_795D)(GBState *, uint16_t));

/**
 * RenderActiveEntitySpritesRectUsingAllOAM (00:3CE0)
 * Renders a rectangle of sprites starting at wOAMBuffer (0xC000).
 *
 * @param gb Pointer to Game Boy hardware state
 * @param display_list Pointer to OAM display list (4 bytes per sprite)
 * @param sprite_count Number of sprites to render
 * @param func_015_795D Callback for post-rendering logic in Bank $15
 */
void RenderActiveEntitySpritesRectUsingAllOAM(GBState *gb, const uint8_t *display_list, uint8_t sprite_count, void (*func_015_795D)(GBState *, uint16_t));

/**
 * func_015_7964_trampoline (00:3DA0)
 * Switches to Bank $15, calls func_015_7964, and restores saved bank via ReloadSavedBank.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param func_015_7964 Target callback in Bank $15
 */
void func_015_7964_trampoline(GBState *gb, void (*func_015_7964)(GBState *));

/**
 * EntityInitMiniMoldorm_trampoline (00:3DAB)
 */
void EntityInitMiniMoldorm_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitMoldorm_trampoline (00:3DB6)
 */
void EntityInitMoldorm_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitFacade_trampoline (00:3DC1)
 */
void EntityInitFacade_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitSlimeEye_trampoline (00:3DCC)
 */
void EntityInitSlimeEye_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitGenie_trampoline (00:3DD7)
 */
void EntityInitGenie_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitSlimeEel_trampoline (00:3DE2)
 */
void EntityInitSlimeEel_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitDodongoSnake_trampoline (00:3DED)
 */
void EntityInitDodongoSnake_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitHotHead_trampoline (00:3DF8)
 */
void EntityInitHotHead_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * EntityInitEvilEagle_trampoline (00:3E03)
 */
void EntityInitEvilEagle_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * Entity67Handler_trampoline (00:3E0E)
 */
void Entity67Handler_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * CheckPositionForMapTransition_trampoline (00:3E19)
 */
void CheckPositionForMapTransition_trampoline(GBState *gb, void (*check_pos)(GBState *));

/**
 * GhiniMovement_trampoline (00:3E29)
 */
void GhiniMovement_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * SmashRock_trampoline (00:3E34)
 */
void SmashRock_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * LoadHeartsAndRupeesCount (00:3E3F)
 */
void LoadHeartsAndRupeesCount(GBState *gb, void (*load_rupees)(GBState *), void (*load_hearts)(GBState *));

/**
 * SpawnChestWithItemAndRestoreBank3 (00:3E4D)
 */
void SpawnChestWithItemAndRestoreBank3(GBState *gb, void (*spawn_chest)(GBState *));

/**
 * DrawABButtonSlots (00:3E5A)
 */
void DrawABButtonSlots(GBState *gb, void (*draw_slots)(GBState *, uint16_t bc, uint8_t e));

/**
 * GiveInventoryItem_trampoline (00:3E6B)
 */
void GiveInventoryItem_trampoline(GBState *gb, uint8_t item, void (*give_item)(GBState *, uint8_t item));

/**
 * func_006_783C_trampoline (00:3E76)
 */
void func_006_783C_trampoline(GBState *gb, void (*func)(GBState *));

/**
 * UnloadAllEntities (00:3E83)
 * Clears all entity status slots (0..15) in wEntitiesStatusTable to 0.
 */
void UnloadAllEntities(GBState *gb);

/**
 * label_3E8E (00:3E8E)
 * Spawns a smoke VFX if entity has power recoil active and frame counter matches.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 */
void label_3E8E(GBState *gb, uint16_t entity_index);

/**
 * StopEntityRecoilOnCollision (00:3EAF)
 * Clears wEntitiesIgnoreHitsCountdownTable when a recoiling entity collides with obstacles.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 */
void StopEntityRecoilOnCollision(GBState *gb, uint16_t entity_index);

/**
 * ReturnIfNonInteractive_03 (03:7F78)
 * Returns early if entity is not active or game is in dialog/transition.
 * Has an entry point .allowInactiveEntity that skips the entity status check.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param allow_inactive_entity If true, skip the entity status check
 * @return true if should return early, false to continue
 */
bool ReturnIfNonInteractive_03(GBState *gb, bool allow_inactive_entity);

/**
 * ApplyRecoilIfNeeded_03 (03:7FA9)
 * If entity has ignore-hits countdown, applies recoil velocity and handles collision.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void ApplyRecoilIfNeeded_03(GBState *gb, uint16_t entity_index);

/**
 * BouncingEntityPhysics (03:60B3)
 * Updates entity position with speed, handles bouncing physics for side-scrolling and top-down.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void BouncingEntityPhysics(GBState *gb, uint16_t entity_index);

/**
 * SetEntityVariantForDirection_03 (03:58FC)
 * Sets entity sprite variant based on direction and inertia.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void SetEntityVariantForDirection_03(GBState *gb, uint16_t entity_index);

/**
 * UpdateEntityPosWithSpeed_03 (03:7F25)
 * Updates entity position using speed values.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void UpdateEntityPosWithSpeed_03(GBState *gb, uint16_t entity_index);

/**
 * ApplyEntityInteractionWithBackground (03:7893)
 * Applies entity interaction with background tiles.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void ApplyEntityInteractionWithBackground(GBState *gb, uint16_t entity_index);

/**
 * func_003_6B7B (03:6B7B)
 * Helper function used by BouncingEntityPhysics.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void func_003_6B7B(GBState *gb, uint16_t entity_index);

/**
 * ApplySwordIntersectionWithObjects (03:7CAB)
 * Applies sword intersection with objects.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
bool ApplySwordIntersectionWithObjects(GBState *gb, uint16_t entity_index);

/**
 * AnimateRoamingEnemy (03:583C)
 * Animates a roaming enemy (Octorok, Moblin, etc.).
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void AnimateRoamingEnemy(GBState *gb, uint16_t entity_index);

/**
 * CheckLinkCollisionWithProjectile (03:6C72)
 * Checks collision between Link and a projectile entity.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 * @return true if collision detected, false otherwise
 */
bool CheckLinkCollisionWithProjectile(GBState *gb, uint16_t entity_index);

/**
 * CheckLinkCollisionWithEnemy (03:6C72)
 * Checks collision between Link and an enemy entity.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 * @return true if collision detected, false otherwise
 */
bool CheckLinkCollisionWithEnemy(GBState *gb, uint16_t entity_index);

/**
 * ApplyLinkCollisionWithEnemy (03:6CD5)
 * Applies collision damage to Link from an enemy entity.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void ApplyLinkCollisionWithEnemy(GBState *gb, uint16_t entity_index);

/**
 * DefaultEnemyDamageCollisionHandler (03:6E2B)
 * Handles default enemy damage collision with Link's sword.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void DefaultEnemyDamageCollisionHandler(GBState *gb, uint16_t entity_index);

/**
 * ApplySwordDamagesToEnemy (03:7267)
 * Applies sword damage to enemy entities.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void ApplySwordDamagesToEnemy(GBState *gb, uint16_t entity_index);

/**
 * func_003_6B7B (03:6B7B)
 * Applies gravity and underwater physics for entities.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void func_003_6B7B(GBState *gb, uint16_t entity_index);

/**
 * func_003_75A2 (03:75A2)
 * Helper function used by EntityThrownHandler.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void func_003_75A2(GBState *gb, uint16_t entity_index);

/**
 * AddEntitySpeedToPos_03 (03:7F32)
 * Updates entity position using speed values (alternate entry point).
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void AddEntitySpeedToPos_03(GBState *gb, uint16_t entity_index);

/**
 * EntityCheckThrowAtTriggers (03:5438)
 * Checks if thrown entity hit a trigger.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void EntityCheckThrowAtTriggers(GBState *gb, uint16_t entity_index);

/**
 * func_003_6E2B (03:6E2B)
 * Helper function used by EntityStunnedHandler.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void func_003_6E2B(GBState *gb, uint16_t entity_index);

/**
 * func_003_51C9 (03:51C9)
 * Helper function called when pushing blocks to check triggers.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 * @param data_ptr Pointer to data table
 * @param b_val Value for register B
 */
void func_003_51C9(GBState *gb, uint16_t entity_index, const uint8_t *data_ptr, uint8_t b_val);

/**
 * func_003_6C6B (03:6C6B)
 * Helper function for default enemy damage collision handling.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
bool func_003_6C6B(GBState *gb, uint16_t entity_index);

/**
 * func_003_6DDF (03:6DDF)
 * Handles various enemy damage reactions.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 */
void func_003_6DDF(GBState *gb, uint16_t entity_index);

/**
 * GetVectorTowardsLink (03:7E45)
 * Gets vector towards Link for recoil calculations.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param x Output X component
 * @param y Output Y component
 */
void GetVectorTowardsLink(GBState *gb, uint8_t *x, uint8_t *y);

/**
 * ConfigureEntityRecoil (03:6FCC)
 * Configures entity recoil after collision.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Entity slot index (0..15)
 * @param a_val Value for register A
 */
void ConfigureEntityRecoil(GBState *gb, uint16_t entity_index, uint8_t a_val);

/**
 * CopyLinkFinalPositionToActivePosition (03:0CBE)
 * Copies Link's final position to active position.
 *
 * @param gb Pointer to Game Boy hardware state
 */
void CopyLinkFinalPositionToActivePosition(GBState *gb);

/**
 * OpenDialogInTable0_trampoline (00:3B0C)
 * Opens a dialog from table 0.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param dialog_id Dialog ID to open
 */
void OpenDialogInTable0_trampoline(GBState *gb, uint8_t dialog_id);


/**
 * BossIntro (00:3EE8)
 * Handles boss/miniboss music trigger and intro dialog playback.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 * @param open_dialog Callback to OpenDialogInTable0
 */
void BossIntro(GBState *gb, uint16_t entity_index, void (*open_dialog)(GBState *, uint8_t dialog_id));

/**
 * DidKillEnemy (00:3F50)
 * Handles enemy defeat drops, kill count/order recording, room clearing flags, and unloads entity.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 * @param spawn_enemy_drop Callback to SpawnEnemyDrop in Bank $03
 */
void DidKillEnemy(GBState *gb, uint16_t entity_index, void (*spawn_enemy_drop)(GBState *, uint16_t));

/**
 * DidKillEnemy.label_3F78 (00:3F78)
 * Checks load_order < 8, sets cleared room bit, and unloads entity.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 * @param load_order Entity load order
 */
void DidKillEnemy_label_3F78(GBState *gb, uint16_t entity_index, uint8_t load_order);

/**
 * UnloadEntity / UnloadEntityAndReturn (00:3F8D)
 * Sets entity status to 0 (disabled).
 *
 * @param gb Pointer to Game Boy hardware state
 * @param entity_index Active entity index
 */
void UnloadEntity(GBState *gb, uint16_t entity_index);
void UnloadEntityAndReturn(GBState *gb, uint16_t entity_index);


/**
 * LoadEntityFromDefinition (00:3883)
 * Loads an entity for the current room from an entity definition.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param def_ptr Pointer to the entity definition address in ROM/RAM (advanced by 2 bytes)
 * @param configure_new_entity Callback for ConfigureNewEntity_helper (bank $03, 0x6524)
 * @param prepare_entity_position Callback for PrepareEntityPositionForRoomTransition (bank $01, 0x5EAB)
 * @return Entity slot index loaded (0..15), or 0xFF if skipped / no slot free
 */
uint8_t LoadEntityFromDefinition(GBState *gb, uint16_t *def_ptr,
                                 void (*configure_new_entity)(GBState *, uint8_t slot),
                                 void (*prepare_entity_position)(GBState *, uint8_t slot));

/**
 * LoadRoomEntities (00:37FE)
 * Retrieves the entities list for this room, and loads each entity from its definition.
 *
 * @param gb Pointer to Game Boy hardware state
 * @param update_recent_rooms_list Callback for UpdateRecentRoomsList (bank $01, 0x5F02)
 * @param spawn_new_entity Callback for SpawnNewEntity (bank $03)
 * @param configure_new_entity Callback for ConfigureNewEntity_helper (bank $03, 0x6524)
 * @param prepare_entity_position Callback for PrepareEntityPositionForRoomTransition (bank $01, 0x5EAB)
 */
void LoadRoomEntities(GBState *gb,
                      void (*update_recent_rooms_list)(GBState *),
                      uint16_t (*spawn_new_entity)(GBState *, uint8_t entity_type),
                      void (*configure_new_entity)(GBState *, uint8_t slot),
                      void (*prepare_entity_position)(GBState *, uint8_t slot));

/* Droppable Item Callback Stubs */

void DroppableDisappearIfNeeded(GBState *gb, uint16_t entity_index);
void func_003_61C0(GBState *gb, uint16_t entity_index);
bool DroppableRevealOrReturnIfNeeded(GBState *gb, uint16_t entity_index);
void func_003_7E0E(GBState *gb, uint16_t entity_index);
bool PickableHandleGrabbedByItemIfNeeded(GBState *gb, uint16_t entity_index);
bool PickableCollectIfNeeded(GBState *gb, uint16_t entity_index);
void PickDroppableMagicPowder(GBState *gb, uint16_t entity_index);
void PickSecretSeashell(GBState *gb, uint16_t entity_index);
void IncreaseValueAtHLClampAt99(GBState *gb);
void PickDroppableArrows(GBState *gb, uint16_t entity_index);
void PickDroppableBombs(GBState *gb, uint16_t entity_index);
void PickSirensInstrument(GBState *gb, uint16_t entity_index);
void HoldPickupInTheAir(GBState *gb, uint16_t entity_index);
void PickHeartContainer(GBState *gb, uint16_t entity_index);
void PickToadstoolOrDungeonKey(GBState *gb, uint16_t entity_index);
void PickHeartPiece(GBState *gb, uint16_t entity_index);
void PickGuardianAcorn(GBState *gb, uint16_t entity_index);
void PickPieceOfPower(GBState *gb, uint16_t entity_index);
void ProcessPowerUp(GBState *gb, uint16_t entity_index);
void MovePickupInTheAir(GBState *gb, uint16_t entity_index);
void PickSword(GBState *gb, uint16_t entity_index);
void GiveInventoryItem(GBState *gb, uint16_t entity_index);
void PickDroppableKey(GBState *gb, uint16_t entity_index);
void PickDroppableHeart(GBState *gb, uint16_t entity_index);
void PickDroppableRupee(GBState *gb, uint16_t entity_index);
void PickDroppableFairy(GBState *gb, uint16_t entity_index);

/* Entity Spawning Callbacks */
void SpawnNewEntity(GBState *gb, uint16_t entity_index);
void SpawnNewEntityInRange(GBState *gb, uint16_t entity_index);
void ConfigureNewEntity_helper(GBState *gb, uint16_t entity_index);

/* Moblin/Roaming Enemy Callbacks */
void AnimateRoamingEnemy(GBState *gb, uint16_t entity_index);
void SpawnMoblinArrow(GBState *gb, uint16_t entity_index);
void SpawnOctorokRock(GBState *gb, uint16_t entity_index);

/* Bomb Callbacks */
void RenderBombExplosion(GBState *gb, uint16_t entity_index);
void RenderBomb(GBState *gb, uint16_t entity_index);
void CheckForBombDestroyableObjectPuzzle(GBState *gb, uint16_t entity_index, uint16_t de);
void CheckForBombDestroyableObjectBasic(GBState *gb, uint16_t entity_index, uint16_t de);
void CheckExplosionInteractionWithEntities(GBState *gb, uint16_t entity_index);

/* Magic Rod Fireball Callback */
void RevealObjectUnderObject_trampoline(GBState *gb, void (*reveal_object)(GBState *));

#ifdef __cplusplus
}
#endif

#endif /* LADX_HOME_ENTITIES_H */
