add_library(math_nonlinear_StepMinimizer
            StepMinimizer.cpp
            StepMinimizer.h
           )

cxx_test(math_nonlinear_StepMinimizerTest
         StepMinimizerTest.cpp
         math_nonlinear_StepMinimizer
         gtest_main
        )

  
add_library(math_nonlinear_RungeKutta
            RungeKutta.h
            RungeKutta.cpp
           )
target_link_libraries(math_nonlinear_RungeKutta
                      common_Function
                      common_logging
                     )
cxx_test(math_nonlinear_RungeKuttaTest
         RungeKuttaTest.cpp
         math_nonlinear_RungeKutta
         gtest_main
        )    
        
        
cxx_test(math_nonlinear_SignalUtilsTest
         SignalUtilsTest.cpp
         gtest_main
        )       
        
add_library(math_nonlinear_SignalUtils
    SignalUtils.h
    SignalUtils.cpp
   )
 

add_library(math_nonlinear_DataFit
            DataFit.h
            DataFit.cpp
           )
           
target_link_libraries(math_nonlinear_DataFit
                      math_irls
                      common_math
                      common_ScopedLog
                      common_logging
                      math_nonlinear_SignalUtils
                      plot_gnuplot
                     )           
        
cxx_test(math_nonlinear_DataFitTest
         DataFitTest.cpp
         gtest_main
         math_nonlinear_DataFit
        )

cxx_test(nonlinear_SpatialMedianTest
         SpatialMedianTest.cpp
         gtest_main
        )

add_library(nonlinear_CeresTrajectoryFilter
   CeresTrajectoryFilter.h
   CeresTrajectoryFilter.cpp
  )

target_link_libraries(nonlinear_CeresTrajectoryFilter
  common_TimeStamp
 )
   
target_depends_on_ceres(nonlinear_CeresTrajectoryFilter)

cxx_test(nonlinear_CeresTrajectoryFilterTest
         CeresTrajectoryFilterTest.cpp
         nonlinear_CeresTrajectoryFilter
         gtest_main
        )

cxx_test(nonlinear_IrlsTrajectoryFilterTest
         IrlsTrajectoryFilterTest.cpp
         gtest_main
         math_irls
        )