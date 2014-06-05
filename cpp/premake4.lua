local function pkgconfig(opt, lib)
    local cmdStream = assert(io.popen("pkg-config --" .. opt .. " " .. lib))
    local cmdOutput = assert(cmdStream:read("*l"))
    cmdStream:close()
    return cmdOutput;
end

solution "psinc"
	language		"C++"
	targetdir		"lib"
	configurations	"default"
	platforms		"native"
	includedirs		"include"
	libdirs			"lib"
	buildoptions	{ "-Wall", "-Wno-sign-compare", "-std=c++11", "-O3", "-fPIC", "-D_FORTIFY_SOURCE=2" }
	excludes		{ "**.bak", "**~" }
	
	configuration "linux"
		flags	"Symbols"

	project "libpsinc"
		kind				"SharedLib"
		targetname			"psinc"
		links				{ "emergent", "usb-1.0" }
		linkoptions 		{ "-Wl,-soname,libpsinc.so.0" }
		files				{ "include/psinc/**h", "src/psinc/**.cpp" }
		configuration "linux"
			postbuildcommands	"./strip lib/libpsinc.so"

	project "iconograph"
		kind				"WindowedApp"
		targetdir			"bin"
		buildoptions		{ pkgconfig("cflags", "gtk+-3.0") }
		linkoptions			{ pkgconfig("libs", "gtk+-3.0") }
		links				{ "libpsinc", "emergent", "freeimage" }
		files				{ "include/iconograph/**h", "src/iconograph/**.cpp" }
		configuration "linux"
			postbuildcommands	"./strip bin/iconograph"

