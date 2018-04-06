add_library(astra_AstraLoader
  AstraLoader.h
  AstraLoader.cpp
  )
target_link_libraries(astra_AstraLoader
  common_string
  common_TimeStamp
  )

cxx_test(astra_AstraLoaderTest
  AstraLoaderTest.cpp
  gtest_main
  common_PathBuilder
  common_Env
  astra_AstraLoader
  )
