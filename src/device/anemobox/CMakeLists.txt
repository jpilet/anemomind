add_subdirectory(logger)

cxx_test(anemobox_ValueDispatcherTest ValueDispatcherTest.cpp
         common_TimeStamp
         gmock
         gtest_main)

add_library(anemobox_Dispatcher
            Dispatcher.h
            Dispatcher.cpp
            ValueDispatcher.h
            FakeClockDispatcher.h
            )
target_link_libraries(anemobox_Dispatcher
                      common_string
                      common_TimeStamp
                     )

add_library(anemobox_Nmea0183Source
            Nmea0183Source.h
            Nmea0183Source.cpp
            )
target_link_libraries(anemobox_Nmea0183Source
                      anemobox_Dispatcher
                      device_NmeaParser
                     )

add_executable(anemobox main.cpp)
target_link_libraries(anemobox
               anemobox_Dispatcher
               anemobox_Nmea0183Source
              )

add_library(anemobox_DispatcherFilter
            DispatcherFilter.h
            DispatcherFilter.cpp)
target_link_libraries(anemobox_DispatcherFilter
                      anemobox_Dispatcher
                      common_TimeStamp
                     )

cxx_test(anemobox_DispatcherFilterTest DispatcherFilterTest.cpp
         anemobox_Dispatcher
         anemobox_DispatcherFilter
         gtest_main
        )
