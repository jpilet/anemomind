add_library(geometry_SimplifyCurve
            SimplifyCurve.h
            SimplifyCurve.cpp)
target_link_libraries(geometry_SimplifyCurve common_logging)

cxx_test(geometry_SimplifyCurveTest
         SimplifyCurveTest.cpp
         geometry_SimplifyCurve
         gtest_main)
