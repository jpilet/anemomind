include_directories(${CMAKE_BINARY_DIR}/src/device/anemobox/logger)

add_library(logimport_LogLoader
            LogLoader.h
            LogLoader.cpp
           )
target_link_libraries(logimport_LogLoader
                      anemobox_Logger
                      anemobox_Dispatcher
                      anemobox_Logger
                      nautical_NavDataset
                      device_NmeaParser
                      common_filesystem
                      logimport_CsvLoader
                      anemobox_Nmea0183Source
                     )

cxx_test(logimport_LogLoaderTest 
         LogLoaderTest.cpp
         logimport_LogLoader
         gtest_main
        )
                     
add_library(logimport_CsvLoader
            CsvLoader.h
            CsvLoader.cpp
           )
           
target_link_libraries(logimport_CsvLoader
                      common_CsvParser
                      logimport_SourceGroup
                     )           

add_library(logimport_SourceGroup
            SourceGroup.h
            SourceGroup.cpp
           )

                     
                     
cxx_test(logs_Nmea0183Test
         Nmea0183Test.cpp
         logimport_LogLoader
         gtest_main
         nautical_NavCompatibility
        )
        
cxx_test(logs_CsvTest
         CsvTest.cpp
         logimport_LogLoader
         common_Env
         common_PathBuilder
         gtest_main
        )        
        
cxx_test(logs_ProtobufLogTest
         ProtobufLogTest.cpp
         logimport_LogLoader
         common_Env
         common_PathBuilder
         gtest_main
        )     
        
add_library(logimport_TestdataNavs
            TestdataNavs.h
            TestdataNavs.cpp
           )
target_link_libraries(logimport_TestdataNavs
                      common_Env
                      common_PathBuilder
                      common_ArgMap
                      logimport_LogLoader
                      nautical_NavCompatibility
                     )


add_library(logimport_NKEParser
            NKEParser.h
            NKEParser.cpp
           )                  
target_depends_on_poco_foundation(logimport_NKEParser)

target_link_libraries(logimport_NKEParser
                      common_logging
                      common_string
                      common_MeanAndVar
                      common_TimeStamp
                      nautical_NavCompatibility
                     )             
                     
add_executable(logimport_NKEParserDemo
               NKEParserDemo.cpp
              )
target_link_libraries(logimport_NKEParserDemo
                      logimport_NKEParser
                      common_ArgMap
                     )
                     
cxx_test(logimport_NKEParserTest
         NKEParserTest.cpp
         gtest_main
         logimport_NKEParser
        )
        
add_library(logimport_TrzParser
            TrzParser.h
            TrzParser.cpp
           )
target_link_libraries(logimport_TrzParser
                      math_hmm_StateAssign
                      common_Hierarchy
                      common_string
                      common_HNodeGroup
                      common_logging
                      nautical_grammars_StaticCostFactory
                     )
add_executable(logimport_TrzParserExample
               TrzParserExample.cpp
              )
target_link_libraries(logimport_TrzParserExample
                      logimport_TrzParser
                     ) 

add_executable(logimport_checkSanity
               checkSanity.cpp
              )
              
target_link_libraries(logimport_checkSanity
                      logimport_LogLoader
                      nautical_AbsoluteOrientation
                     )                                             