# usage:
#
# mkdir build; cd build; cmake ..
# or (on edison)
# mkdir build; cd build; cmake -DFULL_BUILD=OFF ..
#
cmake_minimum_required(VERSION 2.8.11)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include("cmake/FindCXXFeatures.cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS} -march=native")

include("cmake/GTest.cmake")

set(FULL_BUILD ON CACHE BOOL "enables full build or restrict to minimum")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

if (${FULL_BUILD})
  include("cmake/ADOLC.cmake")
  include("cmake/POCO.cmake")
  include("cmake/ceres-solver.cmake")
  include("cmake/FindMongoDB.cmake")
  include("cmake/FindF2C.cmake")

  find_package(Cairo )
  if (CAIRO_INCLUDE_DIRS)
    include_directories(${CAIRO_INCLUDE_DIRS})
  endif()

  find_package(Eigen3 REQUIRED)
  include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})

  find_package(LAPACK REQUIRED)
  find_package(BLAS REQUIRED)
  find_package(Armadillo REQUIRED)
  add_definitions(-DENABLE_HACKS)
  add_definitions(-DARMA_DONT_USE_WRAPPER)
  add_definitions(-DARMA_USE_LAPACK)

  function(target_depends_on_armadillo target)
      target_link_libraries(${target} ${ARMADILLO_LIBRARY} ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
  endfunction()

  function(target_depends_on_f2c target)
    target_link_libraries(${target} ${F2C_LIBRARY})
  endfunction()

  find_library(M_LIB m)
  function(target_depends_on_m target)
    target_link_libraries(${target} ${M_LIB})
  endfunction()

  

# this include dir should be set per-target.
# However, cmake does not propagate this property to dependents. As a hack,
# the armadillo include dir will be accessible to all files.
# In the future, a variation of the following line might be useful:
# set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${ARMADILLO_INCLUDE_DIR})
  include_directories(${ARMADILLO_INCLUDE_DIR})
  include_directories(${F2C_INCLUDE_DIR})
  include_directories(${LibM_INCLUDES})
endif ()

find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED COMPONENTS system iostreams filesystem)

if (APPLE)
  set(WHOLE_ARCHIVE "-Wl,-force_load")
  set(NOWHOLE_ARCHIVE "")
else ()
  set(WHOLE_ARCHIVE "-Wl,--whole-archive")
  set(NOWHOLE_ARCHIVE "-Wl,--no-whole-archive")
endif ()

include_directories(${CMAKE_SOURCE_DIR}/src)

# For protocol-buffer generated .h files, we need to have build dir as include
# path, too.
include_directories(${CMAKE_BINARY_DIR}/src)

add_definitions("-DON_SERVER")

add_subdirectory(src)

