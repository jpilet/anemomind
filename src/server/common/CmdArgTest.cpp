/*
 * CmdArgTest.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/common/CmdArg.h>
#include <gtest/gtest.h>

using namespace sail;


TEST(CmdArgTest, InputFormTest) {
  double a = 0.0;
  double b = 0.0;

  auto form = inputForm(
      [&](double amp, double phase) {
    a = amp;
    b = phase;
    return true;
  },
  Arg<double>("amp").describe("The amplitude, in meters"),
  Arg<double>("phase").describe("The phase, in radians"));

  EXPECT_TRUE(bool(form.parse({"3", "4"})));
  EXPECT_NEAR(a, 3.0, 1.0e-5);
  EXPECT_NEAR(b, 4.0, 1.0e-5);

  EXPECT_FALSE(bool(form.parse({"a", "b"})));
  EXPECT_NEAR(a, 3.0, 1.0e-5);
  EXPECT_NEAR(b, 4.0, 1.0e-5);
}

TEST(CmdArgTest, EntryTest) {
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  std::string d = "";

  Entry e({"--wave", "-w"}, {
    inputForm([&](double amp) {
      a = amp;
      return true;
    }, Arg<double>("amplitude"))
      .describe("Wave with specified amplitude and 0 phase"),
    inputForm([&](double amp, double phase) {
      b = amp;
      c = phase;
      return true;
    }, Arg<double>("amplitude"),
       Arg<double>("phase")).describe(
           "Wave with specified amplitude and phase"),
    inputForm([&](std::string f) {
      d = f;
      return true;
    }, Arg<std::string>("whatever").describe("Put any expr here"))
      .describe("Freeform wave spec")
  });

  {
    std::vector<InputForm::Result> reasons;
    Array<std::string> args{"kattskit"};
    EXPECT_TRUE(e.parse(&reasons, &args));
  }
}

TEST(CmdArgTest, BasicUsage) {
  CmdArg cmd("This is the message shown at the top");
}


