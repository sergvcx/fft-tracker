#!lua

-- A solution contains projects, and defines the available configurations
solution "tracker-mc12101"
   configurations { "Debug", "Release" }

   -- A project defines one build target
   project "mc12101-x86"
      kind "ConsoleApp"
      language "C++"
    files {"../src_host/*.cpp","../src_host/*.c"}
	libdirs { "$(MC12101)/lib","$(VSHELL)/lib","$(NMPP)/lib","$(HAL)/lib"}
	includedirs { "$(MC12101)/include","$(HAL)/include","$(VSHELL)/include","$(NMPP)/include","../../include"}
	links { "mc12101load.lib","vshell.lib","nmpp-x86.lib","hal-mc12101-x86.lib","mc12101-nmc4-0","mc12101-nmc4-1"}
    targetdir (".") 
      configuration "Debug"
         defines { "DEBUG" }
         symbols  "On" 

      configuration "Release"
         defines { "NDEBUG" }
         symbols  "Off" 
		 -- "mc12101-nmc4-0"}
		 
		 
	project "mc12101-nmc4-0"
      kind "Makefile"
      files { "../src_target0/*.*", "../../src_proc0/nm/*.*","../../src_proc0/common/*.*", "mc12101brd-nmc0.cfg", "Makefile0" }
	  includedirs { "$(MC12101)/include","$(HAL)/include","$(NMPP)/include","../../include"}
	 
	  configuration "Debug"
		   buildcommands {"make DEBUG=y -f Makefile0"}
		   rebuildcommands {"make -B DEBUG=y -f Makefile0"}
		   cleancommands {"make clean"}
		   
	  configuration "Release"
		   buildcommands {"make -f Makefile0"}
		   rebuildcommands {"make -B -f Makefile0"}
		   cleancommands {"make clean"}		   
		   
	project "mc12101-nmc4-1"
      kind "Makefile"
      files { "../src_target1/*.*", "../../src_proc1/nm/*.*","../../src_proc1/common/*.*", "mc12101brd-nmc1.cfg", "Makefile1" }
	  includedirs { "$(MC12101)/include","$(HAL)/include","$(NMPP)/include","../../include","$(NMPROFILER)/include"}
	 
	  configuration "Debug"
		   buildcommands {"make DEBUG=y -f Makefile1"}
		   rebuildcommands {"make -B DEBUG=y -f Makefile1"}
		   cleancommands {"make clean"}
		   
	  configuration "Release"
		   buildcommands {"make -f Makefile1"}
		   rebuildcommands {"make -B -f Makefile1"}
		   cleancommands {"make clean"}		   
		   
		  