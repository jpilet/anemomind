add_library(nautical_polar_BasicPolar
            BasicPolar.h
            BasicPolar.cpp
           )
target_link_libraries(nautical_polar_BasicPolar
                      common_Histogram
                      plot_gnuplot
                     )
cxx_test(nautical_polar_BasicPolarTest
         BasicPolarTest.cpp
         gtest_main
         nautical_polar_BasicPolar
        )                     
                                                              
add_executable(nautical_polar_PolarPlot
               PolarPlot.cpp
              )
target_link_libraries(nautical_polar_PolarPlot
                      nautical_polar_BasicPolar
                      nautical_TestdataNavs
                      common_logging
                     )                          

add_library(nautical_polar_PolarCurves
            PolarCurves.h
            PolarCurves.cpp
           )
target_link_libraries(nautical_polar_PolarCurves)           
