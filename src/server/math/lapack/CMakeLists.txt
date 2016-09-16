cxx_test(lapack_BandMatrixTest
         BandMatrixTest.cpp
         gtest_main
        )

add_library(lapack_BandWrappers
            BandWrappers.h
            BandWrappers.cpp
           )
           
target_link_libraries(lapack_BandWrappers
                      common_logging
                      lapack
                      blas
                     )        
                     
cxx_test(lapack_BandWrappersTest
         BandWrappersTest.cpp
         gtest_main
         lapack_BandWrappers
        )        