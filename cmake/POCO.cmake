include(ExternalProject)

ExternalProject_Add(poco_ext
        URL "http://pocoproject.org/releases/poco-1.6.0/poco-1.6.0-all.tar.gz"
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/poco-build-src"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/poco-build-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/poco-install"
        CONFIGURE_COMMAND "${CMAKE_BINARY_DIR}/third-party/poco-build-src/configure"
            "--prefix=${CMAKE_BINARY_DIR}/third-party/poco-install"
            "--omit=Data/MySQL,Data/ODBC,NetSSL_OpenSSL,Crypto"
            "--no-tests" "--no-samples"
        UPDATE_COMMAND ""
        INSTALL_COMMAND make install
        )

link_directories("${CMAKE_BINARY_DIR}/third-party/poco-install/lib")

set_property(TARGET poco_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/poco-install/include")

include_directories("${CMAKE_BINARY_DIR}/third-party/poco-install/include")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(POCO_SUFFIX "d")
else ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(POCO_SUFFIX "")
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")


function(target_depends_on_poco_json target)
    add_dependencies(${target} poco_ext)
    target_link_libraries(${target} PocoJSON${POCO_SUFFIX} PocoFoundation${POCO_SUFFIX})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/include")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/lib")
endfunction()

function(target_depends_on_poco_foundation target)
    add_dependencies(${target} poco_ext)
    target_link_libraries(${target} PocoFoundation${POCO_SUFFIX})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/include")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/lib")
endfunction()

function(target_depends_on_poco_util target)
    add_dependencies(${target} poco_ext)
    target_link_libraries(${target} PocoUtil${POCO_SUFFIX})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/include")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/lib")
endfunction()

function(target_depends_on_poco_xml target)
    add_dependencies(${target} poco_ext)
    target_link_libraries(${target} PocoXML${POCO_SUFFIX})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/include")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/poco-install/lib")
endfunction()
