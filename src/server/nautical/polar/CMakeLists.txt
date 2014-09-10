add_library(nautical_polar_BasicPolar
            BasicPolar.h
            BasicPolar.cpp
           )
target_link_libraries(nautical_polar_BasicPolar
                      common_Histogram
                      plot_gnuplot
                      nautical_polar_PolarPoint
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
                      nautical_polar_PolarCurves
                      nautical_polar_PolarPointConv
                      common_logging
                     )                          

add_library(nautical_polar_PolarCurves
            PolarCurves.h
            PolarCurves.cpp
           )
target_link_libraries(nautical_polar_PolarCurves
                      nautical_polar_PolarDensity
                     )           


add_library(nautical_polar_PolarPoint
            PolarPoint.h
            PolarPoint.cpp
           )

add_library(nautical_polar_PolarDensity
            PolarDensity.h
            PolarDensity.cpp
           )
target_link_libraries(nautical_polar_PolarDensity
                      nautical_polar_PolarPoint
                      common_LineKM
                     )         

add_library(nautical_polar_PolarPointConv
            PolarPointConv.h
            PolarPointConv.cpp
           )
target_link_libraries(nautical_polar_PolarPointConv
                      nautical_polar_PolarPoint
                     )
                      