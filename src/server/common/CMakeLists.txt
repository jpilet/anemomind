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

add_library(common_filesystem
            filesystem.h
            filesystem.cpp
           )
target_depends_on_poco_foundation(common_filesystem)
target_link_libraries(common_filesystem
                      common_string
                     )           

add_library(common_ScopedLog
            ScopedLog.h
            ScopedLog.cpp)
target_link_libraries(common_ScopedLog common_logging)

add_library(common_TimeStamp
            TimeStamp.h
            TimeStamp.cpp
           )
target_depends_on_poco_foundation(common_TimeStamp)
target_link_libraries(common_TimeStamp
                      common_logging
                      common_string
                     )           
cxx_test(common_TimeStampTest
         TimeStampTest.cpp
         common_TimeStamp
         common_string 
         gtest_main
        )


        
add_library(common_Json
            Json.h
            Json.cpp
           )
target_depends_on_poco_json(common_Json)                                                     
cxx_test(common_JsonTest
         JsonTest.cpp
         common_Json
         gtest_main
        )                     
                   
        
add_library(common_TimeStampJson
            TimeStampJson.cpp
            TimeStampJson.h
           )
target_depends_on_poco_foundation(common_TimeStampJson)

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

add_library(common_HierarchyJson
            HierarchyJson.cpp        
            HierarchyJson.h
           )
target_link_libraries(common_HierarchyJson
                      common_Hierarchy
                      common_Json
                     )
target_depends_on_poco_json(common_HierarchyJson)

cxx_test(common_HierarchyJsonTest
         HierarchyJsonTest.cpp
         common_HierarchyJson
         common_HNodeGroup
         gtest_main
        )                                

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

cxx_test(common_pocoTest
         pocoTest.cpp
         gtest_main
        )
target_depends_on_poco_json(common_pocoTest) 

add_library(common_ConcatFunction
            ConcatFunction.h
            ConcatFunction.cpp
           )
target_link_libraries(common_ConcatFunction
                      common_Function
                     )                
cxx_test(common_ConcatFunctionTest
         ConcatFunctionTest.cpp
         common_ConcatFunction
         gtest_main
        )

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

add_library(common_LineKMJson
            LineKMJson.cpp
            LineKMJson.h
           )
target_link_libraries(common_LineKMJson
                      common_LineKM
                     )
target_depends_on_poco_json(common_LineKMJson)                                

cxx_test(common_SpanOverlapTest
         SpanOverlapTest.cpp
         gtest_main
        )                                
target_depends_on_poco_json(common_LineKMJson)


add_library(common_JsonObjDeserializer
            JsonObjDeserializer.h
            JsonObjDeserializer.cpp
           )
target_depends_on_poco_json(common_JsonObjDeserializer)

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

add_library(common_JsonIO
            JsonIO.h
            JsonIO.cpp
           )
target_depends_on_poco_json(common_JsonIO)


add_library(common_ProportionateSampler
           ProportionateSampler.cpp
           ProportionateSampler.h
          )

cxx_test(common_ProportionateSamplerTest
         ProportionateSamplerTest.cpp
         common_ProportionateSampler
         gtest_main
        )

add_library(common_DataSplits
            DataSplits.h
            DataSplits.cpp
           )
           
target_link_libraries(common_DataSplits
                      common_Uniform
                     )           

add_library(common_MeanAndVar
            MeanAndVar.h
            MeanAndVar.cpp
           )
