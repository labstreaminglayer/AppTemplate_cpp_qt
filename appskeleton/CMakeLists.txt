cmake_minimum_required(VERSION 3.5)
project(BestPracticesGUI
	LANGUAGES CXX
	VERSION 0.1)

# set up LSL if not done already
if(NOT TARGET LSL::lsl)
	# when building out of tree LSL_ROOT needs to be specified on the cmd line
	file(TO_CMAKE_PATH "${LSL_INSTALL_ROOT}" LSL_INSTALL_ROOT)
	list(APPEND LSL_INSTALL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/install")
	find_package(LSL HINTS ${LSL_INSTALL_ROOT}/share/LSL/ ${LSL_INSTALL_ROOT}/LSL/share/LSL QUIET)
	if(NOT LSL_FOUND)
		message(FATAL_ERROR "Precompiled LSL was not found. See https://github.com/labstreaminglayer/labstreaminglayer/blob/master/doc/BUILD.md#lsl_install_root for more information.")
	endif()
	list(APPEND CMAKE_MODULE_PATH ${LSL_DIR})
	message(STATUS "Looking for LSLCMake in ${LSL_DIR}")
	include(LSLCMake)
endif()

# create an imported target we later add the required information to
#add_library(vendorsdk SHARED IMPORTED)
# create an imported target we later add the required information to
#add_library(vendorsdk SHARED IMPORTED)
# find the vendorsdk_research.h header in the subfolder vendorsdk/include
#find_path(vendorsdk_SDK_INCLUDE_PATH
#	name "vendorsdk.h"
#	PATHS "vendorsdk/include"
#)
# find a shared library called e.g. vendorsdk_research.lib or libvendorsdk_research.so
# in vendorsdk/lib
#find_library(vendorsdk_SDK_LIB vendorsdk
#	PATHS "vendorsdk/lib"
#)
#if(NOT vendorsdk_SDK_LIB)
#	message(FATAL_ERROR "vendorsdk SDK library not found")
#endif()
# Windows needs additional help
#get_filename_component(libext vendorsdk_SDK_LIB EXT)
#if(libext STREQUAL ".lib")
#	set_target_properties(vendorsdk PROPERTIES IMPORTED_IMPLIB ${vendorsdk_SDK_LIB})
#	string(REPLACE ".lib" ".dll" vendorsdk_SDK_LIB ${vendorsdk_SDK_LIB})
#endif()
# add the paths we just found to the vendorsdk library target
#set_target_properties(vendorsdk PROPERTIES
#	INTERFACE_INCLUDE_DIRECTORIES ${vendorsdk_SDK_INCLUDE_PATH}
#	IMPORTED_LOCATION ${vendorsdk_SDK_LIB}
#)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
	sophisticated_recording_device.h
	sophisticated_recording_device.cpp
)
target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Qt5::Widgets
	Threads::Threads
	LSL::lsl
#	vendorsdk
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_auto_type cxx_lambda_init_captures)

installLSLApp(${PROJECT_NAME})
installLSLAuxFiles(${PROJECT_NAME}
	${PROJECT_NAME}.cfg
)
LSLGenerateCPackConfig()
