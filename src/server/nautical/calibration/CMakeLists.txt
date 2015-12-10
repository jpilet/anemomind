add_library(calibration_LinearCalibration
            LinearCalibration.h
            LinearCalibration.cpp
           )
           
target_link_libraries(calibration_LinearCalibration
                    nautical_nav
                    common_Span
                    math_nonlinear_DataFit
                    math_EigenUtils
                    math_Random
                    common_logging
                   )

target_depends_on_ceres(calibration_LinearCalibration)                   
target_depends_on_armadillo(calibration_LinearCalibration)

cxx_test(calibration_LinearCalibrationTest
         LinearCalibrationTest.cpp
         calibration_LinearCalibration
         nautical_NavNmeaScan
         common_PathBuilder
         common_Env
         gtest_main
         math_Random
        )           
        
add_library(calibration_LinearFit
            LinearFit.h
            LinearFit.cpp
           )
           
target_link_libraries(calibration_LinearFit
                      math_nonlinear_DataFit
                      common_logging
                      math_EigenUtils
                      calibration_LinearCalibration
                     )
           
cxx_test(calibration_LinearFitTest
         LinearFitTest.cpp
         calibration_LinearFit
         gtest_main
         nautical_synthtest_NavalSimulationPrecomp
        )