cmake_minimum_required(VERSION 3.14)
project(back VERSION 0.1.0)

set(NMPP 	$ENV{NMPP})
set(IPP 	$ENV{IPP})
set(VSHELL 	$ENV{VSHELL})
set(DTP 	$ENV{DTP})
set(HAL 	$ENV{HAL})
set(DEV  	set(VSHELL 	$ENV{VSHELL}))

include_directories(${DTP}/include 	${IPP}/include  ${NMPP}/include  ${VSHELL}/include c:/git/nmc-dev/include ${HAL}/include ../../include)
link_directories   (${DTP}/lib 		${IPP}/lib      ${NMPP}/lib      ${VSHELL}/lib    )
link_libraries(dtp-x64.lib nmpp-x64d.lib vshell.lib ippi.lib  ipps.lib ippcore.lib)

add_executable(back 	main0.cpp main1.cpp)
#set_property(TARGET MyTarget PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

configure_file ("./Makefile"  				"Makefile" )
configure_file ("./Makefile0"  				"Makefile0" )
configure_file ("./Makefile1"  				"Makefile1" )
configure_file ("./mc12101-nmpu0.lds" 		"mc12101-nmpu0.lds" )
configure_file ("./mc12101-nmpu1.lds" 		"mc12101-nmpu1.lds" )
	
	
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT back)



#add_executable(compress compress.cpp)
#add_executable(decompress decompress.cpp)
#add_subdirectory(test)
