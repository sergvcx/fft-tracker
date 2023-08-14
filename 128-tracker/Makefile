

all:
	cd front && cmake -B ../build/front front -G "Visual Studio 15 2017 Win64" 
	cd back  && cmake -B ../build/back  back  -G "Visual Studio 15 2017 Win64" 
	cd model && cmake -B ../build/model model -G "Visual Studio 15 2017 Win64" 

mon:	
	cd vsmon && cmake -B ../build/vsmon vsmon -G "Visual Studio 15 2017 Win64" 
	
#	cd vsmon && cmake -B ../build/vsmon vsmon  -G "Visual Studio 16 2019" 