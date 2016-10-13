include(ExternalProject)

ExternalProject_Add(
  pixman_ext
  URL https://www.cairographics.org/snapshots/pixman-0.33.6.tar.gz
  PREFIX ${CMAKE_BINARY_DIR}/third-party/pixman
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/third-party/pixman
  BUILD_IN_SOURCE 1
)

ExternalProject_Add(
  cairo_ext
  URL https://www.cairographics.org/snapshots/cairo-1.15.2.tar.xz
  DEPENDS pixman_ext #fontconfig
  PREFIX ${CMAKE_BINARY_DIR}/third-party/cairo
  CONFIGURE_COMMAND ./autogen.sh COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/third-party/cairo
  BUILD_IN_SOURCE 1
)


# I don't know what this code means, just based it on the code of ADOL-C
link_directories("${CMAKE_BINARY_DIR}/third-party/cairo/lib" "${CMAKE_BINARY_DIR}/third-party/pixman/lib")
set_property(TARGET cairo_ext PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/third-party/cairo/include" "${CMAKE_BINARY_DIR}/third-party/pixman/include")

function(target_depends_on_cairo target)
    add_dependencies(${target} cairo_ext)
    target_link_libraries(${target} cairo pixman-1)
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/cairo/include"
                 "${CMAKE_BINARY_DIR}/third-party/pixman/include")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DIRECTORIES 
                 "${CMAKE_BINARY_DIR}/third-party/pixman/lib"
                 "${CMAKE_BINARY_DIR}/third-party/cairo/lib")
endfunction()

# http://stackoverflow.com/questions/5971921/building-a-library-using-autotools-from-cmake