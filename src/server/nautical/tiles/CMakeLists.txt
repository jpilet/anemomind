add_library(tiles_NavTileGenerator
            NavTileGenerator.cpp
           )

target_link_libraries(tiles_NavTileGenerator
                      common_Array
                      nautical_nav
                      geometry_SimplifyCurve
                     )


cxx_test(tiles_NavTileGeneratorTest
         NavTileGeneratorTest.cpp
         tiles_NavTileGenerator
         gtest_main
        )
