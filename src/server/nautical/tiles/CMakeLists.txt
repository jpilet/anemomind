configure_file("${CMAKE_CURRENT_SOURCE_DIR}/generateDevDB.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generateDevDB.sh" @ONLY)

add_library(tiles_NavTileGenerator
            NavTileGenerator.cpp
           )

target_link_libraries(tiles_NavTileGenerator
                      common_Array
                      nautical_NavCompatibility
                      geometry_SimplifyCurve
                     )

cxx_test(tiles_NavTileGeneratorTest
         NavTileGeneratorTest.cpp
         tiles_NavTileGenerator
         gtest_main
         nautical_tiles_TileUtils
        )

  add_library(tiles_MongoUtils MongoUtils.h MongoUtils.cpp)
  target_link_libraries(tiles_MongoUtils
                        common_logging
                        common_TimeStamp
                       )
  target_depends_on_mongoc(tiles_MongoUtils)                                              
                                              
  add_library(tiles_NavTileUploader
              NavTileUploader.cpp
             )
  target_link_libraries(tiles_NavTileUploader
                        tiles_NavTileGenerator
                        common_logging
                        nautical_NavCompatibility
                        nautical_MaxSpeed
                        tiles_MongoUtils
                       )

  add_library(nautical_tiles_TileUtils
              TileUtils.h
              TileUtils.cpp
             )
  target_link_libraries(nautical_tiles_TileUtils
                        anemobox_SimulateBox
                        common_ArgMap
                        common_logging
                        filters_SmoothGpsFilter
                        logimport_LogLoader
                        math_hmm_StateAssign
                        nautical_downsamplegps
                        nautical_grammars_WindOrientedGrammar
                        tiles_NavTileUploader
                       )

  add_library(tiles_ChartTiles ChartTiles.h ChartTiles.cpp)
  target_link_libraries(tiles_ChartTiles
                        nautical_NavDataset
                        tiles_MongoUtils
                        common_MeanAndVar
                       )

  cxx_test(tiles_ChartTilesTest
           ChartTilesTest.cpp
           gtest_main
           gmock
           tiles_ChartTiles
          )
