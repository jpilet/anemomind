{
  "targets": [
    {
      'target_name': 'action_before_build',
      'type': 'none',
      'hard_dependency': 0,
      'actions': [
        {
          'action_name': 'run_protoc',
          'inputs': [
            '../logger/logger.proto'
          ],
          'outputs': [
            "<(SHARED_INTERMEDIATE_DIR)/device/anemobox/logger/logger.pb.cc"
          ],
          'action': ['protoc','-I..', '--cpp_out=<(SHARED_INTERMEDIATE_DIR)/device/anemobox/','../logger/logger.proto']
        }
      ]
    },
    {
      "target_name": "anemonode",
      "sources": [
        "src/anemonode.cpp",
        "src/JsDispatcher.h",
        "src/JsDispatcher.cpp",
        "src/JsNmea0183Source.h",
        "src/JsNmea0183Source.cpp",
        "src/JsNmea2000Source.h",
        "src/JsNmea2000Source.cpp",
        "src/NodeNmea2000.cpp",
        "src/NodeNmea2000.h",
        "src/JsDispatchData.h",
        "src/JsDispatchData.cpp",
        "src/JsLogger.h",
        "src/JsLogger.cpp",
        "src/JsEstimator.h",
        "src/JsEstimator.cpp",
        "../Dispatcher.cpp",
        "../Dispatcher.h",
        "../TimedSampleCollection.h",
        "../DispatcherFilter.cpp",
        "../DispatcherFilter.h",
        "../DispatcherTrueWindEstimator.h",
        "../DispatcherTrueWindEstimator.cpp",
        "../Nmea0183Source.cpp",
        "../Nmea0183Source.h",
        "../Nmea2000Source.cpp",
        "../Nmea2000Source.h",
        "../ValueDispatcher.h",
        "../logger/Logger.h",
        "../logger/Logger.cpp",
        "../n2k/BitStream.cpp",
        "../n2k/BitStream.h",
        "../n2k/N2kField.cpp",
        "../n2k/N2kField.h",
        "../n2k/PgnClasses.cpp",
        "../n2k/PgnClasses.h",
        "../../Arduino/libraries/ChunkFile/ChunkFile.cpp",
        "../../Arduino/libraries/ChunkFile/ChunkFile.h",
        "../../Arduino/libraries/NmeaParser/NmeaParser.cpp",
        "../../Arduino/libraries/NmeaParser/NmeaParser.h",
        "../../Arduino/libraries/TargetSpeed/TargetSpeed.cpp",
        "../../Arduino/libraries/TargetSpeed/TargetSpeed.h",
        "../../../server/common/TimeStamp.cpp",
        "../../../server/common/TimeStamp.h",
        "../../../server/common/string.h",
        "../../../server/common/string.cpp",
        "../../../server/common/logging.h",
        "../../../server/common/logging.cpp",
        "../../../third_party/NMEA2000/src/N2kDef.h",
        "../../../third_party/NMEA2000/src/N2kDeviceList.cpp",
        "../../../third_party/NMEA2000/src/N2kDeviceList.h",
        "../../../third_party/NMEA2000/src/N2kGroupFunction.cpp",
        "../../../third_party/NMEA2000/src/N2kGroupFunctionDefaultHandlers.cpp",
        "../../../third_party/NMEA2000/src/N2kGroupFunctionDefaultHandlers.h",
        "../../../third_party/NMEA2000/src/N2kGroupFunction.h",
        "../../../third_party/NMEA2000/src/N2kMessages.cpp",
        "../../../third_party/NMEA2000/src/N2kMessagesEnumToStr.h",
        "../../../third_party/NMEA2000/src/N2kMessages.h",
        "../../../third_party/NMEA2000/src/N2kMsg.cpp",
        "../../../third_party/NMEA2000/src/N2kMsg.h",
        "../../../third_party/NMEA2000/src/N2kStream.cpp",
        "../../../third_party/NMEA2000/src/N2kStream.h",
        "../../../third_party/NMEA2000/src/NMEA2000_CompilerDefns.h",
        "../../../third_party/NMEA2000/src/NMEA2000.cpp",
        "../../../third_party/NMEA2000/src/NMEA2000.h",
        "../../../third_party/NMEA2000/src/Seasmart.cpp",
        "../../../third_party/NMEA2000/src/Seasmart.h",
        "<(SHARED_INTERMEDIATE_DIR)/device/anemobox/logger/logger.pb.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "../../..",
        "../../../third_party/NMEA2000/src",
        "../../../../build/third-party/poco-install/include",
        "<(SHARED_INTERMEDIATE_DIR)",
        "<(SHARED_INTERMEDIATE_DIR)/device/anemobox",
      ], 
      'cflags_cc!': [ '-fno-rtti' ],
      "cflags_cc": [
	"-std=c++11",
        "-fexceptions",
        '<!@(pkg-config protobuf --cflags)'
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'GCC_ENABLE_CPP_RTTI': 'YES',
        'OTHER_CFLAGS': [
          "-std=c++11",
          "-stdlib=libc++",
          "-fexceptions",
          '<!@(pkg-config protobuf --cflags)'
        ]
      },
      "defines": [ "ON_SERVER", "HAVE_CLOCK_GETTIME" ],
      "libraries" : [
        "<!@(pkg-config protobuf --libs-only-L)",
        "-lprotobuf",
        "-lrt",
	"-L/usr/lib",
	"-lboost_iostreams",
	"-lboost_iostreams-mt"
      ],
      "conditions" : [
        [ "OS=='mac'", { "libraries!": ["-lboost_iostreams"]  } ],
        [ "OS=='linux'",{ "libraries!": ["-lboost_iostreams-mt"] }  ]
      ]
    }
  ]
}
