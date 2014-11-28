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
                     
                     
add_library(graphics_PBM
            PBM.h
            PBM.cpp
           )                      

add_library(graphics_AnemoFont
            AnemoFont.h
            AnemoFont.cpp
           )
target_link_libraries(graphics_AnemoFont
                      common_logging
                      common_LineKM
                     )                       
                     
                     
add_library(graphics_BitMapText
            BitMapText.h
            BitMapText.cpp
           )         
                     
add_executable(graphics_MakeLogo
            MakeLogo.cpp
           )
target_link_libraries(graphics_MakeLogo
                      graphics_AnemoFont
                      graphics_PPM
                      graphics_PBM
                      graphics_BitMapText
                      graphics_TextDistort
                     )
                     
add_library(graphics_AShape
            AShape.h
            AShape.cpp
           )
target_depends_on_armadillo(graphics_AShape)

add_library(graphics_GShape
            GShape.h
            GShape.cpp
           )

                     
add_library(graphics_TextDistort
            TextDistort.h
            TextDistort.cpp
           )
target_link_libraries(graphics_TextDistort
                     )

target_depends_on_armadillo(graphics_TextDistort)                                                                                     