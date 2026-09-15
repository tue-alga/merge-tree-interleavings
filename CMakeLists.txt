cmake_minimum_required(VERSION 3.16)

project(InterleavingDistance LANGUAGES CXX)

add_library(interleavingdistance
    src/interleavingdistance.cpp
    src/interleaving.cpp
    src/mergetree.cpp
    src/augmentedtree.cpp
    src/columntree.cpp
)

target_compile_features(interleavingdistance PUBLIC cxx_std_20)

target_include_directories(interleavingdistance
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(test_app
    main.cpp
)

target_link_libraries(test_app
    PRIVATE interleavingdistance
)

add_subdirectory(tests)