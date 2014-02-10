add_library(plot_gnuplot
  extra.h
  extra.cpp
  gnuplot_i.hpp
  gnuplot_i.cpp)

target_link_libraries(plot_gnuplot common_string)