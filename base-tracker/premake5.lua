#!lua

-- A solution contains projects, and defines the available configurations
solution "base-tracker-x64"
	configurations { "Debug", "Release" }
	platforms { "x64"}

   -- A project defines one build target
   project "base-tracker-x64"
      kind "ConsoleApp"
      language "C++"
    files {"*.cpp"}
	libdirs {"$(VSHELL)/lib","$(IPP)/lib", "$(IPP)/stublib","$(NMPP)/lib"}
	includedirs { "$(VSHELL)/include","$(IPP)/include","$(NMPP)/include"}
	links { "vshell.lib","nmpp-x64.lib","ippi.lib","ipps.lib","ippcore.lib"}


      configuration "Debug"
         defines { "DEBUG" }
         symbols  "On" 

      configuration "Release"
         defines { "NDEBUG" }
         symbols  "Off" 

		  