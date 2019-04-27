solution "SmugEngine"
    configurations { "Debug", "Release" }
        flags{ "NoPCH" }
        local vulkan_dir = os.getenv("VULKAN_SDK");
        libdirs { "lib", vulkan_dir .. "/bin", vulkan_dir .. "/lib", "lib/spirv_cross" }
        includedirs { "include", vulkan_dir .. "/include"}
        platforms{"x64" }

        local location_path = "solution"
        if _ACTION then
	        defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX"  }
            location_path = location_path .. "/" .. _ACTION
            location ( location_path )
            location_path = location_path .. "/projects"
        end
    newoption{
        trigger = "rtx-on",
        description = "Compile with ray-tracing enabled"
    }
    disablewarnings { "4251" }
    --global defines
    defines{"AS_USE_NAMESPACE"}

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
        defines { "USE_IMGUI","GLM_FORCE_DEPTH_ZERO_TO_ONE", "AS_USE_NAMESPACE" }
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/Core/**", "src/Imgui/**"}
		includedirs { "include", "src" }
		links {  "Graphics", "AssetLoader", "Physics", "glfw3", "Utility", "Script"}

    project "Graphics"
    	targetname "Graphics"
    	defines { "VK_USE_PLATFORM_WIN32_KHR", "USE_IMGUI", "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
    	debugdir ""
    	location (location_path)
    	language("C++")
    	files { "src/Graphics/**", "src/Imgui/**"}
    	includedirs { "include", "src" }
    	kind "StaticLib"
    	links {"AssetLoader", "glfw3", "Utility"}
    	configuration { "Debug" }
            links { "spirv-cross-coreD", "spirv-cross-glslD", "angelscript64d", "as_integrationD" }
        configuration { "Release" }
            links { "spirv-cross-core", "spirv-cross-glsl", "angelscript64", "as_integration" }
        configuration { "rtx-on" }
            defines { "RTX_ON" }

    project "AssetLoader"
    	targetname "AssetLoader"
    	defines { "ASSET_EXPORT" }
    	debugdir ""
    	location (location_path)
    	language("C++")
    	files { "src/Assetloader/**"}
    	includedirs { "include", "src" }
        kind "StaticLib"
        links { "assimp", "Utility", "Script" }
        configuration { "Debug" }
                links { "spirv-cross-coreD", "spirv-cross-glslD", "angelscript64d", "as_integrationD"  }
        configuration { "Release" }
                links { "spirv-cross-core", "spirv-cross-glsl", "angelscript64", "as_integration" }

    project "Physics"
        targetname "Physics"
        defines { "PHYSICS_EXPORT", "PX_PHYSX_CHARACTER_STATIC_LIB"}
        debugdir ""
        location (location_path)
        language("C++")
        kind "SharedLib"
        files { "src/Physics/**"}
        includedirs { "include", "src", "include/PhysX4.0" }
        links { "Utility" }
        configuration { "Debug" }
        	libdirs{ "lib/physx4.0/debug" }
            links { "PhysX_64", "PhysXCommon_64", "PhysXFoundation_64", "PhysXExtensions_static_64", "PhysXCharacterKinematic_static_64", "PhysXCooking_64", "PhysXPvdSDK_static_64"}
        configuration { "Release" }
        	libdirs{ "lib/physx4.0/checked" }
            links { "PhysX_64", "PhysXCommon_64", "PhysXFoundation_64", "PhysXExtensions_static_64", "PhysXCharacterKinematic_static_64", "PhysXCooking_64"}
            
    project "Script"
        targetname "Script"
        defines { "AS_USE_NAMESPACE", "SCRIPT_EXPORT" }
        debugdir ""
        location (location_path)
        language("C++")
        files { "src/AngelScript/**"}
        includedirs { "include", "src" }
        kind "StaticLib"
        links { "Utility"}
        configuration { "Debug" }
                links { "angelscript64d", "as_integrationD"}
        configuration { "Release" }
                links { "angelscript64", "as_integration"}
        

    project "Utility"
        targetname "Utility"
        defines {}
        debugdir ""
        location (location_path)
        language("C++")
        files { "src/Utility/**"}
    	includedirs { "include", "src" }
        links { "EASTL"}
    	kind "StaticLib"
        
