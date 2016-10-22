/*
 * CmdArgTest.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/CmdArg.h>
#include <server/common/logging.h>

using namespace sail;
using namespace sail::CmdArg;

struct TestSetup {
  double a = 0.0;
  double b = 0.0;
  double value = 0.0;
  std::string unit;
};

TEST(CmdArgTest, Tutorial) {
  TestSetup setup;


  // Here we configure the parser.
  Parser cmd("CompleteExample: This is the message shown at the top");

  // 'bind' is used to add an option.
  cmd.bind({"--wave", "-w"}, {

      // Just like functions in C++ can be overloaded for
      // different arguments, so can InputForm's here. Here we
      // have two such InputForm's. The forms are visited
      // in the order listed, and the first form that is
      // compatible with the input arguments will be the one used.

      // The inputForm variadic template function takes
      // as first argument a functor, followed by a variadic
      // number of Arg<...> objects that specify the arguments
      // that the functor accepts.

      /*1st form*/ inputForm([&](double amp, double phase) {
        setup.a = amp;
        setup.b = phase;

        // We must return something from which a Result
        // object can be constructed. Returning true
        // means success.
        return true;
      }, Arg<double>("amp"),   // Relates to 'double amp' in the functor above
         Arg<double>("phase")  // Relates to 'double phase' in the functor above
        .describe("in radians")) // (1/3) Individual arguments can be described
       .describe("Specify a wave by amp and phase"), // (2/3) Input forms can be described

       // This form has fewer argument than the first form.
       // That means that, whatever input the first form accepts,
       // so does this form. Therefore, it must follow the first
       // form because otherwise that form would never be called.
       /*2nd form*/inputForm([&](double amp) {
         setup.a = amp;
         setup.b = 0.0;
         return true;
       }, Arg<double>("amp"))
       .describe("Specify a wave with default phase 0")

  }).describe("Specify a wave"); // (3/3) Options can be described
  // As you may see, there are three different things that can
  // be described: Invidivual arguments, input forms, and
  // entire options. The provided descriptions are used
  // to generate documentation.

  cmd.bind({"--sampling"}, {

      inputForm([&](double v, std::string u) {
        if (u != "hz" && u != "s") {
          // We can return a failure from inside a functor.
          // This will show up as a message in the output
          // and parsing will be interupted if no other form
          // was able to handle the input.
          return Result::failure("Illegal sampling unit: " + u);
        }
        setup.value = v;
        setup.unit = u;
        return Result::success();

      }, Arg<double>("value"),
         Arg<std::string>("unit")
           .describe("Frequency in hz or period time in s"))
      .describe("A value and a unit")

  }).describe("Specify the sampling");

  // Make some fake input.
  const int argc = 7;
  const char *argv[argc] = {
      "prg-name", "--wave", "9", "7",
      "--sampling", "6", "hz"};

  // Here we run the parser
  EXPECT_EQ(Parser::Continue, cmd.parse(argc, argv));

  // Parser::Continue means that the parser didn't encounter
  // any problems, and we can move on.
  EXPECT_EQ(setup.a, 9.0);
  EXPECT_EQ(setup.b, 7.0);
  EXPECT_EQ(setup.value, 6.0);
  EXPECT_EQ(setup.unit, "hz");
  EXPECT_EQ(0, cmd.freeArgs().size());
}






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
    std::vector<Result> reasons;
    Array<std::string> args{"kattskit"};
    EXPECT_TRUE(e.parse(&reasons, &args));
    EXPECT_EQ(d, "kattskit");
    EXPECT_EQ(reasons.size(), 2);
  }{
    std::vector<Result> reasons;
    Array<std::string> args;
    EXPECT_FALSE(e.parse(&reasons, &args));
    EXPECT_EQ(reasons.size(), 3);
  }{
    std::vector<Result> reasons;
    Array<std::string> args{"9"};
    EXPECT_TRUE(e.parse(&reasons, &args));
    EXPECT_EQ(reasons.size(), 1);
    EXPECT_NEAR(a, 9.0, 1.0e-6);
  }
}




