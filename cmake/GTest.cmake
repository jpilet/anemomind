
# gtest.

include(ExternalProject)

ExternalProject_Add(gtest_ext
        URL "https://github.com/google/googletest/archive/release-1.8.0.zip"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/gmock-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/gmock-src"
        CMAKE_ARGS "${gtest_cmake_args}"
          "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        INSTALL_COMMAND ""
            )
set(GTEST_INCLUDE_DIRS
    "${CMAKE_BINARY_DIR}/third-party/gmock-src/googlemock/include"
    "${CMAKE_BINARY_DIR}/third-party/gmock-src/googletest/include"
   )
link_directories(
    "${CMAKE_BINARY_DIR}/third-party/gmock-build/googlemock"
    "${CMAKE_BINARY_DIR}/third-party/gmock-build/googlemock/gtest"
    )


enable_testing()

find_package(Threads)

function(testing_library name)
    set_property(TARGET ${name} APPEND PROPERTY INCLUDE_DIRECTORIES
                 ${GTEST_INCLUDE_DIRS}
                 )
    add_dependencies(${name} gtest_ext)
endfunction()

function(cxx_test name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} ${ARGN} gtest ${CMAKE_THREAD_LIBS_INIT})
    set_property(TARGET ${name} APPEND PROPERTY INCLUDE_DIRECTORIES
                 ${GTEST_INCLUDE_DIRS}
                 )
    add_dependencies(${name} gtest_ext)
    # Working directory: where the dlls are installed.
    add_test(NAME ${name} 
             COMMAND ${name} "--gtest_break_on_failure")
endfunction()
