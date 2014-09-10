add_library(math_hmm_StateAssign
            StateAssign.cpp
            StateAssign.h
           )
target_link_libraries(math_hmm_StateAssign
                      common_Array
                      common_logging
                     )
cxx_test(math_hmm_StateAssignTest StateAssignTest.cpp
         math_hmm_StateAssign
         gtest_main
        )

add_library(math_hmm_QuantFilter
            QuantFilter.h
            QuantFilter.cpp
           )
target_link_libraries(math_hmm_QuantFilter
                      math_hmm_StateAssign
                      common_LineKM
                     )
                     
cxx_test(math_hmm_QuantFilterTest
         QuantFilterTest.cpp
         math_hmm_QuantFilter
         gtest_main
        )                                        