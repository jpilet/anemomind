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

add_library(math_nonlinear_GridFitter
            GridFitter.cpp
            GridFitter.h
           )
target_link_libraries(math_nonlinear_GridFitter
                      common_Array
                      common_Uniform
                      common_string
                      math_ADFunction
                      math_armaadolc
                      math_mathutils
                      math_nonlinear_Levmar
                      math_nonlinear_StepMinimizer
                      math_pareto
                     )
target_depends_on_armadillo(math_nonlinear_GridFitter)



#################################################################
### SignalFitExamples
#################################################################
add_library(examples_SignalFitExamples
  SignalFitExamples.h
  SignalFitExamples.cpp)
target_link_libraries(examples_SignalFitExamples
  plot_gnuplot
  math_Grid
  math_nonlinear_GridFitter
  common_logging
  common_Uniform)
  
add_executable(examples_SignalFitExample001
  SignalFitExample001.cpp)
target_link_libraries(examples_SignalFitExample001
  examples_SignalFitExamples)
  
add_executable(examples_SignalFitExample002
  SignalFitExample002.cpp)
target_link_libraries(examples_SignalFitExample002
  examples_SignalFitExamples)    
  
add_executable(examples_SignalFitExample003
  SignalFitExample003.cpp)
target_link_libraries(examples_SignalFitExample003
  examples_SignalFitExamples)  