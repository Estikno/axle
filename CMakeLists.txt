cmake_minimum_required(VERSION 3.25)

project(Axle)

# ── Global settings ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Force x64 on Visual Studio (Unix generators default to host arch and don't support this variable)
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_GENERATOR_PLATFORM x64 CACHE STRING "" FORCE)
endif()

# ── Output directories (mirrors Premake's bin/<cfg>-<os>-x64/<project>) ────────
set(OUTPUT_BASE "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64")

# Per-target output dirs are overridden in each CMakeLists to put each project
# in its own subfolder, just like Premake did.

# ── Build configurations ───────────────────────────────────────────────────────
# CMake single-config generators (Makefiles, Ninja): set a default if none given
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Expose the three configurations to multi-config generators (VS, Xcode)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

# ── MSVC UTF-8 source encoding (replaces buildoptions { "/utf-8" }) ────────────
if(MSVC)
    add_compile_options(/utf-8)
endif()

# ── Subprojects ────────────────────────────────────────────────────────────────
add_subdirectory(Axle)
add_subdirectory(Sandbox)

# Tests only exist for Debug builds.
# For multi-config generators we add the directory and let the Tests
# CMakeLists.txt exclude itself from non-Debug configs.
add_subdirectory(Axle/tests)

