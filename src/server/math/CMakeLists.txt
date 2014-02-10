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

add_library(math_ADFunction
            ADFunction.cpp
            ADFunction.h
           )
target_link_libraries(math_ADFunction
                      common_Function
                     )
target_depends_on_adolc(math_ADFunction)

add_subdirectory(hmm)
add_subdirectory(nonlinear)
