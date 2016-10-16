/*
 * v2demo.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/Processor2.h>
#include <server/common/AbstractArray.h>

using namespace sail;

namespace {

NavDataset getDataset(int argc, const char **argv) {
  LogLoader loader;
  if (argc < 2) {
    loader.load(PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets")
      .pushDirectory("AlinghiGC32")
      .pushDirectory("logs").get().toString());
  } else {
    for (int i = 1; i < argc; i++) {
      loader.load(std::string(argv[i]));
    }
  }
  return loader.makeNavDataset();
}

}

int main(int argc, const char **argv) {
  auto ds = getDataset(argc, argv);
  //auto log = HtmlPage::make(
      //Env::BINARY_DIR, "v2demo_log");
  Processor2::runDemoOnDataset(ds/*, log*/);
  return 0;
}


