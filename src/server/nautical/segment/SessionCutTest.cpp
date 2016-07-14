/*
 * SessionCutTest.cpp
 *
 *  Created on: Jun 2, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/segment/SessionCut.h>

using namespace sail;

namespace {

  auto offset = TimeStamp::UTC(2016, 6, 2, 18, 7, 0);
  auto seconds = Duration<double>::seconds(1.0);

  TimeStamp t(double s) {
    return offset + s*seconds;
  }
}

TEST(SessionCutTest, BigGap) {
  Array<TimeStamp> times{
    t(0), t(1), t(2), t(30), t(31), t(32)
  };


  {
    SessionCut::Settings settings;
    settings.cuttingThreshold = 2.0*seconds;
    settings.regularization = 0.0;

    auto spans = SessionCut::cutSessions(
        wrapIndexable<TypeMode::ConstRef>(times), settings);
    EXPECT_EQ(spans.size(), 2);
    auto a = spans[0];
    auto b = spans[1];
    EXPECT_EQ(a.minv(), t(0));
    EXPECT_EQ(a.maxv(), t(2));
    EXPECT_EQ(b.minv(), t(30));
    EXPECT_EQ(b.maxv(), t(32));
  }{
    SessionCut::Settings settings;
    settings.cuttingThreshold = 2.0*seconds;
    settings.regularization = 1.0e3;

    auto spans = SessionCut::cutSessions(
        wrapIndexable<TypeMode::ConstRef>(times), settings);
    EXPECT_EQ(spans.size(), 0);
  }
}


TEST(SessionCutTest, SmallGap) {
  Array<TimeStamp> times{
    t(0), t(1), t(2), t(5), t(6), t(7)
  };

  {
    SessionCut::Settings settings;
    settings.cuttingThreshold = 2.0*seconds;
    settings.regularization = 0.0;

    auto spans = SessionCut::cutSessions(
        wrapIndexable<TypeMode::ConstRef>(times), settings);
    EXPECT_EQ(spans.size(), 2);
    auto a = spans[0];
    auto b = spans[1];
    EXPECT_EQ(a.minv(), t(0));
    EXPECT_EQ(a.maxv(), t(2));
    EXPECT_EQ(b.minv(), t(5));
    EXPECT_EQ(b.maxv(), t(7));
  }{
    SessionCut::Settings settings;
    settings.cuttingThreshold = 2.0*seconds;
    settings.regularization = 1.0e3;

    auto spans = SessionCut::cutSessions(
        wrapIndexable<TypeMode::ConstRef>(times), settings);
    EXPECT_EQ(spans.size(), 1);
    auto span = spans[0];
    EXPECT_EQ(span.minv(), t(0));
    EXPECT_EQ(span.maxv(), t(7));
  }
}


