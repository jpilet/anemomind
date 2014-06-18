/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/grammars/Grammar001.h>
#include <server/plot/extra.h>
#include <server/common/string.h>


using namespace sail;

namespace {
  double unwrap(Angle<double> x) {
    return x.degrees();
  }

  double unwrap(Velocity<double> x) {
    return x.knots();
  }

  MDArray2d getDataToPlot(Array<Nav> navs) {
    int counter = 0;
    int count = navs.size();
    MDArray2d data(count, 3);
    for (int i = 0; i < count; i++) {
      Nav &n = navs[i];

      data(counter, 0) = unwrap(n.aws()); // What about putting the boat speed (gpsSpeed or watSpeed) here in some other experiment?
      data(counter, 1) = unwrap(n.awa().normalizedAt0());
      data(counter, 2) = unwrap((n.magHdg() - n.gpsBearing()).normalizedAt0());

      counter++;
    }
    return data.sliceRowsTo(counter);
  }


  Array<std::string> getPlottableValues() {
    const int count = 4;
    static std::string v[count] = {"awa", "aws", "leeway", "gps-speed"};
    return Array<std::string>(count, v);
  }

  bool isPlottable(const char *arg, const Array<std::string> &plottables) {
    for (auto p : plottables) {
      if (arg == p) {
        return true;
      }
    }
    return false;
  }

  Array<std::string> extractPlottables(int argc, const char **argv) {
    Array<std::string> plottables = getPlottableValues();
    int count = plottables.size();
    Array<std::string> extracted(count);
    int counter = 0;
    for (int i = 0; i < count; i++) {
      const char *arg = argv[i];
      if (isPlottable(arg, plottables)) {
        extracted[counter] = arg;
        counter++;
      }
    }
    return extracted.sliceTo(counter);
  }

  Array<std::string> extractPlottablesOrDefault(int argc, const char **argv) {
    Array<std::string> extracted = extractPlottables(argc, argv);

    const int ndef = 3;
    static std::string def[ndef] = {"awa", "aws", "leeway"};
    Array<std::string> defaultPlottables(ndef, def);

    if (extracted.empty()) {
      return defaultPlottables();
    } else if (extracted.size() != 2 && extracted.size() != 3) {
      std::cout << "Can only make a 2d or 3d scatter plot. Please provide 2 or 3 values to plot, not " << extracted.size() << ":\n";
      std::cout << EXPR_AND_VAL_AS_STRING(extracted) << std::endl;
      return defaultPlottables;
    }
    return extracted;
  }
}

int main(int argc, const char **argv) {
  Array<std::string> toPlot = extractPlottablesOrDefault(argc, argv);

  std::cout << "Loading test data..." << std::endl;
  Array<Nav> allnavs = getTestdataNavs(argc, argv);
  std::cout << "Done loading " << allnavs.size() << " navs." << std::endl;


  Grammar001Settings s;
  Grammar001 g(s);

  std::cout << "Parsing..." << std::endl;
  std::shared_ptr<HTree> tree = g.parse(allnavs);
  std::cout << "Done extracting a tree with " << tree->childCount() << " children." << std::endl;

  std::shared_ptr<HTree> ch = tree->child(index);
  Array<Nav> navs = allnavs.slice(ch->left(), ch->right());
  std::cout << "Create a plot from " << navs.size() << " navs." << std::endl;

  GnuplotExtra plot;
  plot.plot(getDataToPlot(navs).sliceColsFrom(int(plot2d)));
  plot.show();

  return 0;
}


