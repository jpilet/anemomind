#include <device/anemobox/ValueDispatcher.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unistd.h>

using namespace sail;

class MockListener : public Listener<int> {
 public:
  MockListener(Duration<> d = Duration<>::seconds(0)) : Listener<int>(d) { }
  virtual void onNewValue(const ValueDispatcher<int> &dispatcher) {
    gotValue(dispatcher.lastValue());
  }
  MOCK_METHOD1(gotValue, void(int));
};

TEST(ValueDispatcherTest, Notify) {
  MockListener listener;
  ValueDispatcher<int> dispatcher(1);
  dispatcher.subscribe(&listener);

  EXPECT_CALL(listener, gotValue(3));
  dispatcher.setValue(3);

  dispatcher.unsubscribe(&listener);

  dispatcher.setValue(4);
  // listener should not be called.
}

TEST(ValueDispatcher, AvoidFlooding) {
  ValueDispatcher<int> dispatcher(1);
  MockListener listener(Duration<>::milliseconds(5));
  dispatcher.subscribe(&listener);

  EXPECT_CALL(listener, gotValue(3));
  dispatcher.setValue(3);
  dispatcher.setValue(4);  // too close, ignored.

  EXPECT_CALL(listener, gotValue(5));
  usleep(10000);
  dispatcher.setValue(5);
}
