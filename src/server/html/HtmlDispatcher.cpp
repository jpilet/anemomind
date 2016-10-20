/*
 * HtmlDispatcher.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/html/HtmlDispatcher.h>
#include <device/anemobox/DispatcherUtils.h>

namespace sail {

void channelSummaryToHtml(const ChannelSummary &info, DOM::Node dst) {
    auto table = DOM::makeSubNode(dst, "table");
    {
      auto row = DOM::makeSubNode(table, "tr");
      DOM::addSubTextNode(row, "td", "Count");
      DOM::addSubTextNode(row, "td",
          stringFormat("%d", info.count));
    }{
      auto row = DOM::makeSubNode(table, "tr");
      DOM::addSubTextNode(row, "td", "From");
      DOM::addSubTextNode(row, "td", info.from.toString());
    }{
      auto row = DOM::makeSubNode(table, "tr");
      DOM::addSubTextNode(row, "td", "To");
      DOM::addSubTextNode(row, "td", info.to.toString());
    }
  }

void renderChannelSummaryCountToHtml(
    const ChannelSummary &info, DOM::Node dst) {
  DOM::addTextNode(dst, stringFormat("%d", info.count));
}

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

  void addToCode(std::map<DataCode, ChannelSummary> *dst,
      DataCode code, const ChannelSummary &x) {
    if (dst->count(code) == 0) {
      dst->insert({code, x});
    } else {
      auto f = dst->find(code);
      f->second.extend(x);
    }
  }

  struct ListSourcesVisitor {
    std::map<std::pair<DataCode, std::string>, ChannelSummary> data;
    std::set<DataCode> codes;
    std::set<std::string> sources;
    std::map<DataCode, ChannelSummary> countPerCode;

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      sources.insert(sourceName);
      codes.insert(Code);
      int n = coll.size();
      const auto &v = coll.samples();

      ChannelSummary info;
      info.count = n;
      if (!v.empty()) {
        info.from = v.front().time;
        info.to = v.back().time;
      }
      data.insert({{Code, sourceName}, info});
      addToCode(&countPerCode, Code, info);
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
    const Dispatcher *d, const DOM::Node &parent,
    std::function<void(ChannelSummary, DOM::Node)>
            channelInfoRenderer) {

  if (d == nullptr) {
    DOM::addSubTextNode(parent, "p", "Dispatcher is null");
    return;
  }

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
        channelInfoRenderer(f->second, td);
      }
    }
    DOM::addSubTextNode(row, "td",
        stringFormat("%d", d->sourcePriority(kvSrc.second)));
  }{
    auto row = DOM::makeSubNode(table, "tr");
    DOM::addSubTextNode(row, "th", "Total");
    for (auto kvCode: codeMap) {
      auto td = DOM::makeSubNode(row, "td");
      channelInfoRenderer(
          visitor.countPerCode.find(kvCode.second)->second, td);
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



