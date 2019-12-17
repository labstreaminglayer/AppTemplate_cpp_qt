
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
