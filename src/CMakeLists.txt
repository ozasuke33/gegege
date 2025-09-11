
add_library(otsukimi_cpp STATIC
    gegege/otsukimi/otsukimi.cpp
    gegege/otsukimi/graphics.cpp
    gegege/otsukimi/util.cpp
)

set_target_properties(otsukimi_cpp PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(otsukimi_cpp PUBLIC
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    stb
    lua
    glm::glm-header-only
    imgui)

add_executable(otsukimi WIN32
    gegege/otsukimi/main.cpp
)

set_target_properties(otsukimi PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    add_custom_command(
        TARGET otsukimi POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:otsukimi>
        VERBATIM
    )
    add_custom_command(
        TARGET otsukimi POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3_ttf::SDL3_ttf-shared> $<TARGET_FILE_DIR:otsukimi>
        VERBATIM
    )
endif()

target_link_libraries(otsukimi PUBLIC
    otsukimi_cpp)



