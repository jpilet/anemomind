add_library(calibration_LinearCalibration
            LinearCalibration.h
            LinearCalibration.cpp
           )
           
target_link_libraries(calibration_LinearCalibration
                    nautical_nav
                    common_Span
                    math_nonlinear_DataFit
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
        
