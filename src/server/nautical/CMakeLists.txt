add_subdirectory("grammars")
add_subdirectory("synthtest")

add_library(nautical_Ecef
            Ecef.h
            Ecef.cpp)

target_link_libraries(nautical_Ecef common_math)


add_library(nautical_nav
            Nav.h
            Nav.cpp
            NavBBox.h
            NavBBox.cpp
           )
        
target_link_libraries(nautical_nav
                      common_Array
                      common_LineKM
                      common_TimeStamp
                      plot_gnuplot
                      nautical_Ecef
                      common_PeriodicSpan
                     )
  
add_library(nautical_NavNmea
            NavNmea.h  
            NavNmea.cpp
           )
target_link_libraries(nautical_NavNmea
                      nautical_nav
                      device_NmeaParser
                      common_ScopedLog
                     )

cxx_test(nautical_NavNmeaTest
         NavNmeaTest.cpp
         nautical_NavNmea
         gtest_main
        )


cxx_test(nautical_BoatLogProcessorTest
         BoatLogProcessorTest.cpp
         nautical_BoatLogProcessor
         common_Env
         gtest_main
        )

add_library(nautical_DataCalib
            DataCalib.h
            DataCalib.cpp)

target_link_libraries(nautical_DataCalib common_Array)


add_library(nautical_LocalRace
            LocalRace.h
            LocalRace.cpp)
target_link_libraries(nautical_LocalRace
  math_armaadolc)            
            

add_library(nautical_NavNmeaScan            
            NavNmeaScan.h
            NavNmeaScan.cpp
           )
target_link_libraries(nautical_NavNmeaScan
                      nautical_NavNmea
                      common_filesystem
                     )

add_library(nautical_HTreeJson
            HTreeJson.h                     
            HTreeJson.cpp
           )
target_link_libraries(nautical_HTreeJson
                      nautical_nav
                      common_Hierarchy
                     )
target_depends_on_poco_foundation(nautical_HTreeJson)                                
                     
add_executable(nautical_ParseExample
               ParseExample.cpp
              )
target_link_libraries(nautical_ParseExample
                      common_Env
                      common_PathBuilder
                      nautical_NavNmeaScan
                      nautical_grammars_Grammar001
                      nautical_NavJson
                      nautical_HTreeJson
                      common_HierarchyJson
                      math_hmm_StateAssign
                     )
target_depends_on_poco_foundation(nautical_ParseExample)                                                                    
               
            
#################################################################
### PlotTrajectoryExamples
#################################################################
add_library(examples_PlotTrajectoryExamples
  PlotTrajectoryExamples.h
  PlotTrajectoryExamples.cpp)  
target_link_libraries(examples_PlotTrajectoryExamples
  plot_gnuplot
  nautical_LocalRace)  

add_executable(examples_PlotTrajectoryExampleLastRace
  PlotTrajectoryExampleLastRace.cpp)
target_link_libraries(examples_PlotTrajectoryExampleLastRace
  examples_PlotTrajectoryExamples)  

add_executable(nautical_ScanNmeaExample
               ScanNmeaExample.cpp
              )
target_link_libraries(nautical_ScanNmeaExample
                       common_Env
                       nautical_NavNmeaScan
                       common_PathBuilder
                      )
  
add_executable(examples_PlotTrajectoryExampleLocalRace
  PlotTrajectoryExampleLocalRace.cpp)
target_link_libraries(examples_PlotTrajectoryExampleLocalRace
  examples_PlotTrajectoryExamples)             

target_link_libraries(nautical_LocalRace
  nautical_nav
  common_Array
  plot_gnuplot
  math_Grid)


cxx_test(nautical_GeographicPositionTest
  GeographicPositionTest.cpp
  gtest_main)

add_library(nautical_NavJson
            NavJson.cpp
            NavJson.h
           )
target_depends_on_poco_json(nautical_NavJson)



target_link_libraries(nautical_NavJson
                            nautical_nav
                            common_Json
                            common_TimeStampJson
                            common_Json
                           )

cxx_test(nautical_NavJsonTest
         NavJsonTest.cpp
         common_logging
         nautical_NavJson
         gtest_main
        )
        
        
cxx_test(nautical_SpeedCalibTest
         SpeedCalibTest.cpp
         gtest_main
        )        

#################################################################
### CalibrationExamples
#################################################################
add_library(nautical_CalibrationExamples
  CalibrationExamples.h
  CalibrationExamples.cpp)
target_link_libraries(nautical_CalibrationExamples
  nautical_LocalRace
  nautical_DataCalib
  math_nonlinear_GridFitter
  )


add_executable(nautical_CalibrationExample
  CalibrationExample.cpp)
target_link_libraries(nautical_CalibrationExample
  nautical_CalibrationExamples)




cxx_test(nautical_LocalRaceTest
            LocalRaceTest.cpp
            nautical_LocalRace
            common_PathBuilder
            common_Env
            nautical_NavNmea
            gtest_main
           )

cxx_test(nautical_NavTest
          NavTest.cpp
          common_Env
          common_PathBuilder
          nautical_nav gtest_main)

cxx_test(nautical_WGS84Test
            WGS84Test.cpp
            nautical_Ecef
            common_Function
            common_LineKM
            gtest_main)

cxx_test(nautical_BasicTrueWindEstimatorTest
         BasicTrueWindEstimatorTest.cpp
         common_Env
         common_logging
         nautical_nav
         nautical_NavNmea
         gtest_main)

add_library(nautical_BoatLogProcessor
            BoatLogProcessor.cpp
            BoatLogProcessor.h
           )
target_link_libraries(nautical_BoatLogProcessor
                      common_logging
                      common_PathBuilder
                      nautical_NavNmeaScan
                      nautical_grammars_Grammar001
                      nautical_NavJson
                      nautical_HTreeJson
                      common_HierarchyJson
                      math_hmm_StateAssign
                      nautical_TargetSpeed
                     )              
target_depends_on_poco_util(nautical_BoatLogProcessor)
target_depends_on_poco_foundation(nautical_BoatLogProcessor)

add_executable(nautical_processBoatLogs
               processBoatLogs.cpp
              )
target_link_libraries(nautical_processBoatLogs
                      nautical_BoatLogProcessor
                     )              


add_library(nautical_Calibrator
            Calibrator.h
            Calibrator.cpp
           )
target_link_libraries(nautical_Calibrator
                      nautical_grammars_Grammar001
                      nautical_NavNmeaScan
                      common_Histogram
                      plot_gnuplot
                      common_math
                     )
target_depends_on_ceres(nautical_Calibrator)

add_executable(nautical_BasicCalibrate
               BasicCalibrate.cpp)
target_link_libraries(nautical_BasicCalibrate
                      common_logging
                      nautical_Calibrator)


add_library(nautical_GeographicReference
            GeographicReference.h
            GeographicReference.cpp
           )
target_link_libraries(nautical_GeographicReference
                     )
cxx_test(nautical_GeographicReferenceTest
         GeographicReferenceTest.cpp
         nautical_GeographicReference
         gtest_main
        )
                                 
            
add_library(nautical_TargetSpeed
            TargetSpeed.h
            TargetSpeed.cpp
           )
target_link_libraries(nautical_TargetSpeed
                      nautical_nav
                      common_Histogram
                      device_ChunkFile
                     )  

add_executable(nautical_TargetSpeedExample
               TargetSpeedExample.cpp
              )

target_link_libraries(nautical_TargetSpeedExample
                      nautical_TargetSpeed
                      common_PathBuilder
                      common_Env
                      nautical_NavNmeaScan
                      common_Span
                      nautical_grammars_Grammar001
                     )
