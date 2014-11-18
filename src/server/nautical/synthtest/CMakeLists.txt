add_library(nautical_synthtest_TestDomain
            TestDomain.h
            TestDomain.cpp
           )
target_link_libraries(nautical_synthtest_TestDomain
                      nautical_GeographicReference
                      common_TimeStamp
                      common_Span
                     )           
                     
add_library(nautical_synthtest_TestDomainJson
            TestDomainJson.h
            TestDomainJson.cpp
           )
target_link_libraries(nautical_synthtest_TestDomainJson
                      nautical_synthtest_TestDomain
                      nautical_GeographicReferenceJson
                      common_TimeStampJson
                     )
                     
cxx_test(nautical_synthtest_TestDomainTest
         TestDomainTest.cpp
         gtest_main
         nautical_synthtest_TestDomainJson
        )                     
        
add_library(nautical_synth_BoatSim
            BoatSim.h
            BoatSim.cpp
           )
target_link_libraries(nautical_synth_BoatSim
                      math_nonlinear_RungeKutta
                     )                   
                     
cxx_test(nautical_synth_BoatSimTest
            BoatSimTest.cpp
            gtest_main
            nautical_synth_BoatSim
           )                     