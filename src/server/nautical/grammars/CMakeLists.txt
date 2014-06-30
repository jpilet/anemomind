cxx_test(nautical_grammars_GrammarTest
         GrammarTest.cpp
         nautical_grammars_WindOrientedGrammar
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
                     
add_library(nautical_grammars_WindOrientedGrammar
            WindOrientedGrammar.cpp
            WindOrientedGrammar.h
           )            
           
target_link_libraries(nautical_grammars_WindOrientedGrammar
                      common_string
                      nautical_Grammar
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
                      nautical_grammars_WindOrientedGrammar
                      common_Env
                     )
                     
add_library(nautical_grammars_UserHint
            UserHint.h
            UserHint.cpp
           )
target_link_libraries(nautical_grammars_UserHint
                      common_TimeStamp
                      common_Array
                     )        
                     
add_library(nautical_grammars_UserHintJson
            UserHintJson.h
            UserHintJson.cpp
           )         
target_link_libraries(nautical_grammars_UserHintJson 
                      nautical_grammars_UserHint
                      common_JsonObjDeserializer
                      common_TimeStampJson
                     )
cxx_test(nautical_grammars_UserHintTest 
                      UserHintTest.cpp
                      nautical_grammars_UserHint
                      nautical_grammars_UserHintJson
                      gtest_main
                     )     

add_library(nautical_grammars_StaticCostFactory
            StaticCostFactory.h            
            StaticCostFactory.cpp
           )
target_link_libraries(nautical_grammars_StaticCostFactory
                      common_Hierarchy
                     )
cxx_test(nautical_grammars_StaticCostFactoryTest
         StaticCostFactoryTest.cpp
         nautical_grammars_StaticCostFactory
         common_HNodeGroup
         gtest_main
        )

add_library(nautical_grammars_TrzParser
            TrzParser.h
            TrzParser.cpp
           )
target_link_libraries(nautical_grammars_TrzParser
                      math_hmm_StateAssign
                      common_Hierarchy
                      common_string
                      common_HNodeGroup
                      common_logging
                      nautical_grammars_StaticCostFactory
                     )
add_executable(nautical_grammars_TrzParserExample
               TrzParserExample.cpp
              )
target_link_libraries(nautical_grammars_TrzParserExample
                      nautical_grammars_TrzParser
                     ) 