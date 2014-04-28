include(ExternalProject)


ExternalProject_Add(gflags_ext
        GIT_REPOSITORY "https://github.com/schuhschuh/gflags.git"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/gflags-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/gflags-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install"
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/ceres-install"
        "-DGFLAGS_NAMESPACE=google"
        )

ExternalProject_Add(glog_ext
        URL "http://google-glog.googlecode.com/files/glog-0.3.3.tar.gz"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/glog-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/glog-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install"
        CONFIGURE_COMMAND "${CMAKE_BINARY_DIR}/third-party/glog-src/configure"
            "--prefix=${CMAKE_BINARY_DIR}/third-party/ceres-install"
        )

ExternalProject_Add(ceres_ext
        URL "http://ceres-solver.googlecode.com/files/ceres-solver-1.8.0.tar.gz"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/ceres-install"
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/ceres-install"
        "-DHAVE_LTO_SUPPORT=OFF"
        "-DCMAKE_CXX_FLAGS=-Wall"
        )

add_dependencies(ceres_ext glog_ext gflags_ext)

link_directories("${CMAKE_BINARY_DIR}/third-party/ceres-install/lib")

set_property(TARGET ceres_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/ceres-install/include")

include_directories("${CMAKE_BINARY_DIR}/third-party/ceres-install/include")

function(target_depends_on_ceres target)
    add_dependencies(${target} ceres_ext)
#target_link_libraries(${target} PocoJSON${POCO_SUFFIX} PocoFoundation${POCO_SUFFIX})
#    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
#                 "${CMAKE_BINARY_DIR}/third-party/ceres-install/include")
#    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
#                 "${CMAKE_BINARY_DIR}/third-party/ceres-install/lib")
endfunction()

