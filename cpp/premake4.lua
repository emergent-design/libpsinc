solution "psinc"
	language		"C++"
	targetdir		"lib"
	configurations	"default"
	platforms		"native"
	includedirs		"include"
	libdirs			"lib"
	excludes		{ "**.bak", "**~" }

	project "libpsinc"
		kind				"SharedLib"
		targetname			"psinc"
		links				{ "emergent", "usb-1.0" }
		files				{ "include/psinc/**h", "src/psinc/**.cpp" }
		configuration "linux"
			postbuildcommands	"./strip lib/libpsinc.so"
		configuration "not vs*"
			flags			"Symbols"
			buildoptions	{ "-Wall", "-Wno-sign-compare", "-std=c++11", "-O3", "-D_FORTIFY_SOURCE=2" }
			linkoptions 	{ "-Wl,-soname,libpsinc.so.0" }
		configuration "vs*"
			kind "StaticLib"
			configuration "debug"
				flags "Symbols"
				targetname "psincd"
			configuration "release"
				flags "Optimize"

