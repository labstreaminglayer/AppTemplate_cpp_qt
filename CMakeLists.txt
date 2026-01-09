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
else()
    # Priority 2 & 3: Try to find installed liblsl
    set(_lsl_hints)
    if(LSL_INSTALL_ROOT)
        list(APPEND _lsl_hints "${LSL_INSTALL_ROOT}")
    endif()
    # Common development layout hints
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _build_type_lower)
    foreach(_root IN ITEMS "../liblsl" "../../LSL/liblsl")
        foreach(_build IN ITEMS "build" "cmake-build-${_build_type_lower}")
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
if(APPLE)
    # CLI: @executable_path/Frameworks (from install root)
    # GUI: @executable_path/../Frameworks (inside .app bundle)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/Frameworks;@executable_path")
elseif(UNIX AND NOT ANDROID)
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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
# Detect if liblsl is from FetchContent (regular target) or find_package (imported)
set(_lsl_is_fetched FALSE)
if(TARGET lsl)
    get_target_property(_lsl_imported lsl IMPORTED)
    if(NOT _lsl_imported)
        set(_lsl_is_fetched TRUE)
    endif()
endif()

if(APPLE)
    # macOS: Install framework to multiple destinations (GUI bundle + CLI)
    # We use install(CODE) because we need different destinations for GUI vs CLI
    if(LSLTEMPLATE_BUILD_GUI)
        install(CODE "
            set(_lsl_binary \"$<TARGET_FILE:LSL::lsl>\")
            cmake_path(GET _lsl_binary PARENT_PATH _lsl_fw_dir)  # Versions/A
            cmake_path(GET _lsl_fw_dir PARENT_PATH _lsl_fw_dir)  # Versions
            cmake_path(GET _lsl_fw_dir PARENT_PATH _lsl_fw_dir)  # lsl.framework
            message(STATUS \"Bundling lsl.framework for GUI from: \${_lsl_fw_dir}\")
            file(COPY \"\${_lsl_fw_dir}\"
                DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}/${PROJECT_NAME}.app/Contents/Frameworks\"
                USE_SOURCE_PERMISSIONS
            )
        ")
    endif()
    if(LSLTEMPLATE_BUILD_CLI)
        install(CODE "
            set(_lsl_binary \"$<TARGET_FILE:LSL::lsl>\")
            cmake_path(GET _lsl_binary PARENT_PATH _lsl_fw_dir)
            cmake_path(GET _lsl_fw_dir PARENT_PATH _lsl_fw_dir)
            cmake_path(GET _lsl_fw_dir PARENT_PATH _lsl_fw_dir)
            message(STATUS \"Bundling lsl.framework for CLI from: \${_lsl_fw_dir}\")
            file(COPY \"\${_lsl_fw_dir}\"
                DESTINATION \"\${CMAKE_INSTALL_PREFIX}/Frameworks\"
                USE_SOURCE_PERMISSIONS
            )
        ")
    endif()
else()
    # Windows/Linux: Use modern CMake install commands that handle symlinks properly
    if(_lsl_is_fetched)
        # FetchContent: install the target directly
        install(TARGETS lsl
            RUNTIME DESTINATION "${INSTALL_BINDIR}"  # Windows DLLs
            LIBRARY DESTINATION "${INSTALL_LIBDIR}"  # Linux .so with symlinks
        )
    else()
        # find_package: install imported runtime artifacts (CMake 3.21+)
        install(IMPORTED_RUNTIME_ARTIFACTS LSL::lsl
            RUNTIME DESTINATION "${INSTALL_BINDIR}"
            LIBRARY DESTINATION "${INSTALL_LIBDIR}"
        )
    endif()
endif()

# =============================================================================
# Qt Deployment
# =============================================================================
if(LSLTEMPLATE_BUILD_GUI)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)

    if(WIN32)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
        if(WINDEPLOYQT_EXECUTABLE)
            install(CODE "
                message(STATUS \"Running windeployqt...\")
                execute_process(
                    COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                        --no-translations
                        --no-system-d3d-compiler
                        --no-opengl-sw
                        --no-compiler-runtime
                        --dir \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}\"
                        \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}/${PROJECT_NAME}.exe\"
                )
            ")
        endif()
    elseif(APPLE)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
        if(MACDEPLOYQT_EXECUTABLE)
            install(CODE "
                message(STATUS \"Running macdeployqt...\")
                execute_process(
                    COMMAND \"${MACDEPLOYQT_EXECUTABLE}\"
                        \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}/${PROJECT_NAME}.app\"
                        -verbose=0
                        -always-overwrite
                    RESULT_VARIABLE _deploy_result
                    ERROR_QUIET
                )
                if(NOT _deploy_result EQUAL 0)
                    message(WARNING \"macdeployqt returned \${_deploy_result}\")
                endif()
            ")
        endif()
    endif()
endif()

# =============================================================================
# macOS: Code Sign
# This runs after Qt deployment. We sign the entire bundle with entitlements.
# The lsl.framework is copied with its original install_name (@rpath/lsl.framework/Versions/A/lsl)
# so no install_name_tool fixups are needed.
# =============================================================================
if(APPLE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.labstreaminglayer.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")

    # Sign GUI app bundle
    if(LSLTEMPLATE_BUILD_GUI)
        install(CODE "
            set(_app \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}/${PROJECT_NAME}.app\")
            set(_ent \"${CMAKE_CURRENT_SOURCE_DIR}/app.entitlements\")

            # Sign the entire bundle (ad-hoc signature for local development)
            # --deep signs all nested code including frameworks
            message(STATUS \"Signing app bundle...\")
            execute_process(
                COMMAND codesign --force --deep --sign - --entitlements \"\${_ent}\" \"\${_app}\"
                RESULT_VARIABLE _sign_result
            )

            # Verify
            execute_process(COMMAND codesign --verify --verbose \"\${_app}\" RESULT_VARIABLE _verify_result)
            if(_verify_result EQUAL 0)
                message(STATUS \"App bundle signature verified successfully\")
            else()
                message(WARNING \"App bundle signature verification failed!\")
            endif()
        ")
    endif()

    # Sign CLI and its framework
    if(LSLTEMPLATE_BUILD_CLI)
        install(CODE "
            set(_fw \"\${CMAKE_INSTALL_PREFIX}/Frameworks/lsl.framework\")
            set(_exe \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}/${PROJECT_NAME}CLI\")
            set(_ent \"${CMAKE_CURRENT_SOURCE_DIR}/app.entitlements\")

            # Sign the framework first
            message(STATUS \"Signing lsl.framework for CLI...\")
            execute_process(COMMAND codesign --force --sign - \"\${_fw}\")

            # Sign CLI executable
            message(STATUS \"Signing CLI...\")
            execute_process(COMMAND codesign --force --sign - --entitlements \"\${_ent}\" \"\${_exe}\")
        ")
    endif()
endif()

# =============================================================================
# CPack Configuration
# =============================================================================

# Detect target architecture for package naming
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)")
    set(PACKAGE_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)")
    set(PACKAGE_ARCH "amd64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv7|arm)")
    set(PACKAGE_ARCH "armhf")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i.86|x86)")
    set(PACKAGE_ARCH "i386")
else()
    set(PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Detect OS for package naming
if(APPLE)
    set(PACKAGE_OS "macOS")
elseif(WIN32)
    set(PACKAGE_OS "Win")
else()
    # Try to get distro codename (e.g., "jammy", "noble")
    find_program(LSB_RELEASE lsb_release)
    if(LSB_RELEASE)
        execute_process(COMMAND ${LSB_RELEASE} -cs
            OUTPUT_VARIABLE _codename
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
        if(_codename AND NOT _codename STREQUAL "n/a")
            set(PACKAGE_OS "${_codename}")
        else()
            set(PACKAGE_OS "Linux")
        endif()
    else()
        set(PACKAGE_OS "Linux")
    endif()
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Labstreaminglayer")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PACKAGE_OS}_${PACKAGE_ARCH}")
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
