require "vendor/premake/premake_scripts/ecc"

workspace "Axle"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Axle"
    location "Axle"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "axpch.hpp"
    pchsource "Axle/src/axpch.cpp"

    files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp", "%{prj.name}/src/**.h" }

    includedirs { 
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include" 
    }

    -- buildoptions { "/utf-8" }

    filter "action:vs*"
        buildoptions { "/utf-8" }

    filter "system:Windows"
        systemversion "latest"

        defines { "AX_PLATFORM_WINDOWS", "AX_BUILD_DLL" }

        postbuildcommands {
			("{MKDIR} ../bin/" .. outputdir .. "/Sandbox"),
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"),
			("{MKDIR} ../bin/" .. outputdir .. "/Tests"),
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Tests")
        }

    filter "configurations:Debug"
        defines "AX_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "AX_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "AX_DIST"
        optimize "On"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp", "%{prj.name}/src/**.h" }

    includedirs { "Axle/vendor/spdlog/include", "Axle/src" }

    links { "Axle" }

    -- buildoptions { "/utf-8" }

    filter "action:vs*"
        buildoptions { "/utf-8" }

    filter "system:Windows"
        systemversion "latest"

        defines { "AX_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines "AX_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "AX_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "AX_DIST"
        optimize "On"

project "Tests"
    location "Axle/tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Axle/tests/**.cpp",
        "Axle/vendor/doctest/doctest/doctest.h",
    }

    includedirs {
        "Axle/vendor/doctest/doctest",
        "Axle/src",
        "Axle/vendor/spdlog/include"
    }

    links { "Axle" }

    filter "action:vs*"
        buildoptions { "/utf-8" }

    filter "system:Windows"
        systemversion "latest"
        defines { "AX_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "AX_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "AX_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "AX_DIST" }
        optimize "On"
