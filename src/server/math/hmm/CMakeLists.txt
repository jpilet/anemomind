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