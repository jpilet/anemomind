add_library(lapack_DgbsvWrapper
            DgbsvWrapper.h
            DgbsvWrapper.cpp
           )

target_link_libraries(lapack_DgbsvWrapper
                      common_logging
                      lapack
                      blas
                     )

cxx_test(lapack_BandMatrixTest
         BandMatrixTest.cpp
         gtest_main
        )

cxx_test(lapack_DgbsvWrapperTest
         DgbsvWrapperTest.cpp
         gtest_main
         lapack_DgbsvWrapper
        )