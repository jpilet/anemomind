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

