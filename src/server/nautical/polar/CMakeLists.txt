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

add_executable(nautical_polar_FilteredPolarExample
               FilteredPolarExample.cpp
              )

target_link_libraries(nautical_polar_FilteredPolarExample
                      nautical_TestdataNavs
                      plot_gnuplot
                      nautical_polar_PolarPointConv
                      math_hmm_QuantFilter
                      nautical_polar_FilteredPolarPoints
                     )


add_library(nautical_polar_FilteredPolarPoints
            FilteredPolarPoints.h
            FilteredPolarPoints.cpp
           )
target_depends_on_poco_json(nautical_polar_FilteredPolarPoints)
target_link_libraries(nautical_polar_FilteredPolarPoints
                      common_JsonObjDeserializer
                      common_JsonIO
                     )   

add_library(nautical_polar_PolarCurveParam
            PolarCurveParam.h
            PolarCurveParam.cpp
           )
target_link_libraries(nautical_polar_PolarCurveParam
                      math_CurveParam
                      common_LineKM
                     )

cxx_test(nautical_polar_PolarCurveParamTest
         PolarCurveParamTest.cpp
         nautical_polar_PolarCurveParam
         gtest_main
        )