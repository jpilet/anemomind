add_subdirectory("math")
add_subdirectory("common")
add_subdirectory("nautical")
add_subdirectory("plot")

add_executable(cmdapp
               cmdapp.cpp
              )

target_depends_on_poco_util(cmdapp)              