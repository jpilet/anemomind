cxx_test(lapack_BandMatrixTest
         BandMatrixTest.cpp
         gtest_main
        )

add_library(lapack_BandedWrappers
            BandedWrappers.h
            BandedWrappers.cpp
           )
           
target_link_libraries(lapack_BandedWrappers
                      common_logging
                      lapack
                      blas
                     )        
                     
cxx_test(lapack_BandedWrappersTest
         BandedWrappersTest.cpp
         gtest_main
         lapack_BandedWrappers
        )        