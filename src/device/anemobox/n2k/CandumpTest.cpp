/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/n2k/PgnClasses.h>
#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <fstream>
#include <server/common/logging.h>
#include <server/common/string.h>

using namespace PgnClasses;

using namespace sail;

namespace {
  struct CandumpLine {
    CandumpLine(const std::string &s);

    std::string src;
    uint64_t pgn;
    Array<uint8_t> data;
  };

  std::string leftPart(const std::string &s, char c) {
    return s.substr(0, s.find(c));
  }

  std::string rightPart(const std::string &s, char c) {
    int index = s.find_last_of(c) + 1;
    return s.substr(index, s.length() - index);
  }

  std::string removeSpaces(const std::string &src) {
    std::string dst;
    for (auto x: src) {
      if (x != ' ') {
        dst += x;
      }
    }
    return dst;
  }


  uint64_t parseInt(const std::string &s, int base) {
    std::stringstream ss;
    assert(base == 10 || base == 16);
    ss << (base == 10? std::dec : std::hex) << s;
    unsigned int value;
    ss >> value;
    return value;
  }


  Array<uint8_t> parseDataFromHexaDecimal(const std::string &s) {
    int n = s.length()/2;
    CHECK(n*2 == s.length());
    Array<uint8_t> data(n);
    for (int i = 0; i < n; i++) {
      data[i] = static_cast<uint8_t>(parseInt(s.substr(2*i, 2), 16));
    }
    return data;
  }


  CandumpLine::CandumpLine(const std::string &s) {
    auto headerAndData = sail::split(s, '-');
    auto addressAndPgn = sail::split(headerAndData[0], ',');
    auto sizeAndData = sail::split(headerAndData[1], ']');
    src = rightPart(addressAndPgn[0], ':');
    pgn = parseInt(leftPart(addressAndPgn[1], ' '), 16);
    auto size = parseInt(rightPart(sizeAndData[0], '['), 10);
    data = parseDataFromHexaDecimal(removeSpaces(sizeAndData[1])).sliceTo(size);
  }
}

TEST(N2kVisitorTest, TestBAM) { // TODO: Test something interesting...
  auto p = sail::PathBuilder::makeDirectory(sail::Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .makeFile("candump_alinghi_bandg_log.txt").get().toString();
  std::ifstream file(p);
  EXPECT_TRUE(file.good());
  std::string s;
  while (std::getline(file, s)) {
    // TODO: Do something here.
    CandumpLine line(s);
    EXPECT_EQ(line.data.size(), 8);
  }
}

