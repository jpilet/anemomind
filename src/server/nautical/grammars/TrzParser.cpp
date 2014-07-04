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

namespace {
  int countArgsRecord(const ParsedTrzLine &data) {
    const int count = data.childCount();
    int commaCounter = 0;
    std::shared_ptr<HTree> tree = data.tree();
    for (int i = 0; i < count; i++) {
      std::shared_ptr<HTree> child = tree->child(i);
      if (child->index() == 1) {
        commaCounter += child->count();
      }
    }
    return commaCounter + 1;
  }

  int countArgsHeader(const ParsedTrzLine &data) {
    const int count = data.childCount();
    int counter = 0;
    std::shared_ptr<HTree> tree = data.tree();
    for (int i = 0; i < count; i++) {
      std::shared_ptr<HTree> child = tree->child(i);
      if (child->index() != 3) {
        counter++;
      }
    }
    return counter;
  }


  int countArgs(const ParsedTrzLine &data0) {
    if (data0.empty()) {
      return 0;
    }

    ParsedTrzLine data = data0.child(0);

    if (data.index() == 6) {
      return countArgsRecord(data);
    } else {
      assert(data.index() == 7);
      return countArgsHeader(data);
    }
  }

  MDArray<std::string, 2> allocateArgMatrix(Array<ParsedTrzLine> data) {
    int rows = 0;
    int cols = 0;
    for (auto x : data) {
      if (!x.empty()) {
        rows++;
        cols = std::max(cols, countArgs(x));
      }
    }
    return MDArray<std::string, 2>(rows, cols);
  }

  void fillRecordRow(const ParsedTrzLine &src, MDArray<std::string, 2> dst) {
    int index = 0;
    for (int i = 0; i < src.childCount(); i++) {
      ParsedTrzLine ch = src.child(i);
      if (ch.index() == 1) {
        index += ch.dataLength();
      } else {
        dst(0, index) = ch.data();
      }
    }
  }

  void fillHeaderRow(const ParsedTrzLine &src, MDArray<std::string, 2> dst) {
    int index = 0;
    for (int i = 0; i < src.childCount(); i++) {
      ParsedTrzLine ch = src.child(i);
      if (ch.index() == 4) {
        dst(0, index) = ch.data();
        index++;
      }
    }
  }

  bool fillArgRow(const ParsedTrzLine &src0, MDArray<std::string, 2> dst) {
    if (src0.empty()) {
      return false;
    }

    ParsedTrzLine src = src0.child(0);
    if (src.index() == 6) {
      fillRecordRow(src, dst);
    } else {
      assert(src.index() == 7);
      fillHeaderRow(src, dst);
    }
    return true;
  }

  MDArray<std::string, 2> makeArgMatrix(Array<ParsedTrzLine> data) {
    MDArray<std::string, 2> dst = allocateArgMatrix(data);
    int counter = 0;
    for (auto x : data) {
      if (fillArgRow(x, dst.sliceRow(counter))) {
        counter++;
      }
    }
    assert(counter == dst.rows());
    return dst;
  }
}

void exportToMatlab(std::string filename, Array<ParsedTrzLine> data) {
  MDArray<std::string, 2> mat = makeArgMatrix(data);
  std::ofstream file(filename);
  file << mat.rows() << "\n"; // First row in file: Number of matrix rows
  file << mat.cols() << "\n"; // Second row in file: Number of matrix cols
  for (int i = 0; i < mat.rows(); i++) {
    for (int j = 0; j < mat.cols(); j++) {
      file << mat(i, j) << "\n";
    }
  }
}







}
