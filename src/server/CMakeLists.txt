add_subdirectory("common")
add_subdirectory("transducers")

if (${FULL_BUILD})
  add_subdirectory("math")
  add_subdirectory("nautical")
  add_subdirectory("html")
endif ()

add_subdirectory("plot")    
