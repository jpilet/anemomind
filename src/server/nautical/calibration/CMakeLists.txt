add_library(nautical_calibration_LinearCalibration
            LinearCalibration.h
            LinearCalibration.cpp
           )

target_link_libraries(nautical_calibration_LinearCalibration
                      nautical_nav
                     )
           
cxx_test(nautical_calibration_LinearCalibrationTest
         LinearCalibrationTest.cpp
         nautical_calibration_LinearCalibration
         gtest_main
        )
target_depends_on_armadillo(nautical_calibration_LinearCalibrationTest)        