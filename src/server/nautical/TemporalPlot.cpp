/*
 *  Created on: 2014-07-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/common/logging.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/ArrayIO.h>
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

  std::ostream &operator<<(std::ostream &s, Plottable x) {
    s << "Plottable:\n s-expr = " << x.sExpr() << "\n values = " << x.values() << "\n )";
    return s;
  }

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


    virtual void apply(PlotEnv *dst) const = 0;

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

  Plottable top(const std::vector<Plottable> &stack) {
    if (stack.empty()) {
      LOG(FATAL) << "Cannot access top element of stack: it is empty";
    }
    return stack.back();
  }

  class Disp : public PlotCmd {
   public:
    // String that triggers this command
    const char *cmd() const {return "disp";}

    // Should return a single string explaining how to use it
    const char *help() const {return "display the top stack element without popping it.";}


    void apply(PlotEnv *dst) const {
      std::cout << dst->stack().back() << std::endl;
    }
  };

  MDArray2d makeXYData(Arrayd values) {
    int count = values.size();
    MDArray2d XY(count, 2);
    for (int i = 0; i < count; i++) {
      XY(i, 0) = i;
      XY(i, 1) = values[i];
    }
    return XY;
  }

  class Plot : public PlotCmd {
   public:
    // String that triggers this command
    const char *cmd() const {return "plot";}

    // Should return a single string explaining how to use it
    const char *help() const {return "plots the top stack element without popping it.";}


    void apply(PlotEnv *dst) const {
      Plottable x = top(dst->stack());
      dst->plot().plot(makeXYData(x.values()), x.sExpr());
    }
  };

  class Show : public PlotCmd {
   public:
    // String that triggers this command
    const char *cmd() const {return "show";}

    // Should return a single string explaining how to use it
    const char *help() const {return "shows the plot on screen.";}


    void apply(PlotEnv *dst) const {
      dst->plot().show();
    }
  };


  Plottable pop(std::vector<Plottable> &stack) {
    Plottable x = top(stack);
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
    class Classname : public PlotCmd { \
     public: \
      const char *cmd() const {return CmdString;} \
      const char *help() const {return ("Unary op that computes " #XExpr) ;} \
      void apply(PlotEnv *dst) const {applyUnaryOp(CmdString, dst, [=](double x) {return XExpr;});} \
    };

  DECL_UNARY(Log, "log", std::log(x))
  DECL_UNARY(Exp, "exp", std::exp(x))
  DECL_UNARY(Abs, "abs", std::abs(x))

  #define DECL_BINARY(Classname, CmdString, XYExpr) \
    class Classname : public PlotCmd { \
     public: \
      const char *cmd() const {return CmdString;} \
      const char *help() const {return ("Binary op that computes " #XYExpr) ;} \
      void apply(PlotEnv *dst) const {applyBinaryOp(CmdString, dst, [=](double x, double y) {return XYExpr;});} \
    };

  DECL_BINARY(Add, "+", (x + y))
  DECL_BINARY(Sub, "-", (x - y))
  DECL_BINARY(Mul, "*", (x * y))
  DECL_BINARY(Div, "/", (x * y))

  void applyExtraction(const char *cmd, PlotEnv *dst, std::function<double(Nav)> extractor) {
    dst->stack().push_back(Plottable(cmd, dst->navs().map<double>(extractor)));
  }

  #define DECL_EXTRACT(Classname, MethodName, Unit) \
    class Classname : public PlotCmd { \
     public: \
      const char *cmd() const {return #MethodName "-" #Unit;} \
      const char *help() const {return "Extracts " #MethodName " in unit " #Unit " from all navs.";} \
      void apply(PlotEnv *dst) const {applyExtraction(cmd(), dst, [=](const Nav &n) {return n.MethodName().Unit();});} \
    };

  DECL_EXTRACT(AwaDegrees, awa, degrees)

  template <typename T>
  void registerCmd(ArrayBuilder<PlotCmd*> *dst) {
    static T instance;
    dst->add(&instance);
  }

  PlotEnv::PlotEnv(Array<Nav> navs_) : _navs(navs_) {
    _plot.set_style("lines");
    ArrayBuilder<PlotCmd*> builder;
    registerCmd<Add>(&builder);
    registerCmd<Sub>(&builder);
    registerCmd<Mul>(&builder);
    registerCmd<Div>(&builder);
    registerCmd<Log>(&builder);
    registerCmd<Exp>(&builder);
    registerCmd<Abs>(&builder);
    registerCmd<Disp>(&builder);
    registerCmd<Plot>(&builder);
    registerCmd<Show>(&builder);
    registerCmd<AwaDegrees>(&builder);
    _commands = builder.get();
  }

  int PlotEnv::run(int argc, const char **argv) {
    int begin = findArg(argc, argv, "begin");
    if (begin == -1) {
      std::cout << "------> ERROR <------" << std::endl;
      dispHelp();
      return -1;
    }
    for (int i = begin+1; i < argc; i++) {
      std::string cmd(argv[i]);
      if (!parsePlotCommand(cmd)) {
        std::cout << "  No such command: " << cmd << std::endl;
      }
    }
    std::cout << "Done plotting." << std::endl;
    return 0;
  }

  bool PlotEnv::parsePlotCommand(const std::string &cmd) {
    try {
      double x = std::stod(cmd);
      _stack.push_back(Plottable(cmd, Arrayd::args(x)));
      return true;
    } catch (std::exception &e) {
      for (auto x : _commands) {
        if (x->cmd() == cmd) {
          x->apply(this);
          return true;
        }
      }
      return false;
    }
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
