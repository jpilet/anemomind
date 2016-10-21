/*
 * CmdArgTest.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/common/CmdArg.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(CmdArgTest, BasicUsage) {
  CmdArg cmd("This is the message shown at the top");
}


