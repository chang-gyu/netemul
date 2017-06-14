workspace 'NetEmul'
	configurations 	{ 'debug', 'release' }

    includedirs     { 'sdk', 'include' }
	defines 		{ '__LINUX' }
	buildoptions    { '-g -fms-extensions -std=gnu99' }

project 'netemul'
	kind 'ConsoleApp'
    targetdir "."

	files 			{ 'src/**.c' }
	links 			{ 'umpn' }
	libdirs			{ '.' }
	prebuildcommands { "./src/mkver.sh > ./src/version.h" }
