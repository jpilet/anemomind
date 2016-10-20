/*
 * HtmlDispatcher.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/html/HtmlDispatcher.h>
#include <device/anemobox/DispatcherUtils.h>

namespace sail {

namespace {

  struct SampleCount {
    DataCode code;
    std::string src;
    int n;
  };

  struct ListSourcesVisitor {
    std::vector<SampleCount> data;
    std::set<DataCode> codes;
    std::set<std::string> sources;

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      sources.insert(sourceName);
      codes.insert(Code);
      data.push_back(SampleCount{
        Code, sourceName, int(coll.size())
      });
    }
  };


  std::map<int, std::string> enumerateSources(
      const std::set<std::string> &sources) {
    std::map<int, std::string> dst;
    int counter = 0;
    for (auto s: sources) {
      dst[counter++] = s;
    }
    return dst;
  }
}

// Make a table to inspect a dispatcher
void renderDispatcherTableOverview(
    const Dispatcher *d, const DOM::Node &parent) {
  auto table = DOM::makeSubNode(parent, "table");

  ListSourcesVisitor visitor;
  visitDispatcherChannelsConst(d, &visitor);


}


}



