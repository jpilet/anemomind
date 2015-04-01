{
  "targets": [
    {
      "target_name": "anemonode",
      "sources": [
        "anemonode.cpp",
        "../Dispatcher.cpp",
        "../Dispatcher.h",
        "../main.cpp",
        "../Nmea0183Source.cpp",
        "../Nmea0183Source.h",
        "../ValueDispatcher.h",
        "../../Arduino/libraries/NmeaParser/NmeaParser.cpp",
        "../../Arduino/libraries/NmeaParser/NmeaParser.h",
        "../../../server/common/TimeStamp.cpp",
        "../../../server/common/TimeStamp.h",
        "../../../server/common/string.h",
        "../../../server/common/string.cpp",
        "../../../server/common/logging.h",
        "../../../server/common/logging.cpp"
      ],
      "include_dirs": [
        "../../.."
      ], 
      "cflags_cc": [
	"-std=c++11", "-fexceptions"
      ],
      "defines": [ "ON_SERVER" ],
      "libraries" : [ "-lPocoFoundation", "-lPocoUtil" ]
    }
  ]
}
