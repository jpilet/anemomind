# The C-driver

include(ExternalProject)

set(MONGO_C_BUILD_STATUS "MONGO_C_NOT_BUILT" 
    CACHE STRING "Mongo build status.
    Default: Auto dl, config, generate, build and install Mongo project"
)
set_property(CACHE MONGO_C_BUILD_STATUS PROPERTY STRINGS
             "MONGO_C_NOT_BUILT"
             "MONGO_C_BUILT"
)

set(MONGO_C_CMAKE_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-install/lib/cmake/libmongoc-static-1.0")
set(BSON_CMAKE_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-install/lib/cmake/libbson-static-1.0")


if (MONGO_C_BUILD_STATUS MATCHES "MONGO_C_BUILT")

   # Not sure if this works:
   	set(CMAKE_MODULE_PATH "${MONGO_C_CMAKE_DIR}" ${CMAKE_MODULE_PATH} )
   	set(CMAKE_MODULE_PATH "${BSON_CMAKE_DIR}" ${CMAKE_MODULE_PATH} )



  find_package(libbson-static-1.0 REQUIRED)
  find_package(libmongoc-static-1.0 REQUIRED)

function(target_depends_on_mongoc target)
    target_link_libraries(${target} ${MONGOC_STATIC_LIBRARIES} ${BSON_STATIC_LIBRARIES})
    target_include_directories(${target} PRIVATE 
      ${MONGOC_STATIC_INCLUDE_DIRS} ${BSON_STATIC_INCLUDE_DIRS} )
endfunction()

add_definitions(-DMONGOC_STATIC -DBSON_STATIC)

else (MONGO_C_BUILD_STATUS MATCHES "MONGO_C_BUILT") 

ExternalProject_Add(mongo_ext
  URL "https://github.com/mongodb/mongo-c-driver/releases/download/1.7.0/mongo-c-driver-1.7.0.tar.gz"
  BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-build"
  SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-src"
  INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/mongo-c-install"
  CONFIGURE_COMMAND "${CMAKE_BINARY_DIR}/third-party/mongo-c-src/configure" "--prefix=${CMAKE_BINARY_DIR}/third-party/mongo-c-install" "--disable-automatic-init-and-cleanup"
"--enable-static" "--disable-shared" "--enable-debug"
  UPDATE_COMMAND ""
  INSTALL_COMMAND make install
)

## Re-run CMake at make time.
## So the first pass of cmake->make with autoBuild_at_Make_Time option will install 3rdParty
## and we launch the second pass of cmake->make with use_external_build witch auto find 3rdParty
## and update the makeFiles for the superProject
ExternalProject_Add_Step(mongo_ext after_install
    COMMENT "--- mongo-c install finished. Start re-run CMake ---"
    COMMAND cd "${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} "-DMONGO_C_BUILD_STATUS=MONGO_C_BUILT" "-DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/third-party/mongo-c-install" "${CMAKE_SOURCE_DIR}"
    DEPENDEES install
)

function(target_depends_on_mongoc target)
    add_dependencies(${target} mongo_ext)
endfunction()

endif (MONGO_C_BUILD_STATUS MATCHES "MONGO_C_BUILT")

