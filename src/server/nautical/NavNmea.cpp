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


  NmeaParser::NmeaSentence processNmeaLineSub(NmeaParser *parser,
          std::string line) {
    int len = line.size();
    const char *cstr = line.c_str();
    NmeaParser::NmeaSentence retval = NmeaParser::NMEA_NONE;
    for (int i = 0; i < len; i++) {
      char c = cstr[i];
      retval = parser->processByte(c);
    }

    if (retval == NmeaParser::NMEA_NONE) {
      retval = parser->processByte('\n');
      assert(retval != NmeaParser::NMEA_NONE);
    }
    return retval;
  }



  void processNmeaLine(NmeaParser *parser, Nav *nav,
        std::string line, std::vector<Nav> *navs, Arrayb *fieldsRead) {
    NmeaParser::NmeaSentence s = processNmeaLineSub(parser, line);

    // Whenever new time data is received allocate a new record.
    if (s == NmeaParser::NMEA_TIME_POS) {
      navs->push_back(*nav);
      *nav = Nav();
    }

    (*fieldsRead)[s] = true;
  }
}

NavNmeaData::NavNmeaData(std::string filename) {
  std::ifstream file(filename);
  init(file);
}

NavNmeaData::NavNmeaData(std::istream &file) {
  init(file);
}

void NavNmeaData::init(std::istream &file) {
  _sentencesReceived = Arrayb(7);
  _sentencesReceived.setTo(false);

  std::vector<Nav> navAcc;
  NmeaParser parser;
  parser.setIgnoreWrongChecksum(true);
  std::string line;
  int lineCounter = 0;
  Nav nav;
  while (std::getline(file, line)) {
    processNmeaLine(&parser, &nav, line, &navAcc, &_sentencesReceived);
    lineCounter++;
  }
  _navs = Array<Nav>::referToVector(navAcc).dup();
}

} /* namespace sail */
