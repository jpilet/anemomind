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
                      nautical_NavCompatibility
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
                      nautical_grammars_StaticCostFactory
                      nautical_grammars_HintedStateAssignFactory
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
                      logimport_LogLoader
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
                     
cxx_test(nautical_grammars_UserHintTest 
                      UserHintTest.cpp
                      nautical_grammars_UserHint
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

add_library(nautical_grammars_AngleCost
            AngleCost.h
            AngleCost.cpp
           )             
cxx_test(nautical_grammars_AngleCostTest
            AngleCostTest.cpp
            gtest_main
            nautical_grammars_AngleCost
           )                   
add_library(nautical_grammars_CommonRaceGrammar
            CommonRaceGrammar.cpp
            CommonRaceGrammar.h
           )
target_link_libraries(nautical_grammars_CommonRaceGrammar
                      nautical_Grammar
                      nautical_grammars_AngleCost
                      nautical_grammars_StaticCostFactory
                      common_HNodeGroup
                      nautical_grammars_HintedStateAssignFactory
                     )
add_executable(nautical_grammars_CommonRaceGrammarExample
               CommonRaceGrammarExample.cpp
              )
target_link_libraries(nautical_grammars_CommonRaceGrammarExample
                      nautical_grammars_CommonRaceGrammar
                      nautical_grammars_TreeExplorer
                      logimport_TestdataNavs
                     )

add_library(nautical_grammars_HintedStateAssign
            HintedStateAssign.h                      
            HintedStateAssign.cpp
           )
target_link_libraries(nautical_grammars_HintedStateAssign
                      common_logging
                      math_hmm_StateAssign
                     )
cxx_test(nautical_grammars_HintedStateAssignTest
         HintedStateAssignTest.cpp
         nautical_grammars_HintedStateAssign
         gtest_main
        )

        
add_library(nautical_grammars_TransitionHint
            TransitionHint.cpp
            TransitionHint.h
           )
target_link_libraries(nautical_grammars_TransitionHint
                      nautical_grammars_HintedStateAssign
                      nautical_NavCompatibility
                     )
                     
add_library(nautical_grammars_HintedStateAssignFactory                     
            HintedStateAssignFactory.cpp
            HintedStateAssignFactory.h
           )
target_link_libraries(nautical_grammars_HintedStateAssignFactory
                      nautical_grammars_TransitionHint
                     )  
                     
cxx_test(nautical_grammars_WindOrientedGrammarTest
         WindOrientedGrammarTest.cpp
         gtest_main
         nautical_grammars_WindOrientedGrammar
         logimport_LogLoader
         nautical_grammars_TreeExplorer
         common_PathBuilder
         common_Env
        )                              