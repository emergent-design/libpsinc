solution "psinc"
	language		"C++"
	targetdir		"lib"
	includedirs		"include"
	libdirs			{ "lib" }
	excludes		{ "**.bak", "**~" }

	configuration "linux"
		toolset "clang"
	configuration "not vs*"
		configurations  { "default" }
		platforms		"native"
	configuration "vs*"
		configurations  { "debug", "release" }
		platforms		{ "x32", "x64" }

	project "libpsinc"
		kind				"SharedLib"
		targetname			"psinc"
		links				{ "usb-1.0", "freeimage" }
		files				{ "include/psinc/**h", "src/psinc/**.cpp", "src/psinc-c/**.cpp" }

		configuration "linux"
			flags				"Symbols"
			postbuildcommands	"./strip lib/libpsinc.so"

		configuration "not vs*"
			buildoptions	{ "-Wall", "-Wno-sign-compare", "-std=c++14", "-O3", "-D_FORTIFY_SOURCE=2" }
			linkoptions 	{ "-Wl,-soname,libpsinc.so.0" }

		configuration "vs*"
			kind		"StaticLib"
			defines		"NOMINMAX"
			targetname	"psinc_%{cfg.buildcfg}_%{cfg.platform}"

			configuration "debug"
				flags		"Symbols"
			configuration "release"
				optimize	"Full"
