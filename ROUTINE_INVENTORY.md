# LADX Authoritative Routine Inventory & Scope Audit

- **Generated:** 2026-10-11 12:54:12 UTC
- **Git Commit:** `ed78cae` on `main` (Working tree clean: `False`)
- **Disassembly Source of Truth:** `LADX-Disassembly/src/` (RGBDS Assembly)
- **C Codebase Target:** Strict C11 (`-std=c11 -Wall -Wextra -Werror -pedantic`)

---

## 1. Executive Summary & Dual-Scope Progress

| Scope Definition | Total Standalone Routines | Verified | Decompiled (Untested) | Missing | Completion % |
|:---|:---:|:---:|:---:|:---:|:---:|
| **Subsystem Scope (Banks 00–03)** | **1143** | **897** | **100** | **146** | **78.48%** |
| **Whole-Game Scope (All 64 Banks)** | **6775** | **899** | **101** | **5775** | **13.27%** |

> [!NOTE]
> The project tracks a dual scope: the **Banks 00–03 Subsystem Scope** covers the core Game Boy engine, world handler, physics, and base entity system; the **Whole-Game Scope** covers all 64 ROM banks (including 25 unstarted code banks).

---

## 2. Subsystem Scope Breakdown (Banks 00–03)

| Bank | Routines | Aliases | Alt Entries | Data Tables | Verified | Decompiled | Missing | % Verified |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Bank $00 | 447 | 3 | 0 | 60 | 364 | 27 | 56 | 81.4% |
| Bank $01 | 257 | 2 | 0 | 142 | 191 | 37 | 29 | 74.3% |
| Bank $02 | 185 | 0 | 0 | 144 | 124 | 3 | 58 | 67.0% |
| Bank $03 | 254 | 7 | 16 | 137 | 218 | 33 | 3 | 85.8% |

---

## 3. Bank 3 Remaining Routines Census

### Group A: Implemented in C, Pending Formal Behavioral Test (3 routines)
- **`func_003_52D4`** (`03:52D4`) in `code/entities/bank3.asm:2007` -> `src/bank3/entities_pushed_block.c:249`
- **`HeartPieceState1Handler`** (`03:5A6D`) in `code/entities/bank3.asm:2863` -> `src/bank3/entities_droppable.c:193`
- **`HeartPieceState2Handler`** (`03:5A7C`) in `code/entities/bank3.asm:2872` -> `src/bank3/entities_droppable.c:210`
- **`HeartPieceState3Handler`** (`03:5A83`) in `code/entities/bank3.asm:2877` -> `src/bank3/entities_droppable.c:219`
- **`HeartPieceState4Handler`** (`03:5A8A`) in `code/entities/bank3.asm:2882` -> `src/bank3/entities_droppable.c:228`
- **`HeartPieceState5Handler`** (`03:5A98`) in `code/entities/bank3.asm:2889` -> `src/bank3/entities_droppable.c:242`
- **`HeartPieceState6Handler`** (`03:5ABB`) in `code/entities/bank3.asm:2911` -> `src/bank3/entities_droppable.c:274`
- **`HeartPieceState7Handler`** (`03:5AF0`) in `code/entities/bank3.asm:2944` -> `src/bank3/entities_droppable.c:318`
- **`HeartPieceState8Handler`** (`03:5B05`) in `code/entities/bank3.asm:2956` -> `src/bank3/entities_droppable.c:338`
- **`HeartPieceState0Handler`** (`03:5B52`) in `code/entities/bank3.asm:3009` -> `src/bank3/entities_droppable.c:184`
- **`SwordShieldPickableState0Handler`** (`03:5BBA`) in `code/entities/bank3.asm:3088` -> `src/bank3/entities_droppable.c:451`
- **`SwordShieldPickableState1Handler`** (`03:5BE4`) in `code/entities/bank3.asm:3119` -> `src/bank3/entities_droppable.c:496`
- **`SwordShieldPickableState2Handler`** (`03:5C01`) in `code/entities/bank3.asm:3135` -> `src/bank3/entities_droppable.c:524`
- **`SwordShieldPickableState3Handler`** (`03:5C0F`) in `code/entities/bank3.asm:3145` -> `src/bank3/entities_droppable.c:543`
- **`AfterSirensInstrumentD1`** (`03:5E0C`) in `code/entities/bank3.asm:3474` -> `src/bank3/entities_droppable.c:888`
- **`AfterSirensInstrumentD3`** (`03:5E18`) in `code/entities/bank3.asm:3487` -> `src/bank3/entities_droppable.c:901`
- **`AfterSirensInstrumentNone`** (`03:5E1E`) in `code/entities/bank3.asm:3495` -> `src/bank3/entities_droppable.c:913`
- **`SirensInstrumentState0Handler`** (`03:5EA3`) in `code/entities/bank3.asm:3599` -> `src/bank3/entities_droppable.c:1239`
- **`func_003_5ED5`** (`03:5ED5`) in `code/entities/bank3.asm:3632` -> `src/bank3/entities_droppable.c:1072`
- **`func_003_5F0C`** (`03:5F0C`) in `code/entities/bank3.asm:3698` -> `src/bank3/entities_droppable.c:1109`
- **`func_003_5F33`** (`03:5F33`) in `code/entities/bank3.asm:3727` -> `src/bank3/entities_droppable.c:1143`
- **`func_003_5FBC`** (`None`) in `code/entities/bank3.asm:3812` -> `src/bank3/entities_droppable.c:1227`
- **`func_003_5FBF`** (`03:5FBF`) in `code/entities/bank3.asm:3825` -> `src/bank3/entities_droppable.c:1233`
- **`ProcessPowerUp`** (`03:6400`) in `code/entities/bank3.asm:4566` -> `src/bank3/entities_droppable.c:2075`
- **`MovePickupInTheAir`** (`03:641E`) in `code/entities/bank3.asm:4581` -> `src/bank3/entities_droppable.c:1979`
- **`SpawnNewEntityInRange`** (`03:64CC`) in `code/entities/bank3.asm:4736` -> `src/bank3/entities_droppable.c:2206`
- **`func_003_6B7B`** (`03:6B7B`) in `code/entities/bank3.asm:4871` -> `src/bank3/entities_physics.c:145`
- **`label_003_71C0`** (`03:71C0`) in `code/entities/bank3.asm:6140` -> `src/bank3/entities_collision.c:802`
- **`func_003_77D6`** (`03:77D6`) in `code/entities/bank3.asm:7264` -> `src/bank3/entities_collision.c:1539`
- **`Entity4BHandler`** (`03:5326`) in `code/entities/03_liftable_rock.asm:1` -> `src/bank3/entities_liftable_rock.c:35`
- **`LiftableRockEntityHandler`** (`03:5328`) in `code/entities/03_liftable_rock.asm:6` -> `src/bank3/entities_liftable_rock.c:45`
- **`LiftableRockIntactHandler`** (`None`) in `code/entities/03_liftable_rock.asm:107` -> `src/bank3/entities_liftable_rock.c:129`
- **`MagicRodFireballEntityHandler`** (`03:69B2`) in `code/entities/03_magic_rod_fireball.asm:10` -> `src/bank3/entities_magic_rod.c:46`

