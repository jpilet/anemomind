add_library(plot_gnuplot
  extra.h
  extra.cpp
  gnuplot_i.hpp
  gnuplot_i.cpp)

target_link_libraries(plot_gnuplot common_string)

#################################################################
### GnuplotExample
#################################################################
add_executable(examples_GnuplotExample
  GnuplotExample.cpp)
target_link_libraries(examples_GnuplotExample
  common_LineKM
  plot_gnuplot) 

add_executable(plot_cairoDemo
  cairoDemo.cpp
  )
target_link_libraries(plot_cairoDemo
                      ${CAIRO_LIBRARIES})


add_library(plot_CairoUtils
  CairoUtils.h
  CairoUtils.cpp
  )

target_link_libraries(plot_CairoUtils
  ${CAIRO_LIBRARIES}
  )
  
add_library(plot_PlotUtils
  PlotUtils.h
  PlotUtils.cpp
  )

cxx_test(plot_PlotUtilsTest
  PlotUtilsTest.cpp
  gtest_main
  plot_PlotUtils
  common_Env
  )
