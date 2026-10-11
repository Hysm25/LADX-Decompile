cmake_minimum_required(VERSION 3.16)
if(NOT DEFINED ENV{PREFIX} AND EXISTS "/data/data/com.termux/files/usr")
    set(ENV{PREFIX} "/data/data/com.termux/files/usr")
endif()
project(ladxdc C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic -Wno-unused-parameter")

# Options
option(BUILD_TESTING "Build the test suite and verification runners" ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Core library (production code only)
set(LADX_CORE_SOURCES
    src/gb.c
    src/home/animated_tiles.c
    src/home/audio.c
    src/home/bank.c
    src/home/check_items_to_use.c
    src/home/clear_memory.c
    src/home/copy_data.c
    src/home/dialog.c
    src/home/entities.c
    src/home/gameplay.c
    src/home/gfx.c
    src/home/lcd.c
    src/home/link.c
    src/home/room.c
    src/home/ui.c
    src/home/vfx.c
    src/bank1/face_shrine_mural.c
    src/bank1/file_menu.c
    src/bank1/game_over.c
    src/bank1/intro.c
    src/bank1/marin_beach.c
    src/bank1/room_transition.c
    src/bank1/save.c
    src/bank1/siren_instruments.c
    src/bank1/world_handler.c
    src/bank1/world_map.c
    src/bank2/audio.c
    src/bank2/falling.c
    src/bank2/items.c
    src/bank2/link_animation.c
    src/bank2/link_motion.c
    src/bank2/ocarina.c
    src/bank2/revolving_door.c
    src/bank2/room_effects.c
    src/bank2/room_events.c
    src/bank2/room_transition.c
    src/bank2/room_triggers.c
    src/bank2/shovel.c
    src/bank2/swimming.c
    src/bank2/vfx.c
    src/bank3/entities_physics.c
    src/bank3/entities_collision.c
    src/bank3/entities_droppable.c
    src/bank3/entities_moblin.c
    src/bank3/entities_bomb.c
    src/bank3/entities_arrow.c
    src/bank3/entities_magic_rod.c
    src/bank3/entities_pushed_block.c
    src/bank3/entities_liftable_rock.c
    src/bank3/entities_handlers.c
    src/bank3/entities_init_core.c
    src/bank3/entities_init_basic.c
    src/bank3/entities_init_extended.c
)

add_library(ladx_core STATIC ${LADX_CORE_SOURCES})

# Test targets
if(BUILD_TESTING)
    enable_testing()

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    # Common test support
    add_library(ladx_test_support_objs OBJECT
        tests/bank2/test_support.c
    )

    # Bank 0 / Home tests
    set(LADX_TEST_HOME_SOURCES
        tests/test_home.c
        tests/test_animated_tiles.c
        tests/test_audio.c
        tests/test_bank.c
        tests/test_check_items_to_use.c
        tests/test_clear_memory.c
        tests/test_copy_data.c
        tests/test_dialog.c
        tests/test_entities.c
        tests/test_gameplay.c
        tests/test_gfx.c
        tests/test_lcd.c
        tests/test_link.c
        tests/test_room.c
        tests/test_ui.c
        tests/test_vfx.c
    )
    add_library(ladx_test_home_objs OBJECT ${LADX_TEST_HOME_SOURCES})

    # Bank 1 tests
    set(LADX_TEST_BANK1_SOURCES
        tests/test_bank1.c
        tests/bank1/test_face_shrine_mural.c
        tests/bank1/test_file_menu.c
        tests/bank1/test_game_over.c
        tests/bank1/test_intro.c
        tests/bank1/test_marin_beach.c
        tests/bank1/test_room_transition.c
        tests/bank1/test_save.c
        tests/bank1/test_siren_instruments.c
        tests/bank1/test_world_handler.c
        tests/bank1/test_world_map.c
    )
    add_library(ladx_test_bank1_objs OBJECT ${LADX_TEST_BANK1_SOURCES})

    # Bank 2 tests
    set(LADX_TEST_BANK2_SOURCES
        tests/test_bank2.c
        tests/bank2/test_audio.c
        tests/bank2/test_chest.c
        tests/bank2/test_clamp_item.c
        tests/bank2/test_func_60E0.c
        tests/bank2/test_falling.c
        tests/bank2/test_got_item.c
        tests/bank2/test_hookshot.c
        tests/bank2/test_link_animation.c
        tests/bank2/test_link_motion.c
        tests/bank2/test_magic_rod.c
        tests/bank2/test_ocarina.c
        tests/bank2/test_revolving_door.c
        tests/bank2/test_room.c
        tests/bank2/test_room_dispatch.c
        tests/bank2/test_room_effect_appearance.c
        tests/bank2/test_key_drop_effect.c
        tests/bank2/test_object_reveal.c
        tests/bank2/test_room_effects.c
        tests/bank2/test_room_events.c
        tests/bank2/test_room_triggers.c
        tests/bank2/test_shovel.c
        tests/bank2/test_shutter_effects.c
        tests/bank2/test_swimming.c
        tests/bank2/test_sword.c
        tests/bank2/test_tables.c
        tests/bank2/test_vfx.c
        tests/bank2/test_link_ground_physics.c
        tests/bank2/test_link_motion_helpers.c
        tests/bank2/test_room_transition.c
    )
    add_library(ladx_test_bank2_objs OBJECT ${LADX_TEST_BANK2_SOURCES})

    # Bank 3 tests
    set(LADX_TEST_BANK3_SOURCES
        tests/test_bank3.c
        tests/bank3/test_entities.c
        tests/bank3/test_entities_physics.c
        tests/bank3/test_entities_collision.c
        tests/bank3/test_entities_droppable.c
        tests/bank3/test_entities_bomb.c
        tests/bank3/test_entities_arrow.c
        tests/bank3/test_entities_moblin.c
        tests/bank3/test_entities_handlers.c
    )
    add_library(ladx_test_bank3_objs OBJECT ${LADX_TEST_BANK3_SOURCES})

    # Focused test executables
    add_executable(ladx_test_home
        tests/home_runner.c
        $<TARGET_OBJECTS:ladx_test_home_objs>
    )
    target_link_libraries(ladx_test_home PRIVATE ladx_core)

    add_executable(ladx_test_bank1
        tests/bank1_runner.c
        $<TARGET_OBJECTS:ladx_test_bank1_objs>
    )
    target_link_libraries(ladx_test_bank1 PRIVATE ladx_core)

    add_executable(ladx_test_bank2
        tests/bank2_runner.c
        $<TARGET_OBJECTS:ladx_test_bank2_objs>
        $<TARGET_OBJECTS:ladx_test_support_objs>
    )
    target_link_libraries(ladx_test_bank2 PRIVATE ladx_core)

    add_executable(ladx_test_bank3
        tests/bank3_runner.c
        $<TARGET_OBJECTS:ladx_test_bank3_objs>
        $<TARGET_OBJECTS:ladx_test_support_objs>
    )
    target_link_libraries(ladx_test_bank3 PRIVATE ladx_core)

    # Full unified test runner (runs all tests; supports CLI target filtering)
    add_executable(ladx_tests
        tests/test_runner.c
        $<TARGET_OBJECTS:ladx_test_home_objs>
        $<TARGET_OBJECTS:ladx_test_bank1_objs>
        $<TARGET_OBJECTS:ladx_test_bank2_objs>
        $<TARGET_OBJECTS:ladx_test_bank3_objs>
        $<TARGET_OBJECTS:ladx_test_support_objs>
    )
    target_link_libraries(ladx_tests PRIVATE ladx_core)

    # CTest registrations
    add_test(NAME ladx_unit_tests COMMAND ladx_tests)
    add_test(NAME test_home COMMAND ladx_test_home)
    add_test(NAME test_bank1 COMMAND ladx_test_bank1)
    add_test(NAME test_bank2 COMMAND ladx_test_bank2)
    add_test(NAME test_bank3 COMMAND ladx_test_bank3)
endif()
