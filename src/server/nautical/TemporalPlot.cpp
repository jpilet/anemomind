/*
 *  Created on: 2014-07-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>

namespace sail {

  class Plottable {
   public:
    Plottable(std::string sExpr_, Arrayd values_) :
      _sExpr(sExpr_), _values(values_) {}

    const std::string &sExpr() const {
      return _sExpr;
    }

    Arrayd values() const {
      return _values;
    }
   private:
    std::string _sExpr;
    Arrayd _values;
  };

  class PlotCmd {
   public:
    virtual const char *cmd() = 0;
    virtual void apply(std::vector<Plottable> *stack, GnuplotExtra *dst) = 0;
    virtual ~PlotCmd() {}
  };

  class PlotEnv {
   public:
    int run(Array<Nav> navs, int argc, const char **argv);
   private:
    std::vector<Plottable> _stack;
    GnuplotExtra _plot;
  };

  int PlotEnv::run(Array<Nav> navs, int argc, const char **argv) {
    int begin = findArg(argc, argv, "begin");
    if (begin == -1) {
      std::cout << "Please begin your plot command sequence with 'begin'." << std::endl;
      return -1;
    }
    for (int i = begin+1; i < argc; i++) {
      parsePlotCommand();
    }
    return 0;
  }
}

int main(int argc, const char **argv) {
  using namespace sail;
  Array<Nav> navs = getTestdataNavs();
  PlotEnv env;
  return env.run(navs, argc, argv);
}
