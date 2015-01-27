/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/NKEParser.h>

using namespace sail;

int main(int argc, const char **argv) {
  ArgMap amap;
  std::string filename;
  amap.registerOption("--file", "Provide name of CSV file to parse").store(&filename);

  NKEParser parser;
  NKEData data = parser.load(filename);

  return 0;
}
