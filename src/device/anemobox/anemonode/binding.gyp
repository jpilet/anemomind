{
  "targets": [
    {
      "target_name": "anemonode",
      "sources": [
        "anemonode.cpp",
        "JsNmea0183Source.h",
        "JsNmea0183Source.cpp",
        "JsDispatchData.h",
        "JsDispatchData.cpp",
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
        "<!(node -e \"require('nan')\")",
        "../../..",
        "../../../../build/third-party/poco-install/include"
      ], 
      "cflags_cc": [
	"-std=c++11", "-fexceptions"
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.7',

        'OTHER_CFLAGS': [
          "-std=c++11",
          "-stdlib=libc++",
          "-fexceptions"
        ]
      },
      "defines": [ "ON_SERVER" ],
      "libraries" : [
        "-L../../../../../build/third-party/poco-install/lib",
        "-lPocoFoundation",
        "-lPocoUtil"
      ]
    }
  ]
}
