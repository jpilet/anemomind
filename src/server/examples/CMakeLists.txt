add_executable(examples_GnuplotExample
  GnuplotExample.cpp)

target_link_libraries(examples_GnuplotExample
  common_LineKM
  plot_gnuplot)