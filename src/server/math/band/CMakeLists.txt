cxx_test(band_BandMatrixTest
         BandMatrixTest.cpp
         gtest_main
        )

add_library(band_BandWrappers
            BandWrappers.h
            BandWrappers.cpp
           )
           
target_link_libraries(band_BandWrappers
                      common_logging
                      lapack
                      blas
                     )        
                     
cxx_test(band_BandWrappersTest
         BandWrappersTest.cpp
         gtest_main
         band_BandWrappers
        )
                
add_library(band_BandedIrls
  BandedIrls.h
  BandedIrls.cpp
  )

target_link_libraries(band_BandedIrls
  common_logging
  band_BandWrappers
  )

cxx_test(band_BandedIrlsTest
  BandedIrlsTest.cpp
  band_BandedIrls
  gtest_main
  math_OutlierRejector
  )