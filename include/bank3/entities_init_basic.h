#ifndef LADX_BANK3_ENTITIES_INIT_BASIC_H
#define LADX_BANK3_ENTITIES_INIT_BASIC_H

#include "gb.h"

/* Entity Init Functions (03:4926-03:4B56) */
void EntityInitHorsePiece(GBState *gb);
void EntityInitMarinAtTalTalHeights(GBState *gb);
void EntityInitSnake(GBState *gb);
void EntityInitSideViewPlatformVertical(GBState *gb);
void EntityInitZol(GBState *gb);
void EntityInitMarinAtTheShore(GBState *gb);
void EntityInitBomber(GBState *gb);
void EntityInitBushCrawler(GBState *gb);
void EntityInitTarinBeekeeper(GBState *gb);
void EntityInitTelephone(GBState *gb);
void EntityInitRichard(GBState *gb);
void SetMusicTrackIfHasSword(GBState *gb, uint8_t music_track);
void SetMusicTrack(GBState *gb, uint8_t music_track);
void EntityInitFinalNightmare(GBState *gb);
void EntityInitDreamShrineBed(GBState *gb);
void EntityInitFishermanUnderBridge(GBState *gb);
void EntityInitKikiTheMonkey(GBState *gb);
void EntityInitFireballShooter(GBState *gb);
void EntityInitAntiKirby(GBState *gb);
void EntityInitMovingBlockMover(GBState *gb);
void EntityInitDesertLanmola(GBState *gb);
void EntityInitFloatingItem2(GBState *gb);
void EntityInitFloatingItem(GBState *gb);
void SetZPosForFloatingItem(GBState *gb, uint16_t bc);
void EntityInitKid71(GBState *gb);
void EntityInitKid72(GBState *gb);
void EntityInitMrWrite(GBState *gb);
void EntityInitBigFairy(GBState *gb);
void EntityInitBowWow(GBState *gb);
void EntityInitOwlEvent(GBState *gb);
void EntityInitSword(GBState *gb);
void UnloadEntityIfRoomStatusSet(GBState *gb);
void EntityInitMarin(GBState *gb);
void EntityInitTarin(GBState *gb);
void EntityInitMadamMeowMeow(GBState *gb);
void EntityInitRaftRaftOwner(GBState *gb);
void EntityInitNpcFacingDown(GBState *gb, uint16_t bc);
void EntityInitStoreOwner(GBState *gb, uint16_t bc);
void EntityInitWitch(GBState *gb);
void EntityInitShopOwner(GBState *gb);
void EntityInitShopOwner_setDirectionLeft(GBState *gb, uint16_t bc);
void EntityInitWithRandomDirection(GBState *gb);
void SetEntityDirection(GBState *gb, uint16_t bc, uint8_t direction);
void EntityInitNoop(GBState *gb);
void EntityInitEntity13(GBState *gb);

/* Helper Functions */
void EntityShiftPosition(GBState *gb, uint16_t bc);
void EntityShiftPosition_shiftBy8(GBState *gb, uint16_t bc, uint16_t sign_table, uint16_t pos_table);

#endif /* LADX_BANK3_ENTITIES_INIT_BASIC_H */