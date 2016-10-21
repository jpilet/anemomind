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
  auto form = inputForm(
      [&](double amp, double phase) {
    return InputForm::Result::success();
  },
  Arg<double>("amp").describe("The amplitude, in meters"),
  Arg<double>("phase").describe("The phase, in radians"));
}

TEST(CmdArgTest, BasicUsage) {
  CmdArg cmd("This is the message shown at the top");
}


