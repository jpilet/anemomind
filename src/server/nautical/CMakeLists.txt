add_subdirectory("grammars")
add_subdirectory("tgtspeed")
add_subdirectory("synthtest")
add_subdirectory("tiles")
add_subdirectory("logimport")
add_subdirectory("calib")
add_subdirectory("filters")
add_subdirectory("types")
add_subdirectory("segment")


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
target_link_libraries(nautical_BoatLogProcessor
                      anemobox_SimulateBox
                      common_HierarchyJson
                      common_PathBuilder
                      common_logging
                      device_ChunkFile
                      math_hmm_StateAssign
                      calib_Calibrator
                      nautical_HTreeJson
                      nautical_NavJson
                      nautical_TargetSpeed
                      nautical_downsamplegps
                      nautical_grammars_WindOrientedGrammar
                      nautical_tiles_TileUtils
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

cxx_test(nautical_InvWGS84Test
         InvWGS84Test.cpp
         gtest_main
         nautical_InvWGS84
        )

add_library(nautical_Visualize
  Visualize.h
  Visualize.cpp
  )

target_link_libraries(nautical_Visualize
  plot_Plot
  )

add_library(nautical_downsamplegps
            DownsampleGps.h
            DownsampleGps.cpp
           )
target_link_libraries(nautical_downsamplegps
                      nautical_NavDataset
                      )

add_executable(nautical_v2demo
  v2demo.cpp
  )
  
target_link_libraries(nautical_v2demo
  common_Env
  logimport_LogLoader
  common_PathBuilder
  nautical_Processor2
  )

add_library(nautical_Processor2
  Processor2.h
  Processor2.cpp
  )

target_link_libraries(nautical_Processor2
  segment_SessionCut
  common_Span
  filters_SmoothGpsFilter
  nautical_Reconstructor
  )

cxx_test(nautical_Processor2Test
  Processor2Test.cpp
  gtest_main
  nautical_Processor2
  )

add_library(nautical_BoatState
  BoatState.h
  BoatState.cpp
  )

add_library(nautical_Reconstructor
  Reconstructor.h
  Reconstructor.cpp
  )

target_link_libraries(nautical_Reconstructor)

cxx_test(nautical_BoatStateTest
  BoatStateTest.cpp
  gtest_main
  )
  