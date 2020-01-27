add_subdirectory("sqlite")
add_subdirectory("NMEA2000/src")
target_link_libraries(nmea2000 anemobox_millis)
