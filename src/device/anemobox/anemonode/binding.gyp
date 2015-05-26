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
            "<(SHARED_INTERMEDIATE_DIR)/logger.pb.cc"
          ],
          'action': ['protoc','-I..', '--cpp_out=<(SHARED_INTERMEDIATE_DIR)/','../logger/logger.proto']
        }
      ]
    },
    {
      "target_name": "anemonode",
      "sources": [
        "anemonode.cpp",
        "JsNmea0183Source.h",
        "JsNmea0183Source.cpp",
        "JsDispatchData.h",
        "JsDispatchData.cpp",
        "JsLogger.h",
        "JsLogger.cpp",
        "JsEstimator.h",
        "JsEstimator.cpp",
        "../Dispatcher.cpp",
        "../Dispatcher.h",
        "../DispatcherFilter.cpp",
        "../DispatcherFilter.h",
        "../DispatcherTrueWindEstimator.h",
        "../DispatcherTrueWindEstimator.cpp",
        "../Nmea0183Source.cpp",
        "../Nmea0183Source.h",
        "../ValueDispatcher.h",
        "../logger/Logger.h",
        "../logger/Logger.cpp",
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
        "<(SHARED_INTERMEDIATE_DIR)/logger/logger.pb.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "../../..",
        "../../../../build/third-party/poco-install/include",
        "<(SHARED_INTERMEDIATE_DIR)",
        "<(SHARED_INTERMEDIATE_DIR)/logger",
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
      "defines": [ "ON_SERVER" ],
      "libraries" : [
        "<!@(pkg-config protobuf --libs-only-L)",
        "-lprotobuf",
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
