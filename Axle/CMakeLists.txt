# ══════════════════════════════════════════════════════════════════════════════
#  Axle/CMakeLists.txt
#  Defines: Glad, GLFW, ImGui (vendored static libs) + Axle (shared lib)
# ══════════════════════════════════════════════════════════════════════════════

# ── Vendor include paths (used by multiple targets below) ─────────────────────
set(VENDOR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

set(INC_GLFW    "${VENDOR}/GLFW/include")
set(INC_GLAD    "${VENDOR}/Glad/include")
set(INC_SPDLOG  "${VENDOR}/spdlog/include")
set(INC_GLM     "${VENDOR}/glm")
set(INC_IMGUI   "${VENDOR}/imgui")
set(INC_DOCTEST "${VENDOR}/doctest/doctest")

# Make these available to Sandbox and Tests via a cached variable
set(AXLE_VENDOR_INCLUDES
    "${INC_GLFW}"
    "${INC_GLAD}"
    "${INC_SPDLOG}"
    "${INC_GLM}"
    CACHE INTERNAL "Vendor include dirs needed by Axle consumers")

set(AXLE_DOCTEST_INCLUDE "${INC_DOCTEST}" CACHE INTERNAL "")

# ══════════════════════════════════════════════════════════════════════════════
#  Glad  (C static lib)
# ══════════════════════════════════════════════════════════════════════════════
add_library(Glad STATIC
    "${VENDOR}/Glad/src/gl.c"
    "${VENDOR}/Glad/include/glad/gl.h"
    "${VENDOR}/Glad/include/KHR/khrplatform.h"
)

target_include_directories(Glad PUBLIC "${INC_GLAD}")

set_target_properties(Glad PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Glad"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Glad"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Glad"
)

# PIC required on Linux so it can be linked into the Axle shared lib
if(UNIX)
    set_target_properties(Glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# ══════════════════════════════════════════════════════════════════════════════
#  GLFW  (C static lib)
# ══════════════════════════════════════════════════════════════════════════════

# GLFW shared sources (needed on all platforms)
set(GLFW_COMMON_SOURCES
    "${VENDOR}/GLFW/src/context.c"
    "${VENDOR}/GLFW/src/init.c"
    "${VENDOR}/GLFW/src/input.c"
    "${VENDOR}/GLFW/src/monitor.c"
    "${VENDOR}/GLFW/src/vulkan.c"
    "${VENDOR}/GLFW/src/window.c"
    # GLFW 3.4+: platform dispatcher and null (headless) backend
    "${VENDOR}/GLFW/src/platform.c"
    "${VENDOR}/GLFW/src/null_init.c"
    "${VENDOR}/GLFW/src/null_joystick.c"
    "${VENDOR}/GLFW/src/null_monitor.c"
    "${VENDOR}/GLFW/src/null_window.c"
)

# Collect platform-specific GLFW sources
if(WIN32)
    file(GLOB GLFW_PLATFORM_SOURCES
        "${VENDOR}/GLFW/src/win32_*.c"
        "${VENDOR}/GLFW/src/wgl_context.c"
        "${VENDOR}/GLFW/src/egl_context.c"
        "${VENDOR}/GLFW/src/osmesa_context.c"
    )
    set(GLFW_PLATFORM_DEFINES _GLFW_WIN32 _CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    file(GLOB GLFW_PLATFORM_SOURCES
        "${VENDOR}/GLFW/src/x11_*.c"
        "${VENDOR}/GLFW/src/glx_context.c"
        "${VENDOR}/GLFW/src/egl_context.c"
        "${VENDOR}/GLFW/src/osmesa_context.c"
        "${VENDOR}/GLFW/src/posix_time.c"
        "${VENDOR}/GLFW/src/posix_thread.c"
        # GLFW 3.4+ additional sources
        "${VENDOR}/GLFW/src/posix_module.c"   # _glfwPlatformLoadModule / GetModuleSymbol / FreeModule
        "${VENDOR}/GLFW/src/posix_poll.c"     # _glfwPollPOSIX
        "${VENDOR}/GLFW/src/xkb_unicode.c"    # _glfwKeySym2Unicode
        "${VENDOR}/GLFW/src/linux_joystick.c" # _glfwInitJoysticksLinux etc.
    )
    set(GLFW_PLATFORM_DEFINES _GLFW_X11)
endif()

add_library(GLFW STATIC ${GLFW_COMMON_SOURCES} ${GLFW_PLATFORM_SOURCES})

target_include_directories(GLFW PUBLIC "${INC_GLFW}")
target_compile_definitions(GLFW PRIVATE ${GLFW_PLATFORM_DEFINES})

set_target_properties(GLFW PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/GLFW"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/GLFW"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/GLFW"
)

if(UNIX)
    set_target_properties(GLFW PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_link_libraries(GLFW PUBLIC GL m dl X11 pthread Xrandr Xi Xxf86vm Xcursor)
endif()

# ══════════════════════════════════════════════════════════════════════════════
#  ImGui  (C++20 static lib)
# ══════════════════════════════════════════════════════════════════════════════
file(GLOB IMGUI_SOURCES
    "${VENDOR}/imgui/*.cpp"
    "${VENDOR}/imgui/*.h"
    "${VENDOR}/imgui/backends/imgui_impl_glfw.cpp"
    "${VENDOR}/imgui/backends/imgui_impl_glfw.h"
    "${VENDOR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${VENDOR}/imgui/backends/imgui_impl_opengl3.h"
    "${VENDOR}/imgui/misc/cpp/imgui_stdlib.cpp"
    "${VENDOR}/imgui/misc/cpp/imgui_stdlib.h"
)

add_library(ImGui STATIC ${IMGUI_SOURCES})

target_include_directories(ImGui PUBLIC "${INC_IMGUI}" "${INC_GLFW}")

set_target_properties(ImGui PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/ImGui"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/ImGui"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/ImGui"
)

if(WIN32)
    # Match Premake's staticruntime "On" for ImGui on Windows
    set_target_properties(ImGui PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(UNIX)
    set_target_properties(ImGui PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# ══════════════════════════════════════════════════════════════════════════════
#  Axle  (C++20 shared lib)
# ══════════════════════════════════════════════════════════════════════════════
file(GLOB_RECURSE AXLE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.h"
)

add_library(Axle SHARED ${AXLE_SOURCES})

target_include_directories(Axle
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${INC_SPDLOG}"
        "${INC_GLFW}"
        "${INC_GLAD}"
        "${INC_GLM}"
        "${INC_IMGUI}"
)

target_compile_definitions(Axle
    PRIVATE
        GLFW_INCLUDE_NONE
        AX_BUILD_DLL
        # Config-specific defines via generator expressions
        $<$<CONFIG:Debug>:AX_DEBUG>
        $<$<CONFIG:Debug>:AXLE_TESTING>
        $<$<CONFIG:Release>:AX_RELEASE>
        $<$<CONFIG:Dist>:AX_DIST>
)

# Platform defines
if(WIN32)
    target_compile_definitions(Axle PRIVATE AX_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(Axle PRIVATE AX_PLATFORM_LINUX)
endif()

if(UNIX)
    # On Linux, static libs linked into a shared lib need --whole-archive
    # otherwise the linker silently drops unreferenced symbols
    target_link_libraries(Axle PUBLIC
        -Wl,--whole-archive GLFW Glad ImGui -Wl,--no-whole-archive
    )
else()
    target_link_libraries(Axle PUBLIC GLFW Glad ImGui)
endif()

# ── Precompiled header ─────────────────────────────────────────────────────────
target_precompile_headers(Axle PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/axpch.hpp"
)

# ── Output directory ──────────────────────────────────────────────────────────
set_target_properties(Axle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Axle"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Axle"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Axle"
)

# ── Post-build: copy the Axle DLL/SO into Sandbox and Tests output dirs ────────
set(AXLE_LIB_OUTPUT "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Axle")
set(SANDBOX_OUTPUT  "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Sandbox")
set(TESTS_OUTPUT    "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Tests")

add_custom_command(TARGET Axle POST_BUILD
    # Ensure destination dirs exist
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SANDBOX_OUTPUT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${TESTS_OUTPUT}"
    # Copy the built shared library file
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Axle>"
        "${SANDBOX_OUTPUT}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Axle>"
        "${TESTS_OUTPUT}"
    COMMENT "Copying Axle shared lib to Sandbox and Tests output dirs"
)

# ── Config-specific optimisation flags ────────────────────────────────────────
target_compile_options(Axle PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<CONFIG:Release,Dist>>:-O2>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release,Dist>>:/O2>
)

