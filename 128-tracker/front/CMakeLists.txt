cmake_minimum_required(VERSION 3.14)
project(front-tracker VERSION 0.1.0)

set(NMPP 	$ENV{NMPP})
set(IPP 	$ENV{IPP})
set(VSHELL 	$ENV{VSHELL})
set(HAL 	$ENV{HAL})
set(DTP 	$ENV{DTP})
set(MC12101	$ENV{MC12101})
set(DEV  	set(VSHELL 	$ENV{VSHELL}))
set(CMAKE_DEBUG_POSTFIX d)
include_directories(${DTP}/include 	${IPP}/include  ${NMPP}/include  ${VSHELL}/include c:/git/nmc-dev/include ${MC12101}/include ../../include ${HAL}/include)
link_directories   (${DTP}/lib 		${IPP}/lib      ${NMPP}/lib      ${VSHELL}/lib    ${MC12101}/lib)
link_libraries( nmpp-x64d.lib vshell.lib ippi.lib  ipps.lib ippcore.lib mc12101load.lib
    optimized dtp-x64.lib
	debug dtp-x64d.lib
)
add_executable(${PROJECT_NAME} 	sobel.cpp	front.cpp )
set_target_properties(${PROJECT_NAME} 
    PROPERTIES
        DEBUG_POSTFIX d
        # RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
        # RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
        # RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        )
		


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})


#add_executable(compress compress.cpp)
#add_executable(decompress decompress.cpp)
#add_subdirectory(test)
