/*
 *  Created on: Jun 26, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <iostream>
#include <server/nautical/grammars/TrzParser.h>

using namespace sail;

namespace {
  std::string header = "Trace V07.03 13/02/2014 16:09:53 13/02/2014 16:33:29 freq 1 Ville_de_Genève M34 1 0.0055832 0.0016091";
  std::string A = "Ais130214160953!AIVDM,1,1,,B,13HjGn?P1j0Bnk>Hr2SQvgwD0`He,0*04";
  std::string B = "$TANAV,13/02/2014 16:09:53,N,4315.986,N,00522.206,E,T,0.00,N,000,N,0.00,M,000,N,000,N,000,N,0.0,N,000,N,0.0,N,0.0,N,0.00,N,I,13/02/2014,17:09:53,-1,FFFFFFFF000,IIIIII,IIIIIIIII,II,0.000,N,0.00,N,0.00,N,,,0,,,,,,,,,,,,,,,,,,,,,0.00,---,A,0.00,---,N,1.07,0.47,72,M";
}

void parseSub(std::string filename) {
  TrzParser parser;

  parser.parse(B);

  //Array<ParsedTrzLine> result = parser.parseFile(filename);
  //std::cout << "Successfully reached end." << std::endl;

}

int main(int argc, char **argv) {
  std::string filename = "/home/jonas/data/datasets/trz/13_02_14_a_utf8.trz";
  if (argc <= 1) {
    std::cout << "Loading trz data from default file " << filename << std::endl;
  } else {
    filename = argv[1];
    std::cout << "Loading trz data from user supplied file " << filename << std::endl;
  }

  parseSub(filename);

  return 0;
}
