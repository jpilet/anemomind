/*
 *  Created on: Jun 26, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <iostream>
#include <server/nautical/grammars/TrzParser.h>

using namespace sail;

int main(int argc, char **argv) {
  std::string filename = "/home/jonas/data/datasets/trz/13_02_14_a.trz";
  if (argc <= 1) {
    std::cout << "Loading trz data from default file " << filename << std::endl;
  } else {
    filename = argv[1];
    std::cout << "Loading trz data from user supplied file " << filename << std::endl;
  }

  TrzParser parser;

  Array<ParsedTrzLine> result = parser.parseFile(filename);

  std::cout << "Successfully reached end." << std::endl;

  return 0;
}
