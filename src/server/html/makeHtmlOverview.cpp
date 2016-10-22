/*
 * makeHtmlOverview.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/html/HtmlDispatcher.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/html/TimedValueDiagram.h>
#include <server/plot/CairoUtils.h>
#include <server/common/LineKM.h>
#include <cairo/cairo-svg.h>

using namespace sail;


struct DiagramVisitor {
  int counter = 0;
  TimedValueDiagram *diagram;
  LineKM hueMap;

  DiagramVisitor(TimedValueDiagram *d,
      const LineKM &hm) : diagram(d),
      hueMap(hm) {}

  template <DataCode Code, typename T>
  void visit(
      const char *shortName,
      const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
    Cairo::setSourceColor(diagram->context(),
        PlotUtils::HSV::fromHue(hueMap(counter)*1.0_deg));
    const auto &s = coll.samples();
    cairo_set_line_width(diagram->context(), 4);
    diagram->addTimedValues(
        sourceName, s.begin(), s.end());
    counter++;
  }
};

void renderTimedValueDiagram(
    const std::string &dstFilename,
    const Dispatcher *d,
    TimeStamp fromTime, TimeStamp toTime,
    DOM::Node *dst) {
  using namespace Cairo;

  int n = countChannels(d);
  LineKM hueMap(0.0, n, 0.0, 360);

  TimedValueDiagram::Settings settings;
  settings.timeWidth = 1024;

  std::string imageName = dstFilename + ".svg";
  const char *cc = imageName.c_str();

  double labelMarg = 100;

  auto surface = sharedPtrWrap(
      cairo_svg_surface_create(imageName.c_str(),
          settings.timeWidth + labelMarg, (n + 2)*settings.verticalStep));
  auto cr = sharedPtrWrap(cairo_create(surface.get()));

  std::cout << "From " << fromTime << " to " << toTime << std::endl;

  cairo_set_font_size (cr.get(), 11);
  cairo_select_font_face (cr.get(), "Georgia",
      CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  TimedValueDiagram diagram(cr.get(), fromTime, toTime, settings);

  DiagramVisitor v(&diagram, hueMap);
  visitDispatcherChannelsConst(d, &v);

  auto img = DOM::makeSubNode(dst, "img");
  img.element->setAttribute(
        Poco::XML::toXMLString("src"),
        Poco::XML::toXMLString(imageName));
}

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
    auto d = ds.dispatcher().get();

    renderDispatcherTableOverview(d, &dst);

    std::string dstFilename = argv[argc-1];

    auto fromTime = earliestTimeStamp(d);
    auto toTime = latestTimeStamp(d);
    renderTimedValueDiagram(
        dstFilename,
        d, fromTime, toTime, &dst);

    DOM::writeHtmlFile(dstFilename, dst.document);
  }
  return 0;
}



