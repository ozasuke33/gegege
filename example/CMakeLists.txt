add_executable(example_gegege_helloworld
    helloworld/helloworld.cpp
)
target_link_libraries(example_gegege_helloworld gegege_cpp)

set_target_properties(example_gegege_helloworld PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
