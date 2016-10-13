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

add_library(plot_SvgPlot
  SvgPlot.h
  SvgPlot.cpp
  )

target_link_libraries(plot_SvgPlot
  common_HtmlGen
  )

cxx_test(plot_SvgPlotTest
  SvgPlotTest.cpp
  gtest_main
  plot_SvgPlot
  common_Env
  )