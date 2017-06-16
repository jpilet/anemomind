add_subdirectory("grammars")
add_subdirectory("tgtspeed")
add_subdirectory("synthtest")
add_subdirectory("tiles")
add_subdirectory("logimport")
add_subdirectory("calib")
add_subdirectory("filters")
add_subdirectory("types")
add_subdirectory("segment")
add_subdirectory("timesets")


add_library(nautical_nav
            Nav.h
            Nav.cpp
           )
        
target_link_libraries(nautical_nav
                      common_Array
                      common_TimeStamp
                     )
  


cxx_test(nautical_BoatLogProcessorTest
         BoatLogProcessorTest.cpp
         nautical_BoatLogProcessor
         device_TargetSpeed
         common_Env
         gtest_main
        )
target_depends_on_mongoc(nautical_BoatLogProcessorTest)        

                     
add_executable(nautical_ParseExample
               ParseExample.cpp
              )
target_link_libraries(nautical_ParseExample
                      common_Env
                      common_PathBuilder
                      nautical_grammars_WindOrientedGrammar
                      math_hmm_StateAssign
                      logimport_LogLoader
                     )
target_depends_on_poco_foundation(nautical_ParseExample)                                                                    
               



cxx_test(nautical_GeographicPositionTest
  GeographicPositionTest.cpp
  gtest_main)

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
            common_Function
            common_LineKM
            gtest_main)

add_library(nautical_BoatLogProcessor
            BoatLogProcessor.cpp
            BoatLogProcessor.h
           )
target_depends_on_mongoc(nautical_BoatLogProcessor)
target_link_libraries(nautical_BoatLogProcessor
                      nautical_BoatSpecificHacks
                      anemobox_SimulateBox
                      common_PathBuilder
                      common_logging
                      device_ChunkFile
                      math_hmm_StateAssign
                      calib_Calibrator
                      nautical_TargetSpeed
                      nautical_downsamplegps
                      nautical_grammars_WindOrientedGrammar
                      nautical_grammars_TreeExplorer
                      nautical_tiles_TileUtils
                      common_ArgMap
                      plot_gnuplot
                      common_ScopedLog
                      logimport_LogLoader
                      tiles_ChartTiles
                      common_DOMUtils
                     )              
target_depends_on_poco_util(nautical_BoatLogProcessor)
target_depends_on_poco_foundation(nautical_BoatLogProcessor)

add_executable(nautical_processBoatLogs
               processBoatLogs.cpp
              )
target_link_libraries(nautical_processBoatLogs
                      nautical_BoatLogProcessor
                     )              
target_depends_on_mongoc(nautical_processBoatLogs)        



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

add_library(nautical_FlowErrors
            FlowErrors.h
            FlowErrors.cpp
           )              
target_link_libraries(nautical_FlowErrors
                      common_MeanAndVar
                     )     
                     

add_executable(nautical_exportNavs
               exportNavs.cpp
              )
              
target_link_libraries(nautical_exportNavs
                      common_logging
                      common_ArgMap
                      calib_Calibrator
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
                      anemobox_Sources
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
                      anemobox_Dispatcher                      
                      anemobox_Sources
                      common_LineKM
                      common_PeriodicSpan
                      common_TimeStamp
                      nautical_NavDataset
                      nautical_nav
                      plot_gnuplot
                     )

add_library(nautical_AbsoluteOrientation
  AbsoluteOrientation.h
  AbsoluteOrientation.cpp
  )

add_library(nautical_InvWGS84
            InvWGS84.h
            InvWGS84.cpp
           )

target_depends_on_ceres(nautical_InvWGS84)

target_link_libraries(nautical_InvWGS84
  common_StreamUtils
  )

cxx_test(nautical_InvWGS84Test
         InvWGS84Test.cpp
         gtest_main
         nautical_InvWGS84
        )

add_library(nautical_downsamplegps
            DownsampleGps.h
            DownsampleGps.cpp
           )
target_link_libraries(nautical_downsamplegps
                      nautical_NavDataset
                      )

add_library(nautical_MaxSpeed
            MaxSpeed.h
            MaxSpeed.cpp
           )

target_link_libraries(nautical_MaxSpeed
                      nautical_NavDataset
                      common_TimeStamp
                      )

add_library(nautical_BoatSpecificHacks
  BoatSpecificHacks.h
  BoatSpecificHacks.cpp
  )

add_executable(nautical_boatDatInfo boatDatInfo.cpp)
target_link_libraries(nautical_boatDatInfo 
  anemobox_DispatcherTrueWindEstimator
  )

add_library(nautical_Process
  Process.h
  Process.cpp
  )

target_link_libraries(nautical_Process
  logimport_LogLoader
  common_DynamicUtils
  common_Keyword
  math_SegmentOutlierFilter
  )

cxx_test(nautical_ProcessTest
  ProcessTest.cpp
  gtest_main
  nautical_Process
  nautical_InvWGS84
  )

add_executable(nautical_process3
  process3.cpp
  )

target_link_libraries(nautical_process3
  common_ArgMap
  logimport_LogLoader
  common_PathBuilder
  common_DOMUtils
  nautical_Process
  )

add_library(nautical_DynamicChannelValue
  DynamicChannelValue.h
  DynamicChannelValue.cpp
  )

target_link_libraries(nautical_DynamicChannelValue
  common_logging
  nautical_NavDataset
  common_Keyword
  )