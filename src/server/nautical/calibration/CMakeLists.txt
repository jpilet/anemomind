add_library(nautical_calibration_LinearCalibration
            LinearCalibration.h
            LinearCalibration.cpp
           )

target_link_libraries(nautical_calibration_LinearCalibration
                      nautical_nav
                      math_SparsityConstrained
                      math_BandMat
                      common_ScopedLog
                      common_string
                     )
           
cxx_test(nautical_calibration_LinearCalibrationTest
         LinearCalibrationTest.cpp
         nautical_calibration_LinearCalibration
         gtest_main
        )
target_depends_on_armadillo(nautical_calibration_LinearCalibrationTest)        