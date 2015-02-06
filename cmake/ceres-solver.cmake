include(ExternalProject)
set(CERES_BUILD_STATUS "CERES_NOT_BUILT" 
    CACHE STRING "Ceres build status.
    Default: Auto dl, config, gener, build and install Ceres project"
)
set_property(CACHE CERES_BUILD_STATUS PROPERTY STRINGS
             "CERES_NOT_BUILT"
             "CERES_BUILT"
)

if (CERES_BUILD_STATUS MATCHES "CERES_BUILT")
    set(Ceres_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install/share/Ceres")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/third-party/ceres-install/share/Ceres")
    set(GLOG_CHECK_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/third-party/ceres-install/include")
    set(GLOG_CHECK_LIBRARY_DIRS "${CMAKE_BINARY_DIR}/third-party/ceres-install/lib")
    find_package(Glog)
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

ExternalProject_Add(glog_ext
        URL "http://google-glog.googlecode.com/files/glog-0.3.3.tar.gz"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/glog-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/glog-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install"
        PATCH_COMMAND patch -p3 -t -N < "${CMAKE_SOURCE_DIR}/cmake/glog.patch"
        CONFIGURE_COMMAND "${CMAKE_BINARY_DIR}/third-party/glog-src/configure"
            "--prefix=${CMAKE_BINARY_DIR}/third-party/ceres-install"
        )

ExternalProject_Add(ceres_ext
        URL "http://ceres-solver.org/ceres-solver-1.10.0.tar.gz"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install"
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/ceres-install"
        "-DHAVE_LTO_SUPPORT=OFF"
        "-DSUITESPARSE=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DBUILD_TESTING=OFF"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_CXX_FLAGS=-Wall"
        )
    set_property(TARGET ceres_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/ceres-install/include")

add_dependencies(ceres_ext glog_ext gflags_ext)

## Re-run CMake at make time.
## So the first pass of cmake->make with autoBuild_at_Make_Time option will install 3rdParty
## and we launch the second pass of cmake->make with use_external_build witch auto find 3rdParty
## and update the makeFiles for the superProject
ExternalProject_Add_Step(ceres_ext after_install
    COMMENT "--- ceres install finished. Start re-run CMake ---"
    COMMAND cd "${CMAKE_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -DCERES_BUILD_STATUS=CERES_BUILT -DCeres_DIR="${CMAKE_BINARY_DIR}/third-party/ceres-install/share/Ceres" "${CMAKE_SOURCE_DIR}"
    DEPENDEES install
)

    function(target_depends_on_ceres target)
        add_dependencies(${target} ceres_ext)
    endfunction()
endif (CERES_BUILD_STATUS MATCHES "CERES_BUILT")

