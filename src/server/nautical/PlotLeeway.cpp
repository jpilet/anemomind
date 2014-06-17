/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/grammars/Grammar001.h>
#include <server/plot/extra.h>


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

      data(counter, 0) = unwrap(n.aws());
      data(counter, 1) = unwrap(n.awa().normalizedAt0());
      data(counter, 2) = unwrap((n.magHdg() - n.gpsBearing()).normalizedAt0());

      counter++;
    }
    return data.sliceRowsTo(counter);
  }
}

int main(int argc, char **argv) {
  std::cout << "A simple program that displays a plot of a\n"
               "raw/course estimate of the leeway angle,\n"
               "assuming no drift and perfectly calibrated\n"
               "instruments\n";
  "\n";
  "Arg 1 (int)  : Index of race to plot (defaults to 0)\n";
  "Arg 2 (bool) : True if we should make a 2d plot (defaults to false, that is a 3d plot)\n";
  int index = 0;
  bool plot2d = false;
  if (argc >= 1) {
    {
      std::stringstream ss;
      ss << argv[1];
      ss >> index;
      std::cout << "Using user chosen tree node " << index << std::endl;
    }
    if (argc >= 2) {
      std::stringstream ss;
      ss << argv[2];
      ss >> plot2d;
    }
  }

  std::cout << "Loading test data..." << std::endl;
  Array<Nav> allnavs = getTestdataNavs();
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


