workspace "Librpc"
    configurations { "Debug", "Release" }

project "rpc_netemul"
    kind 'StaticLib'
    targetdir "."

    files           { '**.c' }
    includedirs     { '../sdk', '../include', '../librpc' }
    buildoptions    { '-fms-extensions -std=gnu99' }
    defines         { 'LINUX', '__LINUX'}

