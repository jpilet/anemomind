/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmea.h"
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <fstream>
#include <vector>
#include <iostream>


namespace sail {

namespace {
const char *getNmeaSentenceLabel(NmeaParser::NmeaSentence s) {
  typedef const char *Str;
  Str labels[] = {"0", //"NMEA_NONE",
      "NMEA_UNKNOWN",
      "NMEA_TIME_POS",
      "NMEA_AW",
      "NMEA_TW",
      "NMEA_WAT_SP_HDG",
      "NMEA_VLW"};
  return labels[s];
}

  void processNmeaLine(NmeaParser *parser, std::vector<Nav> *navAcc,
      std::string line) {
    int len = line.size();
    const char *cstr = line.c_str();
    for (int i = 0; i < len; i++) {
      std::cout << getNmeaSentenceLabel(parser->processByte(cstr[i])) << " ";
    }
    std::cout << "\n";
  }

}


Array<Nav> loadNavsFromNmea(std::istream &file) {
  std::vector<Nav> navAcc;
  NmeaParser parser;
  std::string line;
  while (std::getline(file, line)) {
    cout << "PARSE A LINE: " << line << endl;
    processNmeaLine(&parser, &navAcc, line);
  }
  return Array<Nav>::referToVector(navAcc).dup();
}

Array<Nav> loadNavsFromNmea(std::string filename) {
  std::ifstream file(filename);
  return loadNavsFromNmea(file);
}

} /* namespace sail */
