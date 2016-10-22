/*
 * process2.cpp
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#include <server/common/CmdArg.h>
#include <server/common/Env.h>
#include <server/nautical/Processor2.h>

using namespace sail;

int main(int argc, const char **argv) {
  using namespace CmdArg;

  Processor2::Settings settings;

  LogLoader loader;

  Parser parser(
      "Boat log processor"
      "  * Calibrates"
      "  * Filters"
      "  * Segments");

  parser.bind({"--path", "-p"}, {
      inputForm([&](const std::string &p) {
        return loader.load(p);
      }, Arg<std::string>("path")
        .describe("to file or directory of log files"))
      .describe("Provide a path to load")
  }).describe("Load log files");

  parser.bind({"--alinghi-demo"}, {
      inputForm([&]() {
        loader.load(
            PathBuilder::makeDirectory(Env::SOURCE_DIR)
              .pushDirectory("datasets")
              .pushDirectory("AlinghiGC32")
              .pushDirectory("logs").get());
        return true;
      })
  }).describe("Load the alinghi dataset");

  auto status = parser.parse(argc, argv);

  switch (status) {
  case Parser::Continue:

  case Parser::Done:
    return 0;
  case Parser::Error:
    return 1;
  };

  return 0;
}


