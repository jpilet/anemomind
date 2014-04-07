add_library(nautical_grammars_Grammar
            Grammar.h
            Grammar.cpp
           )
cxx_test(nautical_grammars_GrammarTest
         GrammarTest.cpp
         nautical_grammars_Grammar
         gtest_main
        )
        
add_library(nautical_grammars_Grammar001
            Grammar001.cpp
            Grammar001.h
           )            
target_link_libraries(nautical_grammars_Grammar001
                      nautical_grammars_Grammar
                     )        