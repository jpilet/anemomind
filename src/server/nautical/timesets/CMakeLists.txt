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

add_library(timesets_IgnoreData
  IgnoreData.h
  IgnoreData.cpp
  )  

target_link_libraries(timesets_IgnoreData
  timesets_TimeSets
  )

cxx_test(timesets_IgnoreDataTest
  IgnoreDataTest.cpp
  timesets_IngoreData
  gtest_main
  )
  