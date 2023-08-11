#!lua

-- A solution contains projects, and defines the available configurations
solution "tracker-host-x86"
   configurations { "Debug", "Release" }

   -- A project defines one build target
   project "tracker-host-x86"
      kind "ConsoleApp"
      language "C++"
      files {"../src_host/*.*"}
	  libdirs { "$(VSHELL)/lib","$(NMPP)/lib", "$(HAL)/lib","$(IPP)/lib", "$(IPP)/stublib"}
	  includedirs { "$(HAL)/include","$(VSHELL)/include","$(NMPP)/include","$(HAL)/include", "../../include","$(IPP)/include"}
	  links { "vshell.lib","nmpp-x86.lib", "hal-virtual-x86.lib","ippi.lib","ipps.lib","ippcore.lib"}
	  -- targetdir (".") 
      configuration "Debug"
         defines { "DEBUG" }
         symbols  "On" 

      configuration "Release"
         defines { "NDEBUG" }
         symbols  "Off" 
		 
		 
		 
solution "tracker-target0-x86"
   configurations { "Debug", "Release" }
   --platforms{"x64","x86"}

   -- A project defines one build target
   project "tracker-target0-x86"
      kind "ConsoleApp"
      language "C++"
      files { "../src_target0/*.*", "../../src_proc0/pc/*.*","../../src_proc0/common/*.*","../../include/*.h","../../src/pc-sim/*.*" }
	  libdirs {"$(NMPP)/lib", "$(HAL)/lib"}
	  includedirs { "$(HAL)/include","$(NMPP)/include","../../include"}
	  links { "nmpp-x86.lib", "hal-virtual-x86.lib"}
	  
      configuration "Debug"
         defines { "DEBUG" }
         symbols  "On" 
		 

      configuration "Release"
         defines { "NDEBUG" }
         symbols  "Off" 
		 
solution "tracker-target1-x86"
   configurations { "Debug", "Release" }
   --platforms{"x64","x86"}

   -- A project defines one build target
   project "tracker-target1-x86"
      kind "ConsoleApp"
      language "C++"
      files { "../src_target1/*.*", "../../src_proc1/pc/*.*","../../src_proc1/common/*.*","../../include/*.h" }
	  libdirs {"$(NMPP)/lib", "$(HAL)/lib"}
	  includedirs { "$(HAL)/include","$(NMPP)/include","../../include"}
	  links { "nmpp-x86.lib", "hal-virtual-x86.lib"}

      configuration "Debug"
         defines { "DEBUG" }
         symbols  "On" 
		 

      configuration "Release"
         defines { "NDEBUG" }
         symbols  "Off" 
		 
		 
		 
