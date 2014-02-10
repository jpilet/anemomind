add_library(math_nonlinear_StepMinimizer
            StepMinimizer.cpp
            StepMinimizer.h
           )

cxx_test(math_nonlinear_StepMinimizerTest
         StepMinimizerTest.cpp
         math_nonlinear_StepMinimizer
         gtest_main
        )

add_library(math_nonlinear_Levmar
            Levmar.cpp
            Levmar.h
            LevmarSettings.cpp
            LevmarSettings.h
           )
target_link_libraries(math_nonlinear_Levmar
                      common_Array
                      common_logging
                      math_ADFunction
                      math_armaadolc
                      math_mathutils
                     )

cxx_test(math_nonlinear_LevmarTest
         LevmarTest.cpp
         math_nonlinear_Levmar
         gtest_main
        )
