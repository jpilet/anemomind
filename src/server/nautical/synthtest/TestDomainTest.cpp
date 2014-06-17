/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/TestDomain.h>
#include <server/nautical/synthtest/TestDomainJson.h>
#include <Poco/JSON/Stringifier.h>

using namespace sail;

TEST(TestDomainTest, TestTimeDomain) {
  TimeSpan ts(Duration<double>::minutes(-90),
              Duration<double>::minutes(90));
  TimeStamp x = TimeStamp::UTC(2014, 06, 17,
        11, 02, 0);

  TestTimeDomain td(TimeStamp::UTC(2014, 06, 17,
      11, 48, 0), ts);

  EXPECT_NEAR(td.fromLocal(td.toLocal(x)).toMilliSecondsSince1970(), x.toMilliSecondsSince1970(), 2);

//  Poco::Dynamic::Var jsonobj = json::serialize(td);
//  Poco::JSON::Stringifier::stringify(jsonobj, std::cout);
//
//  TestTimeDomain td2;
//  json::deserialize(jsonobj, &td2);
//  EXPECT_EQ(td, td2);
}


