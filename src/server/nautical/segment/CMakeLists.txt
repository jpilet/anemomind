add_library(segment_SessionCut
  SessionCut.h
  SessionCut.cpp
  )

target_link_libraries(segment_SessionCut
  math_hmm_StateAssign
  common_TimeStamp
  )

cxx_test(segment_SessionCutTest
  SessionCutTest.cpp
  gtest_main
  segment_SessionCut
  )