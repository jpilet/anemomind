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
                      nautical_GeographicReferenceJson
                      common_TimeStampJson
                     )