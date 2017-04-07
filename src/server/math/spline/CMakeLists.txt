add_library(spline_Spline
  Spline.h
  Spline.cpp
  )

target_link_libraries(spline_Spline
  common_logging
  )


cxx_test(spline_SplineTest
  SplineTest.cpp
  gtest_main
  spline_Spline
  )
  
add_library(spline_SplineUtils
  SplineUtils.h
  SplineUtils.cpp
  )

target_link_libraries(spline_SplineUtils
  band_BandWrappers
  spline_Spline
  common_TimeStamp
  )