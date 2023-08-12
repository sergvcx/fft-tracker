ROOT = ..
include $(ROOT)/global.mk
# BUILD AND RUN MAKEFILE
# This makefile builds simple nmc-application containing single cpp-file  
# Rule 'run' executes the application on MC12101 board and stores return code in ".exitcode"  file

.SUFFIXES:


ifndef MC12101
$(warning *** WARNING: 'MC12101' environment variable is not defined! Board SDK might not have installed correctly. )
endif 



TARGET  = test-x64

VS ?= vs2017 
#PATH:=$(MC12101)/bin;$(PATH)
all:
	cmake -B build -A x64 

#$(TARGET) 

$(TARGET): $(TARGET).vcxproj

vs2015:	$(TARGET).vcxproj
	"$(VS140COMNTOOLS)vsvars32" && msbuild $(HOST).vcxproj /p:Configuration=Debug

vs2017:   
	cmake -B build -G "Visual Studio 15 2017 Win64"
#	premake5 vs2017


$(TARGET).vcxproj:
	premake5 vs2015


run: $(TARGET)
	bin\debug\$(TARGET)
	
include $(ROOT)/clean.mk