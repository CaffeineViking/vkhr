function os.winSdk()
    local reg_arch = iif( os.is64bit(), "\\Wow6432Node\\", "\\" )
    local sdk_version = os.getWindowsRegistry( "HKLM:SOFTWARE" .. reg_arch .."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion" )
    if sdk_version ~= nil then return sdk_version else return "8" end -- Assumes that at least the Windows 8 SDK is installed somewhere...
end

function os.vpaths()
    files { "share/scenes/**.vkhr" }
    files { "share/shaders/**.glsl",
            "share/shaders/**.vert",
            "share/shaders/**.geom",
            "share/shaders/**.tesc",
            "share/shaders/**.tese",
            "share/shaders/**.frag",
            "share/shaders/**.comp" }
    vpaths {
        ["src/*"] = "src/**.cc",
        ["include/*"] = "include/**.hh",
        ["foreign/*"] = { "foreign/**.hpp",
                          "foreign/**.h",
                          "foreign/**.cpp",
                          "foreign/**.c" },
        ["scenes/*"]  = { "share/scenes/**.vkhr" },
        ["shaders/*"] = { "share/shaders/**.glsl",
                          "share/shaders/**.vert",
                          "share/shaders/**.geom",
                          "share/shaders/**.tesc",
                          "share/shaders/**.tese",
                          "share/shaders/**.frag",
                          "share/shaders/**.comp" }
    }
end

