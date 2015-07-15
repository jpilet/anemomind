add_subdirectory("common")

if (${FULL_BUILD})
  add_subdirectory("math")
  add_subdirectory("nautical")
endif ()

add_subdirectory("plot")    
