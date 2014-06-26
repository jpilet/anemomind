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

  double symbolButSpaceNoPenalty(char c) {
    if (isBlank(c)) {
      return huge;
    }
    return 0;
  }

  double symbolButSpace(char c) {
    return low + symbolButSpaceNoPenalty(c);
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
                HNodeGroup(20, "Date",
                    HNodeGroup(5, "Value")
                    +
                    HNodeGroup(6, "Separator")
                )
                +
                HNodeGroup(7, "WhiteSpace")
                +
                HNodeGroup(21, "TimeOfDay",
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
            HNodeGroup(12, "SymbolButSpace")
            +
            HNodeGroup(13, "Number")
            +
            HNodeGroup(26, "Time",
                HNodeGroup(24, "Date",
                    HNodeGroup(14, "Value")
                    +
                    HNodeGroup(15, "Separator")
                )
                +
                HNodeGroup(16, "WhiteSpace")
                +
                HNodeGroup(25, "TimeOfDay",
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
    table[10] = true;
    return table;
  }

  bool isValidStartState(int index) {
    static Arrayb table = makeValidStartStateTable();
    return table[index];
  }

  Arrayb makeValidEndStateTable() {
    Arrayb table = Arrayb::fill(terminalCount, false);
    table[1] = true;
    table[3] = true;
    table[2] = true;
    table[4] = true;
    table[8] = true;
    table[10] = true;
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

  double whiteSpaceCost(char c) {
    return charCost(c, ' ');//(isBlank(c)? 0 : huge);
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
      return integerCost(c);
    case 9:
      return charCost(c, ':');
    case 10:
      return prefixWordCost(c, timeIndex, headerPrefix);
    case 11:
      return whiteSpaceCost(c);
    case 12:
      return symbolButSpace(c);
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
  f.connectNoCost(2, 5, true);
  f.connectNoCost(2, 8, true);
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
  if (line.empty()) {
    return ParsedTrzLine(std::shared_ptr<HTree>(), line);
  }

  TrzAutomaton p(line, _prec);
  Arrayi parsed = p.solve();
  double cost = p.calcCost(parsed);
  if (huge <= cost) {
    LOG(WARNING) << stringFormat("Failed to parse %s", line.c_str());
    std::cout << EXPR_AND_VAL_AS_STRING(parsed) << std::endl;
  }
  std::shared_ptr<HTree> tree = _h.parse(parsed);
  ParsedTrzLine x(tree, line);
  disp(&std::cout, x);
  std::cout << EXPR_AND_VAL_AS_STRING(cost) << std::endl;
  return x;
}

Array<ParsedTrzLine> TrzParser::parseFile(std::istream &file) {
  ArrayBuilder<ParsedTrzLine> parsed;
  std::string line;
  int counter = 0;
  while (getline(file, line)) {
    counter++;
    std::cout << "Parse trz line " << counter << ": " << line << std::endl;
    ParsedTrzLine x = parse(line);
    parsed.add(x);
    if (counter == 5) {
      break;
    }
  }
  Array<ParsedTrzLine> result = parsed.get();
  LOG(INFO) << stringFormat("Parsed Trz file with %d lines.", result.size());
  return result;
}

Array<ParsedTrzLine> TrzParser::parseFile(std::string filename) {
  std::ifstream file(filename);
  return parseFile(file);
}

void TrzParser::disp(std::ostream *dst, const ParsedTrzLine &line) {
  disp(dst, line.tree(), line.input(), 0);
}

void TrzParser::disp(std::ostream *dst, std::shared_ptr<HTree> tree, const std::string &s, int depth) {
  indent(dst, 3*depth);
  *dst << _h.node(tree->index()).description() << " (" << tree->index() << ")";
  if (tree->childCount() == 0) {
    *dst << ": " << "'" << s.substr(tree->left(), tree->count()) << "'" << " (length " << tree->count() << ")";
  }
  *dst << std::endl;
  auto ch = tree->children();
  for (auto c : ch) {
    disp(dst, c, s, depth+1);
  }
}




}
