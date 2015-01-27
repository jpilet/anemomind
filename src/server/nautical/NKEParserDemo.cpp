/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/NKEParser.h>
#include <server/common/logging.h>

using namespace sail;

int main(int argc, const char **argv) {
  ArgMap amap;
  std::string filename;
  amap.registerOption("--file", "Provide name of CSV file to parse").store(&filename);
  if (!amap.parseAndHelp(argc, argv)) {
    return -1;
  }

  NKEParser parser;
  NKEData data = parser.load(filename);

  LOG(INFO) << "Successfully loaded CSV file.";
  LOG(INFO) << "Loaded a table with " << data.rows() << " rows and " << data.cols() << " columns.";

  return 0;
}
