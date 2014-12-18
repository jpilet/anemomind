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
                     
add_library(nautical_synthtest_NavalSimulation
            NavalSimulation.h
            NavalSimulation.cpp
           )
           
target_link_libraries(nautical_synthtest_NavalSimulation
                      nautical_GeographicReference
                      common_ProportionateIndexer
                      common_TimeStamp
                      nautical_synthtest_BoatSim
                      nautical_nav
                      common_MeanAndVar
                      nautical_synthtest_Flow
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
                     )
cxx_test(nautical_synthtest_FlowTest
         FlowTest.cpp
         gtest_main
         nautical_synthtest_Flow
        )
        
add_executable(nautical_synthtest_EvaluateCalibrator
               EvaluateCalibrator.cpp
              )
target_link_libraries(nautical_synthtest_EvaluateCalibrator
                      nautical_synthtest_NavalSimulation
                      nautical_Calibrator
                     )