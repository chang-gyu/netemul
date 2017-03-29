workspace 'NetEmul'
	configurations 	{ 'debug', 'release' }

    includedirs     { 'sdk', 'include', 'librpc' }
	defines 		{ '__LINUX' }
	buildoptions    { '-fms-extensions -std=gnu99' }

project "rpc_netemul"
    kind 'StaticLib'
    targetdir "librpc"

    files           { 'librpc/**.c' }
    defines         { 'LINUX' }

project 'netemul'
	kind 'ConsoleApp'
    targetdir "."

	files 			{ 'src/**.c' }
	buildoptions 	{ '-fms-extensions -std=gnu99' }
	defines			{ 'NET_CONTROL'}
	links 			{ 'umpn', 'rpc_netemul' }
	libdirs			{ '.', 'librpc' }
