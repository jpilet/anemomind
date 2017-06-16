configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Env.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/Env.cpp" @ONLY)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/Env.cpp.in" Env.cpp)

cxx_test(common_ArrayBuilderTest
         ArrayBuilderTest.cpp
         gtest_main
        )

add_library(common_Env
            "${CMAKE_CURRENT_BINARY_DIR}/Env.cpp"
            Env.h
           )

if (${FULL_BUILD})
  add_library(common_PathBuilder
              PathBuilder.h
              PathBuilder.cpp
             )
  target_depends_on_poco_foundation(common_PathBuilder)
           
  cxx_test(common_MDIndsTest
           MDIndsTest.cpp
           gtest_main
          )           
             
  cxx_test(common_EnvTest
           EnvTest.cpp
           common_Env
           gtest_main
          )           
  target_depends_on_poco_foundation(common_EnvTest)
endif ()

cxx_test(common_stringTest
         stringTest.cpp
         common_string
         gtest_main
        )

add_library(common_string
  string.h
  string.cpp)


add_library(common_logging
            logging.h
            logging.cpp)

cxx_test(common_logging_test logging_test.cpp common_logging gtest_main)

if (${FULL_BUILD})
  add_library(common_filesystem
              filesystem.h
              filesystem.cpp
             )
  target_depends_on_poco_foundation(common_filesystem)
  target_link_libraries(common_filesystem
                        common_string
                       )           
endif ()

add_library(common_ScopedLog
            ScopedLog.h
            ScopedLog.cpp)
target_link_libraries(common_ScopedLog common_logging)


# clock_gettime is linux-specific and might be in libc or librt.
include(CheckFunctionExists)
include(CheckLibraryExists)
check_library_exists(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME)
set(CLOCK_GETTIME_LIB "")
if (HAVE_CLOCK_GETTIME)
    set(CLOCK_GETTIME_LIB "rt")
else()
    # might also be in libc
    check_library_exists(c clock_gettime "" HAVE_CLOCK_GETTIME)
endif()

if (HAVE_CLOCK_GETTIME)
  add_definitions(-DHAVE_CLOCK_GETTIME)
endif()

add_library(common_TimeStamp
            TimeStamp.h
            TimeStamp.cpp
           )
target_link_libraries(common_TimeStamp
                      common_logging
                      common_string
                      ${CLOCK_GETTIME_LIB}
                     )           
cxx_test(common_TimeStampTest
         TimeStampTest.cpp
         common_TimeStamp
         common_string 
         gtest_main
        )
        
add_library(common_Array
            Array.cpp
            Array.h
            MDArray.h
            MDInds.h
            ArrayIO.h
            )
cxx_test(common_ArrayTest ArrayTest.cpp
         common_Array gtest_main)

add_library(common_PeriodicSpan
            PeriodicSpan.h
            PeriodicSpan.cpp
           )
target_link_libraries(common_PeriodicSpan common_Array)
cxx_test(common_PeriodicSpanTest PeriodicSpanTest.cpp
         common_PeriodicSpan gtest_main)

add_library(common_Span
            Span.h
            Span.cpp
            )

if (${FULL_BUILD})
add_library(common_Hierarchy
            Hierarchy.cpp
            Hierarchy.h
           )
target_link_libraries(common_Hierarchy
                      common_Array
                      common_string
                      common_logging
                     )
cxx_test(common_HierarchyTest
         HierarchyTest.cpp
         common_Hierarchy
         common_HNodeGroup
         gtest_main
        )

add_library(common_HNodeGroup
            HNodeGroup.h
            HNodeGroup.cpp
           )
target_link_libraries(common_HNodeGroup
                      common_Hierarchy
                      common_string
                     )
cxx_test(common_HNodeGroupTest
         HNodeGroupTest.cpp
         common_HNodeGroup
         gtest_main
        )

endif ()

add_library(common_Function
            Function.cpp
            Function.h
           )
target_link_libraries(common_Function
                      common_Array
                      common_math
                     )

add_library(common_BBox
            BBox.cpp
            BBox.h
           )
target_link_libraries(common_BBox
                      common_Span
                     )

add_library(common_LineKM
            LineKM.cpp
            LineKM.h
           )

add_library(common_Uniform
            Uniform.cpp
            Uniform.h
           )
target_link_libraries(common_Uniform
                      common_LineKM
                     )


add_library(common_math
  math.h
  math.cpp)
