/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmea.h"
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <fstream>
#include <vector>


namespace sail {

namespace {
  void processNmeaLine(NmeaParser *parser, std::vector<Nav> *navAcc,
      std::string line) {
    int len = line.size();
    const char *cstr = line.c_str();
    for (int i = 0; i < len; i++) {
      parser->processByte(cstr[i]);
    }
  }
}

Array<Nav> loadNavsFromNmea(std::istream &file) {
  std::vector<Nav> navAcc;
  NmeaParser parser;
  std::string line;
  while (std::getline(file, line)) {
    processNmeaLine(&parser, &navAcc, line);
  }
  return Array<Nav>::referToVector(navAcc).dup();
}

Array<Nav> loadNavsFromNmea(std::string filename) {
  std::ifstream file(filename);
  return loadNavsFromNmea(file);
}

} /* namespace sail */
