/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/n2k/PgnClasses.h>
#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <fstream>

using namespace PgnClasses;

using namespace sail;

namespace {
  struct CandumpLine {
    CandumpLine(const std::string &s);

    std::string src;
    uint64_t pgn;
    Array<uint8_t> data;
  };

  std::pair<std::string, std::string> split(const std::string &s, char c) {
    int index = s.find(c);
    if (index == s.npos) {
      return std::pair<std::string, std::string>();
    } else {
      return std::pair<std::string, std::string>(s.substr(0, index), s.substr(index+1, s.length() - index - 1));
    }
  }

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



  int parseHexDigit(char c) {
    if ('0' <= c && c <= '9') {
      return c - '0';
    }
    if ('a' <= c && c <= 'f') {
      return (c - 'a') + 10;
    }
    return -1;
  }

  uint64_t parseInt(const std::string &s, int base) {
    uint64_t result = 0;
    for (auto x: s) {
      result = parseHexDigit(x) + base*result;
    }
    return result;
  }

  uint8_t parseByte(const std::string &s) {
    auto value = 16*parseHexDigit(s[0]) + parseHexDigit(s[1]);
    return uint8_t(value);
  }


  Array<uint8_t> parseDataFromHexaDecimal(const std::string &s) {
    int n = s.length()/2;
    assert(n*2 == s.length());
    Array<uint8_t> data(n);
    for (int i = 0; i < n; i++) {
      data[i] = parseByte(s.substr(2*i, 2));
    }
    return data;
  }


  CandumpLine::CandumpLine(const std::string &s) {
    auto headerAndData = split(s, '-');
    auto addressAndPgn = split(headerAndData.first, ',');
    auto sizeAndData = split(headerAndData.second, ']');
    src = rightPart(addressAndPgn.first, ':');
    pgn = parseInt(leftPart(addressAndPgn.second, ' '), 16);
    auto size = parseInt(rightPart(sizeAndData.first, '['), 10);
    data = parseDataFromHexaDecimal(removeSpaces(sizeAndData.second)).sliceTo(size);
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

