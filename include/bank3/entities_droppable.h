#ifndef LADX_BANK3_ENTITIES_DROPPABLE_H
#define LADX_BANK3_ENTITIES_DROPPABLE_H

#include "gb.h"

/* Droppable Item Handlers (03:59DC-03:61BF) */
void HeartContainerEntityHandler(GBState *gb, uint16_t bc);
void HeartPieceEntityHandler(GBState *gb, uint16_t bc);
void HeartPieceState0Handler(GBState *gb, uint16_t bc);
void HeartPieceState1Handler(GBState *gb, uint16_t bc);
void HeartPieceState2Handler(GBState *gb, uint16_t bc);
void HeartPieceState3Handler(GBState *gb, uint16_t bc);
void HeartPieceState4Handler(GBState *gb, uint16_t bc);
void HeartPieceState5Handler(GBState *gb, uint16_t bc);
void HeartPieceState6Handler(GBState *gb, uint16_t bc);
void HeartPieceState7Handler(GBState *gb, uint16_t bc);
void HeartPieceState8Handler(GBState *gb, uint16_t bc);
void DrawHeartPiecesInDialog(GBState *gb, uint16_t bc);
void GuardianAcornEntityHandler(GBState *gb, uint16_t bc);
void PieceOfPowerEntityHandler(GBState *gb, uint16_t bc);
void IronMasksMaskEntityHandler(GBState *gb, uint16_t bc);
void SwordShieldPickableEntityHandler(GBState *gb, uint16_t bc);
void SwordShieldPickableState0Handler(GBState *gb, uint16_t bc);
void SwordShieldPickableState1Handler(GBState *gb, uint16_t bc);
void SwordShieldPickableState2Handler(GBState *gb, uint16_t bc);
void SwordShieldPickableState3Handler(GBState *gb, uint16_t bc);
void KeyDropPointEntityHandler(GBState *gb, uint16_t bc);
void HookshotDropEntityHandler(GBState *gb, uint16_t bc);
void DroppableHeartEntityHandler(GBState *gb, uint16_t bc);
void SleepyToadstoolEntityHandler(GBState *gb, uint16_t bc);
void SirensInstrumentEntityHandler(GBState *gb, uint16_t bc);
void SirensInstrumentState0Handler(GBState *gb, uint16_t bc);
void SirensInstrumentState1Handler(GBState *gb, uint16_t bc);
void SirensInstrumentState2Handler(GBState *gb, uint16_t bc);
void func_003_5ED5(GBState *gb, uint16_t bc);
void func_003_5F0C(GBState *gb, uint16_t bc);
void func_003_5F33(GBState *gb, uint16_t bc);
void func_003_5FBC(GBState *gb, uint16_t bc);
void func_003_5FBF(GBState *gb, uint16_t bc);
void animateSirensInstrumentPickup(GBState *gb, uint16_t bc, uint8_t slow_countdown);
void AfterSirensInstrumentD1(GBState *gb);
void AfterSirensInstrumentD2(GBState *gb);
void AfterSirensInstrumentD3(GBState *gb);
void AfterSirensInstrumentD4(GBState *gb);
void AfterSirensInstrumentNone(GBState *gb);
void AfterSirensInstrumentD6(GBState *gb);
void AfterSirensInstrumentD7(GBState *gb);
void DroppableBombsEntityHandler(GBState *gb, uint16_t bc);
void DroppableSeashellEntityHandler(GBState *gb, uint16_t bc);
void HidingSlimeKeyEntityHandler(GBState *gb, uint16_t bc);
void DroppableFairyEntityHandler(GBState *gb, uint16_t bc);
void DroppableMagicPowderEntityHandler(GBState *gb, uint16_t bc);
void DroppableArrowsEntityHandler(GBState *gb, uint16_t bc);
void DroppableRupeeEntityHandler(GBState *gb, uint16_t bc);
void PickableHandler(GBState *gb, uint16_t bc);

/* Droppable Helper Functions (03:5A17-03:629D) */
void HoldEntityAboveLink(GBState *gb, uint16_t bc);
void func_003_5A2E(GBState *gb, uint16_t bc);
void DroppableDisappearIfNeeded(GBState *gb, uint16_t bc);
void func_003_61C0(GBState *gb, uint16_t bc);
void DroppableRevealOrReturnIfNeeded(GBState *gb, uint16_t bc);
void func_003_7E0E(GBState *gb, uint16_t bc);
void PickableCanBeCollectedBySwordTable(GBState *gb);
void PickableHandleGrabbedByItemIfNeeded(GBState *gb, uint16_t bc);
void PickableCollectIfNeeded(GBState *gb, uint16_t bc);

/* Pickable Item Collection Functions (03:6350-03:64C8) */
void PickDroppableMagicPowder(GBState *gb, uint16_t bc);
void PickSecretSeashell(GBState *gb, uint16_t bc);
void IncreaseValueAtHLClampAt99(GBState *gb);
void IncreaseValueAtHLClampAt99_addr(GBState *gb, uint16_t addr);
void PickDroppableArrows(GBState *gb, uint16_t bc);
void PickDroppableBombs(GBState *gb, uint16_t bc);
void PickSirensInstrument(GBState *gb, uint16_t bc);
void HoldPickupInTheAir(GBState *gb, uint16_t bc);
void PickHeartContainer(GBState *gb, uint16_t bc);
void PickToadstoolOrDungeonKey(GBState *gb, uint16_t bc);
void PickHeartPiece(GBState *gb, uint16_t bc);
void PickGuardianAcorn(GBState *gb, uint16_t bc);
void PickPieceOfPower(GBState *gb, uint16_t bc);
void ProcessPowerUp(GBState *gb, uint16_t bc);
void MovePickupInTheAir(GBState *gb, uint16_t bc);
void PickSword(GBState *gb, uint16_t bc);
void GiveInventoryItem(GBState *gb, uint16_t item);
void PickDroppableKey(GBState *gb, uint16_t bc);
void PickDroppableHeart(GBState *gb, uint16_t bc);
void PickDroppableRupee(GBState *gb, uint16_t bc);
void PickDroppableFairy(GBState *gb, uint16_t bc);

/* Entity Spawning Functions (03:64CA-03:652D) */
void SpawnNewEntity(GBState *gb, uint16_t bc);
void SpawnNewEntityInRange(GBState *gb, uint16_t bc);
uint16_t SpawnNewEntity_slot(GBState *gb, uint8_t entity_type);
uint16_t SpawnNewEntityInRange_slot(GBState *gb, uint8_t entity_type, uint8_t start_slot);
uint16_t SpawnNewEntityInRange_impl(GBState *gb, uint8_t entity_type, uint16_t bc, uint8_t start_e);
void ConfigureNewEntity_helper(GBState *gb, uint16_t bc);

#endif /* LADX_BANK3_ENTITIES_DROPPABLE_H */