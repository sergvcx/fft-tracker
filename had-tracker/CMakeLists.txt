cmake_minimum_required(VERSION 3.14)
project(tracker VERSION 0.1.0)

set(NMPP 	$ENV{NMPP})
set(IPP 	$ENV{IPP})
set(VSHELL 	$ENV{VSHELL})
set(DEV  	set(VSHELL 	$ENV{VSHELL}))

include_directories(${IPP}/include  ${NMPP}/include  ${VSHELL}/include c:/git/nmc-dev/include)
link_directories   (${IPP}/lib      ${NMPP}/lib      ${VSHELL}/lib    )
link_libraries(nmpp-x64d.lib vshell.lib ippi.lib  ipps.lib ippcore.lib)

add_executable(had-tracker 	main.cpp c:/git/nmc-dev/common/hadamard_init.cpp c:/git/nmc-dev/common/hadamard.cpp c:/git/nmc-dev/pc/transpose.cpp c:/git/nmc-dev/pc/mulmm_2col.cpp)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT had-tracker)

#add_executable(compress compress.cpp)
#add_executable(decompress decompress.cpp)
#add_subdirectory(test)
