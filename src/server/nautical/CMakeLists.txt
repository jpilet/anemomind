add_library(nautical_nav
            Nav.h
            Nav.cpp
            NavBBox.h
            NavBBox.cpp)
        
target_link_libraries(nautical_nav
  common_Array
  common_LineKM
  common_Duration
  plot_gnuplot
  nautical_Ecef
  common_ContinuousRange)


          
add_library(nautical_ecef
            Ecef.h
            Ecef.cpp)

target_link_libraries(nautical_ecef common_math)



add_library(nautical_DataCalib
            DataCalib.h
            DataCalib.cpp)

target_link_libraries(nautical_DataCalib common_Array)


add_library(nautical_LocalRace
            LocalRace.h
            LocalRace.cpp)

target_link_libraries(nautical_LocalRace nautical_nav common_Array plot_gnuplot math_Grid)