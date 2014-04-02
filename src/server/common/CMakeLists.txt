add_library(common_string
  string.h
  string.cpp)


add_library(common_logging
            logging.h
            logging.cpp)

add_library(common_Statistics
            Statistics.cpp
           )            

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
                     )           
cxx_test(common_TimeStampTest
         TimeStampTest.cpp
         common_TimeStamp 
         gtest_main
        )
        
add_library(common_TimeStampJson
            TimeStampJson.cpp
            TimeStampJson.h
           )
target_depends_on_poco_foundation(common_TimeStampJson)

target_depends_on_poco_foundation(common_TimeStampJson)           

cxx_test(common_PhysicalQuantityTest
  PhysicalQuantityTest.cpp
  gtest_main)

add_library(common_split
            split.cpp
            split.h
           )
target_link_libraries(common_split
                      common_Array
                      common_Uniform
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

add_library(common_Hierarchy
            Hierarchy.cpp
            Hierarchy.h
           )
target_link_libraries(common_Hierarchy
                      common_Array
                     )
cxx_test(common_HierarchyTest
         HierarchyTest.cpp
         common_Hierarchy
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

add_library(common_ConcatFunction
            ConcatFunction.h
            ConcatFunction.cpp
           )
target_link_libraries(common_ConcatFunction
                      common_Function
                     )

add_library(common_MeanStabilizer
            MeanStabilizer.cpp                     
            MeanStabilizer.h
           )
target_link_libraries(common_MeanStabilizer
                      common_Function
                     )           

cxx_test(common_ConcatFunctionTest
         ConcatFunctionTest.cpp
         common_ConcatFunction
         gtest_main
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

add_library(common_PeriodicHist
            PeriodicHist.cpp
            PeriodicHist.h
           )
target_link_libraries(common_PeriodicHist
                      common_LineKM
                      common_Array
                      common_math
                      common_string
                     )           

cxx_test(common_PeriodicHistTest
         PeriodicHistTest.cpp
         common_PeriodicHist
         gtest_main
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
