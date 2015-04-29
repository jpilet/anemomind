cmake_minimum_required(VERSION 2.8.11)

include("cmake/GTest.cmake")
include("cmake/ADOLC.cmake")
include("cmake/POCO.cmake")
include("cmake/FindCXXFeatures.cmake")
include("cmake/ceres-solver.cmake")
include("cmake/FindMongoDB.cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")

find_package(Armadillo REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED COMPONENTS system iostreams filesystem)

function(target_depends_on_armadillo target)
    target_link_libraries(${target} ${ARMADILLO_LIBRARY})
endfunction()

# this include dir should be set per-target.
# However, cmake does not propagate this property to dependents. As a hack,
# the armadillo include dir will be accessible to all files.
# In the future, a variation of the following line might be useful:
# set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${ARMADILLO_INCLUDE_DIR})
include_directories(${ARMADILLO_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)

add_definitions("-DON_SERVER")

add_subdirectory(src)
