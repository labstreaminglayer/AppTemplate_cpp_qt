# =============================================================================
# LSL Application Template - CMakeLists.txt
# =============================================================================
# This is the reference template for building Lab Streaming Layer applications.
# It demonstrates:
#   - 4-tier liblsl discovery (source, install_root, system, FetchContent)
#   - CLI and GUI application separation with shared core library
#   - Qt6 integration
#   - Cross-platform packaging with CPack
#   - macOS entitlements for network capabilities
#
# Copy this template to start a new LSL application.
# =============================================================================

cmake_minimum_required(VERSION 3.28)

# CMP0177: install() DESTINATION paths are normalized (CMake 3.31+)
if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif()

project(LSLTemplate
    VERSION 2.0.0
    DESCRIPTION "LSL Application Template"
    HOMEPAGE_URL "https://github.com/labstreaminglayer/AppTemplate_cpp_qt"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Build Options
# =============================================================================
option(LSLTEMPLATE_BUILD_GUI "Build the GUI application (requires Qt6)" ON)
option(LSLTEMPLATE_BUILD_CLI "Build the CLI application" ON)

# =============================================================================
# LSL Discovery Options
# =============================================================================
# Priority 1: Build from local source (for parallel liblsl development)
set(LSL_SOURCE_DIR "" CACHE PATH "Path to liblsl source directory")

# Priority 2: Use explicit installation path
set(LSL_INSTALL_ROOT "" CACHE PATH "Path to installed liblsl")

# Priority 3: System installation (searched automatically)

# Priority 4: Fetch from GitHub if not found
option(LSL_FETCH_IF_MISSING "Fetch liblsl from GitHub if not found locally" ON)
# TODO: Change back to version tag (e.g., "v1.16.2") once apple_framework branch is merged
set(LSL_FETCH_REF "cboulay/apple_framework" CACHE STRING "liblsl git ref to fetch (tag, branch, or commit)")

# =============================================================================
# Find/Fetch liblsl
# =============================================================================
if(LSL_SOURCE_DIR)
    # Priority 1: Build from local source (parallel development)
    if(NOT EXISTS "${LSL_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "LSL_SOURCE_DIR set to '${LSL_SOURCE_DIR}' but no CMakeLists.txt found there")
    endif()
    message(STATUS "Using local liblsl source: ${LSL_SOURCE_DIR}")
    add_subdirectory("${LSL_SOURCE_DIR}" liblsl_bin EXCLUDE_FROM_ALL)
    if(NOT TARGET LSL::lsl)
        add_library(LSL::lsl ALIAS lsl)
    endif()
    set(LSL_FOUND TRUE)
    include("${LSL_SOURCE_DIR}/cmake/LSLCMake.cmake")
else()
    # Priority 2 & 3: Try to find installed liblsl
    set(_lsl_hints)
    if(LSL_INSTALL_ROOT)
        list(APPEND _lsl_hints "${LSL_INSTALL_ROOT}")
    endif()
    # Common development layout hints (including CLion cmake-build-* directories)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _build_type_lower)
    if(NOT _build_type_lower)
        set(_build_type_lower "release")
    endif()
    if(MSVC)
        set(_clion_build_dir "cmake-build-${_build_type_lower}-visual-studio")
    else()
        set(_clion_build_dir "cmake-build-${_build_type_lower}")
    endif()
    foreach(_root IN ITEMS "../liblsl" "../../LSL/liblsl")
        foreach(_build IN ITEMS "build" "${_clion_build_dir}")
            list(APPEND _lsl_hints "${CMAKE_CURRENT_LIST_DIR}/${_root}/${_build}/install")
        endforeach()
    endforeach()

    set(_lsl_suffixes
        share/LSL
        lib/cmake/LSL
        cmake
        Frameworks/lsl.framework/Resources/CMake  # macOS framework layout
    )

    # First try: Search only in hints (prefer local builds over system)
    find_package(LSL QUIET
        HINTS ${_lsl_hints}
        PATH_SUFFIXES ${_lsl_suffixes}
        NO_DEFAULT_PATH
    )

    # Second try: Search system paths if not found in hints
    if(NOT LSL_FOUND)
        find_package(LSL QUIET
            PATH_SUFFIXES ${_lsl_suffixes}
        )
    endif()

    if(LSL_FOUND)
        message(STATUS "Found installed liblsl: ${LSL_DIR}")
        include("${LSL_DIR}/LSLCMake.cmake")
    elseif(LSL_FETCH_IF_MISSING)
        # Priority 4: Fetch from GitHub
        message(STATUS "liblsl not found locally, fetching ${LSL_FETCH_REF} from GitHub...")
        include(FetchContent)
        # Disable liblsl extras we don't need
        set(LSL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(LSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
        # EXCLUDE_FROM_ALL prevents liblsl's install rules from running
        # (we handle liblsl bundling ourselves)
        FetchContent_Declare(liblsl
            GIT_REPOSITORY https://github.com/sccn/liblsl.git
            GIT_TAG ${LSL_FETCH_REF}
            GIT_SHALLOW ON
            EXCLUDE_FROM_ALL
        )
        FetchContent_MakeAvailable(liblsl)
        if(NOT TARGET LSL::lsl)
            add_library(LSL::lsl ALIAS lsl)
        endif()
        set(LSL_FOUND TRUE)
        include("${liblsl_SOURCE_DIR}/cmake/LSLCMake.cmake")
        message(STATUS "liblsl fetched and configured")
    else()
        message(FATAL_ERROR
            "liblsl not found. Options:\n"
            "  1. Set LSL_SOURCE_DIR to liblsl source directory\n"
            "  2. Set LSL_INSTALL_ROOT to installed liblsl location\n"
            "  3. Install liblsl system-wide\n"
            "  4. Enable LSL_FETCH_IF_MISSING=ON to auto-fetch from GitHub"
        )
    endif()
endif()

# =============================================================================
# Qt6 (for GUI build only)
# =============================================================================
if(LSLTEMPLATE_BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
    # Note: targeting lastest LTS release according to https://wiki.qt.io/QtReleasing

    # macOS: Work around deprecated AGL framework issue in some Qt6 builds
    if(APPLE AND TARGET WrapOpenGL::WrapOpenGL)
        get_target_property(_wrap_gl_libs WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
        if(_wrap_gl_libs)
            list(FILTER _wrap_gl_libs EXCLUDE REGEX ".*AGL.*")
            set_target_properties(WrapOpenGL::WrapOpenGL PROPERTIES INTERFACE_LINK_LIBRARIES "${_wrap_gl_libs}")
        endif()
    endif()
endif()

# =============================================================================
# Common Dependencies
# =============================================================================
find_package(Threads REQUIRED)

# =============================================================================
# RPATH Configuration (must be set before targets are created)
# =============================================================================
LSL_configure_rpath()

# =============================================================================
# Targets
# =============================================================================

# Core library (Qt-independent, shared between CLI and GUI)
add_subdirectory(src/core)

# CLI application
if(LSLTEMPLATE_BUILD_CLI)
    add_subdirectory(src/cli)
endif()

# GUI application
if(LSLTEMPLATE_BUILD_GUI)
    add_subdirectory(src/gui)
endif()

# =============================================================================
# Installation
# =============================================================================
include(GNUInstallDirs)

# Platform-specific install directories
if(WIN32)
    set(INSTALL_BINDIR ".")
    set(INSTALL_LIBDIR ".")
    set(INSTALL_DATADIR ".")
elseif(APPLE)
    set(INSTALL_BINDIR ".")
    set(INSTALL_LIBDIR ".")
    set(INSTALL_DATADIR ".")
else()
    # Linux: use standard FHS layout
    set(INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
    set(INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
    set(INSTALL_DATADIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
endif()

# Install CLI
if(LSLTEMPLATE_BUILD_CLI)
    install(TARGETS ${PROJECT_NAME}CLI
        RUNTIME DESTINATION "${INSTALL_BINDIR}"
    )
endif()

# Install GUI
if(LSLTEMPLATE_BUILD_GUI)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION "${INSTALL_BINDIR}"
        BUNDLE DESTINATION "${INSTALL_BINDIR}"
    )
endif()

# Install config file
set(_config_dest "${INSTALL_DATADIR}")
if(APPLE AND LSLTEMPLATE_BUILD_GUI)
    set(_config_dest "${INSTALL_BINDIR}/${PROJECT_NAME}.app/Contents/MacOS")
endif()
install(FILES ${PROJECT_NAME}.cfg DESTINATION "${_config_dest}")

# =============================================================================
# Bundle liblsl with the application
# =============================================================================
if(APPLE)
    # macOS: Install framework to GUI bundle and/or CLI Frameworks directory
    if(LSLTEMPLATE_BUILD_GUI)
        LSL_install_liblsl(
            FRAMEWORK_DESTINATION "${INSTALL_BINDIR}/${PROJECT_NAME}.app/Contents/Frameworks"
        )
    endif()
    if(LSLTEMPLATE_BUILD_CLI)
        LSL_install_liblsl(FRAMEWORK_DESTINATION "Frameworks")
    endif()
else()
    # Windows/Linux: Single install location for all targets
    LSL_install_liblsl(DESTINATION "${INSTALL_LIBDIR}")
endif()

# =============================================================================
# Qt Deployment
# =============================================================================
if(LSLTEMPLATE_BUILD_GUI)
    LSL_deploy_qt(TARGET "${PROJECT_NAME}" DESTINATION "${INSTALL_BINDIR}")
endif()

# =============================================================================
# MinGW Runtime Deployment
# =============================================================================
LSL_install_mingw_runtime(DESTINATION "${INSTALL_BINDIR}")

# =============================================================================
# macOS: Code Sign
# =============================================================================
if(APPLE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.labstreaminglayer.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")

    if(LSLTEMPLATE_BUILD_GUI)
        LSL_codesign(
            TARGET "${PROJECT_NAME}"
            DESTINATION "${INSTALL_BINDIR}"
            ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/app.entitlements"
            BUNDLE
        )
    endif()

    if(LSLTEMPLATE_BUILD_CLI)
        LSL_codesign(
            TARGET "${PROJECT_NAME}CLI"
            DESTINATION "${INSTALL_BINDIR}"
            ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/app.entitlements"
            FRAMEWORK "Frameworks/lsl.framework"
        )
    endif()
endif()

# =============================================================================
# CPack Configuration
# =============================================================================
LSL_get_target_arch()
LSL_get_os_name()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Labstreaminglayer")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${LSL_OS}_${LSL_ARCH}")
set(CPACK_STRIP_FILES ON)

if(WIN32)
    set(CPACK_GENERATOR ZIP)
elseif(APPLE)
    set(CPACK_GENERATOR TGZ)
else()
    set(CPACK_GENERATOR DEB TGZ)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "LabStreamingLayer Developers")
    set(CPACK_DEBIAN_PACKAGE_SECTION "science")
    set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}.deb")
    # Note: shlibdeps doesn't work well with bundled liblsl
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
    if(LSLTEMPLATE_BUILD_GUI)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6")
    endif()
endif()

include(CPack)
