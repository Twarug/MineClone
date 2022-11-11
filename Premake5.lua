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
    group ("")

project("MineClone")
    location ("MineClone")
    kind ("ConsoleApp")
    language ("C++")
    cppdialect ("C++latest")
    staticruntime ("on")

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    pchheader ("mcpch.h")
    pchsource ("%{prj.name}/src/mcpch.cpp")
    
    files ({
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.tpp",
        "%{prj.name}/out/**",
        "Premake5.lua"
    })
    
    defines ({
		"_CRT_SECURE_NO_WARNINGS"
	})

    includedirs ({ 
        "%{prj.name}/src", -- project
        "%{prj.name}/vendor/glfw/include", -- GLFW 
    })

    links ({
        "GLFW"
    })

    debugdir ("run")
    
    filter ("system:windows")
		systemversion ("latest")
		
	filter ("configurations:Debug")
		runtime ("Debug")
		symbols ("On")
        defines ({
            "NDEBUG"
        })

	filter ("configurations:Release")
		runtime ("Release")
		optimize ("on")

    filter ("configurations:Dist")
        runtime ("Release")
        optimize ("on")
        defines ({
            "APP_DISTRIBUTION"
        })