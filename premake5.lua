solution "psinc"
	language		"C++"
	targetdir		"lib"
	includedirs		"include"
	libdirs			{ "lib" }
	excludes		{ "**.bak", "**~" }

	filter "system:linux"
		toolset "clang"
	filter "action:not vs*"
		configurations  { "default" }
		platforms		"native"
	filter "action:vs*"
		configurations  { "debug", "release" }
		platforms		{ "x32", "x64" }

	project "libpsinc"
		kind				"SharedLib"
		targetname			"psinc"
		links				{ "usb-1.0", "freeimage" }
		files				{ "include/psinc/**h", "src/psinc/**.cpp", "src/psinc-c/**.cpp" }

		filter "system:linux"
			symbols	"On"

		filter "system:windows"
			kind "StaticLib"

		filter "action:not vs*"
			buildoptions	{ "-Wall", "-Wno-sign-compare", "-std=c++17", "-O3", "-D_FORTIFY_SOURCE=2" }
			linkoptions 	{ "-Wl,-soname,libpsinc.so.0" }

		filter "action:vs*"
			kind		"StaticLib"
			defines		"NOMINMAX"
			targetname	"psinc_%{cfg.buildcfg}_%{cfg.platform}"

			filter "configurations:debug"
				symbols		"On"
			filter "configurations:release"
				optimize	"Full"
