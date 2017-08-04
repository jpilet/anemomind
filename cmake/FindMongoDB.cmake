# The C-driver

include(ExternalProject)

ExternalProject_Add(mongo_ext
  URL "https://github.com/mongodb/mongo-c-driver/releases/download/1.6.3/mongo-c-driver-1.6.3.tar.gz"
  BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-build"
  SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-src"
  INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-install"
  CONFIGURE_COMMAND "${CMAKE_BINARY_DIR}/third-party/mongo-c-src/configure" "--prefix=${CMAKE_BINARY_DIR}/third-party/mongo-c-install" "--disable-automatic-init-and-cleanup"
  UPDATE_COMMAND ""
  INSTALL_COMMAND make install
)

link_directories("${CMAKE_BINARY_DIR}/third-party/mongo-c-install/lib")

set_property(TARGET mongo_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/mongo-install/include/libmongoc-1.0")

include_directories("${CMAKE_BINARY_DIR}/third-party/mongo-c-install/include/libmongoc-1.0")

function(target_depends_on_mongoc target)
    add_dependencies(${target} mongo_ext)
    target_link_libraries(${target} mongoc-1.0 bson-1.0)
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/mongo-c-install/include/libmongoc-1.0" "${CMAKE_BINARY_DIR}/third-party/mongo-c-install/include/libbson-1.0")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/mongoc-install/lib")
endfunction()
