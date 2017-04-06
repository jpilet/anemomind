include(ExternalProject)
set(CERES_BUILD_STATUS "CERES_NOT_BUILT" 
    CACHE STRING "Ceres build status.
    Default: Auto dl, config, gener, build and install Ceres project"
)
set_property(CACHE CERES_BUILD_STATUS PROPERTY STRINGS
             "CERES_NOT_BUILT"
             "CERES_BUILT"
)

set(CERES_CMAKE_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install/lib/cmake/Ceres")

if (CERES_BUILD_STATUS MATCHES "CERES_BUILT")
    set(Ceres_DIR "${CERES_CMAKE_DIR}")
    set(CMAKE_MODULE_PATH "${CERES_CMAKE_DIR}" ${CMAKE_MODULE_PATH} )
    find_package(Ceres)
    find_package(Threads)

    include_directories(${CERES_INCLUDE_DIRS})

    function(target_depends_on_ceres target)
        target_link_libraries(${target} ${CERES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    endfunction()

else (CERES_BUILD_STATUS MATCHES "CERES_BUILT")

ExternalProject_Add(gflags_ext
        GIT_REPOSITORY "https://github.com/schuhschuh/gflags.git"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/gflags-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/gflags-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install"
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/ceres-install"
        "-DGFLAGS_NAMESPACE=google"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        )

ExternalProject_Add(ceres_ext
        URL "http://ceres-solver.org/ceres-solver-1.12.0.tar.gz"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install"
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/ceres-install"
        "-DHAVE_LTO_SUPPORT=OFF"
        "-DSUITESPARSE=ON"
        # Using the full glog library causes compilation issues with gcc 5.2,
        # so use this minimal replacement instead.
        "-DMINIGLOG=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DBUILD_TESTING=OFF"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_CXX_FLAGS=-Wall"
        )
    set_property(TARGET ceres_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/ceres-install/include")

add_dependencies(ceres_ext gflags_ext)

## Re-run CMake at make time.
## So the first pass of cmake->make with autoBuild_at_Make_Time option will install 3rdParty
## and we launch the second pass of cmake->make with use_external_build witch auto find 3rdParty
## and update the makeFiles for the superProject
ExternalProject_Add_Step(ceres_ext after_install
    COMMENT "--- ceres install finished. Start re-run CMake ---"
    COMMAND cd "${CMAKE_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -DCERES_BUILD_STATUS=CERES_BUILT -DCeres_DIR="${CERES_CMAKE_DIR}" "${CMAKE_SOURCE_DIR}"
    DEPENDEES install
)

    function(target_depends_on_ceres target)
        add_dependencies(${target} ceres_ext)
    endfunction()
endif (CERES_BUILD_STATUS MATCHES "CERES_BUILT")

