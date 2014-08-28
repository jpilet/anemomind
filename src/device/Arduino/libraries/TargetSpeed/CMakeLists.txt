add_library(device_TargetSpeed
            TargetSpeed.h
            TargetSpeed.cpp
           )

# The dependency exists only when building with ON_SERVER defined.
target_link_libraries(device_TargetSpeed
                      plot_gnuplot
                      device_ChunkFile
                     )

add_subdirectory("test")
