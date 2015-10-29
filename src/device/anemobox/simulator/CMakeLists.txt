add_library(anemobox_ReplayDispatcher
            ReplayDispatcher.h
            ReplayDispatcher.cpp
           )

target_link_libraries(anemobox_ReplayDispatcher
                      anemobox_Dispatcher
                      nautical_nav
                      common_TimeStamp
                     )

cxx_test(anemobox_ReplayDispatcherTest
         ReplayDispatcherTest.cpp
         anemobox_ReplayDispatcher
         gtest_main
        )

add_library(anemobox_SimulateBox
            SimulateBox.h
            SimulateBox.cpp
           )
target_link_libraries(anemobox_SimulateBox
                      anemobox_DispatcherTrueWindEstimator
                      anemobox_ReplayDispatcher
                      nautical_nav
                     )

