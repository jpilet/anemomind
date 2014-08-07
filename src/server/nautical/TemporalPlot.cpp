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

  Plottable pop(std::vector<Plottable> &stack) {
    Plottable x = top(stack);
    stack.pop_back();
    return x;
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

  class PlotSignal : public PlotCmd {
   public:
    // String that triggers this command
    const char *cmd() const {return "plot-signal";}

    // Should return a single string explaining how to use it
    const char *help() const {return "plots the top stack element as a temporal signal.";}


    void apply(PlotEnv *dst) const {
      Plottable x = pop(dst->stack());
      dst->plot().plot(makeXYData(x.values()), x.sExpr());
    }
  };

  class PlotXY : public PlotCmd {
     public:
      // String that triggers this command
      const char *cmd() const {return "plot-xy";}

      // Should return a single string explaining how to use it
      const char *help() const {return "Makes a 2d plot using the two top elements of the stack";}


      void apply(PlotEnv *dst) const {
        Plottable y = pop(dst->stack());
        Plottable x = pop(dst->stack());
        dst->plot().plot_xy(x.values(), y.values(), "x=" + x.sExpr() + " y=" + y.sExpr());
      }
    };

  class PlotXYZ : public PlotCmd {
     public:
      // String that triggers this command
      const char *cmd() const {return "plot-xyz";}

      // Should return a single string explaining how to use it
      const char *help() const {return "Makes a 3d plot using the three top elements of the stack";}


      void apply(PlotEnv *dst) const {
        Plottable z = pop(dst->stack());
        Plottable y = pop(dst->stack());
        Plottable x = pop(dst->stack());
        dst->plot().plot_xy(x.values(), y.values(), "x=" + x.sExpr() + " y=" + y.sExpr() + " z=" + z.sExpr());
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
  DECL_UNARY(Cos, "cos", std::cos(x))
  DECL_UNARY(Sin, "sin", std::sin(x))
  DECL_UNARY(Rad2Deg, "rad2deg", Angle<double>::radians(x).degrees())
  DECL_UNARY(Deg2Rad, "deg2rad", Angle<double>::degrees(x).radians())
  DECL_UNARY(Sqrt, "sqrt", sqrt(x))


  #define DECL_BINARY(Classname, CmdString, XYExpr) \
    class Classname : public PlotCmd { \
     public: \
      const char *cmd() const {return CmdString;} \
      const char *help() const {return ("Binary op that computes " #XYExpr) ;} \
      void apply(PlotEnv *dst) const {applyBinaryOp(CmdString, dst, [=](double x, double y) {return XYExpr;});} \
    };

  DECL_BINARY(Add, "+", (x + y))
  DECL_BINARY(Sub, "-", (x - y))
  DECL_BINARY(Mul, "mul", (x * y))
  DECL_BINARY(Div, "div", (x / y))

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

  Angle<double> getRawLeeway(const Nav &x) {
    return (x.magHdg() - x.gpsBearing()).normalizedAt0();
  }

  Duration<double> getRawTime(const Nav &n) {
    return Duration<double>::seconds(double(n.time().toMilliSecondsSince1970()/int64_t(1000)));
  }

  class LeewayRadians : public PlotCmd {
   public:
    const char *cmd() const {return "leeway-radians";}
    const char *help() const {return "Extracts leeway in unit radians from all navs.";}
    void apply(PlotEnv *dst) const {applyExtraction(cmd(), dst, [=](const Nav &n) {return getRawLeeway(n).radians();});}
  };

  class LeewayDegrees : public PlotCmd {
   public:
    const char *cmd() const {return "leeway-degrees";}
    const char *help() const {return "Extracts leeway in unit degrees from all navs.";}
    void apply(PlotEnv *dst) const {applyExtraction(cmd(), dst, [=](const Nav &n) {return getRawLeeway(n).degrees();});}
  };

  class TimeHours : public PlotCmd {
   public:
    const char *cmd() const {return "time-hours";}
    const char *help() const {return "Extracts time in unit hours from all navs.";}
    void apply(PlotEnv *dst) const {applyExtraction(cmd(), dst, [=](const Nav &n) {return getRawTime(n).hours();});}
  };

  DECL_EXTRACT(AwaDegrees, awa, degrees)
  DECL_EXTRACT(AwaRadians, awa, radians)
  DECL_EXTRACT(AwsKnots, aws, knots)
  DECL_EXTRACT(WatSpeedKnots, watSpeed, knots)
  DECL_EXTRACT(GpsSpeedKnots, gpsSpeed, knots)
  DECL_EXTRACT(GpsBearingDegrees, gpsBearing, degrees)
  DECL_EXTRACT(GpsBearingRadians, gpsBearing, radians)
  DECL_EXTRACT(MagHdgDegrees, magHdg, degrees)
  DECL_EXTRACT(MagHdgRadians, magHdg, radians)
  DECL_EXTRACT(ExtTwaDegrees, externalTwa, degrees)
  DECL_EXTRACT(ExtTwaRadians, externalTwa, radians)
  DECL_EXTRACT(ExtTwsKnots, externalTws, knots)

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
    registerCmd<PlotSignal>(&builder);
    registerCmd<Show>(&builder);
    registerCmd<AwaDegrees>(&builder);
    registerCmd<AwaRadians>(&builder);
    registerCmd<AwsKnots>(&builder);
    registerCmd<WatSpeedKnots>(&builder);
    registerCmd<GpsSpeedKnots>(&builder);
    registerCmd<GpsBearingDegrees>(&builder);
    registerCmd<GpsBearingRadians>(&builder);
    registerCmd<MagHdgDegrees>(&builder);
    registerCmd<MagHdgRadians>(&builder);
    registerCmd<ExtTwaDegrees>(&builder);
    registerCmd<ExtTwaRadians>(&builder);
    registerCmd<ExtTwsKnots>(&builder);
    registerCmd<Cos>(&builder);
    registerCmd<Sin>(&builder);
    registerCmd<Rad2Deg>(&builder);
    registerCmd<Deg2Rad>(&builder);
    registerCmd<Sqrt>(&builder);
    registerCmd<LeewayDegrees>(&builder);
    registerCmd<LeewayRadians>(&builder);
    registerCmd<TimeHours>(&builder);
    registerCmd<PlotXY>(&builder);
    registerCmd<PlotXYZ>(&builder);
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
      _stack.push_back(Plottable(cmd, Arrayd::fill(_navs.size(), x)));
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
