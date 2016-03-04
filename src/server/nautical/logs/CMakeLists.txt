include_directories(${CMAKE_BINARY_DIR}/src/device/anemobox/logger)

add_library(logs_LogLoader
            LogLoader.h
            LogLoader.cpp
           )
target_link_libraries(logs_LogLoader
                      anemobox_Logger
                      anemobox_Dispatcher
                      anemobox_Logger
                      nautical_NavDataset
                      device_NmeaParser
                      common_filesystem
                      common_CsvParser
                     )
                     
cxx_test(logs_LogLoaderTest 
         LogLoaderTest.cpp
         logs_LogLoader
         gtest_main
        )
                     
                     
cxx_test(logs_Nmea0183Test
         Nmea0183Test.cpp
         logs_LogLoader
         gtest_main
         nautical_NavCompatibility
        )
        
cxx_test(logs_CsvTest
         CsvTest.cpp
         logs_LogLoader
         gtest_main
        )        
        
                     