add_library(nautical_Ecef
            Ecef.h
            Ecef.cpp)

target_link_libraries(nautical_Ecef common_math)


add_library(nautical_nav
            Nav.h
            Nav.cpp
            NavBBox.h
            NavBBox.cpp)
        
target_link_libraries(nautical_nav
  common_Array
  common_LineKM
  plot_gnuplot
  nautical_Ecef
  common_ContinuousRange)

add_library(nautical_FilteredNavs
            FilteredNavs.cpp
            FilteredNavs.h
           )
target_link_libraries(nautical_FilteredNavs
                      nautical_nav
                      math_nonlinear_SignalFit
                      common_split
                      math_Grid
                      math_hmm_StateAssign
                     )           

add_executable(nautical_FilteredNavsExample
               FilteredNavsExample.cpp
              )
target_link_libraries(nautical_FilteredNavsExample
                      nautical_FilteredNavs
                      plot_gnuplot
                      common_string
                     )
                      

add_library(nautical_DataCalib
            DataCalib.h
            DataCalib.cpp)

target_link_libraries(nautical_DataCalib common_Array)


add_library(nautical_LocalRace
            LocalRace.h
            LocalRace.cpp)
target_link_libraries(nautical_LocalRace
  math_armaadolc)            
            
            
#################################################################
### PlotTrajectoryExamples
#################################################################
add_library(examples_PlotTrajectoryExamples
  PlotTrajectoryExamples.h
  PlotTrajectoryExamples.cpp)  
target_link_libraries(examples_PlotTrajectoryExamples
  plot_gnuplot
  nautical_LocalRace)  

add_executable(examples_PlotTrajectoryExampleLastRace
  PlotTrajectoryExampleLastRace.cpp)
target_link_libraries(examples_PlotTrajectoryExampleLastRace
  examples_PlotTrajectoryExamples)  
  
add_executable(examples_PlotTrajectoryExampleLocalRace
  PlotTrajectoryExampleLocalRace.cpp)
target_link_libraries(examples_PlotTrajectoryExampleLocalRace
  examples_PlotTrajectoryExamples)             

target_link_libraries(nautical_LocalRace
  nautical_nav
  common_Array
  plot_gnuplot
  math_Grid)


cxx_test(nautical_GeographicPositionTest
  GeographicPositionTest.cpp
  gtest_main)



#################################################################
### CalibrationExamples
#################################################################
add_library(nautical_CalibrationExamples
  CalibrationExamples.h
  CalibrationExamples.cpp)
target_link_libraries(nautical_CalibrationExamples
  nautical_LocalRace
  nautical_DataCalib
  math_nonlinear_GridFitter
  )


add_executable(nautical_CalibrationExample
  CalibrationExample.cpp)
target_link_libraries(nautical_CalibrationExample
  nautical_CalibrationExamples)




cxx_test(nautical_LocalRaceTest
            LocalRaceTest.cpp
            nautical_LocalRace
            gtest_main)

cxx_test(nautical_NavTest NavTest.cpp nautical_nav gtest_main)

cxx_test(nautical_WGS84Test
            WGS84Test.cpp
            nautical_Ecef
            common_Function
            common_LineKM
            gtest_main)
