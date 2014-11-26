add_library(graphics_PPM
            PPM.h
            PPM.cpp
           )
add_executable(graphics_PPMDemo
               PPMDemo.cpp
              )
target_link_libraries(graphics_PPMDemo
                      graphics_PPM
                     )            

add_library(graphics_AnemoFont
            AnemoFont.h
            AnemoFont.cpp
           )
target_link_libraries(graphics_AnemoFont
                      common_logging
                     )                                    