add_library(math_LUImpl
            LUImpl.h
            LUImpl.cpp
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
         math_LUImpl
        )

add_library(math_irls
            irls.cpp
            irls.h
           )
           
target_link_libraries(math_irls
                      common_ScopedLog
                      common_string
                      common_Span
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

add_library(math_SampleUtils
  SampleUtils.h
  SampleUtils.cpp
  )

target_link_libraries(math_SampleUtils
  common_TimeStamp
  )

cxx_test(math_SampleUtilsTest
  SampleUtilsTest.cpp
  math_SampleUtils
  gtest_main
  )        

cxx_test(math_PolynomialTest
  PolynomialTest.cpp
  gtest_main
  )

add_library(math_OutlierRejector
  OutlierRejector.h
  OutlierRejector.cpp
  )

add_library(math_Curve2dFilter
  Curve2dFilter.h
  Curve2dFilter.cpp
  )

target_link_libraries(math_Curve2dFilter
  spline_SplineUtils
  band_BandedIrls
  math_OutlierRejector
  common_DOMUtils
  math_SampleUtils
  )

cxx_test(math_Curve2dFilterTest
  Curve2dFilterTest.cpp
  math_Curve2dFilter
  gtest_main
  )

add_subdirectory(spline)
add_subdirectory(geometry)
add_subdirectory(hmm)
add_subdirectory(nonlinear)
add_subdirectory(band)
