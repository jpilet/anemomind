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

  const int terminalCount = 6;

  Arrayb makeValidStartStateTable() {
    Arrayb table = Arrayb::fill(terminalCount, false);
    table[0] = true;
    table[2] = true;
    return table;
  }

  bool isValidStartState(int index) {
    static Arrayb table = makeValidStartStateTable();
    return table[index];
  }

  Arrayb makeValidEndStateTable() {
    Arrayb table = Arrayb::fill(terminalCount, false);
    table[0] = true;
    table[1] = true;
    table[4] = true;
    table[5] = true;
    return table;
  }

  bool isValidEndState(int index) {
    static Arrayb table = makeValidEndStateTable();
    return table[index];
  }

  double blankCost(char c) {
    return (isBlank(c)? 0 : huge);
  }

  double charCost(char a, char b) {
    return (a == b? 0 : huge);
  }

  double commaCost(char c) {
    return charCost(c, ',');
  }

  double butSymbolCost(char c, char symbol) {
    return (c == symbol? huge : 0);
  }

  double butBlankCost(char c) {
    return (isBlank(c)? huge : 0);
  }

  class TrzAutomaton : public StateAssign {
   public:
    TrzAutomaton(std::string s, Array<Arrayi> prec) : _s(s), _prec(prec) {}

    double getStateCost(int stateIndex, int timeIndex);

     double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
       return 0.0; // Handled by getPrecedingStates
     }

     int getStateCount() {
       return terminalCount;
     }
     int getLength() {
       return _s.length();
     }

     Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
       return _prec[stateIndex];
     }

   private:
    std::string _s;
    Array<Arrayi> _prec;
  };

  double TrzAutomaton::getStateCost(int stateIndex, int timeIndex) {
    if (timeIndex == 0) {
      if (!isValidStartState(stateIndex)) {
        return huge;
      }
    }

    if (timeIndex == getLength() - 1) {
      if (!isValidEndState(stateIndex)) {
        return huge;
      }
    }

    static const std::string headerPrefix("Trace");
    char c = _s[timeIndex];

    switch (stateIndex) {
    case 0:
      return discourage + butSymbolCost(c, ',');
    case 1:
      return commaCost(c);
    case 2:
      return prefixWordCost(c, timeIndex, headerPrefix);
    case 3:
      return blankCost(c);
    case 4:
      return butBlankCost(c);
    case 5:
      return blankCost(c);
    default:
      LOG(FATAL) << "Invalid state index";
      return NAN;
    };
    return NAN;
  }

}



TrzParser::TrzParser() : _h(makeTrzHierarchy()) {
  StaticCostFactory f(_h);

  for (int i = 0; i < terminalCount; i++) {
    f.connectSelf(i);
  }

  f.connectNoCost(0, 1, true);
  f.connectNoCost(0, 5);
  f.connectNoCost(1, 5);

  f.connectNoCost(2, 3);
  f.connectNoCost(3, 4, true);
  f.connectNoCost(4, 5);

  _prec = StateAssign::makePredecessorsPerState(f.connections());
}

ParsedTrzLine TrzParser::parse(std::string line) {
  if (line.empty()) {
    return ParsedTrzLine(std::shared_ptr<HTree>(), line);
  }

  TrzAutomaton p(line, _prec);
  Arrayi parsed = p.solve();
  double cost = p.calcCost(parsed);
  if (huge <= cost) {
    LOG(WARNING) << stringFormat("Failed to parse %s:", line.c_str());
    LOG(WARNING) << EXPR_AND_VAL_AS_STRING(parsed);
    LOG(WARNING) << "Please make sure the file you are opening has been decompressed, e.g. by opening the original *.trz file in gedit and saving it in decompressed format.";
  }
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
