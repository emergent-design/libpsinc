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
	excludes		{ "**.bak", "**~" }
	
	newoption {
	   trigger     = "iconograph",
	   description = "Enable the iconograph project, you must have pkgconfig and gtk+ 3.0 set up"
	}
	
	configuration "not vs*"
		flags			"Symbols"
		buildoptions	{ "-Wall", "-Wno-sign-compare", "-std=c++11", "-O3", "-D_FORTIFY_SOURCE=2" }

	project "libpsinc"
		kind				"SharedLib"
		targetname			"psinc"
		links				{ "emergent", "usb-1.0" }
		files				{ "include/psinc/**h", "src/psinc/**.cpp" }
		configuration "linux"
			postbuildcommands	"./strip lib/libpsinc.so"
		configuration "not vs*"
			linkoptions 		{ "-Wl,-soname,libpsinc.so.0" }
		configuration "vs*"
			kind "StaticLib"
	
	if _OPTIONS.iconograph then
		project				"iconograph"
		kind				"WindowedApp"
		targetdir			"bin"
		buildoptions		{ pkgconfig("cflags", "gtk+-3.0") }
		linkoptions			{ pkgconfig("libs", "gtk+-3.0") }
		links				{ "libpsinc", "emergent", "freeimage" }
		files				{ "include/iconograph/**h", "src/iconograph/**.cpp" }
		configuration "linux"
			postbuildcommands	"./strip bin/iconograph"
	end

