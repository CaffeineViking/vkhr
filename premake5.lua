name = "vkhr"
workspace (name)
    language   "C++"
    location   "build"
    warnings   "Extra"
    cppdialect "C++17"

    configurations { "Debug",
                     "Release" }

    filter "configurations:Debug"
        defines  "DEBUG"
        optimize "Off"
        symbols  "On"

    floatingpoint "Fast"

    filter "configurations:Release"
        defines  "RELEASE"
        optimize "Speed"
        symbols  "Off"

    filter { "action:vs*" }
        include "utils/wsdk.lua"
        startproject "vkhr"
        platforms { "Win32", "Win64" }
        -- Premake5 fails to detect SDK.
        systemversion(os.winSdk()..".0")
        filter "platforms:Win32"
            architecture "x86"
        filter "platforms:Win64"
            architecture "x86_64"

SDK = "$(VULKAN_SDK)"

project (name)
    targetdir "bin"
    kind "ConsoleApp"

    includedirs "include"
    files { "include/**.hh" }
    files { "src/"..name.."/**.cc" }
    files "src/main.cc"

    files { "share/scenes/**.vkhr" }
    files { "share/shader/**.glsl",
            "share/shader/**.vert",
            "share/shader/**.geom",
            "share/shader/**.tesc",
            "share/shader/**.tese",
            "share/shader/**.frag",
            "share/shader/**.comp" }

    vpaths {
        ["src/*"] = "src/**.cc",
        ["include/*"] = "include/**.hh",
        ["scenes/*"]  = { "share/scenes/**.vkhr" },
        ["shaders/*"] = { "share/shader/**.glsl",
                          "share/shader/**.vert",
                          "share/shader/**.geom",
                          "share/shader/**.tesc",
                          "share/shader/**.tese",
                          "share/shader/**.frag",
                          "share/shader/**.comp" }
    }

    -- For header-only libraries.
    includedirs "foreign/include"
    includedirs "foreign/glm"

    filter { "system:windows", "action:gmake" }
        links { "glfw3", "vulkan" }
    filter { "system:windows", "action:vs*" }
        links { SDK.."/lib/vulkan-1.lib" }
        includedirs { SDK.."/include" }
    filter "system:linux or bsd or solaris"
        links { "glfw",  "vulkan" }
