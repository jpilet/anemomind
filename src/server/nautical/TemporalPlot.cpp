/*
 *  Created on: 2014-07-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/common/logging.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>

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

  class PlotEnv;
  class PlotCmd {
   public:
    // String that triggers this command
    virtual const char *cmd() = 0;

    // Should return a single string explaining how to use it
    virtual const char *help() = 0;


    virtual void apply(PlotEnv *dst) = 0;

    virtual ~PlotCmd() {}
  };

  class PlotEnv {
   public:
    PlotEnv(Array<Nav> navs_);

    int run(int argc, const char **argv);

    // Access navs
    Array<Nav> navs() const {return _navs;}

    // Accessors allowing for mutation
    std::vector<Plottable> &stack() {return _stack;}
    GnuplotExtra &plot() {return _plot;}
    void dispHelp();
    void dispCommands();
   private:
    Array<Nav> _navs;
    Array<PlotCmd*> _commands;
    std::vector<Plottable> _stack;
    GnuplotExtra _plot;
    bool parsePlotCommand(const std::string &cmd);
  };

  template <typename T>
  void registerCmd(ArrayBuilder<PlotCmd*> *dst) {
    static T instance;
    dst->add(&instance);
  }

  PlotEnv::PlotEnv(Array<Nav> navs_) : _navs(navs_) {
    ArrayBuilder<PlotCmd*> builder;
    //registerCmd<Add>(&builder);
    _commands = builder.get();
  }

  int PlotEnv::run(int argc, const char **argv) {
    int begin = findArg(argc, argv, "begin");
    if (begin == -1) {
      dispHelp();
      return -1;
    }
    for (int i = begin+1; i < argc; i++) {
      std::string cmd(argv[i]);
      if (!parsePlotCommand(cmd)) {
        std::cout << "  No such command: " << cmd << std::endl;
      }
    }
    return 0;
  }

  bool PlotEnv::parsePlotCommand(const std::string &cmd) {
    for (auto x : _commands) {
      if (x->cmd() == cmd) {
        x->apply(this);
        return true;
      }
    }
    return false;
  }

  void PlotEnv::dispHelp() {
    std::cout << "Please begin your plot command sequence with 'begin'." << std::endl;
    std::cout << "After that, you can use any of the following commands:" << std::endl;
    dispCommands();
  }

  void PlotEnv::dispCommands() {
    for (auto x : _commands) {
      std::cout << "  " << x->cmd() << ": " << x->help() << std::endl;
    }
  }
}

int main(int argc, const char **argv) {
  using namespace sail;
  PlotEnv env(getTestdataNavs(argc, argv));
  return env.run(argc, argv);
}
