# ══════════════════════════════════════════════════════════════════════════════
#  AAP/CMakeLists.txt  —  Axle Asset Pipeline
#  Console application. Depends on Flatbuffers (headers + flatc).
#  Independent from the engine — does NOT link Axle.
# ══════════════════════════════════════════════════════════════════════════════

file(GLOB_RECURSE AAP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.h"
)

add_executable(AAP ${AAP_SOURCES})

target_include_directories(AAP
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${INC_FLATBUFFERS}"
        # Generated flatbuffers headers land here at build time
        "${CMAKE_BINARY_DIR}/generated"
)

target_compile_definitions(AAP
    PRIVATE
        $<$<CONFIG:Debug>:AAP_DEBUG>
        $<$<CONFIG:Release>:AAP_RELEASE>
        $<$<CONFIG:Dist>:AAP_DIST>
)

if(WIN32)
    target_compile_definitions(AAP PRIVATE AX_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(AAP PRIVATE AX_PLATFORM_LINUX)
endif()

set_target_properties(AAP PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/AAP"
)

# First generate all flatbuffers binaries and headers
if(HAS_SCHEMAS)
    add_dependencies(AAP  flatbuffers_generated)
endif()
