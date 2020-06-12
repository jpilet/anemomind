
# gtest.

include(ExternalProject)

ExternalProject_Add(adolc_ext
        URL "https://www.coin-or.org/download/source/ADOL-C/ADOL-C-2.4.1.tgz"
        TLS_VERIFY ON
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/adolc-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/adolc-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/adolc-install"
        CONFIGURE_COMMAND "${CMAKE_BINARY_DIR}/third-party/adolc-src/configure" "--prefix=${CMAKE_BINARY_DIR}/third-party/adolc-install" "--enable-sparse"
        UPDATE_COMMAND ""
        INSTALL_COMMAND make install
        )

link_directories("${CMAKE_BINARY_DIR}/third-party/adolc-install/lib64")

set_property(TARGET adolc_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/adolc-install/include")

include_directories("${CMAKE_BINARY_DIR}/third-party/adolc-install/include")

function(target_depends_on_adolc target)
    add_dependencies(${target} adolc_ext)
    target_link_libraries(${target} adolc)
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/adolc-install/include")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/adolc-install/lib64")
endfunction()

