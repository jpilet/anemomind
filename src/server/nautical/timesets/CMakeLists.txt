add_library(timesets_TimeSets
  TimeSets.h
  TimeSets.cpp
  )
target_depends_on_mongoc(timesets_TimeSets)  

cxx_test(timesets_TimeSetsTest
  TimeSetsTest.cpp
  gtest_main
  timesets_TimeSets
  tiles_MongoUtils
  )
target_depends_on_mongoc(timesets_TimeSetsTest)  