### Group B: Missing Collision / Carry Utilities (Pending Batch Implementation)
- **`KeyCollectDialogs`** (`None`) in `code/entities/bank3.asm:3226`
- **`setCarryAndReturn`** (`03:6E0A`) in `code/entities/bank3.asm:5403`
- **`MagicRodFireballSpriteVariants`** (`03:69AA`) in `code/entities/03_magic_rod_fireball.asm:2`

### Group C: Shared Entry Points & Mid-Routine Fallthrough Labels
- **`NoopFunction`** (`03:4B56`): Identical-address alias covered by primary routine
- **`EntityInitEntity26`** (`None`): Identical-address alias covered by primary routine
- **`Entity25Handler`** (`None`): Identical-address alias covered by primary routine
- **`Entity26Handler`** (`None`): Identical-address alias covered by primary routine
- **`FireSpriteVariants`** (`03:4C44`): Identical-address alias covered by primary routine
- **`EntityInitWithShiftedPosition`** (`None`): Secondary entry point or jump target covered within primary routine
- **`EntityShiftPosition`** (`03:4F83`): Identical-address alias covered by primary routine
- **`label_003_52D7`** (`03:52D7`): Secondary entry point or jump target covered within primary routine
- **`label_003_57E6`** (`03:57E6`): Secondary entry point or jump target covered within primary routine
- **`label_003_5C49`** (`03:5C49`): Secondary entry point or jump target covered within primary routine
- **`label_003_5CD6`** (`03:5CD6`): Secondary entry point or jump target covered within primary routine
- **`label_003_636D`** (`03:636D`): Secondary entry point or jump target covered within primary routine
- **`label_003_63D2`** (`03:63D2`): Secondary entry point or jump target covered within primary routine
- **`label_003_6F04`** (`03:6F04`): Secondary entry point or jump target covered within primary routine
- **`label_003_6F24`** (`03:6F24`): Secondary entry point or jump target covered within primary routine
- **`label_003_73E6`** (`03:73E6`): Secondary entry point or jump target covered within primary routine
- **`label_003_74E1`** (`03:74E1`): Secondary entry point or jump target covered within primary routine
- **`label_003_7715`** (`03:7715`): Secondary entry point or jump target covered within primary routine
- **`label_003_7DCD`** (`03:7DCD`): Secondary entry point or jump target covered within primary routine
- **`label_003_7E05`** (`03:7E05`): Secondary entry point or jump target covered within primary routine
- **`label_003_7E09`** (`03:7E09`): Secondary entry point or jump target covered within primary routine
- **`ApplyVectorTowardsLinkAndReturn`** (`03:7EC7`): Alias of ApplyVectorTowardsLink at 03:7EC7; verified
- **`StopWalkingEnd`** (`03:58F6`): Secondary entry point or jump target covered within primary routine

---

## 4. Methodology & Audit Integrity

1. **Deterministic Include Hierarchy:** Every section and include path is traversed from `LADX-Disassembly/src/main.asm`.
2. **Precise Symbol Classification:** Column 0 symbols are classified as `ROUTINE`, `ALIAS`, `ALTERNATE_ENTRY`, `INTERNAL_LABEL`, or `DATA_TABLE`.
3. **Two-Way Verification Proof:** A routine is marked `VERIFIED` only when it has a concrete implementation in `src/` and verified unit test coverage in `tests/`.
4. **Machine-Readable Artifact:** The complete inventory is generated as `routine_inventory.json` for CI validation and batch scheduling.
