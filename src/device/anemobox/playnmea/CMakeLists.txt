add_executable(anemobox_playnmea PlayNmea.cpp)
target_link_libraries(anemobox_playnmea 
    common_TimeStamp
    device_NmeaParser
)
