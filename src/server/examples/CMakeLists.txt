add_library(examples_ExSwitch
  ExSwitch.h
  ExSwitch.cpp)





add_executable(examples_GnuplotExample
  GnuplotExample.cpp)

target_link_libraries(examples_GnuplotExample
  common_LineKM
  plot_gnuplot)


add_executable(examples_PlotTrajectoryExample
  PlotTrajectoryExample.cpp)
  
target_link_libraries(examples_PlotTrajectoryExample
  examples_ExSwitch
  plot_gnuplot
  common_Duration
  nautical_LocalRace)  




add_executable(examples_SignalFitExample
  SignalFitExample.cpp)

target_link_libraries(examples_SignalFitExample
  examples_ExSwitch
  plot_gnuplot
  math_Grid
  math_nonlinear_GridFitter
  common_Uniform)
  
