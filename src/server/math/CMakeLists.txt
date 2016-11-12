add_library(math_armaadolc
            armaadolc.cpp
            armaadolc.h
           )
target_link_libraries(math_armaadolc
                      common_Array
                     )
target_depends_on_armadillo(math_armaadolc)
target_depends_on_adolc(math_armaadolc)


add_library(math_LUImpl
            LUImpl.h
            LUImpl.cpp
           )
add_library(math_LU
            LU.h
            LU.cpp
           )
target_link_libraries(math_LU
                      math_armaadolc
                      math_LUImpl
                     )
cxx_test(math_LUTest
         LUTest.cpp
         math_LU
         gtest_main
        )                           

add_library(math_Grid
            Grid.cpp
            Grid.h
           )
target_link_libraries(math_Grid
                      common_Array
                      common_BBox
                      common_LineKM
                     )
target_depends_on_armadillo(math_Grid)

cxx_test(math_GridTest
         GridTest.cpp
         math_Grid
         gtest_main
        )

                     
add_library(math_PiecewiseFunction
            PiecewiseFunction.h
            PiecewiseFunction.cpp
           )
target_link_libraries(math_PiecewiseFunction           
                      common_ProportionateIndexer
                      common_LineKM
                     )

cxx_test(math_PiecewiseFunctionTest
         PiecewiseFunctionTest.cpp
         math_PiecewiseFunction
         gtest_main
        )

cxx_test(math_SubdivFractalsTest
         SubdivFractalsTest.cpp
         gtest_main
        )
        
add_library(math_SubdivFractals
            SubdivFractals.cpp
            SubdivFractals.h
           )
target_link_libraries(math_SubdivFractals
                      common_math
                     )
target_depends_on_armadillo(math_SubdivFractals)                                
        
add_executable(math_SubdivFractalsDemo
               SubdivFractalsDemo.cpp
              )
                      
target_link_libraries(math_SubdivFractalsDemo
                      plot_gnuplot
                      common_ArgMap
                      common_LineKM
                      math_SubdivFractals
                     )

cxx_test(math_QuadFormTest
         QuadFormTest.cpp
         gtest_main
         common_LineKM
         math_LUImpl
        )
        

add_library(math_nnls
            nnls.c
            nnls.cpp
           )

cxx_test(math_nnlsTest
         nnlsTest.cpp
         math_nnls
         gtest_main
        )       
        
cxx_test(math_GridSearchTest
         GridSearchTest.cpp
         gtest_main
        )
        


cxx_test(math_PiecewisePolynomialsTest
         PiecewisePolynomialsTest.cpp
         gtest_main
         math_LU
        )

add_library(math_irls
            irls.cpp
            irls.h
           )
           
target_link_libraries(math_irls
                      common_ScopedLog
                      common_string
                      common_Span
                      lapack_BandWrappers
                     )           

cxx_test(math_irlsTest
         irlsTest.cpp
         math_irls
         gtest_main
         plot_gnuplot
        )
        
cxx_test(math_PointQuadTest
         PointQuadTest.cpp
         gtest_main
         common_logging
        )

add_library(math_Resampler
  Resampler.h
  Resampler.cpp
  )

target_link_libraries(math_Resampler
  common_TimeStamp
  )

cxx_test(math_ResamplerTest
  ResamplerTest.cpp
  math_Resampler
  gtest_main
  )        

cxx_test(math_BandedLevMarTest
  BandedLevMarTest.cpp
  lapack_BandWrappers
  common_logging
  gtest_main
  common_LineKM
  )

cxx_test(math_AxisAngleTest
  AxisAngleTest.cpp
  gtest_main
  )

cxx_test(math_BandedDenseTest
  BandedDenseTest.cpp
  gtest_main
  )

add_library(math_Spline
  Spline.h
  Spline.cpp
  )

target_link_libraries(math_Spline
  common_logging
  )


cxx_test(math_SplineTest
  SplineTest.cpp
  gtest_main
  math_Spline
  )
  
cxx_test(math_PolynomialTest
  PolynomialTest.cpp
  gtest_main
  )

add_library(math_SplineUtils
  SplineUtils.h
  SplineUtils.cpp
  )

target_link_libraries(math_SplineUtils
  lapack_BandWrappers
  math_Spline
  common_TimeStamp
  math_OutlierRejector
  )

cxx_test(math_SplineUtilsTest
  SplineUtilsTest.cpp
  math_SplineUtils
  gtest_main
  plot_gnuplot
  )

add_library(math_OutlierRejector
  OutlierRejector.h
  OutlierRejector.cpp
  )

add_subdirectory(geometry)
add_subdirectory(hmm)
add_subdirectory(nonlinear)
add_subdirectory(lapack)
