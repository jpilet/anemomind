cxx_test(nautical_grammars_GrammarTest
         GrammarTest.cpp
         nautical_grammars_Grammar001
         gtest_main
        )
        
add_library(nautical_grammars_Grammar001
            Grammar001.cpp
            Grammar001.h
           )            
target_link_libraries(nautical_grammars_Grammar001
                      common_Hierarchy
                      nautical_nav
                      common_string
                      math_hmm_StateAssign
                     )        