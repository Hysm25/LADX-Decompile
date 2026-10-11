# LADX Authoritative Routine Inventory & Scope Audit

- **Generated:** 2026-10-11 13:50:30 UTC
- **Git Commit:** `c92fccc` on `main` (Working tree clean: `False`)
- **Disassembly Source of Truth:** `LADX-Disassembly/src/` (RGBDS Assembly)
- **C Codebase Target:** Strict C11 (`-std=c11 -Wall -Wextra -Werror -pedantic`)

---

## 1. Executive Summary & Dual-Scope Progress

| Scope Definition | Total Standalone Routines | Verified | Decompiled (Untested) | Missing | Completion % |
|:---|:---:|:---:|:---:|:---:|:---:|
| **Subsystem Scope (Banks 00–03)** | **1127** | **904** | **97** | **126** | **80.21%** |
| **Whole-Game Scope (All 64 Banks)** | **4299** | **906** | **98** | **3295** | **21.07%** |

> [!NOTE]
> **Unified Counting Policy:** The primary progress denominator across all scopes represents **Standalone Callable Routines (`ROUTINE`)**.
> Non-routine symbols (identical-address `ALIAS`, secondary `ALTERNATE_ENTRY`, `INTERNAL_LABEL`, and `DATA_TABLE`) are tracked in distinct columns and never conflated with callable routines.

---

## 2. Progress-Count Contradiction Resolution

Prior reports contained two conflicting metrics:
1. **Historical Subsystem Progress:** `1,212 / 1,212 accounted for (100.0%)`
2. **Historical Whole-Game Progress:** `1,195 / 5,762 routines verified (20.7%)`

### Root Cause of the Discrepancy:
- **Historical 1,212 Baseline Scope:** Established in the pre-Batch 106 audit (`a3fb807`) as a manual tracking total of 1,186 verified routines + 26 Bank 3 unfinished routines. When Batch 111 resolved Bank 3, it verified 9 routines (3 Group A + 6 Group B, raising verified standalone routines to 1,195) and audited 17 Group C items (shared aliases and mid-routine fallthrough labels). To report 100% of the historical target, the report added `1,195 verified standalone routines + 17 Group C entries = 1,212`.
- **Whole-Game Scope (5,762):** Derived from `FULL_PROJECT_SCOPE_AUDIT.md`, which subtracted data tables from total labels across 29 code banks without separating internal jump labels (`jr_`, `ret_`) in Banks 1–3.
- **Automated Generator Defect:** The initial inventory generator misclassified data macros (`dialog_text`, `entity`, `rgb`, audio sequences) as code instructions due to an unconstrained fallback, inflating whole-game routine totals to 6,775.

### Unified Policy Applied:
- Standalone callable routines (`ROUTINE`) are tracked under one consistent rule across all 64 banks.
- **Subsystem Scope Standalone Routines:** **1127** total (**904** verified, **97** decompiled/untested, **126** missing).
- **Whole-Game Scope Standalone Routines:** **4299** total (**906** verified, **98** decompiled/untested, **3295** missing).

---

## 3. Subsystem Scope Breakdown (Banks 00–03)

| Bank | Routines | Aliases | Alt Entries | Data Tables | Internal | Verified | Decompiled | Missing | % Verified |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Bank $00 | 444 | 3 | 0 | 61 | 2 | 364 | 27 | 53 | 82.0% |
| Bank $01 | 247 | 2 | 0 | 151 | 70 | 191 | 37 | 19 | 77.3% |
| Bank $02 | 181 | 1 | 0 | 147 | 114 | 124 | 3 | 54 | 68.5% |
| Bank $03 | 255 | 8 | 16 | 139 | 78 | 225 | 30 | 0 | 88.2% |

---

## 4. Bank 3 Census Status (Batch 111 Complete)

Bank 3 contains **0 missing routines**; all 252 standalone callable routines are implemented in C:

### Group A: Implemented & Verified in Batch 111 (3 routines)
- **`Entity4BHandler`** (`03:5326`): Implemented at src/bank3/entities_liftable_rock.c:35 and verified by tests/bank3/test_entities.c:2783
- **`LiftableRockEntityHandler`** (`03:5328`): Implemented at src/bank3/entities_liftable_rock.c:45 and verified by tests/bank3/test_entities.c:2801
- **`MagicRodFireballEntityHandler`** (`03:69B2`): Implemented at src/bank3/entities_magic_rod.c:46 and verified by tests/bank3/test_entities.c:2824

