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

  template <typename T>
  std::map<int, T> enumerateItems(const std::set<T> &src) {
    std::map<int, T> dst;
    int counter = 0;
    for (auto x: src) {
      dst[counter++] = x;
    }
    return dst;
  }

  void addToCode(std::map<DataCode, int> *dst,
      DataCode code, int n) {
    if (dst->count(code) == 0) {
      dst->insert({code, n});
    } else {
      auto f = dst->find(code);
      f->second += n;
    }
  }

  struct ListSourcesVisitor {
    std::map<std::pair<DataCode, std::string>, int> data;
    std::set<DataCode> codes;
    std::set<std::string> sources;
    std::map<DataCode, int> countPerCode;

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      sources.insert(sourceName);
      codes.insert(Code);
      int n = coll.size();
      data.insert({{Code, sourceName}, int(n)});
      addToCode(&countPerCode, Code, n);
    }
  };

  std::set<DataCode> getAllSources() {
    std::set<DataCode> dst;
#define ADD_DATA_CODE(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
        dst.insert(HANDLE);
    FOREACH_CHANNEL(ADD_DATA_CODE)
#undef ADD_DATA_CODE;
    return dst;
  }

  std::set<DataCode> listMissingCodes(
      const std::set<DataCode> &src) {
    auto all = getAllSources();
    std::set<DataCode> dst;
    for (auto x: all) {
      if (src.count(x) == 0) {
        dst.insert(x);
      }
    }
    return dst;
  }
}

// Make a table to inspect a dispatcher
void renderDispatcherTableOverview(
    const Dispatcher *d, const DOM::Node &parent) {

  ListSourcesVisitor visitor;
  visitDispatcherChannelsConst(d, &visitor);

  auto srcMap = enumerateItems<std::string>(visitor.sources);
  auto codeMap = enumerateItems<DataCode>(visitor.codes);

  auto table = DOM::makeSubNode(parent, "table");
  {
    auto header = DOM::makeSubNode(table, "tr");
    DOM::makeSubNode(header, "th");
    for (auto kv: codeMap) {
      DOM::addSubTextNode(header, "th",
          wordIdentifierForCode(kv.second));
    }
    DOM::addSubTextNode(header, "th", "Priority");
  }
  for (auto kvSrc: srcMap) {
    auto row = DOM::makeSubNode(table, "tr");
    DOM::addSubTextNode(row, "th", kvSrc.second);
    for (auto kvCode: codeMap) {
      auto td = DOM::makeSubNode(row, "td");
      auto f = visitor.data.find({kvCode.second, kvSrc.second});
      if (f != visitor.data.end()) {
        DOM::addTextNode(td, stringFormat("%d", f->second));
      }
    }
    DOM::addSubTextNode(row, "td",
        stringFormat("%d", d->sourcePriority(kvSrc.second)));
  }{
    auto row = DOM::makeSubNode(table, "tr");
    DOM::addSubTextNode(row, "th", "Total");
    for (auto kvCode: codeMap) {
      DOM::addSubTextNode(row, "td",
          stringFormat("%d",
              visitor.countPerCode.find(kvCode.second)->second));
    }
    DOM::makeSubNode(row, "td");
  }

  auto missing = listMissingCodes(visitor.codes);
  if (!missing.empty()) {
    std::stringstream ss;
    ss << "Missing: ";
    for (auto x: missing) {
      ss << wordIdentifierForCode(x) << " ";
    }
    DOM::addSubTextNode(parent, "p", ss.str());
  }
}


}



