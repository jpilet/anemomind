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
                      logimport_LogLoader
                      nautical_FlowErrors
                      nautical_downsamplegps
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


add_library(calib_Fitness
  Fitness.h
  Fitness.cpp
  )

target_link_libraries(calib_Fitness
  anemobox_Dispatcher
  nautical_AbsoluteOrientation
  )

cxx_test(calib_FitnessTest
  FitnessTest.cpp
  calib_Fitness
  gtest_main
  )
  
cxx_test(calib_SensorSetTest
  SensorSetTest.cpp
  anemobox_Dispatcher
  common_logging
  gtest_main
  )

target_depends_on_ceres(calib_SensorSetTest)

add_library(calib_Reconstructor
  Reconstructor.h
  Reconstructor.cpp
  )

target_link_libraries(calib_Reconstructor
  plot_PlotUtils
  plot_AxisTicks
  plot_CairoUtils
  plot_ColorMap
  calib_MagHdgCalib
  calib_MagHdgCalib2
  calib_GameCalib
  )

add_library(calib_MagHdgCalib
  MagHdgCalib.h
  MagHdgCalib.cpp
  )

target_link_libraries(calib_MagHdgCalib
  common_DOMUtils
  plot_CairoUtils
  plot_PlotUtils
  )

target_depends_on_ceres(calib_MagHdgCalib)

add_library(calib_MagHdgCalib2
  MagHdgCalib2.h
  MagHdgCalib2.cpp
  )

target_link_libraries(calib_MagHdgCalib2
  common_DOMUtils
  plot_CairoUtils
  plot_PlotUtils
  math_SineFit
  )

add_library(calib_WatSpeedCalib
  WatSpeedCalib.h
  WatSpeedCalib.cpp
  )

add_library(calib_GameCalib
  GameCalib.h
  GameCalib.cpp
  )

target_link_libraries(calib_GameCalib
  nonlinear_GameSolver
  common_DOMUtils
  )