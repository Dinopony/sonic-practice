cmake_minimum_required(VERSION 3.20.5)

project(sonic_practice VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

macro(WRAPPED_DEPENDENCY)
    message(STATUS "Defining wrapped dependency for ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}")
    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}.hxx"
            COMMAND "${CMAKE_COMMAND}"
            ARGS -DINPUT_FILE="${ARGV1}" -DVARIABLE_NAME="${ARGV2}" -DPROCESS_TYPE="${ARGV0}" -P "./wrap_file.cmake"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endmacro()

wrapped_dependency(RAW "assets/gui_tileset.bin" GUI_TILESET)

set(SOURCES
        "assets/gui_tileset.bin.hxx"

        # --- Megadrive Tools ----------------------------------------
        "src/md_tools/code.cpp"
        "src/md_tools/code.hpp"
        "src/md_tools/md_tools.hpp"
        "src/md_tools/rom.cpp"
        "src/md_tools/rom.hpp"
        "src/md_tools/types.hpp"

        # --- Patches ----------------------------------------
        "src/patches/game_patch_s3k.cpp"
        "src/patches/game_patch.hpp"

        # --- Tools ----------------------------------------
        "src/tools/argument_dictionary.hpp"
        "src/tools/bitstream_reader.cpp"
        "src/tools/bitstream_reader.hpp"
        "src/tools/bitstream_writer.hpp"
        "src/tools/byte_array.hpp"
        "src/tools/stringtools.hpp"
        "src/tools/vectools.hpp"

        # --- Project root ----------------------------------------
        "src/main.cpp"
        src/patches/game_patch_s3k.hpp src/patches/common_gui.cpp src/patches/common_gui.h src/ui/ui_string.hpp src/ui/ui_string.cpp src/ui/ui_info.hpp)

add_executable(sonic_practice "${SOURCES}")
