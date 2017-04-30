solution "Tephra"
    configurations { "Debug", "Release" }
        flags{ "Unicode", "NoPCH" }
        local vulkan_dir = os.getenv("VULKAN_SDK");
        libdirs { "lib", vulkan_dir .. "/bin", vulkan_dir .. "/lib" }
        includedirs { "include", vulkan_dir .. "/include"}
        platforms{"x64" }

        local location_path = "solution"
        if _ACTION then
	        defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX"  }
            location_path = location_path .. "/" .. _ACTION
            location ( location_path )
            location_path = location_path .. "/projects"
        end
    
    disablewarnings { "4251" }

    configuration { "Debug" }
        defines { "DEBUG" }
        flags { "Symbols" }
        targetdir ( "bin/" .. "/debug" )

    configuration { "Release" }
        defines { "NDEBUG", "RELEASE" }
        flags { "Optimize", "FloatFast" }
        targetdir ( "bin/" .. "/release" )


	project "Core"
        targetname "Tephra"
		debugdir ""
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/Core/**"}
		includedirs { "include", "src" }
		links {  "Graphics", "AssetLoader", "glfw3" }

    project "Graphics"
    	targetname "Graphics"
    	defines { "GRAPHICS_EXPORT", "VK_USE_PLATFORM_WIN32_KHR"}
    	debugdir ""
    	location (location_path)
    	language("C++")
    	files { "src/Graphics/**"}
    	includedirs { "include", "src" }
    	kind "SharedLib"
    	links { "vulkan-1", "AssetLoader"}
    	configuration { "Debug" }
                links { "shaderc_combinedD" }
        configuration { "Release" }
                links { "shaderc_combined" }

    project "AssetLoader"
    	targetname "AssetLoader"
    	defines { "ASSET_EXPORT"}
    	debugdir ""
    	location (location_path)
    	language("C++")
    	files { "src/Assetloader/**"}
    	includedirs { "include", "src" }
        links { "assimp" }
    	kind "SharedLib"
