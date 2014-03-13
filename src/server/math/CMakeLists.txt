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

cxx_test(math_BandMatTest
         BandMatTest.cpp
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

add_subdirectory(hmm)
add_subdirectory(nonlinear)
