# CLI Application
add_executable(${PROJECT_NAME}CLI
    main.cpp
)

target_link_libraries(${PROJECT_NAME}CLI
    PRIVATE
        LSLTemplate::core
)

# Windows: Copy DLLs to build directory for debugging
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}CLI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}CLI>
            $<TARGET_FILE_DIR:${PROJECT_NAME}CLI>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs for ${PROJECT_NAME}CLI"
    )
endif()
