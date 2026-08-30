#copy SDL3 dlls
if(PLATFORM_WINDOWS)
    message(STATUS "Copying SDL2 dlls to output directory")
    foreach(sdl_target IN ITEMS SDL2::SDL2 SDL2_ttf::SDL2_ttf)
        if(TARGET ${sdl_target})
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${sdl_target}>
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endforeach()
endif()

if(NOT PLATFORM_WEB)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_ttf_INCLUDE_DIRS})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC 
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)