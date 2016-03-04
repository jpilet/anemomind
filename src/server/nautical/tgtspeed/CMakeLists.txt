add_executable(nautical_tgtspeed_TargetSpeedDemo
               TargetSpeedDemo.cpp
              )
target_link_libraries(nautical_tgtspeed_TargetSpeedDemo
                      nautical_FilteredNavData
                      nautical_TestdataNavs
                      nautical_tgtspeed_TargetSpeedParam
                      nautical_TemporalSplit
                      nautical_tgtspeed_TargetSpeedPoint
                      nautical_tgtspeed_TargetSpeedSolver
                      nautical_tgtspeed_table
                      logs_LogLoader
                     )
                     
add_library(nautical_tgtspeed_TargetSpeedParam
            TargetSpeedParam.h
            TargetSpeedParam.cpp
           )
target_link_libraries(nautical_tgtspeed_TargetSpeedParam
                      math_Grid
                      nautical_tgtspeed_table
                      plot_gnuplot
                     )     

add_library(nautical_tgtspeed_TargetSpeedSolver
            TargetSpeedSolver.h
            TargetSpeedSolver.cpp
           )
           
target_link_libraries(nautical_tgtspeed_TargetSpeedSolver
                      nautical_tgtspeed_TargetSpeedParam
                      math_nnls
                      common_ScopedLog
                     )           
              
                                   
cxx_test(nautical_tgtspeed_TargetSpeedParamTest
         TargetSpeedParamTest.cpp
         nautical_tgtspeed_TargetSpeedParam
         gtest_main
        )
        
add_library(nautical_tgtspeed_TargetSpeedPoint
            TargetSpeedPoint.h
            TargetSpeedPoint.cpp
           )   
           
add_library(nautical_tgtspeed_table
            table.h
            table.cpp
           )               
target_link_libraries(nautical_tgtspeed_table
                      common_string
                     )                                     