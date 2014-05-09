cmake_minimum_required(VERSION 2.8.11)

include("cmake/GTest.cmake")
include("cmake/ADOLC.cmake")
include("cmake/POCO.cmake")
include("cmake/FindCXXFeatures.cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")

find_package(Armadillo REQUIRED)

function(target_depends_on_armadillo target)
    target_link_libraries(${target} ${ARMADILLO_LIBRARY})
endfunction()

set(RUN_TIME_CONSUMING_TESTS ON CACHE BOOL "Disable to accelerate testing")
if (${RUN_TIME_CONSUMING_TESTS})
    add_definitions(-DTIMECONSUMING_TESTS)
endif (${RUN_TIME_CONSUMING_TESTS})

# this include dir should be set per-target.
# However, cmake does not propagate this property to dependents. As a hack,
# the armadillo include dir will be accessible to all files.
# In the future, a variation of the following line might be useful:
# set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${ARMADILLO_INCLUDE_DIR})
include_directories(${ARMADILLO_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src)
