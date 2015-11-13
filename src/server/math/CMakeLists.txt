add_library(math_mathutils
            mathutils.cpp
            mathutils.h
           )
target_link_libraries(math_mathutils
                      common_Array
                     )
target_depends_on_armadillo(math_mathutils)

add_library(math_armaadolc
            armaadolc.cpp
            armaadolc.h
           )
target_link_libraries(math_armaadolc
                      common_Array
                     )
target_depends_on_armadillo(math_armaadolc)
target_depends_on_adolc(math_armaadolc)

add_library(math_MatExpr
            MatExpr.h
            MatExpr.cpp
            )
target_depends_on_armadillo(math_MatExpr)

cxx_test(math_MatExprTest
         MatExprTest.cpp
         math_MatExpr
         gtest_main
        )                   

add_library(math_ADFunction
            ADFunction.cpp
            ADFunction.h
           )
target_link_libraries(math_ADFunction
                      common_Function
                      math_armaadolc
                     )
target_depends_on_adolc(math_ADFunction)

add_library(math_LUImpl
            LUImpl.h
            LUImpl.cpp
           )
add_library(math_LU
            LU.h
            LU.cpp
           )
target_link_libraries(math_LU
                      math_armaadolc
                      math_LUImpl
                     )
cxx_test(math_LUTest
         LUTest.cpp
         math_LU
         math_ADFunction
         gtest_main
        )                           

add_library(math_pareto
            pareto.cpp
            pareto.h
           )
target_link_libraries(math_pareto
                      common_Array
                     )

cxx_test(math_paretoTest
         paretoTest.cpp
         math_pareto
         gtest_main
        )                   

cxx_test(math_mathutilsTest
         mathutilsTest.cpp
         math_mathutils
         gtest_main
        )                   

add_library(math_GemanMcClure
            GemanMcClure.cpp
            GemanMcClure.h
           )
target_link_libraries(math_GemanMcClure
                      common_Array
                      common_math
                      common_Function
                     )
cxx_test(math_GemanMcClureTest
         GemanMcClureTest.cpp
         math_GemanMcClure
         math_nonlinear_Levmar
         common_Uniform
         math_ADFunction
         gtest_main
        )           

add_library(math_Grid
            Grid.cpp
            Grid.h
           )
target_link_libraries(math_Grid
                      common_Array
                      common_BBox
                      common_LineKM
                     )
target_depends_on_armadillo(math_Grid)

cxx_test(math_GridTest
         GridTest.cpp
         math_Grid
         gtest_main
        )

add_library(math_BandMat
            BandMat.h
            BandMat.cpp
           )
target_link_libraries(math_BandMat
                       common_Array
                       common_math
                      )                                    
cxx_test(math_BandMatTest
         BandMatTest.cpp
         math_BandMat
         common_LineKM         
         gtest_main
        )

add_executable(math_ceres_helloworld
               ceres_helloworld.cpp
              )
target_depends_on_ceres(math_ceres_helloworld)

add_library(math_Statistics
            Statistics.h
            Statistics.cpp
           )
cxx_test(math_StatisticsTest
         StatisticsTest.cpp
         math_Statistics
         gtest_main
        )


add_library(math_CumulativeFunction
            CumulativeFunction.cpp
            CumulativeFunction.h
           )
target_link_libraries(math_CumulativeFunction
                      math_armaadolc
                      common_LineKM
                     )

cxx_test(math_CumulativeFunctionTest
         CumulativeFunctionTest.cpp
         gtest_main
         math_CumulativeFunction
        )                     
                     
add_library(math_CleanNumArray
            CleanNumArray.cpp
            CleanNumArray.h
           )
target_link_libraries(math_CleanNumArray
                      math_BandMat
                      common_math
                     )   

cxx_test(math_UniformSamplesTest
         UniformSamplesTest.cpp
         common_LineKM
         gtest_main
         math_nonlinear_SignalUtils
        )
        

add_library(math_PiecewiseFunction
            PiecewiseFunction.h
            PiecewiseFunction.cpp
           )
target_link_libraries(math_PiecewiseFunction           
                      common_ProportionateIndexer
                      common_LineKM
                     )

cxx_test(math_PiecewiseFunctionTest
         PiecewiseFunctionTest.cpp
         math_PiecewiseFunction
         gtest_main
        )

cxx_test(math_SubdivFractalsTest
         SubdivFractalsTest.cpp
         gtest_main
        )
        
add_library(math_SubdivFractals
            SubdivFractals.cpp
            SubdivFractals.h
           )
target_link_libraries(math_SubdivFractals
                      common_math
                     )
target_depends_on_armadillo(math_SubdivFractals)                                
        
add_executable(math_SubdivFractalsDemo
               SubdivFractalsDemo.cpp
              )
                      
target_link_libraries(math_SubdivFractalsDemo
                      plot_gnuplot
                      common_ArgMap
                      common_LineKM
                      math_SubdivFractals
                     )

cxx_test(math_QuadFormTest
         QuadFormTest.cpp
         gtest_main
         common_LineKM
         math_LUImpl
        )
        

add_library(math_nnls
            nnls.c
            nnls.cpp
           )

cxx_test(math_nnlsTest
         nnlsTest.cpp
         math_nnls
         gtest_main
        )       
        
cxx_test(math_GridSearchTest
         GridSearchTest.cpp
         gtest_main
        )
        


cxx_test(math_PiecewisePolynomialsTest
         PiecewisePolynomialsTest.cpp
         gtest_main
         math_LU
        )

add_library(math_Stability
            Stability.h
            Stability.cpp
           )
target_link_libraries(math_Stability
                      common_Span
                      math_LU
                     )           

cxx_test(math_StabilityTest
         StabilityTest.cpp
         gtest_main
         math_Stability
        )

cxx_test(math_SignalCovarianceTest
         SignalCovarianceTest.cpp
         gtest_main
         common_Progress
         common_ScopedLog
        )

add_library(math_irls
            irls.cpp
            irls.h
           )
           
target_link_libraries(math_irls
                      common_ScopedLog
                      common_string
                      common_Span
                      math_LU
                     )           

cxx_test(math_irlsTest
         irlsTest.cpp
         math_irls
         gtest_main
         plot_gnuplot
        )

        
add_library(math_EigenUtils
            EigenUtils.h
            EigenUtils.cpp
           )        
           
target_link_libraries(math_EigenUtils
                      math_Random
                     )           
           
cxx_test(math_EigenUtilsTest
         EigenUtilsTest.cpp
         gtest_main
         math_EigenUtils
        )

add_library(math_Random
            Random.h
            Random.cpp
           )

target_link_libraries(math_Random
                      common_logging
                     )                

add_subdirectory(geometry)
add_subdirectory(hmm)
add_subdirectory(nonlinear)