# LADX Authoritative Routine Inventory & Scope Audit

- **Generated:** 2026-10-11 14:28:12 UTC
- **Git Commit:** `872b441` on `main` (Working tree clean: `False`)
- **Disassembly Source of Truth:** `LADX-Disassembly/src/` (RGBDS Assembly)
- **C Codebase Target:** Strict C11 (`-std=c11 -Wall -Wextra -Werror -pedantic`)

---

## 1. Executive Summary & Dual-Scope Progress

| Scope Definition | Total Standalone Routines | Verified | Decompiled (Untested) | Missing | Completion % |
|:---|:---:|:---:|:---:|:---:|:---:|
| **Subsystem Scope (Banks 00–03)** | **1126** | **904** | **97** | **125** | **80.28%** |
| **Whole-Game Scope (All 64 Banks)** | **4298** | **906** | **98** | **3294** | **21.08%** |

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
- **Subsystem Scope Standalone Routines:** **1126** total (**904** verified, **97** decompiled/untested, **125** missing).
- **Whole-Game Scope Standalone Routines:** **4298** total (**906** verified, **98** decompiled/untested, **3294** missing).

---

## 3. Comprehensive Per-Bank Census (All 64 ROM Banks)

The table below documents the status of all symbols across all 64 ROM banks ($00–$3F). Every row and the project-wide totals reconcile mathematically: `Routines = Verified + Decompiled + Missing + Blocked`.

