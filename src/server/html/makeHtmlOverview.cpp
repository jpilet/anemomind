/*
 * makeHtmlOverview.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/html/HtmlDispatcher.h>
#include <server/nautical/logimport/LogLoader.h>

using namespace sail;

int main(int argc, const char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: src-path-1 ... src-path-N output-filename\n";
    return 1;
  } else {
    auto dst = DOM::makeBasicHtmlPage("Dispatcher overview");
    auto ul = DOM::makeSubNode(&dst, "ul");
    LogLoader loader;
    for (int i = 1; i < argc-1; i++) {
      std::string p = argv[i];
      DOM::addSubTextNode(&ul, "li", p);
      if (!loader.load(p)) {
        return -1;
      }
    }
    auto ds = loader.makeNavDataset();
    renderDispatcherTableOverview(
        ds.dispatcher().get(), &dst);
    DOM::writeHtmlFile(argv[argc-1], dst.document);
  }
  return 0;
}



