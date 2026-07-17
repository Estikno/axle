cmake_minimum_required(VERSION 3.25)

project(Axle)

# ── Global settings ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# ── Architecture guard ─────────────────────────────────────────────────────────
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)")
        set(AX_ARCH "arm64")
    else()
        set(AX_ARCH "x64")
    endif()
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(AX_ARCH "x86")
else()
    set(AX_ARCH "unknown")
endif()

if(NOT AX_ARCH STREQUAL "x64")
    message(FATAL_ERROR "Axle only supports x64. Detected architecture: ${AX_ARCH}")
endif()

# ── Sanitizers ─────────────────────────────────────────────────────────────────
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

if(ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)

if(ENABLE_TSAN)
    if(MSVC)
        message(WARNING "Thread Sanitizer is not supported on MSVC")
    else()
        add_compile_options(-fsanitize=thread)
        add_link_options(-fsanitize=thread)
    endif()
endif()

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ENABLE_ASAN and ENABLE_TSAN cannot be enabled at the same time")
endif()

# ── Compile commands for clangd ────────────────────────────────────────────────
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Flatbuffers (root vendor, shared between Axle and AAP) ────────────────────
set(FLATBUFFERS_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_INSTALL          OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATHASH   OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATLIB    OFF CACHE BOOL "" FORCE) # header-only, no lib needed
set(FLATBUFFERS_BUILD_CPP_SAMPLE OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/flatbuffers")

# Export the include path and flatc target for all subprojects
set(INC_FLATBUFFERS "${CMAKE_SOURCE_DIR}/vendor/flatbuffers/include" CACHE INTERNAL "")

# ── Flatbuffers schema generation (shared between Axle and AAP) ───────────────
file(GLOB_RECURSE ALL_SCHEMAS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/assets/schemas/**.fbs")

if(ALL_SCHEMAS)
    set(FLATBUFFERS_FLATC_SCHEMA_EXTRA_ARGS "--gen-mutable" "--gen-object-api")

    build_flatbuffers(
        "${ALL_SCHEMAS}"
        ""
        flatbuffers_generated        # target name
        ""
        "${CMAKE_BINARY_DIR}/generated"  # shared output dir
        ""
        ""
    )

    set(HAS_SCHEMAS ON CACHE INTERNAL "")
else()
    set(HAS_SCHEMAS OFF CACHE INTERNAL "")
endif()

# ── Subprojects ────────────────────────────────────────────────────────────────
add_subdirectory(Axle)
add_subdirectory(Sandbox)
add_subdirectory(AAP)

# Tests only exist for Debug builds.
# For multi-config generators we add the directory and let the Tests
# CMakeLists.txt exclude itself from non-Debug configs.
add_subdirectory(Axle/tests)
