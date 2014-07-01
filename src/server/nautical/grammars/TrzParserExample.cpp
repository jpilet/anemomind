/*
 *  Created on: 2014-06-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <iostream>
#include <server/nautical/grammars/TrzParser.h>

using namespace sail;


int main(int argc, char **argv) {
  std::string filename = "/home/jonas/data/datasets/trz/14_02_14_a_utf8.trz";
  if (argc <= 1) {
    std::cout << "Loading trz data from default file " << filename << std::endl;
  } else {
    filename = argv[1];
    std::cout << "Loading trz data from user supplied file " << filename << std::endl;
  }


  TrzParser parser;
  Array<ParsedTrzLine> parsed = parser.parseFile(filename);

  if (parsed.empty()) {
    std::cout << "No data in file" << std::endl;
  } else {
    std::cout << "Successfully parsed " << parsed.size() << " lines" << std::endl;
    int answer = 0;
    do {
      std::cout << "Which line (1.." << parsed.size() << ") do you want to inspect? Or do you want to quit (0)?" << std::endl;
      std::cout << " Type -1 to output as Matlab cell array." << std::endl;
      cin >> answer;
      if (1 <= answer && answer <= parsed.size()) {
        parser.disp(&std::cout, parsed[answer-1]);
      } else if (answer == -1) {
        std::string outfile, funname;
        std::cout << "Filename? " << std::endl;
        std::cin >> outfile;
        exportToMatlab(outfile, parsed);
      }
    } while (answer != 0);
  }
  return 0;
}
