add_library(nautical_synth_BoatSim
            BoatSim.h
            BoatSim.cpp
           )
target_link_libraries(nautical_synth_BoatSim
                      math_nonlinear_RungeKutta
                      common_LineKM
                      common_ProportionateIndexer
                      plot_gnuplot                      
                     )                   
                     
cxx_test(nautical_synth_BoatSimTest
            BoatSimTest.cpp
            gtest_main
            nautical_synth_BoatSim
           )      
           
add_executable(nautical_synth_BoatSimDemo
               BoatSimDemo.cpp
              )
               
target_link_libraries(nautical_synth_BoatSimDemo
                      nautical_synth_BoatSim
                     )
                     
add_library(nautical_synthtest_Testcase
            Testcase.h
            Testcase.cpp
           )