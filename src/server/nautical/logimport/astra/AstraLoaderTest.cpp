/*
 * AstraLoaderTest.cpp
 *
 *  Created on: 9 Mar 2018
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/logimport/astra/AstraLoader.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/Optional.h>
#include <fstream>
#include <server/common/Functional.h>
#include <server/common/string.h>
#include <regex>
#include <server/transducers/ParseTransducers.h>

using namespace sail;

TEST(TestAstraLoader, RegexTest) {
  EXPECT_EQ(
      tryParseAstraHeader("-------- abc ------").get().value,
      "abc");
  EXPECT_FALSE(
      tryParseAstraHeader("-------- abc ").defined());

  EXPECT_FALSE(
      tryParseAstraHeader(" abc ------").defined());

}

namespace {
  Optional<AstraColSpec> tryParseAstraColSpecStr(const std::string& s) {
    auto tokens = tokenizeAstra(s);
    return tryParseAstraColSpec(tokens);
  }
}

TEST(TestAstraLoader, ColSpecTest) {
  EXPECT_EQ(tryParseAstraColSpecStr("   AWA    AWS BS").get().size(), 3);
  EXPECT_EQ(tryParseAstraColSpecStr("   AWA  TWA  AWS BS").get().size(), 4);
  EXPECT_EQ(tryParseAstraColSpecStr("  AWA     ").get().size(), 1);
  EXPECT_EQ(tryParseAstraColSpecStr("AWA     ").get().size(), 1);
  EXPECT_FALSE(tryParseAstraColSpecStr("  kattskit     ").defined());
  EXPECT_FALSE(tryParseAstraColSpecStr("kattskit").defined());
  EXPECT_FALSE(tryParseAstraColSpecStr("kattskit     ").defined());
  EXPECT_FALSE(tryParseAstraColSpecStr("   kattskit").defined());
  EXPECT_FALSE(tryParseAstraColSpecStr("").defined());
  EXPECT_FALSE(tryParseAstraColSpecStr("   ").defined());
}

TEST(TestAstraLoader, TimeOfDayParseTest) {
  EXPECT_EQ(
      tryParseAstraTimeOfDay("08:02:31").get(),
      (8.0_h + 2.0_minutes + 31.0_seconds));
  EXPECT_FALSE(
      tryParseAstraTimeOfDay("08:02").defined());
}

TEST(TestAstraLoader, DateTest) {
  EXPECT_EQ(
      tryParseAstraDate("2018/03/09").get(),
      TimeStamp::UTC(2018, 3, 9, 0, 0, 0));
}

auto testTransducer = trStreamLines() // All the lines of the file
        |
        trTake(12) // No need to load the entire file.
        |
        trFilter(complementFunction(&isBlankString))
        |
        trPreparseAstraLine() // Identify the type of line: header, column spec or data?
        |
        trMakeAstraData();

std::string dinghyFilename = PathBuilder::makeDirectory(Env::SOURCE_DIR)
  .pushDirectory("datasets")
  .pushDirectory("astradata")
  .pushDirectory("Log from Dinghy")
  .makeFile("Device___15___2018-03-02.log").get().toString();

std::string coachFilename = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("astradata")
    .pushDirectory("Coach")
    .makeFile("log1Hz20180215_0957_Charts.log").get().toString();




TEST(TestAstraLoader, TestLoadDinghy) {
  auto results = transduce(
      makeOptional(std::make_shared<std::ifstream>(dinghyFilename)),
      testTransducer, // Produce structs from table rows.
      IntoArray<AstraData>());
  EXPECT_LT(0, results.size());

  auto x = results[5];
  EXPECT_NEAR(
      (x.fullTimestamp() - TimeStamp::UTC(2018, 3, 2, 8,2,36)).seconds(),
      0.0, 0.1);

  EXPECT_EQ(x.userId.get(), "59cd04e5805f02002beba652");

  EXPECT_NEAR(x.lat.get().degrees(), 45.797494, 1.0e-9);
  EXPECT_EQ(x.logType, AstraLogType::RawDinghy);
  EXPECT_TRUE(x.COG.defined());
  EXPECT_TRUE(x.SOG.defined());
}

TEST(TestAstraLoader, ParseParameters) {
  EXPECT_TRUE(tryParseNamedNumericParameters(" kattskit :  934.3 ").defined());
  EXPECT_TRUE(tryParseNamedNumericParameters(" kattskit :  934.3    a: 3.4").defined());
  EXPECT_TRUE(tryParseNamedNumericParameters("b:9  c:10").defined());
  EXPECT_FALSE(tryParseNamedNumericParameters("b:").defined());
  EXPECT_FALSE(tryParseNamedNumericParameters(":::b:").defined());
  EXPECT_TRUE(tryParseNamedNumericParameters("   Zis value is good: 119").defined());
  //EXPECT_FALSE(tryParseNamedParameters("  kattskit: 934.3 ").defined());
}

TEST(TestAstraLoader, TestLoadCoach) {
  auto results = transduce(
      makeOptional(std::make_shared<std::ifstream>(coachFilename)),
      testTransducer, // Produce structs from table rows.
      IntoArray<AstraData>());
  EXPECT_LT(0, results.size());
  EXPECT_EQ(results[0].logType, AstraLogType::ProcessedCoach);
}

TEST(TestAstraLoader, TestIdentifyHeader) {
  EXPECT_TRUE(isDinghyLogHeader("Device___23423"));
  EXPECT_FALSE(isDinghyLogHeader("Devic___23423"));
  EXPECT_TRUE(isProcessedCoachLogHeader("asdfsadf_Charts"));
  EXPECT_FALSE(isProcessedCoachLogHeader("asdfsadf_Chart"));
}

template <typename T>
bool hasSomeData(
    const std::map<std::string, T>& src) {
  for (auto keyAndSamples: src) {
    for (auto x: keyAndSamples.second) {
      return true;
    }
  }
  return false;
}

TEST(TestAstraLoader, TestLoadLogs) {
  LogAccumulator acc;
  EXPECT_TRUE(accumulateAstraLogs(dinghyFilename, &acc));
  EXPECT_TRUE(accumulateAstraLogs(coachFilename, &acc));

  EXPECT_TRUE(hasSomeData(acc._GPS_BEARINGsources));
  EXPECT_TRUE(hasSomeData(acc._GPS_SPEEDsources));
  EXPECT_TRUE(hasSomeData(acc._GPS_POSsources));
  EXPECT_TRUE(hasSomeData(acc._TWDIRsources));
  EXPECT_TRUE(hasSomeData(acc._TWSsources));
}
