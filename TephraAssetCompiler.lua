solution "TephraAssetCompiler"
    configurations { "Debug", "Release" }
        flags{ "NoPCH" }
        local vulkan_dir = os.getenv("VULKAN_SDK");
        libdirs { "lib", vulkan_dir .. "/bin", vulkan_dir .. "/lib", "lib/physx" }
        includedirs { "include", vulkan_dir .. "/include"}
        platforms{"x64" }

        local location_path = "solution"
        if _ACTION then
	        defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX"  }
            location_path = location_path .. "/" .. _ACTION
            location ( location_path )
            location_path = location_path .. "/projects"
        end

    configuration { "Debug" }
        defines { "DEBUG" }
        symbols "On"
        targetdir ( "bin/" .. "/debug" )

    configuration { "Release" }
        defines { "NDEBUG", "RELEASE" }
        optimize "on"
        floatingpoint "fast"
        targetdir ( "bin/" .. "/release" )


	project "AssetCompiler"
        targetname "TephraAssetCompiler"
		debugdir ""
        defines { "AS_USE_NAMESPACE" }
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/AssetCompiler/**"}
		includedirs { "include", "src" }
		links { "AssetLoader", "Utility", "vulkan-1" }
        configuration { "Debug" }
                links { "angelscript64d", "as_integrationD", "shaderc_combinedD" }
        configuration { "Release" }
                links { "angelscript64", "as_integration", "shaderc_combined" }

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
        
