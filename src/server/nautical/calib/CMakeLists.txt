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
         

         
         
         
add_library(calib_CalibExperiment
	CalibExperiment.h
	CalibExperiment.cpp
	)

target_link_libraries(calib_CalibExperiment
	logimport_LogLoader
	nautical_NavCompatibility
	common_Env
	common_PathBuilder
	)
	
cxx_test(calib_CalibExperimentTest
	CalibExperimentTest.cpp
	calib_CalibExperiment
	gtest_main
	)
	
add_executable(calib_mainCalibExperiment
	mainCalibExperiment.cpp
	)
	
target_link_libraries(calib_mainCalibExperiment
	calib_CalibExperiment
    )