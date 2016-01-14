add_library(n2k_BitStream
            BitStream.h
            BitStream.cpp
           )

cxx_test(n2k_BitStreamTest BitStreamTest.cpp n2k_BitStream gtest_main)
