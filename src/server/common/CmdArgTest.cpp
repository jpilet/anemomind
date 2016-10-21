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

    // Put the form with more arguments before the
    // version with fewer, if there is ambiguity.
    inputForm([&](double amp, double phase) {
      b = amp;
      c = phase;

      // ALWAYS return something here from which
      // an CmdArgResult object can be constructed.
      return true;

    }, // Specify the meaning of each argument:
        Arg<double>("amplitude"),
        Arg<double>("phase").describe("in radians"))
        // ...and the meaning of the entire form
          .describe(
           "Wave with specified amplitude and phase"),

     inputForm([&](double amp) {
       a = amp;
       return true;
     }, Arg<double>("amplitude"))
       .describe("Wave with specified amplitude and 0 phase"),

    inputForm([&](std::string f) {
      d = f;
      return true;
    }, Arg<std::string>("whatever").describe("Put any expr here"))
      .describe("Freeform wave spec")
  });

  {
    std::vector<CmdArgResult> reasons;
    Array<std::string> args{"kattskit"};
    EXPECT_TRUE(e.parse(&reasons, &args));
    EXPECT_EQ(d, "kattskit");
    EXPECT_EQ(reasons.size(), 2);
  }{
    std::vector<CmdArgResult> reasons;
    Array<std::string> args;
    EXPECT_FALSE(e.parse(&reasons, &args));
    EXPECT_EQ(reasons.size(), 3);
  }{
    std::vector<CmdArgResult> reasons;
    Array<std::string> args{"9"};
    EXPECT_TRUE(e.parse(&reasons, &args));
    EXPECT_EQ(reasons.size(), 1);
    EXPECT_NEAR(a, 9.0, 1.0e-6);
  }
}

struct TestSetup {
  double a = 0.0;
  double b = 0.0;
  double value = 0.0;
  std::string unit;
};

void withTestSetup(std::function<void(TestSetup,CmdArg*)> handler) {
  TestSetup setup;

  CmdArg cmd("This is the message shown at the top");

  cmd.bind({"--wave", "-w"}, {

      inputForm([&](double amp, double phase) {
        setup.a = amp;
        setup.b = phase;
        return true;
      }, Arg<double>("amp"), Arg<double>("phase")
        .describe("in radians"))
       .describe("Specify a wave by amp and phase")

  }).describe("Specify a wave");

  cmd.bind({"--sampling"}, {

      inputForm([&](double v, std::string u) {
        if (u != "hz" && u != "s") {
          return CmdArgResult::failure("Illegal sampling unit: " + u);
        }
        setup.value = v;
        setup.unit = u;
        return CmdArgResult::success();

      }, Arg<double>("value"),
         Arg<std::string>("unit")
           .describe("Frequency in hz or period time in s"))
      .describe("A value and a unit")

  }).describe("Specify the sampling");

  handler(setup, &cmd);
}

TEST(CmdArgTest, BasicTesting) {
  {
    bool called = false;
    withTestSetup([&](const TestSetup &s, CmdArg *cmd) {

      const int argc = 7;
      const char *argv[argc] = {
          "prg-name", "--wave", "9", "7",
          "--sampling", "7", "hz"};

      EXPECT_EQ(CmdArg::Continue, cmd->parse(argc, argv));

      called = true;
    });
    EXPECT_TRUE(called);
  }
}

