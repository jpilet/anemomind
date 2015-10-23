add_library(nautical_calibration_LinearCalibration
            LinearCalibration.h
            LinearCalibration.cpp
           )

target_link_libraries(nautical_calibration_LinearCalibration
                      nautical_nav
                      common_ScopedLog
                      common_string
                      common_math
                      math_nonlinear_SignalUtils
                     )
           
cxx_test(nautical_calibration_LinearCalibrationTest
         LinearCalibrationTest.cpp
         nautical_calibration_LinearCalibration
         gtest_main
        )
        
        
add_library(nautical_calibration_LinearOptCalib
            LinearOptCalib.h
            LinearOptCalib.cpp
           )        
           
target_link_libraries(nautical_calibration_LinearOptCalib
                      nautical_calibration_LinearCalibration
                      math_irls
                      math_nonlinear_DataFit
                     )
        
cxx_test(nautical_calibration_LinearOptCalibTest
         LinearOptCalibTest.cpp
         nautical_calibration_LinearOptCalib
         common_Env
         common_PathBuilder
         nautical_NavNmeaScan
         gtest_main
        )        
        
target_depends_on_armadillo(nautical_calibration_LinearCalibrationTest)        