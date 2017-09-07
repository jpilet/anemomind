add_library(timesets_TimeSets
  TimeSets.h
  TimeSets.cpp
  )
target_depends_on_mongoc(timesets_TimeSets)

target_link_libraries(timesets_TimeSets
  common_TimeStamp
  tiles_MongoUtils
  )

cxx_test(timesets_TimeSetsTest
  TimeSetsTest.cpp
  gtest_main
  timesets_TimeSets
  )
target_depends_on_mongoc(timesets_TimeSetsTest)  

add_library(timesets_IgnoreData
  IgnoreData.h
  IgnoreData.cpp
  )  
target_depends_on_mongoc(timesets_IgnoreData)    

target_link_libraries(timesets_IgnoreData
  timesets_TimeSets
  anemobox_Dispatcher
  )

cxx_test(timesets_IgnoreDataTest
  IgnoreDataTest.cpp
  timesets_IgnoreData
  gtest_main
  )
target_depends_on_mongoc(timesets_IgnoreDataTest)  
  