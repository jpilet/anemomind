add_subdirectory(logger)
add_subdirectory(n2k)
add_subdirectory(playnmea)
add_subdirectory(simulator)

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
cxx_test(anemobox_DispatcherTest DispatcherTest.cpp
         anemobox_Dispatcher
         gtest_main
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

add_library(anemobox_DispatcherTrueWindEstimator
            DispatcherTrueWindEstimator.h
            DispatcherTrueWindEstimator.cpp)
target_link_libraries(anemobox_DispatcherTrueWindEstimator
                      anemobox_Dispatcher
                      anemobox_DispatcherFilter
                      device_ChunkFile
                      device_TargetSpeed
                     )
cxx_test(anemobox_DispatcherTrueWindEstimatorTest
         DispatcherTrueWindEstimatorTest.cpp
         anemobox_DispatcherTrueWindEstimator
         common_Env
         gtest_main
        )

cxx_test(anemobox_TimedSampleCollectionTest TimedSampleCollectionTest.cpp
         common_TimeStamp
         gtest_main
         )
