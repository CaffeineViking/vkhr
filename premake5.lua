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

    filter "configurations:Release"
        defines  "RELEASE"
        optimize "Speed"
        symbols  "Off"

    floatingpoint "Fast"

    filter { "action:vs*" }
        include "utils/wsdk.lua"
        startproject "vkhr"
        platforms { "Win64" }
        -- Don't handle Win32 for now...
        -- Premake5 fails to detect SDK.
        systemversion(os.winSdk()..".0")
        filter "platforms:Win64"
            architecture "x86_64"

SDK  = "$(VULKAN_SDK)"

project (name)
    targetdir "bin"
    kind "ConsoleApp"

    includedirs "include"
    files { "include/**.hh" }
    files { "src/"..name.."/**.cc" }
    files { "src/vkpp/**.cc" }
    files "src/main.cc"

    os.vpaths() -- Virtual paths.

    -- For header-only libraries.
    includedirs "foreign/include"
    includedirs "foreign/imgui"
    files { "foreign/imgui/*.h",
            "foreign/imgui/imgui_draw.cpp",
            "foreign/imgui/imgui_widgets.cpp",
            "foreign/imgui/imgui.cpp" }
    includedirs "foreign/json/include"
    includedirs "foreign/tinyobjloader"
    files { "foreign/tinyobjloader/tiny_obj_loader.cc",
            "foreign/tinyobjloader/tiny_obj_loader.h" }
    includedirs "foreign/stb"
    files { "foreign/stb/*.h" }
    files { "foreign/glm/glm/**.hpp" }
    includedirs "foreign/glm"

    filter { "system:windows", "action:gmake" }
        links { "glfw3", "vulkan" }
    filter { "system:windows", "action:vs*" }
        links { SDK.."/lib/vulkan-1.lib" }
        includedirs { SDK.."/include" }
        includedirs { "foreign/glfw" }
        files { "foreign/glfw/GLFW/*.h" }
        links { "foreign/glfw/glfw3.lib" }
    filter "system:linux or bsd or solaris"
        links { "glfw",  "vulkan" }
