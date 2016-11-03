add_library(filters_TimedValueIntegrator
  TimedValueIntegrator.h
  TimedValueIntegrator.cpp
 )

target_link_libraries(filters_TimedValueIntegrator
   common_TimeStamp
  )

cxx_test(filters_TimedValueIntegratorTest
         TimedValueIntegratorTest.cpp
         gtest_main
         filters_TimedValueIntegrator
        )

add_library(filters_TimedAngleIntegrator
            TimedAngleIntegrator.h
            TimedAngleIntegrator.cpp
           )

target_link_libraries(filters_TimedAngleIntegrator
                      filters_TimedValueIntegrator
                     )

cxx_test(filters_TimedAngleIntegratorTest
         TimedAngleIntegratorTest.cpp
         gtest_main
         filters_TimedAngleIntegrator
        )

cxx_test(filters_MotionsFromPairsTest
         MotionsFromPairsTest.cpp
         gtest_main
         common_logging
         common_TimeStamp
        )

add_library(filters_GpsUtils
            GpsUtils.h
            GpsUtils.cpp
           )

target_link_libraries(filters_GpsUtils
                      common_logging
                      common_TimeStamp
                      nautical_NavDataset
                      nautical_InvWGS84
                      nautical_GeographicReference
                     )

cxx_test(filters_GpsUtilsTest
         GpsUtilsTest.cpp
         filters_GpsUtils
         gtest_main
        )                     

add_library(filters_SmoothGpsFilter
            SmoothGpsFilter.h
            SmoothGpsFilter.cpp
           )

target_link_libraries(filters_SmoothGpsFilter
                      filters_GpsUtils
                      nonlinear_CeresTrajectoryFilter
                      math_Resampler
                      common_Progress
                     )

cxx_test(filters_SmoothGpsFilterTest
         SmoothGpsFilterTest.cpp
         gtest_main
         filters_SmoothGpsFilter
         common_PathBuilder
         logimport_LogLoader
         common_Env
        )

add_executable(filters_tryNewGpsFilter
  tryNewGpsFilter.cpp
  )

target_link_libraries(filters_tryNewGpsFilter
  common_TimeStamp
  segment_SessionCut
  common_ArgMap
  common_PathBuilder
  filters_SmoothGpsFilter
  logimport_LogLoader
  )

add_library(filters_SplineGpsFilter
  SplineGpsFilter.h
  SplineGpsFilter.cpp
  )

target_link_libraries(filters_SplineGpsFilter
  common_TimeStamp
  math_Spline
  math_OutlierRejector
  lapack_BandWrappers
  )

cxx_test(filters_SplineGpsFilterTest
  SplineGpsFilterTest.cpp
  gtest_main
  filters_SplineGpsFilter
  )