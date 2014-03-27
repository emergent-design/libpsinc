solution "psinc"
	language		"C++"
	targetdir		"lib"
	flags			"Symbols"
	configurations	"default"
	platforms		"native"
	includedirs		"include"
	libdirs			"lib"
	buildoptions	{ "-Wall", "-Wno-sign-compare", "-std=c++11", "-O3", "-fPIC", "-D_FORTIFY_SOURCE=2" }
	excludes		{ "**.bak", "**~" }

	project "libpsinc"
		kind				"SharedLib"
		targetname			"psinc"
		links				"emergent"
		postbuildcommands	"./strip lib/libpsinc.so"
		linkoptions 		{ "-Wl,-soname,libpsinc.so.0", "`pkg-config --libs libusb-1.0`" }
		files				{ "include/psinc/**h", "src/psinc/**.cpp" }

	project "iconograph"
		kind				"WindowedApp"
		targetdir			"bin"
		buildoptions		"`pkg-config --cflags gtk+-3.0`"
		linkoptions			"`pkg-config --libs gtk+-3.0`"
		postbuildcommands	"./strip bin/iconograph"
		links				{ "libpsinc", "emergent", "freeimageplus" }
		files				{ "include/iconograph/**h", "src/iconograph/**.cpp" }

