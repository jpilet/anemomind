cmake_minimum_required(VERSION 2.8.11)

include("cmake/GTest.cmake")
include("cmake/ADOLC.cmake")
include("cmake/FindCXXFeatures.cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")

find_package(Armadillo REQUIRED)

function(target_depends_on_armadillo target)
    target_link_libraries(${target} ${ARMADILLO_LIBRARY})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${ARMADILLO_INCLUDE_DIR})
endfunction()

include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src)
