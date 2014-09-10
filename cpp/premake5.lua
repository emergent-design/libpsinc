solution "psinc"
	language		"C++"
	targetdir		"lib"
	includedirs		"include"
	libdirs			{ "lib" }
	excludes		{ "**.bak", "**~" }
	
	configuration "not vs*"
		configurations  { "default" }
		platforms		"native"
	configuration "vs*"
		configurations  { "debug", "release" }
		platforms		{ "x32", "x64" }

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
			kind		"StaticLib"
			defines		"NOMINMAX"
			targetname	"psinc_%{cfg.buildcfg}_%{cfg.platform}"
			
			configuration "debug"
				flags		"Symbols"
			configuration "release"
				optimize	"Full"
