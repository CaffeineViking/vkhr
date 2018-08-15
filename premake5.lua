name = "vkhr"
workspace (name)
    language "C++"
    location "build"
    warnings "Extra"
    cppdialect "C++17"
    configurations { "Debug",
                     "Release" }

    startproject "vkhr"

    filter "configurations:Debug"
        floatingpoint "Strict"
        defines "DEBUG"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        floatingpoint "Fast"
        defines "RELEASE"
        optimize "Speed"
        symbols "Off"

SDK = "$(VULKAN_SDK)"

project (name)
    targetdir "bin"
    kind "WindowedApp"

    files "src/main.cc"
    files "foreign/src/**.c"
    files "foreign/src/**.cpp"
    files { "src/"..name.."/**.cc" }

    includedirs "foreign/include"
    includedirs "foreign/glm"
    includedirs "include"

    filter { "system:windows", "action:gmake" }
        links { "glfw3", "vulkan" }
    filter { "system:windows", "action:vs*" }
        links { SDK.."/lib/vulkan-1.lib" }
        includedirs { SDK.."/include" }
    filter "system:linux or bsd or solaris"
        links { "glfw3", "vulkan" }
