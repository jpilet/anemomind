/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <device/anemobox/DispatcherUtils.h>

using namespace sail;

TEST(DispatcherUtilsTest, Merging) {
  auto offset = sail::TimeStamp::UTC(2016, 02, 26, 16, 4, 0);
  auto s = Duration<double>::seconds(1.0);
  auto kn = Velocity<double>::knots(1.0);

  const DataCode Code = AWS;
  typedef TypeForCode<Code>::type T;
  typedef TimedSampleCollection<T>::TimedVector TimedVector;

  {
    TimedVector A{
      {offset + 0.3*s,    3.0*kn},
      {offset + 0.6*s,    4.0*kn},
      {offset + 1233.0*s, 13.0*kn}
    };
    TimedVector B{
      {offset + 0.4*s,    17.0*kn}
    };

    std::map<std::string, std::shared_ptr<DispatchData>> sources{
      {"A", makeDispatchDataFromSamples<Code>("A", A)},
      {"B", makeDispatchDataFromSamples<Code>("B", B)},
    };

    std::map<std::string, int> priorities{
      {"A", 1},
      {"B", 2}
    };

    auto merged = mergeChannels(Code, "C", priorities, sources);
    auto values = toTypedDispatchData<Code>(merged.get())->dispatcher()->values().samples();
    EXPECT_EQ(values.size(), 2);
    EXPECT_NEAR((values[0].time - offset).seconds(), 0.4, 1.0e-6);
    EXPECT_NEAR(values[0].value.knots(), 17.0, 1.0e-6);
    EXPECT_NEAR((values[1].time - offset).seconds(), 1233.0, 1.0e-6);
    EXPECT_NEAR(values[1].value.knots(), 13.0, 1.0e-6);
  }


}

