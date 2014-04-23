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
                      common_ScopedLog
                      common_string
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
                      common_ScopedLog
                      common_Uniform
                      common_string
                      math_ADFunction
                      math_armaadolc
                      math_mathutils
                      math_nonlinear_Levmar
                      math_nonlinear_StepMinimizer
                      math_pareto
                      math_MatExpr
                     )
target_depends_on_armadillo(math_nonlinear_GridFitter)

add_library(math_nonlinear_GridFitterTestData
            GridFitterTestData.cpp
            GridFitterTestData.h
           )
target_link_libraries(math_nonlinear_GridFitterTestData
                      common_Array
                     )


cxx_test(math_nonlinear_GridFitterTest
         GridFitterTest.cpp
         examples_SignalFitExamples
         math_nonlinear_GridFitterTestData
         gtest_main
        )
  


add_library(math_nonlinear_NoisyStep
  NoisyStep.cpp
  NoisyStep.h)
target_link_libraries(math_nonlinear_NoisyStep
  math_ADFunction
  common_Uniform)
  


#################################################################
### SignalFitExamples
#################################################################
add_library(examples_SignalFitExamples
  SignalFitExamples.h
  SignalFitExamples.cpp)
target_link_libraries(examples_SignalFitExamples
  math_nonlinear_NoisyStep
  plot_gnuplot
  math_Grid
  math_nonlinear_GridFitter
  common_logging
  common_Uniform)
  
add_executable(examples_SignalFitExampleFixedReg
  SignalFitExampleFixedReg.cpp)
target_link_libraries(examples_SignalFitExampleFixedReg
  examples_SignalFitExamples)
  
add_executable(math_nonlinear_SignalFitExampleDivideAndConquer  
               SignalFitExampleDivideAndConquer.cpp
              )
target_link_libraries(math_nonlinear_SignalFitExampleDivideAndConquer
                      math_nonlinear_NoisyStep
                      plot_gnuplot
                     )   
add_library(math_nonlinear_Multiplayer
            Multiplayer.h
            Multiplayer.cpp
           )
target_link_libraries(math_nonlinear_Multiplayer
                      )                                 
  
add_executable(examples_SignalFitExampleAutoRegFirstOrder
  SignalFitExampleAutoRegFirstOrder.cpp)
target_link_libraries(examples_SignalFitExampleAutoRegFirstOrder
  examples_SignalFitExamples)    
  
add_executable(examples_SignalFitExampleAutoReg1st2ndOrder
  SignalFitExampleAutoReg1st2ndOrder.cpp)
target_link_libraries(examples_SignalFitExampleAutoReg1st2ndOrder
  examples_SignalFitExamples)  