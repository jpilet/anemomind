/*
 * v2demo.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/logimport/LogLoader.h>

using namespace sail;

int main(int argc, const char **argv) {
  LogLoader loader;
  loader.load(PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("AlinghiGC32")
    .pushDirectory("logs").get().toString());
  auto ds = loader.makeNavDataset();

  std::cout << "Loaded dataset" << std::endl;
  ds.outputSummary(&std::cout);

  return 0;
}


