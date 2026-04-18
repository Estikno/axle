# ══════════════════════════════════════════════════════════════════════════════
#  Sandbox/CMakeLists.txt
# ══════════════════════════════════════════════════════════════════════════════

file(GLOB_RECURSE SANDBOX_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.h"
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_include_directories(Sandbox
    PRIVATE
        "${CMAKE_SOURCE_DIR}/Axle/src"
        ${AXLE_VENDOR_INCLUDES}   # set by Axle/CMakeLists.txt
)

target_compile_definitions(Sandbox
    PRIVATE
        $<$<CONFIG:Debug>:AX_DEBUG>
        $<$<CONFIG:Release>:AX_RELEASE>
        $<$<CONFIG:Dist>:AX_DIST>
)

# Platform defines
if(WIN32)
    target_compile_definitions(Sandbox PRIVATE AX_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(Sandbox PRIVATE AX_PLATFORM_LINUX)
endif()

target_link_libraries(Sandbox PRIVATE Axle)

set_target_properties(Sandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Sandbox"
)
