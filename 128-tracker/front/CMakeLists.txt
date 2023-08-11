cmake_minimum_required(VERSION 3.14)
project(front-tracker VERSION 0.1.0)

set(NMPP 	$ENV{NMPP})
set(IPP 	$ENV{IPP})
set(VSHELL 	$ENV{VSHELL})
set(DTP 	$ENV{DTP})
set(MC12101	$ENV{MC12101})
set(DEV  	set(VSHELL 	$ENV{VSHELL}))

include_directories(${DTP}/include 	${IPP}/include  ${NMPP}/include  ${VSHELL}/include c:/git/nmc-dev/include ${MC12101}/include)
link_directories   (${DTP}/lib 		${IPP}/lib      ${NMPP}/lib      ${VSHELL}/lib    ${MC12101}/lib)
link_libraries(dtp-x64.lib nmpp-x64d.lib vshell.lib ippi.lib  ipps.lib ippcore.lib mc12101load.lib)

add_executable(front-tracker 		front.cpp )

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT front-tracker)

#add_executable(compress compress.cpp)
#add_executable(decompress decompress.cpp)
#add_subdirectory(test)
