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

namespace sail {

namespace {
  double huge = 1.0e5; // a huge, prohibitive cost.
  double low = 1.0; // a low cost that does not probit a state, but discourages


  double prefixWordCost(char c, int index, const std::string &word) {
    if (index < word.length()) {
      return (word[index] == c? 0 : huge);
    }
    return huge;
  }

  double symbolButCommaNoPenalty(char c) {
    return (c == ','? huge : 0.0);
  }


  double charCost(char a, char b) {
    return (a == b? 0 : huge);
  }


  double commaCost(char c) {
    return charCost(c, ',');
  }

  double symbolButComma(char c) {
    return symbolButCommaNoPenalty(c) + low;
  }


  bool isDecimalSeparator(char c) {
    return c == '.';
  }

  bool isSign(char c) {
    return (c == '+') || (c == '-');
  }

  bool isNumeric(char c) {
    return isDecimalSeparator(c) || isDigit(c) || isSign(c);
  }


  double numericCost(char c) {
    return (isNumeric(c)? 0.0 : huge);
  }

  double integerCost(char c) {
    return (isDigit(c) || isSign(c)? 0 : huge);
  }

  double dollarSignCost(char c) {
    return (c == '$'? 0 : huge);
  }

  Hierarchy makeTrzHierarchy() {
    return HNodeGroup(28, "Top",
        HNodeGroup(23, "Record",
            HNodeGroup(19, "Prefix",
                HNodeGroup(0, "DollarSign")
                +
                HNodeGroup(1, "Symbol")
            )
            +
            HNodeGroup(2, "Separator")
            +
            HNodeGroup(3, "SymbolButComma")
            +
            HNodeGroup(4, "Number")
            +
            HNodeGroup(22, "Time",
                HNodeGroup(20, "TimeOfDay",
                    HNodeGroup(5, "Value")
                    +
                    HNodeGroup(6, "Separator")
                )
                +
                HNodeGroup(7, "WhiteSpace")
                +
                HNodeGroup(21, "Date",
                    HNodeGroup(8, "Value")
                    +
                    HNodeGroup(9, "Separator")
                )
            )
        )
        +
        HNodeGroup(27, "Header",
            HNodeGroup(10, "Prefix")
            +
            HNodeGroup(11, "Separator")
            +
            HNodeGroup(12, "SymbolButComma")
            +
            HNodeGroup(13, "Number")
            +
            HNodeGroup(26, "Time",
                HNodeGroup(24, "TimeOfDay",
                    HNodeGroup(14, "Value")
                    +
                    HNodeGroup(15, "Separator")
                )
                +
                HNodeGroup(16, "WhiteSpace")
                +
                HNodeGroup(25, "Date",
                    HNodeGroup(17, "Value")
                    +
                    HNodeGroup(18, "Separator")
                )
            )
        )
    ).compile("Trz-%03d");
  }

  const int terminalCount = 19;

  Arrayb makeValidStartStateTable() {
    Arrayb table = Arrayb::fill(terminalCount, false);
    table[0] = true;
    table[3] = true;
    return table;
  }

  bool isValidStartState(int index) {
    static Arrayb table = makeValidStartStateTable();
    return table[index];
  }

  Arrayb makeValidEndStateTable() {
    Arrayb table = Arrayb::fill(terminalCount, false);
    table[3] = true;
    table[2] = true;
    table[4] = true;
    table[8] = true;
    table[12] = true;
    table[13] = true;
    table[11] = true;
    table[17] = true;
    return table;
  }

  bool isValidEndState(int index) {
    static Arrayb table = makeValidEndStateTable();
    return table[index];
  }

  bool whiteSpaceCost(char c) {
    return (isBlank(c)? 0 : huge);
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
      return dollarSignCost(c);
    case 1:
      return symbolButCommaNoPenalty(c);
    case 2:
      return commaCost(c);
    case 3:
      return symbolButComma(c);
    case 4:
      return numericCost(c);
    case 5:
      return integerCost(c);
    case 6:
      return charCost(c, '/');
    case 7:
      return whiteSpaceCost(c);
    case 8:
      return integerCost(stateIndex);
    case 9:
      return charCost(c, ':');
    case 10:
      return prefixWordCost(c, timeIndex, headerPrefix);
    case 11:
      return commaCost(c);
    case 12:
      return symbolButComma(c);
    case 13:
      return numericCost(c);
    case 14:
      return integerCost(c);
    case 15:
      return charCost(c, '/');
    case 16:
      return whiteSpaceCost(c);
    case 17:
      return integerCost(c);
    case 18:
      return charCost(c, ':');
    default:
      LOG(FATAL) << "Invalid state index";
      return NAN;
    };
    return NAN;
  }

}



TrzParser::TrzParser() : _h(makeTrzHierarchy()) {
  StaticCostFactory f(_h);

  // 0..9: Record
  f.connectNoCost(0, 1);
  f.connectSelf(1);
  f.connectNoCost(1, 2);
  f.connectSelf(2);
  f.connectNoCost(2, 3, true);
  f.connectSelf(3);
  f.connectNoCost(2, 4, true);
  f.connectSelf(4);
  f.connectNoCost(2, 5);
  f.connectSelf(5);
  f.connectNoCost(5, 6, true);
  f.connectNoCost(5, 7);
  f.connectSelf(7);
  f.connectNoCost(7, 8);
  f.connectSelf(8);
  f.connectNoCost(8, 9, true);
  f.connectNoCost(8, 2);

  // 10..18: Header
  f.connectSelf(10);
  f.connectNoCost(10, 11);
  f.connectSelf(11);
  f.connectNoCost(11, 12, true);
  f.connectSelf(12);
  f.connectNoCost(11, 13, true);
  f.connectSelf(13);
  f.connectNoCost(11, 14);
  f.connectSelf(14);
  f.connectNoCost(14, 15, true);
  f.connectNoCost(14, 16);
  f.connectSelf(16);
  f.connectNoCost(16, 17);
  f.connectSelf(17);
  f.connectNoCost(17, 18, true);
  f.connectNoCost(17, 11);

  _prec = StateAssign::makePredecessorsPerState(f.connections());
}

ParsedTrzLine TrzParser::parse(std::string line) {
  TrzAutomaton p(line, _prec);
  return ParsedTrzLine(_h.parse(p.solve()), line);
}

Array<ParsedTrzLine> TrzParser::parseFile(std::istream &file) {
  ArrayBuilder<ParsedTrzLine> parsed;
  std::string line;
  int counter = 0;
  while (getline(file, line)) {
    counter++;
    std::cout << "Parse trz line " << counter << std::endl;
    parsed.add(parse(line));
  }
  Array<ParsedTrzLine> result = parsed.get();
  LOG(INFO) << stringFormat("Parsed Trz file with %d lines.", result.size());
  return result;
}

Array<ParsedTrzLine> TrzParser::parseFile(std::string filename) {
  std::ifstream file(filename);
  return parseFile(file);
}



}
