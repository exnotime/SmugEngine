solution "SmugEngine"
    configurations { "Debug", "Release" }
        flags{ "NoPCH" }
        local vulkan_dir = os.getenv("VULKAN_SDK");
        libdirs { "lib", vulkan_dir .. "/bin", vulkan_dir .. "/lib", "lib/physx", "lib/spirv_cross" }
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
        symbols "On"
        targetdir ( "bin/" .. "/debug" )

    configuration { "Release" }
        defines { "NDEBUG", "RELEASE" }
        optimize "on"
        floatingpoint "fast"
        targetdir ( "bin/" .. "/release" )


	project "Core"
        targetname "SmugEngine"
		debugdir ""
        defines { "AS_USE_NAMESPACE", "USE_IMGUI","GLM_FORCE_DEPTH_ZERO_TO_ONE"  }
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/Core/**", "src/Imgui/**"}
		includedirs { "include", "src" }
		links {  "Graphics", "AssetLoader", "Physics", "glfw3", "vulkan-1", "Utility" }
        configuration { "Debug" }
                links { "angelscript64d", "as_integrationD", "shaderc_combinedD" }
        configuration { "Release" }
                links { "angelscript64", "as_integration", "shaderc_combined" }

    project "Graphics"
    	targetname "Graphics"
    	defines { "VK_USE_PLATFORM_WIN32_KHR", "USE_IMGUI", "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
    	debugdir ""
    	location (location_path)
    	language("C++")
    	files { "src/Graphics/**", "src/Imgui/**"}
    	includedirs { "include", "src" }
    	kind "StaticLib"
    	links { "vulkan-1", "AssetLoader", "glfw3", "Utility"}
    	configuration { "Debug" }
                links { "shaderc_combinedD", "spirv-cross-coreD", "spirv-cross-glslD"  }
        configuration { "Release" }
                links { "shaderc_combined", "spirv-cross-core", "spirv-cross-glsl"}

    project "AssetLoader"
    	targetname "AssetLoader"
    	defines { "ASSET_EXPORT"}
    	debugdir ""
    	location (location_path)
    	language("C++")
    	files { "src/Assetloader/**"}
    	includedirs { "include", "src" }
        kind "SharedLib"
        links { "assimp", "Utility" }
        configuration { "Debug" }
                links { "angelscript64d", "as_integrationD", "shaderc_combinedD", "spirv-cross-coreD", "spirv-cross-glslD"  }
        configuration { "Release" }
                links { "angelscript64", "as_integration", "shaderc_combined", "spirv-cross-core", "spirv-cross-glsl" }

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

    project "Utility"
        targetname "Utility"
        defines {}
        debugdir ""
        location (location_path)
        language("C++")
        files { "src/Utility/**"}
    	includedirs { "include", "src" }
        links { }
    	kind "StaticLib"
        
