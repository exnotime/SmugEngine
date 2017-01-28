solution "Tephra"
    configurations { "Debug", "Release" }
        flags{ "Unicode", "NoPCH" }
        local vulkan_dir = os.getenv("VULKAN_SDK");
        libdirs { "lib", vulkan_dir .. "/bin"  }
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
        flags { "Symbols" }
        targetdir ( "bin/" .. "/debug" )

    configuration { "Release" }
        defines { "NDEBUG", "RELEASE" }
        flags { "Optimize", "FloatFast" }
        targetdir ( "bin/" .. "/release" )

	project "Tephra"
        targetname "Tephra"
		debugdir ""
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/**"}
		includedirs { "include", "src" }
		links { "glfw3", "vulkan-1" }
        configuration { "Debug" }
                links { "shaderc_combinedD" }
        configuration { "Release" }
                links { "shaderc_combined" }
