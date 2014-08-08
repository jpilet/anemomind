
# gtest.

include(ExternalProject)

option(USE_GMOCK "If ON, not only gtest, but also gmock will be installed." ON)

if (USE_GMOCK)
  ExternalProject_Add(gtest_ext
          URL "https://googlemock.googlecode.com/files/gmock-1.7.0.zip"
          BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/gmock-build"
          SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/gmock-src"
          CMAKE_ARGS "${gtest_cmake_args}"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
          INSTALL_COMMAND ""
              )
  set(GTEST_INCLUDE_DIRS
                   "${CMAKE_BINARY_DIR}/third-party/gmock-src/gtest/include"
                   "${CMAKE_BINARY_DIR}/third-party/gmock-src/include"
     )
  link_directories(
      "${CMAKE_BINARY_DIR}/third-party/gmock-build"
      "${CMAKE_BINARY_DIR}/third-party/gmock-build/gtest"
      )
else (USE_GMOCK)
  ExternalProject_Add(gtest_ext
          SVN_REPOSITORY "http://googletest.googlecode.com/svn/tags/release-1.7.0"
          BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/gtest-build"
          SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/gtest-src"
          INSTALL_COMMAND ""
          CMAKE_ARGS "${gtest_cmake_args}"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
          )
  set(GTEST_INCLUDE_DIRS
          "${CMAKE_BINARY_DIR}/third-party/gtest-src/include"
     )
endif (USE_GMOCK)

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
