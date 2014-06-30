/*
 *  Created on: 2014-06-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TrzParser.h"
#include <server/common/HNodeGroup.h>
#include <iostream>
#include <fstream>
#include <server/common/string.h>
#include <server/common/logging.h>
#include <server/nautical/grammars/StaticCostFactory.h>
#include <cmath>
#include <server/math/hmm/StateAssign.h>
#include <server/common/ArrayIO.h>
#include <cctype>

namespace sail {

namespace {
  double huge = 1.0e5; // a huge, prohibitive cost.
  double discourage = 1.0; // a low cost that does not probit a state, but discourages


  double prefixWordCost(char c, int index, const std::string &word) {
    if (index < word.length()) {
      return (word[index] == c? 0 : huge);
    }
    return huge;
  }

  Hierarchy makeTrzHierarchy() {
    return HNodeGroup(8, "Top",
                HNodeGroup(6, "Record", // Matches a common record, e.g. starting with $TANAV,...
                    HNodeGroup(0, "Data") + HNodeGroup(1, "Separator")
                )
                +
                HNodeGroup(7, "Header", // Matches the first non-empty line of the file, starting with 'Trace'
                    HNodeGroup(2, "Prefix") + HNodeGroup(3, "Separator") + HNodeGroup(4, "Data")
                )
                +
              HNodeGroup(5, "FinalWhiteSpace") // Matches any whitespace at the end of the line
          )
    .compile("Trz-%03d");
  }

  Arrayi parseTrzSub(std::string line) {
    const int len = line.length();
    Arrayi mapped(len);

    static const std::string headerprefix("Trace");
    const int preflen = headerprefix.length();
    if (line.substr(0, preflen) == headerprefix) {
      mapped.slice(0, preflen).setTo(2);
      for (int i = preflen; i < len; i++) {
        if (isblank(line[i])) {
          mapped[i] = 3;
        } else {
          mapped[i] = 4;
        }
      }
    } else {
      for (int i = 0; i < len; i++) {
        if (line[i] == ',') {
          mapped[i] = 1;
        } else {
          mapped[i]= 0;
        }
      }
    }
    return mapped;
  }
}

TrzParser::TrzParser() : _h(makeTrzHierarchy()) {
}

ParsedTrzLine TrzParser::parse(std::string line) {
  if (line.empty()) {
    return ParsedTrzLine(std::shared_ptr<HTree>(), line);
  }

  Arrayi parsed = parseTrzSub(line);
  std::shared_ptr<HTree> tree = _h.parse(parsed);
  ParsedTrzLine x(tree, line);
  return x;
}

Array<ParsedTrzLine> TrzParser::parseFile(std::istream &file) {
  ArrayBuilder<ParsedTrzLine> parsed;
  std::string line;
  while (getline(file, line)) {
    ParsedTrzLine x = parse(line);
    parsed.add(x);
  }
  Array<ParsedTrzLine> result = parsed.get();
  LOG(INFO) << stringFormat("Parsed Trz file with %d lines.", result.size());
  return result;
}

Array<ParsedTrzLine> TrzParser::parseFile(std::string filename) {
  std::ifstream file(filename);
  return parseFile(file);
}


void TrzParser::disp(std::ostream *dst, const ParsedTrzLine &data, int depth) {
  indent(dst, 3*depth);
  if (data.empty()) {
    *dst << "Empty node" << std::endl;
    return;
  }

  *dst << _h.node(data.index()).description() << " (" << data.index() << ")";
  if (data.isLeaf()) {
    *dst << ": " << "'" << data.data() << "'" << std::endl;
  } else {
    *dst << " with " << data.childCount() << " children:" << std::endl;
    for (int i = 0; i < data.childCount(); i++) {
      disp(dst, data.child(i), depth+1);
    }
  }
}




}
