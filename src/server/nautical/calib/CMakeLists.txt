add_subdirectory("experiment")
         
add_library(calib_Calibrator
            Calibrator.h
            Calibrator.cpp
           )
target_link_libraries(calib_Calibrator
                      anemobox_SimulateBox
                      common_Histogram
                      common_math
                      nautical_FlowErrors
                      nautical_grammars_WindOrientedGrammar
                      plot_gnuplot
                     )
target_depends_on_ceres(calib_Calibrator)

add_executable(calib_BasicCalibrate
               BasicCalibrate.cpp)
target_link_libraries(calib_BasicCalibrate
                      common_Env
                      common_logging
                      calib_Calibrator
                      logimport_LogLoader
                     )

add_library(calib_CornerCalibTestData
   CornerCalibTestData.h
   CornerCalibTestData.cpp
  )


cxx_test(calib_CornerCalibTest
   CornerCalibTest.cpp
   gtest_main
   calib_CornerCalibTestData
   common_logging
  )

target_depends_on_ceres(calib_CornerCalibTest)


add_library(calib_CurrentCalib
   CurrentCalib.h
   CurrentCalib.cpp
  )

target_link_libraries(calib_CurrentCalib  
                      nautical_NavDataset
                      filters_TimedAngleIntegrator
                     )

cxx_test(calib_CurrentCalibTest
   CurrentCalibTest.cpp
   gtest_main
   calib_CurrentCalib
   calib_CornerCalibTestData
   common_logging
  )
  
target_depends_on_ceres(calib_CurrentCalibTest)