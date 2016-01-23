add_library(n2k_BitStream
            BitStream.h
            BitStream.cpp
           )

cxx_test(n2k_BitStreamTest BitStreamTest.cpp n2k_BitStream gtest_main)

add_library(n2k_N2kField
            N2kField.h
            N2kField.cpp
           )
           
target_link_libraries(n2k_N2kField
                      n2k_BitStream
                      common_TimeStamp
                     )

cxx_test(n2k_N2kFieldTest
         N2kFieldTest.cpp
         n2k_N2kField
         gtest_main
        )

add_library(n2k_PgnClasses
            PgnClasses.h
            PgnClasses.cpp
           )

target_link_libraries(n2k_PgnClasses
                      n2k_N2kField
                     )

cxx_test(n2k_PgnClassesTest
         PgnClassesTest.cpp
         n2k_PgnClasses
         gtest_main
        )