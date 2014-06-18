add_library(math_hmm_StateAssign
            StateAssign.cpp
            StateAssign.h
           )
target_link_libraries(math_hmm_StateAssign
                      common_Array
                     )
cxx_test(math_hmm_StateAssignTest StateAssignTest.cpp
         math_hmm_StateAssign
         gtest_main
        )

add_library(math_hmm_StateAssignOps
            StateAssignOps.h
            StateAssignOps.cpp
           )
target_link_libraries(math_hmm_StateAssignOps
                      math_hmm_StateAssign
                     )

cxx_test(math_hmm_StateAssignOpsTest
         StateAssignOpsTest.cpp
         math_hmm_StateAssignOps
         common_Uniform
         common_logging
         gtest_main
        )