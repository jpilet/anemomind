#################################################################
### GnuplotExample
#################################################################
add_executable(examples_GnuplotExample
  GnuplotExample.cpp)
target_link_libraries(examples_GnuplotExample
  common_LineKM
  plot_gnuplot)

#################################################################
### PlotTrajectoryExamples
#################################################################
add_library(examples_PlotTrajectoryExamples
  PlotTrajectoryExamples.h
  PlotTrajectoryExamples.cpp)  
target_link_libraries(examples_PlotTrajectoryExamples
  plot_gnuplot
  common_Duration
  nautical_LocalRace)  

add_executable(examples_PlotTrajectoryExample001
  PlotTrajectoryExample001.cpp)
target_link_libraries(examples_PlotTrajectoryExample001
  examples_PlotTrajectoryExamples)  
  
add_executable(examples_PlotTrajectoryExample002
  PlotTrajectoryExample002.cpp)
target_link_libraries(examples_PlotTrajectoryExample002
  examples_PlotTrajectoryExamples)  


#################################################################
### SignalFitExamples
#################################################################
add_library(examples_SignalFitExamples
  SignalFitExamples.h
  SignalFitExamples.cpp)
target_link_libraries(examples_SignalFitExamples
  plot_gnuplot
  math_Grid
  math_nonlinear_GridFitter
  common_Uniform)
  
add_executable(examples_SignalFitExample001
  SignalFitExample001.cpp)
target_link_libraries(examples_SignalFitExample001
  examples_SignalFitExamples)
  
add_executable(examples_SignalFitExample002
  SignalFitExample002.cpp)
target_link_libraries(examples_SignalFitExample002
  examples_SignalFitExamples)    
  
add_executable(examples_SignalFitExample003
  SignalFitExample003.cpp)
target_link_libraries(examples_SignalFitExample003
  examples_SignalFitExamples)  