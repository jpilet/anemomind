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
        
                     
add_library(nautical_synthtest_FractalFlow
            FractalFlow.h
            FractalFlow.cpp
           )
target_link_libraries(nautical_synthtest_FractalFlow
                       nautical_synthtest_Flow
                       math_SubdivFractals
                      ) 
                      
add_executable(nautical_synthtest_FractalFlowDemo
               FractalFlowDemo.cpp
              )

target_link_libraries(nautical_synthtest_FractalFlowDemo
                      nautical_synthtest_FractalFlow
                     )