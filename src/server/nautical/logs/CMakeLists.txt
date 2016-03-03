include_directories(${CMAKE_BINARY_DIR}/src/device/anemobox/logger)

add_library(logs_LogLoader
            LogLoader.h
            LogLoader.cpp
           )
target_link_libraries(logs_LogLoader
                      anemobox_Logger
                     )
                     
cxx_test(logs_LogLoaderTest 
         LogLoaderTest.cpp
         anemobox_Dispatcher
         anemobox_LogToNav
         anemobox_Logger
         logs_LogLoader
         gtest_main
        )
                     