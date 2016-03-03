
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS logger.proto)

add_library(anemobox_Logger Logger.h Logger.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(anemobox_Logger anemobox_Dispatcher ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES})

cxx_test(anemobox_LoggerTest LoggerTest.cpp anemobox_Logger gtest_main)

add_executable(anemobox_logcat logcat.cpp)
target_link_libraries(anemobox_logcat
                      anemobox_Logger
                      common_ArgMap
                      common_logging)

add_executable(anemobox_logToNmea LogToNmea.cpp)
target_link_libraries(anemobox_logToNmea
                      anemobox_Logger
                      common_ArgMap
                      common_logging
                      device_NmeaParser
                      nautical_NavToNmea
                      logs_LogLoader
                     )