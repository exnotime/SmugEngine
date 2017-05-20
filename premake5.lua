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
        defines { "AS_USE_NAMESPACE" }
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/Core/**"}
		includedirs { "include", "src" }
		links {  "Graphics", "AssetLoader", "Physics", "glfw3" }
        configuration { "Debug" }
                links { "angelscript64d", "as_integrationD" }
        configuration { "Release" }
                links { "angelscript64", "as_integration" }

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

    project "Physics"
        targetname "Physics"
        defines { "PHYSICS_EXPORT"}
        debugdir ""
        location (location_path)
        language("C++")
        kind "SharedLib"
        files { "src/Physics/**"}
        includedirs { "include", "src" }
        configuration { "Debug" }
            links { "PhysX3DEBUG_x64", "PhysX3CommonDEBUG_x64.lib", "PxFoundationDEBUG_x64", "PhysX3ExtensionsDEBUG", "PxPvdSDKDEBUG_x64"}
        configuration { "Release" }
            links { "PhysX3CHECKED_x64", "PhysX3CommonCHECKED_x64.lib", "PxFoundationCHECKED_x64", "PhysX3ExtensionsCHECKED"}
        
