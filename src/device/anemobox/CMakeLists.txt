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
                      common_logging
                     )
cxx_test(anemobox_DispatcherTest DispatcherTest.cpp
         anemobox_Dispatcher
         gtest_main
         gmock
        )

add_library(anemobox_Nmea0183Source
            Nmea0183Source.h
            Nmea0183Source.cpp
            )
target_link_libraries(anemobox_Nmea0183Source
                      anemobox_Dispatcher
                      device_NmeaParser
                      nautical_BoatSpecificHacks
                     )



# The Nmea2000Source is actually compiled using
# the script anemobox/anemonode/install.sh, but
# for the conveniency of catching compilation errors on
# a regular basis when we perform a cmake build, compile it here too.
# And probably, we will want to link a testcase to it later.
add_library(anemobox_Nmea2000Source
            Nmea2000Source.h
            Nmea2000Source.cpp
           )
target_link_libraries(anemobox_Nmea2000Source
                      anemobox_Dispatcher
                      n2k_PgnClasses
                     )

cxx_test(anemobox_Nmea2000SourceTest
         Nmea2000SourceTest
         anemobox_Nmea2000Source
         gtest_main)


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


add_library(anemobox_DispatcherUtils
            DispatcherUtils.h
            DispatcherUtils.cpp
           )
           
target_link_libraries(anemobox_DispatcherUtils
                      anemobox_Dispatcher
                      nautical_AbsoluteOrientation
                      anemobox_Logger
                      anemobox_LazyReplayDispatchData
                     )

cxx_test(anemobox_DispatcherUtilsTest
         DispatcherUtilsTest.cpp
         anemobox_DispatcherUtils
         gtest_main
         gmock
        )        

add_library(anemobox_Sources
            Sources.h
            Sources.cpp
           )
target_link_libraries(anemobox_Sources
                      common_logging
                     )

cxx_test(anemobox_SourcesTest
         SourcesTest.cpp
         anemobox_Sources
         gtest_main
        )

add_library(anemobox_BinarySignal
            BinarySignal.h
            BinarySignal.cpp
           )
target_link_libraries(anemobox_BinarySignal
                      common_TimeStamp
                     )
cxx_test(anemobox_BinarySignalTest
         BinarySignalTest.cpp
         gtest_main
         anemobox_BinarySignal
         common_logging
        )

cxx_test(anemobox_LocalArrayCopyTest
  LocalArrayCopyTest.cpp
  gtest_main
  )

add_library(anemobox_LazyReplayDispatchData
  LazyReplayDispatchData.h
  LazyReplayDispatchData.cpp
  )

cxx_test(anemobox_LazyReplayDispatchDataTest
  LazyReplayDispatchDataTest.cpp
  gtest_main
  anemobox_LazyReplayDispatchData
  common_TimeStamp
  )
  
