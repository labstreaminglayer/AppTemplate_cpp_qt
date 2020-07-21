# CMakeLists.txt contains the build configuration that can be used to build
# your App without any IDE or to generate project files for your specific IDE.
# First, set the minimum required CMake version.
# Here, [CMake 3.10](https://cmake.org/cmake/help/latest/release/3.10.html)
# was chosen. Before setting this to a higher version, please check
# if it has arrived in
# debian stable https://packages.debian.org/stable/cmake
# and Ubuntu LTS https://packages.ubuntu.com/bionic/cmake
cmake_minimum_required(VERSION 3.10)

project(AppTemplate_cpp_qt
	LANGUAGES CXX
	VERSION 1.13.0
	)

# also look for CMake modules in the cmake subfolder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find an installed liblsl in paths set by the user (LSL_INSTALL_ROOT)
# and some default paths
find_package(LSL REQUIRED
	HINTS ${LSL_INSTALL_ROOT}
	"${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/install"
	"${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/out/install/x64-Release"
	PATH_SUFFIXES share/LSL)
get_filename_component(LSL_PATH ${LSL_CONFIG} DIRECTORY)
message(STATUS "Found LSL lib in ${LSL_PATH}")
LSLAPP_Setup_Boilerplate()

#: Then we can simply use find_package. This enables us to later link LSL::lsl
#: You can read more about [find_package](https://cmake.org/cmake/help/latest/command/find_package.html).

#: ## Finding vendor SDKs
#: 
#: More often than not, vendors offer a library file (`.so`, `.dylib`, `.dll`)
#: and a header file (`.h`). CMake can search for these files and create an
#: imported target, so all compilers get the correct settings for finding and linking them.
#:
#: Before you begin writing cmake code for the library, search the internet
#: (especially GitHub) for Find<Device>.cmake or Find<Vendor>.cmake
#: Be sure to look through the resources here: https://github.com/onqtam/awesome-cmake#modules
#: If found, then drop that file in your app repository's cmake folder.
#: Be sure to read that cmake file for further instructions, but typically
#: one would be required to set Vendor_ROOT_DIR to tell the package finder
#: where to look.
#: Then one could use `find_package(Vendor REQUIRED)` to import its variables.
#:
#: Note that some (older) find modules will set the following variables:
#:  Vendor_INCLUDE_DIRS -- Use this with target_include_dirs
#:  Vendor_LIBRARIES    -- Use this with target_link_libraries
#:
#: Whereas newer find modules will provide an imported target:
#: (https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#imported-targets)
#: Vendor::Device -- Use this with target_link_libraries.

find_package(Vendor QUIET)

# Find our custom package. But we set this to QUIET because it will fail.
# You should set yours to REQUIRED

#: ## Using Qt
#:
#: Qt is a popular-enough library that cmake already knows how to find it.
#: 
#: Nevertheless, it is often necessary in Windows to give cmake a hint where to
#: find it by adding the following to your cmake invocation:
#: -DQt5_Dir="C:/Qt/<version>/<platform>/lib/cmake/Qt5"
#: Where <version> is 5.something and <platform> is something like msvc2017_64
# If you are on MacOS and you installed Qt5 via homebrew, then use the following:
# -DQt5_DIR=$(brew --prefix qt5)/lib/cmake/Qt5
#:
#: Then, to import Qt, just call `find_package` with the components your app uses
#: (usually just `Widgets`).

#: Needed for ui_XY.h files
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#: Enable automatic compilation of .cpp->.moc, xy.ui->ui_xy.h and resource files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Native `std::thread`s still require a platform thread library.
# CMake can find and link to it with the `Threads` package (link your
# executable with `Threads::Threads` afterwards).
find_package(Threads REQUIRED)

# Add executable targets, the default target has the same name as the project

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32
	main.cpp
	mainwindow.cpp
	mainwindow.hpp
	mainwindow.ui
	reader.hpp
	reader.cpp
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Qt5::Widgets
	Threads::Threads
	LSL::lsl
#	Vendor::DeviceModule
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

# If using QML, uncomment the following line and update the path to the qml directory:
# set_target_properties(${PROJECT_NAME} PROPERTIES qml_directory "${CMAKE_CURRENT_LIST_DIR}/src/qml")

# Setting up deployment (let CPack generate a zip/dmg file)
installLSLApp(${PROJECT_NAME})
installLSLAuxFiles(${PROJECT_NAME}
	${PROJECT_NAME}.cfg
)
LSLGenerateCPackConfig()
