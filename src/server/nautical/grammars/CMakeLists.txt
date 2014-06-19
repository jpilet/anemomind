cxx_test(nautical_grammars_GrammarTest
         GrammarTest.cpp
         nautical_grammars_Grammar001
         gtest_main
        )


add_library(nautical_Grammar
            Grammar.h
            Grammar.cpp
           )
            
target_link_libraries(nautical_Grammar            
                      common_Hierarchy
                      nautical_nav
                      math_hmm_StateAssign
                     )
                     
add_library(nautical_grammars_Grammar001
            Grammar001.cpp
            Grammar001.h
           )            
           
target_link_libraries(nautical_grammars_Grammar001
                      common_string
                      nautical_Grammar
                      math_hmm_StateAssignOps
                      common_HNodeGroup
                     )
                     
add_library(nautical_grammars_TreeExplorer
            TreeExplorer.h
            TreeExplorer.cpp
           )
target_link_libraries(nautical_grammars_TreeExplorer
                      common_Hierarchy
                      common_string
                     )           

add_executable(nautical_grammars_TreeExplorerExample
               TreeExplorerExample.cpp
              )
target_link_libraries(nautical_grammars_TreeExplorerExample
                      nautical_grammars_TreeExplorer
                      common_PathBuilder
                      nautical_NavNmeaScan
                      nautical_grammars_Grammar001
                      common_Env
                     )
                     
add_library(nautical_grammars_UserHint
            UserHint.h
            UserHint.cpp
           )
target_link_libraries(nautical_grammars_UserHint
                      common_TimeStamp
                     )        
                     
add_library(nautical_grammars_UserHintJson
            UserHintJson.h
            UserHintJson.cpp
           )         
target_link_libraries(nautical_grammars_UserHintJson 
                      nautical_grammars_UserHint
                      common_TimeStampJson
                     )
cxx_test(nautical_grammars_UserHintTest 
                      UserHintTest.cpp
                      nautical_grammars_UserHint
                      gtest_main
                     )                                  