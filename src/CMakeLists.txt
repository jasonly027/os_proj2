add_executable(${PROJECT_NAME}
    main.cpp
    page_table.cpp
    reference_generator.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
