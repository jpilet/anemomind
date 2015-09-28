/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/CsvParser.h>
#include <fstream>
#include <server/common/string.h>

namespace sail {
namespace CsvParser {

MDArray<std::string, 2> parse(std::istream *s) {
  std::string line;
  std::vector<Array<std::string> > tokenizedLines;

  int maxCols = 0;
  while (std::getline(*s, line)) {
    auto tokens = tokenize(line, ",");
    maxCols = std::max(maxCols, tokens.size());
    if (!tokens.empty()) {
      tokenizedLines.push_back(tokens);
    }
  }
  MDArray<std::string, 2> results(tokenizedLines.size(), maxCols);
  for (int i = 0; i < tokenizedLines.size(); i++) {
    auto tokens = tokenizedLines[i];
    for (int j = 0; j < tokens.size(); j++) {
      results(i, j) = tokens[j];
    }
  }
  return results;
}

MDArray<std::string, 2> parse(std::string filename) {
  std::ifstream file(filename);
  return parse(&file);
}


}
}
