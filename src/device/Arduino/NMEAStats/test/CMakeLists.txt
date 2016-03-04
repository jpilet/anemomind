# Faking an arduino requires a few hacks. In particular,
# include directories have to be tweeked.
include_directories(
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/NMEAStats/test/MockArduino"
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/libraries/ChunkFile"
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/libraries/FixedPoint"
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/libraries/NmeaParser"
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/libraries/PhysicalQuantity"
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/libraries/Screen"
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/libraries/TargetSpeed"
    "${CMAKE_SOURCE_DIR}/src/device/Arduino/libraries/TrueWindEstimator"
    )

# Let's fake we're running on Arduino UNO
add_definitions("-DON_DEVICE" "-DARDUINO_UNO")
remove_definitions("-DON_SERVER")

add_library(device_DeviceSimulator
            DeviceSimulator.cpp
            DeviceSimulator.h
            ScreenRecordingSimulator.cpp
            ScreenRecordingSimulator.h
            ../NMEAStats.ino
           )
target_link_libraries(device_DeviceSimulator
         common_logging
         device_ChunkFile
         device_NmeaParser
         device_TargetSpeed
         device_PolarSpeedTable
         logs_LogLoader
         anemobox_Nmea0183Source
         )

cxx_test(device_NmeaStatTest
         NmeaStatsTest.cpp
         device_DeviceSimulator
         common_Env
         gmock_main
        )

add_library(device_ScreenRecordingSimulator
            ScreenRecordingSimulator.cpp
            ScreenRecordingSimulator.h
           )
target_link_libraries(device_ScreenRecordingSimulator
                      device_DeviceSimulator
                      common_string
                      nautical_NavCompatibility
                     )
cxx_test(device_ScreenRecordingSimulatorTest
         ScreenRecordingSimulatorTest.cpp
         device_ScreenRecordingSimulator
         common_Env
         logs_LogLoader
         gmock_main
        )

