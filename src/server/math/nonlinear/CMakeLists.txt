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

add_library(math_nonlinear_NoisyStep
  NoisyStep.cpp
  NoisyStep.h)
target_link_libraries(math_nonlinear_NoisyStep
  math_ADFunction
  common_Uniform)
  


add_library(math_nonlinear_Multiplayer
            Multiplayer.h
            Multiplayer.cpp
           )
target_link_libraries(math_nonlinear_Multiplayer
                      math_nonlinear_StepMinimizer
                      common_Function
                      math_pareto
                      )                                 

cxx_test(math_nonlinear_MultiplayerTest
          MultiplayerTest.cpp
          math_nonlinear_Multiplayer
          gtest_main
         )

target_depends_on_armadillo(math_nonlinear_MultiplayerTest)  
  
add_library(math_nonlinear_BandedSolver
            BandedSolver.h
            BandedSolver.cpp
           )
target_link_libraries(math_nonlinear_BandedSolver
                      math_BandMat
                      math_nonlinear_SignalUtils
                      common_math
                     )              
  
add_library(math_nonlinear_GeneralizedTV
            GeneralizedTV.h
            GeneralizedTV.cpp
           )
target_link_libraries(math_nonlinear_GeneralizedTV
                      common_Span
                      common_logging
                      math_nonlinear_BandedSolver
                     )

add_library(math_nonlinear_GeneralizedTVAuto
            GeneralizedTVAuto.h
            GeneralizedTVAuto.cpp
           )
target_link_libraries(math_nonlinear_GeneralizedTVAuto
                      math_nonlinear_GeneralizedTV
                      common_DataSplits
                      math_nonlinear_StepMinimizer
                      common_ScopedLog
                     )
                     
add_executable(math_nonlinear_GeneralizedTVExample
               GeneralizedTVExample.cpp
              )
              
target_link_libraries(math_nonlinear_GeneralizedTVExample
                      math_nonlinear_GeneralizedTVAuto
                      plot_gnuplot
                      common_ArgMap
                     )
                     
cxx_test(math_nonlinear_GeneralizedTVTest
         GeneralizedTVTest.cpp
         math_nonlinear_GeneralizedTV
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
 
cxx_test(math_nonlinear_BandedSolverTest
         BandedSolverTest.cpp
         math_nonlinear_BandedSolver
         plot_gnuplot
         gtest_main
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