void withTestSetup(std::function<void(TestSetup*,Parser*)> handler) {
  TestSetup setup;

  Parser cmd("This is the message shown at the top");

  cmd.bind({"--wave", "-w"}, {

      inputForm([&](double amp, double phase) {
        setup.a = amp;
        setup.b = phase;
        return true;
      }, Arg<double>("amp"), Arg<double>("phase")
        .describe("in radians"))
       .describe("Specify a wave by amp and phase"),


      inputForm([&](double amp) {
        setup.a = amp;
        setup.b = 0.0;
        return true;
      }, Arg<double>("amp"))
      .describe("Specify a wave with default phase 0")

  }).describe("Specify a wave");

  cmd.bind({"--sampling"}, {

      inputForm([&](double v, std::string u) {
        if (u != "hz" && u != "s") {
          return Result::failure("Illegal sampling unit: " + u);
        }
        setup.value = v;
        setup.unit = u;
        return Result::success();

      }, Arg<double>("value"),
         Arg<std::string>("unit")
           .describe("Frequency in hz or period time in s"))
      .describe("A value and a unit")

  }).describe("Specify the sampling");

  handler(&setup, &cmd);
}


TEST(CmdArgTest, BasicTesting) {
  {
    bool called = false;
    withTestSetup([&](TestSetup *s, Parser *cmd) {

      const int argc = 7;
      const char *argv[argc] = {
          "prg-name", "--wave", "9", "7",
          "--sampling", "6", "hz"};

      EXPECT_EQ(Parser::Continue, cmd->parse(argc, argv));

      EXPECT_EQ(s->a, 9.0);
      EXPECT_EQ(s->b, 7.0);
      EXPECT_EQ(s->value, 6.0);
      EXPECT_EQ(s->unit, "hz");
      EXPECT_EQ(0, cmd->freeArgs().size());

      called = true;
    });
    EXPECT_TRUE(called);
  }{
    bool called = false;
    withTestSetup([&](TestSetup *s, Parser *cmd) {

      const int argc = 7;
      const char *argv[argc] = {
          "prg-name", "--wave", "9", "7",
          "--sampling", "6", "asdfasdfasdf"};

      EXPECT_EQ(Parser::Error, cmd->parse(argc, argv));
      called = true;
    });
    EXPECT_TRUE(called);
  }{
    bool called = false;
    withTestSetup([&](TestSetup *s, Parser *cmd) {

      const int argc = 8;
      const char *argv[argc] = {
          "prg-name", "--wave", "9", "7", "this-arg-is-free",
          "--sampling", "6", "hz"};
      EXPECT_EQ(Parser::Continue, cmd->parse(argc, argv));
      EXPECT_EQ(1, cmd->freeArgs().size());
      EXPECT_EQ("this-arg-is-free", cmd->freeArgs()[0]);
      called = true;
    });
    EXPECT_TRUE(called);
  }{
    bool called = false;
    withTestSetup([&](TestSetup *s, Parser *cmd) {

      const int argc = 2;
      const char *argv[argc] = {
          "prg-name", "--help"
      };
      EXPECT_EQ(Parser::Done, cmd->parse(argc, argv));
      EXPECT_EQ(0, cmd->freeArgs().size());
      called = true;
    });
    EXPECT_TRUE(called);
  }
}

TEST(CmdArgTest, AcceptFlagsTest) {

  using namespace CmdArg;
  {
    Parser parser("Test");
    parser.bind({"--name"}, {

      inputForm([&](const std::string &s) {
        return true;
      }, Arg<std::string>("name"))

    }).describe("The name");

    const int n = 3;
    const char *args[n] = {"prg-name", "--name", "--rulle"};
    EXPECT_EQ(Parser::Error, parser.parse(n, args));
  }{
    Parser parser("Test");
    parser.bind({"--name"}, {

      inputForm([&](const std::string &s) {
        return true;
      }, Arg<std::string>("name")
        .dontRejectFlags()) // <-- Will not complain about '--rulle'

    }).describe("The name");

    const int n = 3;
    const char *args[n] = {"prg-name", "--name", "--rulle"};
    EXPECT_EQ(Parser::Continue, parser.parse(n, args));
  }
}

TEST(CmdArgTest, Required) {
  using namespace CmdArg;

  {
    Parser parser("test");
    bool gotIt = false;
    parser.bind({"--age", "-a"}, {
      inputForm([&]() {
        gotIt = true;
        return true;
      })
    }).required();
    const int n = 1;
    const char *args[n] = {"prg-name"};
    EXPECT_EQ(Parser::Error, parser.parse(n, args));
    EXPECT_FALSE(gotIt);
  }
}

TEST(CmdArgTest, MaxCount) {
  using namespace CmdArg;

  {
    Parser parser("test");
    bool gotIt = false;
    parser.bind({"--age", "-a"}, {
      inputForm([&]() {
        gotIt = true;
        return true;
      })
    }).setMaxCount(1);
    const int n = 4;
    const char *args[n] = {"prg-name", "--age", "-a", "-a"};
    EXPECT_EQ(Parser::Error, parser.parse(n, args));
    EXPECT_TRUE(gotIt);
  }
}

