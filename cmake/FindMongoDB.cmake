# - Find MongoDB; original from
# https://raw.githubusercontent.com/ros-planning/warehouse_ros/master/cmake/FindMongoDB.cmake
#
# Find the MongoDB includes and client library
# This module defines
#  MongoDB_INCLUDE_DIR, where to find mongo/client/dbclient.h
#  MongoDB_LIBRARIES, the libraries needed to use MongoDB.
#  MongoDB_FOUND, If false, do not try to use MongoDB.
#  MongoDB_EXPOSE_MACROS, If true, mongo_ros should use '#define MONGO_EXPOSE_MACROS'

set(MongoDB_BUILD_FROM_SOURCES "YES")

find_package(Boost COMPONENTS filesystem regex thread system REQUIRED)

find_package(OpenSSL)

if (MongoDB_BUILD_FROM_SOURCES)

  find_program(SCONS scons)

  message(STATUS "Building mongodb in ${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-src")
  message(STATUS "MongoDB config:"
        "${SCONS} --prefix=${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-install"
        " --c++11=on"
        " --cpppath=${Boost_INCLUDE_DIR}"
        " --libpath=${Boost_LIBRARY_DIRS}"
        " install")

  ExternalProject_Add(mongodb_ext
        GIT_REPOSITORY "https://github.com/mongodb/mongo-cxx-driver.git"
        GIT_TAG legacy
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-src"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-install"

        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND cd "${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-src"
        && test "-f" "${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-install/lib/libmongoclient.a"
        || ${SCONS} --ssl "--prefix=${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-install"
        --c++11=on --disable-warnings-as-errors=on
        "--cpppath=${Boost_INCLUDE_DIR}"
        "--libpath=${Boost_LIBRARY_DIRS}"
        install
        INSTALL_COMMAND ""
        )

  set(MongoDB_INCLUDE_DIR  "${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-install/include")

  set(MongDB_FOUND "YES")

  add_library(mongoclient STATIC IMPORTED)
  set_property(TARGET mongoclient PROPERTY IMPORTED_LOCATION  "${CMAKE_BINARY_DIR}/third-party/mongocxxdriver-install/lib/libmongoclient.a")
  add_dependencies(mongoclient mongodb_ext)

  set (MongoDB_LIBRARIES mongoclient ${OPENSSL_LIBRARIES})

else (MongoDB_BUILD_FROM_SOURCES)

  set(MongoDB_EXPOSE_MACROS "NO")

  set(MongoDB_PossibleIncludePaths
    /usr/include/
    /usr/local/include/
    /usr/include/mongo/
    /usr/local/include/mongo/
    /opt/mongo/include/
    $ENV{ProgramFiles}/Mongo/*/include
    $ENV{SystemDrive}/Mongo/*/include
    )
  find_path(MongoDB_INCLUDE_DIR mongo/client/dbclient.h
    ${MongoDB_PossibleIncludePaths})

  if(MongoDB_INCLUDE_DIR)
    find_path(MongoDB_dbclientinterface_Path mongo/client/dbclientinterface.h
      ${MongoDB_PossibleIncludePaths})
    if (MongoDB_dbclientinterface_Path)
      set(MongoDB_EXPOSE_MACROS "YES")
    endif()
  endif()

  if(WIN32)
    find_library(MongoDB_LIBRARIES NAMES mongoclient
      PATHS
      $ENV{ProgramFiles}/Mongo/*/lib
      $ENV{SystemDrive}/Mongo/*/lib
      )
  else(WIN32)
    find_library(MongoDB_LIBRARIES NAMES libmongoclient.a mongoclient
      PATHS
      /usr/lib
      /usr/lib64
      /usr/lib/mongo
      /usr/lib64/mongo
      /usr/local/lib
      /usr/local/lib64
      /usr/local/lib/mongo
      /usr/local/lib64/mongo
      /opt/mongo/lib
      /opt/mongo/lib64
      )
  endif(WIN32)
endif (MongoDB_BUILD_FROM_SOURCES)

if(MongoDB_INCLUDE_DIR AND MongoDB_LIBRARIES)
  set(MongoDB_FOUND TRUE)
  message(STATUS "Found MongoDB: ${MongoDB_INCLUDE_DIR}, ${MongoDB_LIBRARIES}")
  message(STATUS "MongoDB using new interface: ${MongoDB_EXPOSE_MACROS}")
else(MongoDB_INCLUDE_DIR AND MongoDB_LIBRARIES)
  set(MongoDB_FOUND FALSE)
  if (MongoDB_FIND_REQUIRED)
    message(FATAL_ERROR "MongoDB not found.")
  else (MongoDB_FIND_REQUIRED)
    message(STATUS "MongoDB not found.")
  endif (MongoDB_FIND_REQUIRED)
endif(MongoDB_INCLUDE_DIR AND MongoDB_LIBRARIES)

if (UNIX)
  # MongoDB depends on boost system library.
  find_package(Boost COMPONENTS filesystem regex thread system REQUIRED)
  set(MongoDB_LIBRARIES ${MongoDB_LIBRARIES}
                        ${Boost_REGEX_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_THREAD_LIBRARY}
                        ${Boost_REGEX_LIBRARY}
                        ${Boost_FILESYSTEM_LIBRARY}
			${CMAKE_THREAD_LIBS_INIT}
			ssl crypto)
endif (UNIX)

mark_as_advanced(MongoDB_INCLUDE_DIR MongoDB_LIBRARIES MongoDB_EXPOSE_MACROS)

