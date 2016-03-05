add_subdirectory("grammars")
add_subdirectory("tgtspeed")
add_subdirectory("synthtest")
add_subdirectory("tiles")
add_subdirectory("logimport")


add_library(nautical_Ecef
            Ecef.h
            Ecef.cpp)

target_link_libraries(nautical_Ecef common_math)


add_library(nautical_nav
            Nav.h
            Nav.cpp
           )
        
target_link_libraries(nautical_nav
                      common_Array
                     )
  


cxx_test(nautical_BoatLogProcessorTest
         BoatLogProcessorTest.cpp
         nautical_BoatLogProcessor
         common_Env
         gtest_main
        )

add_library(nautical_HTreeJson
            HTreeJson.h                     
            HTreeJson.cpp
           )
target_link_libraries(nautical_HTreeJson
                      nautical_NavCompatibility
                      common_Hierarchy
                     )
target_depends_on_poco_foundation(nautical_HTreeJson)                                
                     
add_executable(nautical_ParseExample
               ParseExample.cpp
              )
target_link_libraries(nautical_ParseExample
                      common_Env
                      common_PathBuilder
                      nautical_grammars_WindOrientedGrammar
                      nautical_NavJson
                      nautical_HTreeJson
                      common_HierarchyJson
                      math_hmm_StateAssign
                      logimport_LogLoader
                     )
target_depends_on_poco_foundation(nautical_ParseExample)                                                                    
               



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
         logimport_LogLoader
         nautical_NavCompatibility
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


cxx_test(nautical_NavTest
          NavTest.cpp
          common_Env
          common_PathBuilder
          nautical_NavCompatibility
          gtest_main)

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
                      anemobox_SimulateBox
                      common_HierarchyJson
                      common_PathBuilder
                      common_logging
                      device_ChunkFile
                      math_hmm_StateAssign
                      nautical_Calibrator
                      nautical_HTreeJson
                      nautical_NavJson
                      nautical_TargetSpeed
                      nautical_grammars_WindOrientedGrammar
                      common_ArgMap
                      plot_gnuplot
                      common_ScopedLog
                      logimport_LogLoader
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
                      anemobox_SimulateBox
                      common_Histogram
                      common_math
                      nautical_FlowErrors
                      nautical_grammars_WindOrientedGrammar
                      plot_gnuplot
                     )
target_depends_on_ceres(nautical_Calibrator)

add_executable(nautical_BasicCalibrate
               BasicCalibrate.cpp)
target_link_libraries(nautical_BasicCalibrate
                      common_Env
                      common_logging
                      nautical_Calibrator
                      logimport_LogLoader
                     )


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
                      nautical_NavCompatibility
                      common_Histogram
                      device_ChunkFile
                      common_logging
                     )  
cxx_test(nautical_TargetSpeedTest
         TargetSpeedTest.cpp
         nautical_TargetSpeed
         gtest_main
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

add_executable(nautical_SaveMatrix
               SaveMatrix.cpp
              )
target_link_libraries(nautical_SaveMatrix
                      logimport_TestdataNavs
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
                      logimport_TestdataNavs
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
                      common_ScopedLog
                     )

                     
add_executable(nautical_FilteredNavDataDemo
               FilteredNavDataDemo.cpp
              )
target_link_libraries(nautical_FilteredNavDataDemo
                      logimport_TestdataNavs
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
                      logimport_TestdataNavs
                      nautical_TemporalSplit
                     )
                      
                                           
add_library(nautical_FlowErrors
            FlowErrors.h
            FlowErrors.cpp
           )              
target_link_libraries(nautical_FlowErrors
                      common_MeanAndVar
                     )     
                     
add_library(nautical_MinCovCalib
            MinCovCalib.h
            MinCovCalib.cpp
           )
target_depends_on_ceres(nautical_MinCovCalib)
target_link_libraries(nautical_MinCovCalib
                      common_Progress
                      nautical_FilteredNavData
                     )

add_executable(nautical_EvalCalib
               EvalCalib.cpp
              )
target_link_libraries(nautical_EvalCalib
                      nautical_MinCovCalib
                      nautical_Calibrator
                      nautical_CalibrationBenchmark
                      common_ArgMap
                     )              

add_library(nautical_CalibrationBenchmark 
            CalibrationBenchmark.h
            CalibrationBenchmark.cpp
           )      
           
target_link_libraries(nautical_CalibrationBenchmark                         
                      nautical_synthtest_NavalSimulationPrecomp
                      common_EnvUtil
                      logimport_LogLoader
                     )


add_library(nautical_GpsFilter
            GpsFilter.h
            GpsFilter.cpp
           )
target_link_libraries(nautical_GpsFilter
                      nautical_NavCompatibility
                      nautical_GeographicReference
                      math_nonlinear_DataFit
                     )   
                     
cxx_test(nautical_GpsFilterTest
         GpsFilterTest.cpp
         nautical_GpsFilter
         gtest_main
         common_Env
         common_PathBuilder
         plot_gnuplot
         logimport_LogLoader
        )                             
add_executable(nautical_exportNavs
               exportNavs.cpp
              )
              
target_link_libraries(nautical_exportNavs
                      common_logging
                      common_ArgMap
                      nautical_Calibrator
                      logimport_LogLoader
                     )              

add_library(nautical_NavToNmea
            NavToNmea.h
            NavToNmea.cpp)
target_link_libraries(nautical_NavToNmea
                      common_string
                      nautical_NavCompatibility)
cxx_test(nautical_NavToNmeaTest NavToNmeaTest.cpp
         device_NmeaParser
         nautical_NavToNmea
         gtest_main
         )

         
add_executable(nautical_exploreData
               exploreData.cpp
              )
              
target_link_libraries(nautical_exploreData
                      common_ArgMap
                      nautical_nav
                      common_PathBuilder
                      math_CleanNumArray
                      common_Span
                      nautical_GeographicReference
                      nautical_NavCompatibility
                      logimport_LogLoader
                     )

add_executable(nautical_catTargetSpeed
               catTargetSpeed.cpp
              )
target_link_libraries(nautical_catTargetSpeed
                      device_TargetSpeed
                      common_ArgMap
                     )
                     
add_library(nautical_NavDataset
            NavDataset.h
            NavDataset.cpp
           )

target_link_libraries(nautical_NavDataset
                      anemobox_Dispatcher      
                      anemobox_DispatcherUtils                
                     )           

cxx_test(nautical_NavDatasetTest
         NavDatasetTest.cpp
         nautical_NavDataset
         gtest_main
        )

add_library(nautical_NavCompatibility
            NavCompatibility.h
            NavCompatibility.cpp
           )
target_link_libraries(nautical_NavCompatibility
                      nautical_nav
                      nautical_NavDataset
                      common_LineKM
                      common_TimeStamp
                      plot_gnuplot
                      nautical_Ecef
                      common_PeriodicSpan
                      anemobox_Dispatcher                      
                     )

