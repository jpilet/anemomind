add_subdirectory("experiment")

cxx_test(calib_SpeedCalibTest
         SpeedCalibTest.cpp
         gtest_main
        )        

cxx_test(calib_CorrectorTest
         CorrectorTest.cpp
         gtest_main
         )
         
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

add_executable(calib_EvalCalib
               EvalCalib.cpp
              )
target_link_libraries(calib_EvalCalib
                      calib_Calibrator
                      calib_CalibrationBenchmark
                      common_ArgMap
                     )              

add_library(calib_CalibrationBenchmark 
            CalibrationBenchmark.h
            CalibrationBenchmark.cpp
           )      
           
target_link_libraries(calib_CalibrationBenchmark                         
                      nautical_synthtest_NavalSimulationPrecomp
                      common_EnvUtil
                      logimport_LogLoader
                     )

cxx_test(calib_CornerCalibTest
   CornerCalibTest.cpp
   gtest_main
   common_logging
  )

target_depends_on_ceres(calib_CornerCalibTest)
