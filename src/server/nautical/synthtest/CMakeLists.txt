add_library(nautical_synthtest_BoatSim
            BoatSim.h
            BoatSim.cpp
           )
target_link_libraries(nautical_synthtest_BoatSim
                      math_nonlinear_RungeKutta
                      common_LineKM
                      common_ProportionateIndexer
                      plot_gnuplot                      
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
                     
add_library(nautical_synthtest_Testcase
            Testcase.h
            Testcase.cpp
           )
           
target_link_libraries(nautical_synthtest_Testcase
                      nautical_GeographicReference
                      common_ProportionateIndexer
                      common_TimeStamp
                      nautical_synthtest_BoatSim
                      nautical_nav
                     )
           
cxx_test(nautical_synthtest_TestcaseTest
         TestcaseTest.cpp
         nautical_synthtest_Testcase
         gtest_main
        )           