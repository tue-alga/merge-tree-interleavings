cmake_minimum_required(VERSION 3.16)

project(InterleavingDistance LANGUAGES CXX)

add_library(libinterleavingdistance
    src/augmentedtree.cpp
    src/columntree.cpp
    src/interleaving.cpp
    src/interleavingdistance.cpp
    src/mergetree.cpp
    src/mergetreereader.cpp
)

target_compile_features(libinterleavingdistance PUBLIC cxx_std_20)

target_include_directories(libinterleavingdistance
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(interleavingcli
    experiments/interleavingcli.cpp
)
target_link_libraries(interleavingcli
    PRIVATE libinterleavingdistance
)
install(TARGETS interleavingcli DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_subdirectory(tests)
