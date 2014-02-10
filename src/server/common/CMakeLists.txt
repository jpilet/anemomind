add_library(common_logging
            logging.h
            logging.cpp)

cxx_test(common_logging_test logging_test.cpp common_logging gtest_main)

add_library(common_Array
            Array.cpp
            Array.h
            MDArray.h
            MDInds.h
            )
cxx_test(common_ArrayTest ArrayTest.cpp
         common_Array gtest_main)

add_library(common_ContinuousRange
            ContinuousRange.h
            ContinuousRange.cpp
            )
target_link_libraries(common_ContinuousRange common_Array)
cxx_test(common_ContinousRangeTest ContinuousRangeTest.cpp
         common_ContinuousRange gtest_main)

add_library(common_Span
            Span.h
            Span.cpp
            )

add_library(common_Duration
            Duration.h
            Duration.cpp
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
            math.h
           )
target_link_libraries(common_Function
                      common_Array
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

