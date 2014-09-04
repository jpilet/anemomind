cxx_test(device_TrueWindEstimatorTest
         TrueWindEstimatorTest.cpp
         common_Env
         common_logging
         nautical_nav
         nautical_NavNmea
         gtest_main)


add_executable(device_TrueWindPlotExample
               TrueWindPlotExample.cpp
              )

target_link_libraries(device_TrueWindPlotExample
                      common_Env
                      common_logging
                      nautical_nav
                      nautical_NavNmea
                      common_ArgMap
                      plot_gnuplot
                      nautical_TestdataNavs
                     )
