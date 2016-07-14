add_library(anemobox_SimulateBox
            SimulateBox.h
            SimulateBox.cpp
           )
target_link_libraries(anemobox_SimulateBox
                      anemobox_DispatcherTrueWindEstimator
                      nautical_nav
                      nautical_NavDataset
                      anemobox_Dispatcher
                     )

cxx_test(anemobox_SimulateBoxTest
  SimulateBoxTest.cpp
  gtest_main
  anemobox_SimulateBox
  anemobox_DispatcherUtils
  )