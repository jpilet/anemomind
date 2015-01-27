/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/NKEParser.h>
#include <server/common/logging.h>
#include <iostream>
#include <server/common/PhysicalQuantityIO.h>

using namespace sail;

namespace {
  void readIJ(ArgMap &amap, const std::string key, int *i, int *j) {
    auto args = amap.optionArgs(key);
    args[0]->tryParseInt(i);
    args[1]->tryParseInt(j);
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  std::string filename;
  amap.registerOption("--file", "Provide name of CSV file to parse").store(&filename);
  amap.registerOption("--angle", "Provide row and col inds to view angle value").setArgCount(2);
  amap.registerOption("--velocity", "Provide row and col inds to view velocity value").setArgCount(2);
  amap.registerOption("--duration", "Provide row and col inds to view duration value").setArgCount(2);


  if (!amap.parseAndHelp(argc, argv)) {
    return -1;
  }

  NKEParser parser;
  NKEData data = parser.load(filename);

  LOG(INFO) << "Successfully loaded CSV file.";
  LOG(INFO) << "Loaded a table with " << data.rows() << " rows and " << data.cols() << " columns.";

  if (amap.optionProvided("--angle")) {
    int i = 0, j = 0;
    readIJ(amap, "--angle", &i, &j);
    std::cout << "Angle: " << data.col(j).angle(i) << std::endl;
  }

  if (amap.optionProvided("--duration")) {
    int i = 0, j = 0;
    readIJ(amap, "--duration", &i, &j);
    std::cout << "Duration: " << data.col(j).duration(i) << std::endl;
  }

  if (amap.optionProvided("--velocity")) {
    int i = 0, j = 0;
    readIJ(amap, "--velocity", &i, &j);
    std::cout << "Velocity: " << data.col(j).velocity(i) << std::endl;
  }


  return 0;
}
