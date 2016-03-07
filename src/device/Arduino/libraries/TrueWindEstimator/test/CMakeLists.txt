cxx_test(device_TrueWindEstimatorTest
         TrueWindEstimatorTest.cpp
         common_Env
         common_logging
         nautical_nav
         logimport_LogLoader
         nautical_NavCompatibility
         gtest_main)


add_executable(device_TrueWindPlotExample
               TrueWindPlotExample.cpp
              )

target_link_libraries(device_TrueWindPlotExample
                      common_Env
                      common_logging
                      nautical_nav
                      common_ArgMap
                      plot_gnuplot
                      logimport_LogLoader
                      logimport_TestdataNavs
                      nautical_NavCompatibility
                     )