### Group B: Missing Collision, Carry & Utility Routines Verified in Batch 111 (6 routines)
- **`setCarryAndReturn`** (`03:6E0A`): Implemented at src/bank3/entities_physics.c:1514 and verified by tests/bank3/test_entities.c:2851
- **`entitiesLoop`** (`03:75A6`): Implemented at src/bank3/entities_collision.c:1628 and verified by tests/bank3/test_entities.c:2943
- **`forceCollision`** (`03:765F`): Implemented at src/bank3/entities_collision.c:1634 and verified by tests/bank3/test_entities.c:2905
- **`forceCollisionEnd`** (`03:7668`): Implemented at src/bank3/entities_collision.c:1641 and verified by tests/bank3/test_entities.c:2918
- **`checkNextEntity`** (`03:779F`): Implemented at src/bank3/entities_collision.c:1647 and verified by tests/bank3/test_entities.c:2930
- **`ApplyVectorTowardsLinkAndReturn`** (`03:7EC7`): Implemented at src/bank3/entities_physics.c:1521 and verified by tests/bank3/test_entities.c:2863

### Group C: Secondary Entry Points & Mid-Routine Labels (17 entries)
- **`StopWalkingEnd`** (`03:58F6`): Secondary entry point or fallthrough target covered within primary routine
- **`NoopFunction`** (`03:4B56`): Identical-address alias covered by primary routine implementation
- **`EntityInitEntity26`** (`03:4C44`): Implemented at src/bank3/entities_handlers.c:189 and verified by tests/bank3/test_entities_handlers.c
- **`Entity25Handler`** (`03:4C44`): Implemented at src/bank3/entities_handlers.c:193 and verified by tests/bank3/test_entities_handlers.c
- **`Entity26Handler`** (`03:4C44`): Implemented at src/bank3/entities_handlers.c:197 and verified by tests/bank3/test_entities_handlers.c
- **`FireSpriteVariants`** (`03:4C44`): Identical-address alias covered by primary routine implementation
- **`EntityInitWithShiftedPosition`** (`03:4F83`): Secondary entry point or fallthrough target covered within primary routine
- **`EntityShiftPosition`** (`03:4F83`): Implemented at src/bank3/entities_init_basic.c:779 and verified by tests/bank3/test_entities.c:2620
- **`label_003_52D7`** (`03:52D7`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_57E6`** (`03:57E6`): Secondary entry point or fallthrough target covered within primary routine
- **`HeartContainerSpriteVariants`** (`None`): Identical-address alias covered by primary routine implementation
- **`label_003_5C49`** (`03:5C49`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_5CD6`** (`03:5CD6`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_636D`** (`03:636D`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_63D2`** (`03:63D2`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_6F04`** (`03:6F04`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_6F24`** (`03:6F24`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_73E6`** (`03:73E6`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_74E1`** (`03:74E1`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_7715`** (`03:7715`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_7DCD`** (`03:7DCD`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_7E05`** (`03:7E05`): Secondary entry point or fallthrough target covered within primary routine
- **`label_003_7E09`** (`03:7E09`): Secondary entry point or fallthrough target covered within primary routine
- **`ApplyVectorTowardsLinkAndReturn`** (`03:7EC7`): Implemented at src/bank3/entities_physics.c:1521 and verified by tests/bank3/test_entities.c:2863

---

## 5. Methodology & Audit Integrity

1. **Deterministic Include Hierarchy:** Traversed recursively from `LADX-Disassembly/src/main.asm`.
2. **Strict Symbol Categorization:** Column 0 symbols are classified as `ROUTINE`, `ALIAS`, `ALTERNATE_ENTRY`, `INTERNAL_LABEL`, `DATA_TABLE`, or `UNRESOLVED`.
3. **Two-Way Verification Proof:** A routine is marked `VERIFIED` only when it has a concrete implementation in `src/` and verified unit test assertions in `tests/`.
4. **Machine-Readable Artifact:** Output is saved deterministically as `routine_inventory.json` for CI validation.