| Bank | Content Description | Routines | Verified | Decompiled | Missing | Blocked | Unresolved | Aliases | Alt Entries | Data Tables | Internal | % Verified |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **$00** | Core Engine, Home, Interrupts, Math, Dialog, Rendering | 444 | 364 | 27 | 53 | 0 | 0 | 3 | 0 | 61 | 703 | 82.0% |
| **$01** | World Handler, File Menu, Save/Load, Intro Beach | 246 | 191 | 37 | 18 | 0 | 0 | 2 | 0 | 152 | 661 | 77.6% |
| **$02** | Items, Inventory, Link Physics, Room Transitions, Triggers | 181 | 124 | 3 | 54 | 0 | 0 | 1 | 0 | 147 | 706 | 68.5% |
| **$03** | Entity Core, Common Enemies, Physics, Initializers | 255 | 225 | 30 | 0 | 0 | 0 | 8 | 16 | 139 | 1758 | 88.2% |
| **$04** | Entities & Boss AI (Octorok, Moblin, Level Bosses) | 243 | 0 | 0 | 243 | 0 | 0 | 2 | 0 | 140 | 708 | 0.0% |
| **$05** | Entities (BowWow, Cucco, Marin, Tarin, Angler, Evil Eagle) | 225 | 0 | 0 | 225 | 0 | 0 | 0 | 0 | 133 | 637 | 0.0% |
| **$06** | Entities & Overworld NPCs/Enemies | 266 | 0 | 0 | 266 | 0 | 0 | 3 | 0 | 130 | 772 | 0.0% |
| **$07** | Entities & Dungeon Enemies (Spiked Beetle, Flying Tiles) | 277 | 0 | 0 | 277 | 0 | 0 | 1 | 0 | 136 | 811 | 0.0% |
| **$08** | Indoor & Dungeon Object Tilemaps, Object Physics (ROM Tables) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 42 | 1461 | 0.0% |
| **$09** | Overworld Rooms (First Half), Dialog DX Text (ROM Tables/Text) | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 232 | 0 | 0.0% |
| **$0A** | Indoors A & Color Dungeon Room Maps (ROM Tables) | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 283 | 0 | 0.0% |
| **$0B** | Indoors B Room Maps (ROM Tables) | 0 | 0 | 0 | 0 | 0 | 0 | 11 | 0 | 247 | 0 | 0.0% |
| **$0C** | DMG Graphics: Link, Items, Minimap, Animated Tiles (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 16 | 0 | 0.0% |
| **$0D** | DMG Graphics: Dungeons, Walls, Floors, Indoor Sets (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 43 | 0 | 0.0% |
| **$0E** | DMG Graphics: NPCs & Characters (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0.0% |
| **$0F** | DMG Graphics: Menus, Titles, Fonts, Overworld Tiles (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 24 | 0 | 0.0% |
| **$10** | DMG Graphics: Intro Scenes, Portraits, Large Fonts (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 11 | 0 | 0.0% |
| **$11** | DMG Graphics: NPCs 2 (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0.0% |
| **$12** | DMG Graphics: NPCs 3, Nightmares, Dungeon Items (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 12 | 0 | 0.0% |
| **$13** | DMG Graphics: Ending Scenes, Eagle Tower Top (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | 0 | 0.0% |
| **$14** | Dungeon Event Loaders, Chest Item Allocators, Layouts | 39 | 1 | 0 | 38 | 0 | 0 | 5 | 0 | 216 | 398 | 2.6% |
| **$15** | Entities (Moblin King, Pokey, Final Nightmare, Urchin) | 214 | 0 | 0 | 214 | 0 | 0 | 0 | 0 | 162 | 535 | 0.0% |
| **$16** | Entity Placement Pointer Tables & Overworld Placements (Pointers/Text) | 0 | 0 | 0 | 0 | 0 | 0 | 22 | 0 | 882 | 0 | 0.0% |
| **$17** | Ending Credits Sequence & Staff Roll Engine | 196 | 0 | 0 | 196 | 0 | 0 | 2 | 0 | 121 | 433 | 0.0% |
| **$18** | Entities (Mamu, Manbo, Zora, Mr Write, Blaino, Hookshot) | 284 | 0 | 0 | 284 | 0 | 0 | 0 | 0 | 115 | 658 | 0.0% |
| **$19** | Entities (Boomerang, Goriya, Mimic, Rooster, Ghost) | 221 | 0 | 0 | 221 | 0 | 0 | 1 | 0 | 173 | 581 | 0.0% |
| **$1A** | Overworld Attributes & Angler's Tunnel Door Handler | 3 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 145 | 1546 | 0.0% |
| **$1B** | Audio Engine 1 (Music Tracks 1B-1, 1B-2, 1B-3) | 79 | 0 | 0 | 79 | 0 | 0 | 23 | 0 | 507 | 49 | 0.0% |
| **$1C** | Dialog Letter Rendering & Text Box Control Characters | 8 | 0 | 0 | 8 | 0 | 0 | 1 | 0 | 148 | 7 | 0.0% |
| **$1D** | Dialog Scripts (Dialog 4, Dialog 100) (Text) | 0 | 0 | 0 | 0 | 0 | 0 | 6 | 0 | 184 | 0 | 0.0% |
| **$1E** | Audio Engine 2 (Music Tracks 1E-1, 1E-2, 1E-3) | 65 | 0 | 0 | 65 | 0 | 0 | 43 | 0 | 718 | 67 | 0.0% |
| **$1F** | Sound Effects Engine & SFX Audio Drivers | 390 | 0 | 0 | 390 | 0 | 0 | 0 | 0 | 881 | 300 | 0.0% |
| **$20** | Entity Dispatch Table (_handlers.asm) & Tileset Engine | 103 | 0 | 1 | 102 | 0 | 0 | 1 | 0 | 159 | 1184 | 0.0% |
| **$21** | Palette Management & Color Fading Routines | 19 | 0 | 0 | 19 | 0 | 0 | 1 | 0 | 154 | 63 | 0.0% |
| **$22** | Overworld Palette Tables (ROM Tables) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 16 | 4096 | 0.0% |
| **$23** | Credits Background Palette Loader | 1 | 0 | 0 | 1 | 0 | 0 | 1 | 0 | 13 | 2568 | 0.0% |
| **$24** | Background Color Engine & Bulk Palette Updaters | 20 | 1 | 0 | 19 | 0 | 0 | 3 | 0 | 75 | 1862 | 5.0% |
| **$25** | Overworld Object Attributes Set B (ROM Tables) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 16 | 4096 | 0.0% |
| **$26** | Overworld Room GBC Overlay Attributes (ROM Tables) | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 204 | 0 | 0.0% |
| **$27** | Color Attributes & Overworld GBC Overlays | 31 | 0 | 0 | 31 | 0 | 0 | 1 | 0 | 110 | 2086 | 0.0% |
| **$28** | Photo Album UI & Game Boy Printer Driver | 71 | 0 | 0 | 71 | 0 | 0 | 0 | 0 | 44 | 107 | 0.0% |
| **$29** | DMG Photo Graphics: Nice Link, Marin Cliff, Well, Mabe (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0.0% |
| **$2A** | DMG Photo Graphics: Ulrira, BowWow, Shop, Fisherman (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0.0% |
| **$2B** | DMG Photo Graphics: Zora, Kanalet, Ghost, Bridge (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0.0% |
| **$2C** | CGB Graphics: Link, Items, Minimap, Animated Tiles (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$2D** | CGB Graphics: Dungeons, Walls, Floors, Indoor Sets (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 43 | 0 | 0.0% |
| **$2E** | CGB Graphics: NPCs & Characters (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0.0% |
| **$2F** | CGB Graphics: Menus, Titles, Fonts, Overworld Tiles (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 20 | 0 | 0.0% |
| **$30** | CGB Graphics: Intro Scenes, Portraits, Large Fonts (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$31** | CGB Graphics: NPCs 2 (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0.0% |
| **$32** | CGB Graphics: NPCs 3, Nightmares, Dungeon Items (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$33** | CGB Graphics: Ending Scenes, Eagle Tower Top (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$34** | CGB Graphics: Game Boy Printer UI (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$35** | CGB Graphics: Color Dungeon Tiles & Camera Shop UI (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 7 | 0 | 0.0% |
| **$36** | Color Dungeon Entities (Rotoswitch, Color Guardians) | 279 | 0 | 0 | 279 | 0 | 0 | 10 | 0 | 133 | 553 | 0.0% |
| **$37** | Photo Events & Scene Setup Logic | 129 | 0 | 0 | 129 | 0 | 0 | 0 | 0 | 118 | 118 | 0.0% |
| **$38** | Photo Animations Logic & Ending Roll Portraits | 2 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 13 | 3 | 0.0% |
| **$39** | Photo Elements Graphics 1 (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$3A** | Photo Elements Graphics 2 (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$3B** | Photo Elements Graphics 3 & Dizzy Link (Binary) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0.0% |
| **$3C** | Super Game Boy Border Setup & SGB Packet Drivers | 5 | 0 | 0 | 5 | 0 | 0 | 0 | 0 | 21 | 10 | 0.0% |
| **$3D** | Photo Background Renderer | 2 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 15 | 16 | 0.0% |
| **$3E** | Blank Padding Bank (Zero-filled) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **$3F** | Dialog Scripts (Dialog 5) (Empty/Padding) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0.0% |
| **TOTAL** | **All 64 ROM Banks Combined** | **4298** | **906** | **98** | **3294** | **0** | **0** | **155** | **16** | **7384** | **29553** | **21.08%** |

---

## 4. Subsystem Scope (Banks 00–03) Deep Dive & Reconciliation

### Subsystem Per-Bank Status

| Bank | Content Description | Routines | Verified | Decompiled | Missing | Blocked | Aliases | Alt Entries | Data Tables | Internal | % Verified |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **$00** | Core Engine, Home, Interrupts, Math, Dialog, Rendering | 444 | 364 | 27 | 53 | 0 | 3 | 0 | 61 | 703 | 82.0% |
| **$01** | World Handler, File Menu, Save/Load, Intro Beach | 246 | 191 | 37 | 18 | 0 | 2 | 0 | 152 | 661 | 77.6% |
| **$02** | Items, Inventory, Link Physics, Room Transitions, Triggers | 181 | 124 | 3 | 54 | 0 | 1 | 0 | 147 | 706 | 68.5% |
| **$03** | Entity Core, Common Enemies, Physics, Initializers | 255 | 225 | 30 | 0 | 0 | 8 | 16 | 139 | 1758 | 88.2% |
| **SUBTOTAL** | **Banks 00–03 Subsystem** | **1126** | **904** | **97** | **125** | **0** | **14** | **16** | **499** | **3828** | **80.28%** |

### Explanation of Subsystem Missing Routines vs Bank 3 Completeness

A key finding of this audit is explaining how **Bank 3 contains 0 missing routines** while the Banks 00–03 subsystem scope reports **125 missing routines**:

1. **Historical False Premise:** Pre-Batch 106 documentation assumed that Banks 00, 01, and 02 were completely decompiled, declaring that only 26 routines remained in Bank 3. The historical 1,212 baseline target was simply computed as `1,186 verified + 26 Bank 3 unfinished = 1,212`.
2. **Actual Repository Reality:** Batch 111 successfully closed all 26 Bank 3 unfinished routines, bringing Bank 3 standalone routine coverage to **100% (225 verified, 30 decompiled helper states, 0 missing)**.
3. **Omitted Unfinished Work in Banks 00–02:** An authoritative symbol scan against the assembly reveals that Banks 00, 01, and 02 were never 100% finished:
   - **Bank 00 (53 missing):** Core hardware initialization (`Start`, `Init`, `RenderLoop`), VBlank/LCD interrupt handlers (`InterruptLCDStatus`, `InterruptVBlank`, `InterruptSerial`, `LoadTiles`, `LoadEntityTiles`), item collection sequences (`ApplyGotItem`, `InitGotItemSequence`), and interactive overworld handling (`WorldInteractiveHandler`).
   - **Bank 01 (18 missing):** File selection screen interactive state machine (`FileSelectionEntryPoint`, `FileSelectionPrepare0`..`6`), max hearts clamping (`RecalculateMaxHearts`, `ClampMaxHearts`), and OAM DMA transfer (`DMARoutine`).
   - **Bank 02 (54 missing):** Background collision physics subsystem (`BackgroundCollisionHandler`, `ApplyCollisionWithSolid`, `ApplyCollisionWithLedge`, `ApplyCollisionWithDoor`), side-scrolling physics (`LinkSideScrollingPhysicsHandler`, `LinkSideScrollingDivingPhysicsHandler`), interactive block mechanics (`interactiveBlock`), and door unlock collision (`OpenDoor`).
4. **Resolution:** The claim that Banks 00–03 was 100% complete was a genuine historical inconsistency based on an unverified assumption. In reality, Bank 3 is complete, while Banks 00–02 retain 125 genuine standalone routines awaiting decompilation.

---

## 5. Bank 3 Census Status (Batch 111 Complete)

Bank 3 contains **0 missing routines**; all 255 standalone callable routines are implemented in C:

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

## 6. Methodology & Audit Integrity

1. **Deterministic Include Hierarchy:** Traversed recursively from `LADX-Disassembly/src/main.asm`.
2. **Strict Symbol Categorization:** Column 0 symbols are classified as `ROUTINE`, `ALIAS`, `ALTERNATE_ENTRY`, `INTERNAL_LABEL`, `DATA_TABLE`, or `UNRESOLVED`.
3. **Two-Way Verification Proof:** A routine is marked `VERIFIED` only when it has a concrete implementation in `src/` and verified unit test assertions in `tests/`.
4. **Machine-Readable Artifact:** Output is saved deterministically as `routine_inventory.json` for CI validation.
