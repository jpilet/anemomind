add_subdirectory("grammars")
add_subdirectory("polar")
add_subdirectory("synthtest")
add_subdirectory("tiles")


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
                      device_ScreenRecordingSimulator
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
                      nautical_grammars_WindOrientedGrammar
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
                      common_Env
                      nautical_LocalRace
                      plot_gnuplot)

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
         common_Env
         common_logging
         nautical_NavJson
         nautical_NavNmea
         gtest_main
        )
        
        
cxx_test(nautical_SpeedCalibTest
         SpeedCalibTest.cpp
         gtest_main
        )        


cxx_test(nautical_CorrectorTest
         CorrectorTest.cpp
         gtest_main
         )

#################################################################
### CalibrationExamples
#################################################################

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

add_library(nautical_BoatLogProcessor
            BoatLogProcessor.cpp
            BoatLogProcessor.h
           )
target_link_libraries(nautical_BoatLogProcessor
                      common_HierarchyJson
                      common_PathBuilder
                      common_logging
                      device_ChunkFile
                      math_hmm_StateAssign
                      nautical_Calibrator
                      nautical_HTreeJson
                      nautical_NavJson
                      nautical_NavNmeaScan
                      nautical_TargetSpeed
                      nautical_grammars_WindOrientedGrammar
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
                      nautical_grammars_WindOrientedGrammar
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
cxx_test(nautical_TargetSpeedTest
         TargetSpeedTest.cpp
         nautical_TargetSpeed
         gtest_main
        )                     

add_library(nautical_TestdataNavs
            TestdataNavs.h
            TestdataNavs.cpp
           )
target_link_libraries(nautical_TestdataNavs
                      nautical_NavNmeaScan
                      common_Env
                      common_PathBuilder
                      common_ArgMap
                     )


add_library(nautical_GeographicPositionJson
            GeographicPositionJson.h
            GeographicPositionJson.cpp
           )
target_link_libraries(nautical_GeographicPositionJson
                      common_JsonObjDeserializer
                     )

add_library(nautical_GeographicReferenceJson
            GeographicReferenceJson.h
            GeographicReferenceJson.cpp
           )
target_link_libraries(nautical_GeographicReferenceJson
                    nautical_GeographicPositionJson
                   )

add_library(nautical_ParametricHorizontalMotion
            ParametricHorizontalMotion.h
            ParametricHorizontalMotion.cpp
           )
target_link_libraries(nautical_ParametricHorizontalMotion 
                      common_Uniform
                     )
add_library(nautical_WaterCalib
            WaterCalib.h
            WaterCalib.cpp
           )
target_link_libraries(nautical_WaterCalib
                      nautical_nav
                      math_ADFunction
                      math_GemanMcClure
                      common_ScopedLog
                      math_nonlinear_Levmar
                      plot_gnuplot
                      nautical_ParametricHorizontalMotion
                     )
           
add_executable(nautical_WaterCalibExample
               WaterCalibExample.cpp
              )
target_link_libraries(nautical_WaterCalibExample
                      nautical_WaterCalib
                      nautical_TestdataNavs
                     )              

add_executable(nautical_Plot
               Plot.cpp
              )
target_link_libraries(nautical_Plot
                      common_string
                      plot_gnuplot
                      common_logging
                      nautical_TestdataNavs
                      nautical_GeographicReference
                     )
                     
add_executable(nautical_SaveMatrix
               SaveMatrix.cpp
              )
target_link_libraries(nautical_SaveMatrix
                      nautical_TestdataNavs
                     )
add_executable(nautical_deviceOutput
               deviceOutput.cpp
              )
target_link_libraries(nautical_deviceOutput
                      device_ScreenRecordingSimulator
                     )

                     
add_executable(nautical_TgtSpeedDemo
               TgtSpeedDemo.cpp
              )
target_link_libraries(nautical_TgtSpeedDemo
                      nautical_nav
                      common_PathBuilder
                      nautical_TestdataNavs
                      nautical_TargetSpeed
                     )


add_executable(nautical_plotTargetSpeed
               plotTargetSpeed.cpp)
target_link_libraries(nautical_plotTargetSpeed
                      plot_gnuplot
                      device_TargetSpeed
                      device_ChunkFile
                     )

add_library(nautical_FilteredNavData
            FilteredNavData.cpp
            FilteredNavData.h
           )

target_link_libraries(nautical_FilteredNavData
                      math_CleanNumArray
                      common_logging
                      nautical_nav
                      plot_gnuplot
                      math_nonlinear_GeneralizedTV
                      common_MeanAndVar
                     )

                     
add_executable(nautical_FilteredNavDataDemo
               FilteredNavDataDemo.cpp
              )
target_link_libraries(nautical_FilteredNavDataDemo
                      nautical_TestdataNavs
                      common_ArgMap
                      plot_gnuplot
                      common_Span
                      nautical_TemporalSplit
                      math_CleanNumArray
                      math_nonlinear_GeneralizedTVAuto
                      nautical_FilteredNavData
                     )     
                     


add_library(nautical_TemporalSplit
            TemporalSplit.h
            TemporalSplit.cpp
           )
target_link_libraries(nautical_TemporalSplit
                      nautical_nav
                      common_logging
                     )           
            
add_executable(nautical_RaceOverview
               RaceOverview.cpp
              )
target_link_libraries(nautical_RaceOverview
                      nautical_TestdataNavs
                      nautical_TemporalSplit
                     )
                      
                      
add_executable(nautical_CalibrationDemo
               CalibrationDemo.cpp
              )
                                                                                 
target_link_libraries(nautical_CalibrationDemo
                      nautical_AutoCalib
                      common_ArgMap
                      nautical_TestdataNavs
                      nautical_synthtest_NavalSimulation
                     )
                     
add_library(nautical_AutoCalib
            AutoCalib.h
            AutoCalib.cpp
           )
           
target_link_libraries(nautical_AutoCalib
                      nautical_FilteredNavData
                      math_armaadolc
                     )
                     





add_library(nautical_DecorrCalib
            DecorrCalib.h
            DecorrCalib.cpp
           )
target_link_libraries(nautical_DecorrCalib
                      nautical_FilteredNavData
                     )
                     
target_depends_on_ceres(nautical_DecorrCalib)

add_executable(nautical_DecorrCalibDemo
               DecorrCalibDemo.cpp
              )
target_link_libraries(nautical_DecorrCalibDemo
                      nautical_DecorrCalib
                      nautical_FilteredNavData
                      nautical_synthtest_NavalSimulationPrecomp
                      common_ArgMap
                     )
target_depends_on_ceres(nautical_AutoCalib)


add_library(nautical_NKEParser
            NKEParser.h
            NKEParser.cpp
           )                  
target_depends_on_poco_foundation(nautical_NKEParser)

target_link_libraries(nautical_NKEParser
                      common_logging
                      common_string
                      common_MeanAndVar
                      common_TimeStamp
                      nautical_nav
                     )             
                     
add_executable(nautical_NKEParserDemo
               NKEParserDemo.cpp
              )
target_link_libraries(nautical_NKEParserDemo
                      nautical_NKEParser
                      common_ArgMap
                     )
                     
cxx_test(nautical_NKEParserTest
         NKEParserTest.cpp
         gtest_main
         nautical_NKEParser
        )                     

add_library(nautical_SimpleCalibrator
            SimpleCalibrator.h
            SimpleCalibrator.cpp
           )