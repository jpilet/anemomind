add_subdirectory("math")
add_subdirectory("common")
add_subdirectory("nautical")
add_subdirectory("plot")

add_executable(cmdapp
               cmdapp.cpp
              )

target_link_libraries(cmdapp
                      common_logging
                      common_PathBuilder
                      nautical_NavNmeaScan
                      nautical_grammars_Grammar001
                      nautical_NavJson
                      nautical_HTreeJson
                      common_HierarchyJson
                      math_hmm_StateAssign
                     )              

target_depends_on_poco_util(cmdapp)              
target_depends_on_poco_foundation(cmdapp)              