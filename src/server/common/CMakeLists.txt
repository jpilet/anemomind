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

add_library(common_ScopedLog
            ScopedLog.h
            ScopedLog.cpp)
target_link_libraries(common_ScopedLog common_logging)

add_library(common_TimeStamp
            TimeStamp.h
            TimeStamp.cpp
           )
target_link_libraries(common_TimeStamp
                      common_logging
                      common_string
                      common_mkgmtime
                     )           
cxx_test(common_TimeStampTest
         TimeStampTest.cpp
         common_TimeStamp
         common_string 
         gtest_main
        )



        
add_library(common_TimeStampJson
            TimeStampJson.cpp
            TimeStampJson.h
           )
target_depends_on_poco_foundation(common_TimeStampJson)

                              

add_library(common_filesystem
            filesystem.h
            filesystem.cpp
           )
target_depends_on_poco_foundation(common_filesystem)  

cxx_test(common_PhysicalQuantityTest
  PhysicalQuantityTest.cpp
  gtest_main)

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
         gtest_main
        )
        
add_library(common_HierarchyJson
            HierarchyJson.cpp        
            HierarchyJson.h
           )
target_link_libraries(common_HierarchyJson
                      common_Hierarchy
                     )
target_depends_on_poco_json(common_HierarchyJson)

add_library(common_Json
            Json.h
            Json.cpp
           )
target_depends_on_poco_json(common_Json)

cxx_test(common_HierarchyJsonTest
         HierarchyJsonTest.cpp
         common_HierarchyJson
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

add_library(common_mkgmtime
            mkgmtime.cpp
            mkgmtime.h
           )           
