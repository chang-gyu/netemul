workspace 'NetEmul'
		configurations { 'debug', 'release' }

project 'netemul'
	kind 'ConsoleApp'
    targetdir "."

	files 			{ 'src/**.c' }
	includedirs 	{ 'sdk', 'include', 'librpc' }
	buildoptions 	{ '-fms-extensions -std=gnu99' }
	defines			{ '__LINUX' }
	links 			{ 'umpn', 'rpc_netemul' }
	libdirs			{ '.', 'librpc' }
