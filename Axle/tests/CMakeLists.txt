# ══════════════════════════════════════════════════════════════════════════════
#  Axle/tests/CMakeLists.txt
#  Tests only build in Debug configuration.
# ══════════════════════════════════════════════════════════════════════════════

# For single-config generators (Ninja, Make) skip entirely if not Debug
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Tests: skipping (not a Debug build)")
    return()
endif()

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp"
)

add_executable(Tests ${TEST_SOURCES})

target_include_directories(Tests
    PRIVATE
        "${AXLE_DOCTEST_INCLUDE}"   # set by Axle/CMakeLists.txt
        "${CMAKE_SOURCE_DIR}/Axle/src"
        ${AXLE_VENDOR_INCLUDES}     # set by Axle/CMakeLists.txt
)

target_compile_definitions(Tests
    PRIVATE
        AXLE_TESTING
        AX_DEBUG
)

# Platform defines
if(WIN32)
    target_compile_definitions(Tests PRIVATE AX_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(Tests PRIVATE AX_PLATFORM_LINUX)
endif()

target_link_libraries(Tests PRIVATE Axle)

set_target_properties(Tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Tests"
)

# For multi-config generators (Visual Studio): exclude Tests from Release & Dist
# by making it produce no output in those configs
set_target_properties(Tests PROPERTIES
    EXCLUDE_FROM_ALL $<OR:$<CONFIG:Release>,$<CONFIG:Dist>>
)
