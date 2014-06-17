/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/TestDomain.h>
#include <server/nautical/synthtest/TestDomainJson.h>

using namespace sail;

TEST(TestDomainTest, TestTimeDomain) {
  TimeSpan ts(Duration<double>::minutes(-90),
              Duration<double>::minutes(90));
  TimeStamp x = TimeStamp::UTC(2014, 06, 17,
        11, 02, 0);

  TestTimeDomain td(TimeStamp::UTC(2014, 06, 17,
      11, 48, 0), ts);

  EXPECT_NEAR(td.fromLocal(td.toLocal(x)).toMilliSecondsSince1970(), x.toMilliSecondsSince1970(), 2);

  Poco::Dynamic::Var jsonobj = json::serialize(td);
  TestTimeDomain td2;
  json::deserialize(jsonobj, &td2);
  EXPECT_EQ(td, td2);
}

TEST(TestDomainTest, TestSpaceDomain) {
  GeographicPosition<double> pos(Angle<double>::radians(0.5), Angle<double>::radians(0.5), Length<double>::meters(0.0));
  GeographicReference ref(pos);
  LengthSpan span = LengthSpan::centeredAt0(Length<double>::nauticalMiles(60*30));


  TestSpaceDomain sd(pos, span, span);

  GeographicPosition<double> x(Angle<double>::radians(0.57), Angle<double>::radians(0.589), Length<double>::meters(0.0));
  GeographicPosition<double> y = sd.fromLocal(sd.toLocal(x));
  EXPECT_NEAR(x.lon().radians(), y.lon().radians(), 0.001);
  EXPECT_NEAR(x.lat().radians(), y.lat().radians(), 0.001);
  EXPECT_NEAR(x.alt().meters(), y.alt().meters(), 0.001);

  TestSpaceDomain sd2;
  json::deserialize(json::serialize(sd), &sd2);

  EXPECT_EQ(sd, sd2);
}

TEST(TestDomainTest, TestDomain) {
  TimeSpan ts(Duration<double>::minutes(-90),
              Duration<double>::minutes(90));
  TimeStamp x = TimeStamp::UTC(2014, 06, 17,
        11, 02, 0);

  TestTimeDomain td(TimeStamp::UTC(2014, 06, 17,
      11, 48, 0), ts);

  GeographicPosition<double> pos(Angle<double>::radians(0.5), Angle<double>::radians(0.5), Length<double>::meters(0.0));
  GeographicReference ref(pos);
  LengthSpan span = LengthSpan::centeredAt0(Length<double>::nauticalMiles(60*30));

  TestSpaceDomain sd(pos, span, span);

  TestDomain d(sd, td);
  TestDomain d2;
  json::deserialize(json::serialize(d), &d2);
  EXPECT_EQ(d, d2);
}


