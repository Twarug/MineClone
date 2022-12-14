if os.getenv("VULKAN_SDK") == nil then
    print("You need to install Vulkan SDK, or set %VULKAN_SDK% env varialbe.")
    os.exit(-1)
end

workspace("MineClone")
architecture("x86_64")
startproject("MineClone")

configurations({
    "Debug",
    "Release",
    "Dist"
})

flags({
    "MultiProcessorCompile"
})

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group("Vendors")
    include("MineClone/vendor/premake5.lua")
group("")

project("MineClone")
    location("MineClone")
    kind("ConsoleApp")
    language("C++")
    cppdialect("C++latest")
    staticruntime("on")
    
    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")
    
    pchheader("mcpch.h")
    pchsource("%{prj.name}/src/mcpch.cpp")
    
    files({
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.tpp",
        "run/**",
        "Premake5.lua"
    })
    
    defines({
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_VULKAN"
    })
    
    includedirs({
        "%{prj.name}/src", -- project
        "%{prj.name}/vendor/glfw/include", -- GLFW
        "%{prj.name}/vendor/glm", -- glm
        "%{prj.name}/vendor/stb", -- stb
        "%{prj.name}/vendor/ImGUI", -- ImGUI
        os.getenv("VULKAN_SDK") .. "/Include", -- Vulkan
    })
    
    links({
        "GLFW", -- GLFW
        "ImGUI", -- ImGUI
        os.getenv("VULKAN_SDK") .. "/Lib/vulkan-1.lib", -- Vulkan
    })
    
    debugdir("run")
    
    filter("system:windows")
    systemversion("latest")
    
    filter("configurations:Debug")
        runtime("Debug")
        symbols("On")
        defines({
            "NDEBUG"
        })
    
    filter("configurations:Release")
        runtime("Release")
        optimize("on")
    
    filter("configurations:Dist")
        runtime("Release")
        optimize("on")
        defines({
            "APP_DISTRIBUTION"
        })