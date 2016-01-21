add_library(n2k_BitStream
            BitStream.h
            BitStream.cpp
           )

cxx_test(n2k_BitStreamTest BitStreamTest.cpp n2k_BitStream gtest_main)

add_library(n2k_PgnClasses
            PgnClasses.h
            PgnClasses.cpp
           )

target_link_libraries(n2k_PgnClasses
                      n2k_BitStream
                     )

cxx_test(n2k_PgnClassesTest
         PgnClassesTest.cpp
         n2k_PgnClasses
         gtest_main
        )           