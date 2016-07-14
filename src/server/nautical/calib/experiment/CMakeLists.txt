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