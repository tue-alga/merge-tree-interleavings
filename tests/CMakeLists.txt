add_executable(interleavingdistance_test
    tests.cpp
    test_augmentedtree.cpp
    test_interleavingdistance.cpp
    test_mergetree.cpp
    test_partitioner.cpp
)

target_link_libraries(interleavingdistance_test
    PRIVATE interleavingdistance
)

target_include_directories(interleavingdistance_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)