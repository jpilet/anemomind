add_library(experimental_Experimental
	Experimental.h
	Experimental.cpp
	)

target_link_libraries(experimental_Experimental
	logimport_LogLoader
	nautical_NavCompatibility
	common_Env
	common_PathBuilder
	)
	
cxx_test(experimental_experimentalTest
	experimentalTest.cpp
	experimental_Experimental
	gtest_main
	)
	
add_executable(experimental_experiment
	experiment.cpp
	)
	
target_link_libraries(experimental_experiment
	experimental_Experimental
    )		