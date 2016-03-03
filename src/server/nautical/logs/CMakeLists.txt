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
                     )
                     
cxx_test(logs_LogLoaderTest 
         LogLoaderTest.cpp
         logs_LogLoader
         gtest_main
        )
                     