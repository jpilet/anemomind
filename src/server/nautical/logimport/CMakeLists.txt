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
                      common_CsvParser
                     )
                     
cxx_test(logimport_LogLoaderTest 
         LogLoaderTest.cpp
         logimport_LogLoader
         gtest_main
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
                               