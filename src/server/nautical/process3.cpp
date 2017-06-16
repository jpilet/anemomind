/*
 * process3.cpp
 *
 *  Created on: 15 Jun 2017
 *      Author: jonas
 */

#include <string>
#include <server/common/ArgMap.h>
#include <server/common/DynamicUtils.h>
#include <fstream>
#include <server/nautical/logimport/LogLoader.h>
#include <server/common/PathBuilder.h>
#include <Poco/File.h>
#include <server/common/DOMUtils.h>
#include <server/nautical/Process.h>

using namespace sail;

Poco::Path getDstPath(
    const ProcessSettings& settings,
    const std::vector<std::string>& paths) {
  auto p = PathBuilder::makeDirectory(settings.resultsDir).get();
  if (p.isAbsolute()) {
    return p;
  } else {
    return PathBuilder::makeDirectory(paths[0])
      .pushDirectory(settings.resultsDir)
      .get();
  }
}

DOM::Node makeRootNode(const ProcessSettings& settings,
    const Poco::Path& dstPath) {
  if (settings.outputHtml) {
    auto html = PathBuilder::makeDirectory(dstPath)
      .pushDirectory("html").get();
    Poco::File(html).createDirectories();
    return DOM::makeBasicHtmlPage("Boat log processor report",
        html.toString(), "index");
  } else {
    return DOM::Node();
  }
}

void debugOutputOrderedLogFiles(
    const std::vector<LogFileInfo>& files,
    DOM::Node* dst) {
  auto sub = DOM::linkToSubPage(dst, "Ordered input log files");
  DOM::addSubTextNode(&sub, "h1", "Ordered log files to process");

  auto table = DOM::makeSubNode(&sub, "table");
  {
    auto row = DOM::makeSubNode(&table, "tr");
    DOM::addSubTextNode(&row, "th", "Filename");
    DOM::addSubTextNode(&row, "th", "Min time");
    DOM::addSubTextNode(&row, "th", "Median time");
    DOM::addSubTextNode(&row, "th", "Max time");
  }
  for (auto f: files) {
    auto row = DOM::makeSubNode(&table, "tr");
    DOM::addSubTextNode(&row, "td", f.filename);
    DOM::addSubTextNode(&row, "td", f.minTime.toString());
    DOM::addSubTextNode(&row, "td", f.medianTime.toString());
    DOM::addSubTextNode(&row, "td", f.maxTime.toString());
  }
}

int processSub(const ProcessSettings& settings,
    const std::vector<std::string>& inputPaths) {
  auto dstPath = getDstPath(settings, inputPaths);
  Poco::File(dstPath).createDirectories();
  LOG(INFO) << "Process " << inputPaths.size() << " input paths "
      << " and output results to " << dstPath.toString();

  auto report = makeRootNode(settings, dstPath);
  auto rawLogFileInfo = listLogFiles(inputPaths);
  auto logFileInfo = adjustBoundaries(rawLogFileInfo);;
  LOG(INFO) << "Listed all log files";
  debugOutputOrderedLogFiles(logFileInfo, &report);
  LOG(INFO) << "Wrote report for log files";

  auto segmentation = presegmentData(
      logFileInfo, settings, &report);

  if (report.defined()) {
    LOG(INFO) << "Report will be saved to \n\n  " <<
        report.writer->fullFilename() << "    <-- Cmd+Left click\n\n";
  }
  return 0;
}

int main(int argc, const char** argv) {
  std::string directory;

  std::string readConfig, writeConfig;

  ProcessSettings settings;

  ArgMap amap;
  amap.setHelpInfo("Process boat logs"
      "\nExample usage:"
      "\n\n   ./nautical_process3 /Users/jonas/data/boat58b5fde26b146bd2da067681  (Courdileone)");
  amap.registerOption(
      "--read-config",
      "Read configuration file")
      .setArgCount(1).store(&readConfig);

  amap.registerOption("--write-sample-config",
      "Write a sample config file")
          .setArgCount(1)
          .store(&writeConfig);

  switch (amap.parse(argc, argv)) {
    case ArgMap::Done: return 0;
    case ArgMap::Error: return 1;
    case ArgMap::Continue: {
      if (!writeConfig.empty()) {
        JsonSettings js;
        js.indent = 2;
        js.step = 1;
        std::ofstream file(writeConfig);
        if (!writeJson(settings, &file, js)) {
          LOG(ERROR) << "Failed to write config to " << writeConfig;
          return 1;
        }
      }

      if (!readConfig.empty()) {
        std::ifstream file(readConfig);
        if (!readJson(&file, &settings)) {
          LOG(ERROR) << "Failed to read config from " << readConfig;
          return 1;
        }
      }

      std::vector<std::string> inputPaths;
      for (auto p: amap.freeArgs()) {
        inputPaths.push_back(p->value());
      }

      if (inputPaths.empty()) {
        LOG(ERROR) << "No input paths provided";
        return 1;
      }
      auto r = processSub(settings, inputPaths);
      if (r == 0) {
        LOG(INFO) << "Successfully processed everything!";
      }
      return r;
    };
  };
  return 0;
}
