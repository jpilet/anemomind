include(ExternalProject)

ExternalProject_Add(poco_ext
        URL "http://pocoproject.org/releases/poco-1.4.6/poco-1.4.6p2-all.tar.gz"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/poco-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/poco-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/poco-install"
        CONFIGURE_COMMAND "${CMAKE_BINARY_DIR}/third-party/poco-src/configure" "--prefix=${CMAKE_BINARY_DIR}/third-party/poco-install"
        UPDATE_COMMAND ""
        INSTALL_COMMAND make install
        )

link_directories("${CMAKE_BINARY_DIR}/third-party/poco-install/lib")

set_property(TARGET poco_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/poco-install/include")

include_directories("${CMAKE_BINARY_DIR}/third-party/poco-install/include")

function(target_depends_on_poco target)
    add_dependencies(${target} poco_ext)
    target_link_libraries(${target} poco)
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/include")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/lib64")
endfunction()

