add_library(nautical_calibration_LinearCalibration
            LinearCalibration.h
            LinearCalibration.cpp
           )
           
cxx_test(nautical_calibration_LinearCalibrationTest
         LinearCalibrationTest.cpp
         nautical_calibration_LinearCalibration
         gtest_main
        )