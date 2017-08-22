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
 
cxx_test(nonlinear_SpatialMedianTest
         SpatialMedianTest.cpp
         gtest_main
        )