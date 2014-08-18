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
#include <server/nautical/GeographicReference.h>
#include <sstream>
#include <map>

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

    typedef std::function<void(GnuplotExtra &plot, int axis)> AxisOp;

    void setAxisOp(AxisOp op) {_op = op;}
    void applyAxisOp(GnuplotExtra &extra, int i) const {
      if (_op) {
        _op(extra, i);
      }
    }
   private:
    std::string _sExpr;
    Arrayd _values;
    AxisOp _op;
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

  typedef std::map<const char *, PlotCmd*> CmdMap;

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
    CmdMap _commands;
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

  class PlotXY : public PlotCmd {
     public:
      // String that triggers this command
      const char *cmd() const {return "plot-xy";}

      // Should return a single string explaining how to use it
      const char *help() const {return "Makes a 2d plot using the two top elements of the stack";}


      void apply(PlotEnv *dst) const {
        Plottable y = pop(dst->stack());
        Plottable x = pop(dst->stack());
        x.applyAxisOp(dst->plot(), 0);
        y.applyAxisOp(dst->plot(), 1);
        dst->plot().plot_xy(x.values(), y.values(), "x=" + x.sExpr() + " y=" + y.sExpr());
      }
    };

  const int styleCount = 11;
  const char *styles[styleCount] = {"lines", "points",
      "linespoints", "impulses", "dots", "steps", "fsteps", "histeps",
      "boxes", "filledcurves", "histograms"};


  template <int styleIndex>
  class PlotStyle : public PlotCmd {
   public:
    const char *style() const {return styles[styleIndex];}

    const char *cmd() const {
      static std::string cmdstr = std::string("set-style-") + style();
      return cmdstr.c_str();
    }

    const char *help() const {
      static std::string helpstr = std::string("Sets the plot style to ") + style();
      return helpstr.c_str();
    }

    void apply(PlotEnv *dst) const {
      dst->plot().set_style(style());
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
        x.applyAxisOp(dst->plot(), 0);
        y.applyAxisOp(dst->plot(), 1);
        z.applyAxisOp(dst->plot(), 2);
        dst->plot().plot_xyz(x.values(), y.values(), z.values(),
            "x=" + x.sExpr() + " y=" + y.sExpr() + " z=" + z.sExpr());
      }
    };

  class Dup : public PlotCmd {
   public:
    const char *cmd() const {return "dup";}
    const char *help() const {return "duplicates the top element.";}
    void apply(PlotEnv *env) const {
      env->stack().push_back(top(env->stack()));
    }
  };

  class Pop : public PlotCmd {
   public:
    const char *cmd() const {return "pop";}
    const char *help() const {return "pops the top element.";}
    void apply(PlotEnv *env) const {
      pop(env->stack());
    }
  };

  class Fetch : public PlotCmd {
   public:
    const char *cmd() const {return "fetch";}
    const char *help() const {return "fetches the nth element from the top after n has been popped.";}
    void apply(PlotEnv *env) const {
      std::vector<Plottable> &stack = env->stack();
      int n = int(pop(stack).get(0));
      int len = stack.size();
      int at = len - n - 1;
      Plottable nth = stack[at];
      for (int i = 0; i < n; i++) {
        stack[at + i] = stack[at + i + 1];
      }
      stack[len-1] = nth;
    }
  };

  GeographicPosition<double> getFirstGeoPos(Array<Nav> navs) {
    return navs[0].geographicPosition();
  }

  class MakeLocalXY : public PlotCmd {
   public:
    const char *cmd() const {return "make-local-xy";}
    const char *help() const {return "computes an x-"
        "and a y-vector in a local 2d coordinate sys"
        "tem from the geographic positions.";}
    void apply(PlotEnv *env) const;
   private:
  };

  void MakeLocalXY::apply(PlotEnv *env) const {
     GeographicReference ref(getFirstGeoPos(env->navs()));
     typedef GeographicReference::ProjectedPosition GRPP;
     Array<GRPP> pos = env->navs().map<GRPP>([=] (const Nav &n) {
       return ref.map(n.geographicPosition());
     });
     Arrayd X = pos.map<double>([=](const GRPP &pos) {return pos[0].nauticalMiles();});
     Arrayd Y = pos.map<double>([=](const GRPP &pos) {return pos[1].nauticalMiles();});
     env->stack().push_back(Plottable("local-x-nautical-miles", X));
     env->stack().push_back(Plottable("local-y-nautical-miles", Y));
  }

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


  namespace {
    double deg2rad(double x) {
      return Angle<double>::degrees(x).radians();
    }
  }

  DECL_UNARY(Log, "log", std::log(x))
  DECL_UNARY(Exp, "exp", std::exp(x))
  DECL_UNARY(Abs, "abs", std::abs(x))
  DECL_UNARY(Cos, "cos", std::cos(deg2rad(x)))
  DECL_UNARY(Sin, "sin", std::sin(deg2rad(x)))
  DECL_UNARY(Sqrt, "sqrt", sqrt(x))
  #undef DECL_UNARY

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
  DECL_BINARY(Div, "/", (x / y))
  #undef DECL_BINARY

  void applyExtraction(const char *cmd, PlotEnv *dst, std::function<double(Nav)> extractor) {
    dst->stack().push_back(Plottable(cmd, dst->navs().map<double>(extractor)));
  }


  Angle<double> getRawLeeway(const Nav &x) {
    return (x.magHdg() - x.gpsBearing()).normalizedAt0();
  }

  Duration<double> getRawTime(const Nav &n) {
    return Duration<double>::seconds(double(n.time().toMilliSecondsSince1970()/int64_t(1000)));
  }


  class LeewayDegrees : public PlotCmd {
   public:
    const char *cmd() const {return "leeway-degrees";}
    const char *help() const {return "Extracts leeway in unit degrees from all navs.";}
    void apply(PlotEnv *dst) const {applyExtraction(cmd(), dst, [=](const Nav &n) {return getRawLeeway(n).degrees();});}
  };


  void setTimeFormatForAxis(GnuplotExtra &plot, int i) {
    const char axis[3] = {'x', 'y', 'z'};
    char sym = axis[i];
    plot.cmd(stringFormat("set %cdata time\n", sym));
    plot.cmd("set timefmt \"%s\"\n");
    plot.cmd(stringFormat("set format %c \"%s\"\n",
                          sym, "%m/%d/%Y %H:%M:%S"));

    // What does this line do? I copied it from PlotNavs
    plot.cmd("set xtics nomirror rotate by -45\n");
  }

  class TimeSeconds : public PlotCmd {
   public:
    const char *cmd() const {return "time-seconds";}
    const char *help() const {return "Extracts time in unit seconds from all navs.";}
    void apply(PlotEnv *dst) const {
      Plottable x(cmd(), dst->navs().map<double>([=](const Nav &n) {return getRawTime(n).seconds();}));
      x.setAxisOp([=](GnuplotExtra &plot, int i) {setTimeFormatForAxis(plot, i);});
      dst->stack().push_back(x);
    }
  };

  class Inds : public PlotCmd {
   public:
    const char *cmd() const {return "inds";}
    const char *help() const {return "Pushes indices from 0 up to the length of navs";}
    void apply(PlotEnv *dst) const {
      int n = dst->navs().size();
      Arrayd inds(n);
      for (int i = 0; i < n; i++) {
        inds[i] = i;
      }
      dst->stack().push_back(Plottable("inds", inds));
    }
  };

  class TimePlot : public PlotCmd {
   public:
    // String that triggers this command
    const char *cmd() const {return "time-plot";}

    // Should return a single string explaining how to use it
    const char *help() const {return "plots the top stack element as a temporal signal.";}


    void apply(PlotEnv *dst) const {
      Plottable x = pop(dst->stack());
      TimeSeconds().apply(dst);
      dst->stack().push_back(x);
      PlotXY().apply(dst);
    }
  };

  class DifsPred : public PlotCmd {
   public:
    const char *cmd() const {return "difspred";}

    const char *help() const {return "Compute the difference between every element and its predecessor in the top vector of the stack.";}

    void apply(PlotEnv *dst) const {
      Plottable x = pop(dst->stack());
      int n = x.size();
      Arrayd y(n);
      y[0] = 0;
      for (int i = 1; i < n; i++) {
        y[i] = x.get(i) - x.get(i-1);
      }
      dst->stack().push_back(Plottable("(difspred " + x.sExpr(), y));
    }
  };

  #define DECL_EXTRACT(Classname, MethodName, Unit) \
  class Classname : public PlotCmd { \
   public: \
    const char *cmd() const {return #MethodName "-" #Unit;} \
    const char *help() const {return "Extracts " #MethodName " in unit " #Unit " from all navs.";} \
    void apply(PlotEnv *dst) const {applyExtraction(cmd(), dst, [=](const Nav &n) {return n.MethodName().Unit();});} \
  };

  DECL_EXTRACT(AwaDegrees, awa, degrees)
  DECL_EXTRACT(AwsKnots, aws, knots)
  DECL_EXTRACT(WatSpeedKnots, watSpeed, knots)
  DECL_EXTRACT(GpsSpeedKnots, gpsSpeed, knots)
  DECL_EXTRACT(GpsBearingDegrees, gpsBearing, degrees)
  DECL_EXTRACT(MagHdgDegrees, magHdg, degrees)
  DECL_EXTRACT(ExtTwaDegrees, externalTwa, degrees)
  DECL_EXTRACT(ExtTwsKnots, externalTws, knots)

  #undef DECL_EXTRACT


  template <typename T>
  void registerCmd(CmdMap *dst) {
    static T instance;
    (*dst)[instance.cmd()] = &instance;
  }

  template <int N>
  class RegisterPlotStyle {
   public:
    static void exec(CmdMap *builder) {
      registerCmd<PlotStyle<N-1> >(builder);
      RegisterPlotStyle<N-1>::exec(builder);
    }
  };

  template <>
  class RegisterPlotStyle<0> {
   public:
    static void exec(CmdMap *builder) {}
  };



  PlotEnv::PlotEnv(Array<Nav> navs_) : _navs(navs_) {
    _plot.set_style("lines");
    std::map<const char *, PlotCmd*> commands;
    registerCmd<Add>(&commands);
    registerCmd<Sub>(&commands);
    registerCmd<Mul>(&commands);
    registerCmd<Div>(&commands);
    registerCmd<Log>(&commands);
    registerCmd<Exp>(&commands);
    registerCmd<Abs>(&commands);
    registerCmd<Disp>(&commands);
    registerCmd<TimePlot>(&commands);
    registerCmd<Show>(&commands);
    registerCmd<AwaDegrees>(&commands);
    registerCmd<AwsKnots>(&commands);
    registerCmd<WatSpeedKnots>(&commands);
    registerCmd<GpsSpeedKnots>(&commands);
    registerCmd<GpsBearingDegrees>(&commands);
    registerCmd<MagHdgDegrees>(&commands);
    registerCmd<ExtTwaDegrees>(&commands);
    registerCmd<ExtTwsKnots>(&commands);
    registerCmd<Cos>(&commands);
    registerCmd<Sin>(&commands);
    registerCmd<Sqrt>(&commands);
    registerCmd<LeewayDegrees>(&commands);
    registerCmd<TimeSeconds>(&commands);
    registerCmd<PlotXY>(&commands);
    registerCmd<PlotXYZ>(&commands);
    RegisterPlotStyle<styleCount>::exec(&commands);
    registerCmd<Dup>(&commands);
    registerCmd<MakeLocalXY>(&commands);
    registerCmd<Pop>(&commands);
    registerCmd<Fetch>(&commands);
    registerCmd<DifsPred>(&commands);
    registerCmd<Inds>(&commands);
    _commands = commands;
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
        std::cout << "  ----------> ERROR: No such command: " << cmd << std::endl;
      }
    }
    std::cout << "Done plotting." << std::endl;
    return 0;
  }

  bool PlotEnv::parsePlotCommand(const std::string &cmd) {
    try {
      double x = std::stod(cmd);
      _stack.push_back(Plottable(cmd, Arrayd::fill(1, x)));
      return true;
    } catch (std::exception &e) {
      CmdMap::iterator at = _commands.find(cmd.c_str());
      if (at == _commands.end()) {
        return false;
      } else {
        at->second->apply(this);
        return true;
      }
    }
  }

  void PlotEnv::dispHelp() {
    std::cout << "Please begin your plot command sequence with 'begin'." << std::endl;
    std::cout << "After that, you can use any of the following commands:" << std::endl;
    dispCommands();
  }

  void PlotEnv::dispCommands() {
    for (auto x : _commands) {
      std::cout << "  " << x.first << ": " << x.second->help() << std::endl;
    }
  }
}

int main(int argc, const char **argv) {
  using namespace sail;
  PlotEnv env(getTestdataNavs(argc, argv));
  return env.run(argc, argv);
}
