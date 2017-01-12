add_library(nautical_synthtest_BoatSim
            BoatSim.h
            BoatSim.cpp
           )
target_link_libraries(nautical_synthtest_BoatSim
                      math_nonlinear_RungeKutta
                      common_LineKM
                      common_ProportionateIndexer
                      plot_gnuplot              
                      common_Progress
                      common_ScopedLog        
                     )                   
                     
cxx_test(nautical_synthtest_BoatSimTest
            BoatSimTest.cpp
            gtest_main
            nautical_synthtest_BoatSim
           )      
           
add_executable(nautical_synthtest_BoatSimDemo
               BoatSimDemo.cpp
              )
               
target_link_libraries(nautical_synthtest_BoatSimDemo
                      nautical_synthtest_BoatSim
                     )
                     
add_library(nautical_synthtest_NavalSimulation
            NavalSimulation.h
            NavalSimulation.cpp
           )
           
target_link_libraries(nautical_synthtest_NavalSimulation
                      nautical_GeographicReference
                      common_ProportionateIndexer
                      common_TimeStamp
                      nautical_synthtest_BoatSim
                      nautical_NavCompatibility
                      common_MeanAndVar
                      nautical_synthtest_Flow
                      nautical_FlowErrors
                     )
           
cxx_test(nautical_synthtest_NavalSimulationTest
         NavalSimulationTest.cpp
         nautical_synthtest_NavalSimulation
         gtest_main
        )           
        
add_library(nautical_synthtest_Flow
            Flow.h
            Flow.cpp
           )        
target_link_libraries(nautical_synthtest_Flow
                      nautical_GeographicReference
                      common_LineKM
                      plot_gnuplot
                     )
cxx_test(nautical_synthtest_FlowTest
         FlowTest.cpp
         gtest_main
         nautical_synthtest_Flow
        )
        
add_library(nautical_synthtest_NavalSimulationJson
            NavalSimulationJson.h
            NavalSimulationJson.cpp
           )                     
target_link_libraries(nautical_synthtest_NavalSimulationJson
                      nautical_synthtest_NavalSimulation
                      common_Json
                      common_TimeStampJson
                      nautical_GeographicReferenceJson
                      common_JsonObjDeserializer
                      nautical_NavJson
                     )
target_depends_on_poco_foundation(nautical_synthtest_NavalSimulationJson)  
target_depends_on_poco_json(nautical_synthtest_NavalSimulationJson)

cxx_test(nautical_synthtest_NavalSimulationJsonTest
         NavalSimulationJsonTest.cpp
         gtest_main
         nautical_synthtest_NavalSimulationJson
        )                                               

add_library(nautical_synthtest_NavalSimulationPrecomp
            NavalSimulationPrecomp.h
            NavalSimulationPrecomp.cpp
           )
           
target_link_libraries(nautical_synthtest_NavalSimulationPrecomp
                      nautical_synthtest_NavalSimulationJson
                      common_Env
                      common_PathBuilder
                      common_JsonIO
                     )           
