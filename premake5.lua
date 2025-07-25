require "vendor/premake/premake_scripts/ecc"

workspace "Axle"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Axle/vendor/GLFW/include"
IncludeDir["Glad"] = "Axle/vendor/Glad/include"

project "Glad"
	location "Axle/vendor/Glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    -- warnings "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Axle/vendor/Glad/src/gl.c",
        "Axle/vendor/Glad/include/glad/gl.h",
        "Axle/vendor/Glad/include/KHR/khrplatform.h"
    }

    includedirs {
        "Axle/vendor/Glad/include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "system:linux"
        pic "On"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "speed"

    filter "configurations:Dist"
        runtime "Release"
        optimize "speed"
        symbols "off"

project "GLFW"
    location "Axle/vendor/GLFW"
    kind "StaticLib"
    language "C"
    staticruntime "on"
	-- warnings "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Axle/vendor/GLFW/src/**.c",
        "Axle/vendor/GLFW/include/**.h"
    }

    includedirs {
        "Axle/vendor/GLFW/include"
    }

    filter "system:windows"
        systemversion "latest"
        defines { "_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS" }

        files {
            "Axle/vendor/GLFW/src/win32_*.c",
            "Axle/vendor/GLFW/src/wgl_context.c",
            "Axle/vendor/GLFW/src/egl_context.c",
            "Axle/vendor/GLFW/src/osmesa_context.c"
        }

    filter "system:linux"
        systemversion "latest"
        defines { "_GLFW_X11" }
		pic "On"

        files {
            "Axle/vendor/GLFW/src/x11_*.c",
            "Axle/vendor/GLFW/src/glx_context.c",
            "Axle/vendor/GLFW/src/egl_context.c",
            "Axle/vendor/GLFW/src/osmesa_context.c",
            "Axle/vendor/GLFW/src/posix_time.c",
            "Axle/vendor/GLFW/src/posix_thread.c"
        }

        links { "GL", "m", "dl", "X11", "pthread", "Xrandr", "Xi", "Xxf86vm", "Xcursor" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"

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
        "%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}"
    }

    -- buildoptions { "/utf-8" }
	
	links { "GLFW", "Glad" }

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

    filter "system:Linux"
        systemversion "latest"

        defines { "AX_PLATFORM_LINUX", "AX_BUILD_DLL" }

        postbuildcommands {
                        ("{MKDIR} ../bin/" .. outputdir .. "/Sandbox"),
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"),
                        ("{MKDIR} ../bin/" .. outputdir .. "/Tests"),
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Tests")
        }

    filter "configurations:Debug"
        defines { "AX_DEBUG", "AXLE_TESTING" }
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

    includedirs { 
		"Axle/vendor/spdlog/include", 
		"Axle/src", 
		"%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}" 
	}

    links { "Axle" }

    -- buildoptions { "/utf-8" }

    filter "action:vs*"
        buildoptions { "/utf-8" }

    filter "system:Windows"
        systemversion "latest"

        defines { "AX_PLATFORM_WINDOWS" }

    filter "system:Linux"
        systemversion "latest"

        defines { "AX_PLATFORM_LINUX" }

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
	
	defines { "AXLE_TESTING" }

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Axle/tests/**.cpp",
        "Axle/vendor/doctest/doctest/doctest.h",
    }

    includedirs {
        "Axle/vendor/doctest/doctest",
        "Axle/src",
        "Axle/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
    }

    links { "Axle" }

    filter "action:vs*"
        buildoptions { "/utf-8" }

    filter "system:Windows"
        systemversion "latest"
        defines { "AX_PLATFORM_WINDOWS" }

    filter "system:Linux"
        systemversion "latest"
        defines { "AX_PLATFORM_LINUX" }

    filter "configurations:Debug"
        defines { "AX_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "AX_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "AX_DIST" }
        optimize "On"