cxx_test(common_mathTest
         mathTest.cpp
         common_math
         gtest_main
        )  

cxx_test(common_invalidate
         invalidate_test.cpp
         gtest_main)

if (${FULL_BUILD})
cxx_test(common_pocoTest
         pocoTest.cpp
         gtest_main
        )
target_depends_on_poco_json(common_pocoTest) 
endif ()

add_library(common_Histogram
            Histogram.cpp
            Histogram.h
           )

target_link_libraries(common_Histogram
                      common_Array
                      common_LineKM
                      common_Span
                     )                   

cxx_test(common_HistogramTest
         HistogramTest.cpp
         common_Histogram
         gtest_main
        )

if (${FULL_BUILD})
  
  cxx_test(common_SpanOverlapTest
           SpanOverlapTest.cpp
           gtest_main
          )                                


endif ()

add_library(common_ArgMap
            ArgMap.h
            ArgMap.cpp
           )

target_link_libraries(common_ArgMap
                      common_logging
                      common_string
                     )

cxx_test(common_ArgMapTest
         ArgMapTest.cpp
         common_ArgMap
         gtest_main
         common_logging
         common_string
        )

add_library(common_DataSplits
            DataSplits.h
            DataSplits.cpp
           )

add_library(common_MeanAndVar
            MeanAndVar.h
            MeanAndVar.cpp
           )

target_link_libraries(common_MeanAndVar
                      common_Span
                      common_logging
                     )

cxx_test(common_MeanAndVarTest
         MeanAndVarTest.cpp
         gtest_main
         common_MeanAndVar
        )

add_library(common_ProportionateIndexer
            ProportionateIndexer.cpp
            ProportionateIndexer.h
           )

add_library(common_Progress
            Progress.cpp
            Progress.h
           )
target_link_libraries(common_Progress
                       common_TimeStamp
                      )

cxx_test(common_ProportionateIndexerTest
         ProportionateIndexerTest.cpp
         gtest_main
         common_ProportionateIndexer
        )

add_library(common_EnvUtil
            EnvUtil.h
            EnvUtil.cpp
           )

target_link_libraries(common_EnvUtil
                      common_PathBuilder
                      common_Env
                     )    
                     
                     
add_library(common_CsvParser
            CsvParser.h
            CsvParser.cpp
           )           
target_link_libraries(common_CsvParser
                      common_string
                     )             
cxx_test(common_CsvParserTest
         CsvParserTest.cpp
         common_CsvParser
         gtest_main
        )                      
        
cxx_test(common_FunctionalTest
         FunctionalTest.cpp
         gtest_main
        )
        
cxx_test(common_numericsTest
         numericsTest.cpp
         gtest_main
        )        

cxx_test(common_TimedValuePairsTest
         TimedValuePairsTest.cpp
         gtest_main
         common_TimeStamp
         common_logging
        )

cxx_test(common_AbstractArray
         AbstractArrayTest.cpp
         gtest_main
        )

cxx_test(common_PositiveModTest
         PositiveModTest.cpp
         gtest_main
        )

cxx_test(common_MultiMergeTest
         MultiMergeTest.cpp
         gtest_main
        )

add_library(common_DOMUtils
  DOMUtils.h
  DOMUtils.cpp
  )

target_depends_on_poco_foundation(common_DOMUtils)
target_depends_on_poco_xml(common_DOMUtils)

target_link_libraries(common_DOMUtils
  common_PathBuilder
  )

add_library(common_StreamUtils
  StreamUtils.h
  StreamUtils.cpp
  )

add_library(common_DynamicUtils
  DynamicUtils.h
  DynamicUtils.cpp
  )
target_depends_on_poco_json(common_DynamicUtils)

cxx_test(common_DynamicUtilsTest
  DynamicUtilsTest.cpp
  common_DynamicUtils
  gtest_main
  )

cxx_test(common_TransducerTest
  TransducerTest.cpp
  gtest_main
  )

add_library(common_FrequencyLimiter
  FrequencyLimiter.h
  FrequencyLimiter.cpp
  )

target_link_libraries(common_FrequencyLimiter
  common_TimeStamp)

add_library(common_Keyword
  Keyword.h
  Keyword.cpp
  )

target_link_libraries(common_Keyword
  common_logging
  )

cxx_test(common_KeywordTest
  KeywordTest.cpp
  gtest_main
  common_Keyword
  )