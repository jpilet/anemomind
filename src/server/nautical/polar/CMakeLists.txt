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
                      math_ADFunction
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
                      math_CumulativeFunction
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
                      nautical_polar_PolarOptimizer
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
add_library(nautical_polar_PolarSurfaceParam
            PolarSurfaceParam.h
            PolarSurfaceParam.cpp
           )
target_link_libraries(nautical_polar_PolarSurfaceParam 
                      nautical_polar_PolarCurveParam
                      plot_gnuplot
                      common_Uniform
                     ) 
target_depends_on_adolc(nautical_polar_PolarSurfaceParam)
 
cxx_test(nautical_polar_PolarSurfaceParamTest
         PolarSurfaceParamTest.cpp
         gtest_main
         nautical_polar_PolarSurfaceParam
        )
         


cxx_test(nautical_polar_PolarCurveParamTest
         PolarCurveParamTest.cpp
         nautical_polar_PolarCurveParam
         plot_gnuplot
         gtest_main
        )
        
add_library(nautical_polar_PolarOptimizer
            PolarOptimizer.h
            PolarOptimizer.cpp
           )
target_link_libraries(nautical_polar_PolarOptimizer
                      math_nonlinear_Levmar
                      nautical_polar_PolarSurfaceParam
                      nautical_polar_PolarDensity
                     )           