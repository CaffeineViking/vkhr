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
        debugdir "." -- project root.
        -- Don't handle Win32 for now...
        -- Premake5 fails to detect SDK.
        systemversion(os.winSdk()..".0")
        filter "platforms:Win64"
            architecture "x86_64"

SDK  = "$(VULKAN_SDK)"
GLFW = "foreign/glfw/"

-- With MinGW x64, always static link with the default C++ stdlib.
STATIC_LINK = "-static -static-libstdc++ -static-libgcc -lpthread"

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
            "foreign/imgui/examples/imgui_impl_vulkan.h",
            "foreign/imgui/examples/imgui_impl_vulkan.cpp",
            "foreign/imgui/examples/imgui_impl_glfw.cpp",
            "foreign/imgui/examples/imgui_impl_glfw.h",
            "foreign/imgui/imgui.cpp" }
    includedirs "foreign/imgui/examples"
    includedirs "foreign/json/include"
    includedirs "foreign/tinyobjloader"
    files { "foreign/tinyobjloader/tiny_obj_loader.cc",
            "foreign/tinyobjloader/tiny_obj_loader.h" }
    includedirs "foreign/stb"
    files { "foreign/stb/*.h" }
    files { "foreign/glm/glm/**.hpp" }
    includedirs "foreign/glm"

    filter { "system:windows", "action:gmake" }
        links { SDK.."/lib/vulkan-1" }
        linkoptions { STATIC_LINK }
        includedirs { GLFW }
        linkoptions { "-mwindows" }
        links { GLFW.."/lib-mingw-w64/glfw3" }
    filter { "system:windows", "action:vs*" }
        links { SDK.."/lib/vulkan-1.lib" }
        includedirs { SDK.."/include" }
        includedirs { GLFW }
        files { GLFW.."/GLFW/*.h" }
        links { GLFW.."/lib-vc2015/glfw3.lib" }
    filter "system:linux or bsd or solaris"
        links { "glfw",  "vulkan" }
