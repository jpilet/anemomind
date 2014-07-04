/*
 *  Created on: 2014-07-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/common/logging.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>
#include <sstream>

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

    double get(int index) {
      if (_values.size() == 1) { // useful for binary ops with scalars and vectors
        return _values[0];
      }
      return _values[index];
    }

    int size() const {
      return _values.size();
    }
   private:
    std::string _sExpr;
    Arrayd _values;
  };

  int getCommonSize(Plottable a, Plottable b) {
    if (a.size() == 1) {
      return b.size();
    }
    if (b.size() == 1) {
      return a.size();
    }

    if (a.size() != b.size()) {
      std::stringstream err;
      err << "The size of " << a.sExpr() << "(" << a.size() << ") is incompatible with\n"
             "the size of " << b.sExpr() << "(" << b.size() << ").";
      LOG(FATAL) << err.str();
      return -1;
    }
    return a.size();
  }

  Plottable binaryOp(const char *cmd, Plottable a, Plottable b, std::function<double(double,double)> op) {
    std::stringstream sExpr;
    sExpr << "(" << cmd << " " << a.sExpr() << " " << b.sExpr() << ")";

    int n = getCommonSize(a, b);
    Arrayd dst(n);
    for (int i = 0; i < n; i++) {
      dst[i] = op(a.get(i), b.get(i));
    }
    return Plottable(sExpr.str(), dst);
  }

  Plottable unaryOp(const char *cmd, Plottable a, std::function<double(double)> op) {
    std::stringstream sExpr;
    sExpr << "(" << cmd << " " << a.sExpr() << ")";
    return Plottable(sExpr.str(), a.values().map<double>(op));
  }

  class PlotEnv;
  class PlotCmd {
   public:
    // String that triggers this command
    virtual const char *cmd() const = 0;

    // Should return a single string explaining how to use it
    virtual const char *help() const = 0;


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

  Plottable pop(std::vector<Plottable> &stack) {
    if (stack.empty()) {
      LOG(FATAL) << "Cannot pop from stack: it is empty";
    }
    Plottable x = stack.back();
    stack.pop_back();
    return x;
  }

  void applyUnaryOp(const char *cmdString, PlotEnv *dst, std::function<double(double)> op) {
    Plottable arg = pop(dst->stack());
    dst->stack().push_back(unaryOp(cmdString, arg, op));
  }

  void applyBinaryOp(const char *cmdString, PlotEnv *dst, std::function<double(double,double)> op) {
    Plottable b = pop(dst->stack());
    Plottable a = pop(dst->stack());
    dst->stack().push_back(binaryOp(cmdString, a, b, op));
  }

  #define DECL_UNARY(Classname, CmdString, XExpr) \
    class Classname { \
     public: \
      const char *cmd() {return CmdString;} \
      const char *help() const {return ("Unary op that computes " #XExpr) ;} \
      void apply(PlotEnv *dst) {applyUnaryOp(CmdString, dst, [=](double x) {return XExpr;});} \
    };

  #define DECL_BINARY(Classname, CmdString, XYExpr) \
    class Classname { \
     public: \
      const char *cmd() {return CmdString;} \
      const char *help() const {return ("Binary op that computes " #XYExpr) ;} \
      void apply(PlotEnv *dst) {applyUBinaryOp(CmdString, dst, [=](double x, double y) {return XYExpr;});} \
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
