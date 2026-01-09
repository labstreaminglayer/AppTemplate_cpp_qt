# GUI Application
add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    MainWindow.ui
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        LSLTemplate::core
        Qt6::Core
        Qt6::Widgets
)

# macOS bundle properties
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.labstreaminglayer.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
endif()

# Windows: Copy DLLs to build directory for debugging
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs for ${PROJECT_NAME}"
    )
    # Copy Qt plugins
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    execute_process(
        COMMAND ${_qmake_executable} -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE QT_PLUGINS_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_DIR}/platforms"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_DIR}/styles"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/styles"
    )
endif()
