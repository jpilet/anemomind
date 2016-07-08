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

if (MongoDB_LIBRARIES)

  include_directories(${MongoDB_INCLUDE_DIR})
  add_library(tiles_NavTileUploader
              NavTileUploader.cpp
             )
  add_definitions(-DBOOST_THREAD_USE_LIB=1)
  target_link_libraries(tiles_NavTileUploader
                        tiles_NavTileGenerator
                        common_logging
                        nautical_NavCompatibility
                        ${MongoDB_LIBRARIES}

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
                        ${MongoDB_LIBRARIES}
                       )


endif (MongoDB_LIBRARIES)

           
