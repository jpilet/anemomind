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
        
        
add_library(nautical_synthtest_SynthBoatTrajectory
            SynthBoatTrajectory.h
            SynthBoatTrajectory.cpp
           )
           
target_link_libraries(nautical_synthtest_SynthBoatTrajectory
                      common_LineKM
                      common_ProportionateIndexer
                     )           

cxx_test(nautical_synthtest_SynthBoatTrajectoryTest
         SynthBoatTrajectoryTest.cpp         
         gtest_main
         nautical_synthtest_SynthBoatTrajectory
        )
        
target_depends_on_armadillo(nautical_synthtest_SynthBoatTrajectoryTest)        