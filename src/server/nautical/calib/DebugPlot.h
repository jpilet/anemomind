/*
 * DebugPlot.h
 *
 *  Created on: 1 Oct 2016
 *      Author: jonas
 *
 *  Just for debuggin
 */

#ifndef SERVER_NAUTICAL_CALIB_DEBUGPLOT_H_
#define SERVER_NAUTICAL_CALIB_DEBUGPLOT_H_

#include <server/common/TimeStamp.h>
#include <string>
#include <iosfwd>
#include <random>
#include <sstream>
#include <server/common/LineKM.h>

namespace sail {

template <typename T>
struct ValuesPerPixel {
  static double get() {
    return 1.0;
  }
};

template <>
struct ValuesPerPixel<Velocity<double>> {
  static Velocity<double> get() {
    return (1.0/200.0)*20.0_kn;
  }
};

template <>
struct ValuesPerPixel<Angle<double>> {
  static Angle<double> get() {
    return (1.0/200.0)*360.0_deg;
  }
};

template <>
struct ValuesPerPixel<Duration<double>> {
  static Duration<double> get() {
    return (1.0/200.0)*1.0_min;
  }
};

enum class StrokeType {Line, Dot};

template <typename T>
class TemporalSignalPlot {
public:

  struct SignalToPlot {
    StrokeType type;
    std::string color;
    Array<TimedValue<T>> values;
  };

  void add(
      StrokeType type,
      const Array<TimedValue<T>> &values,
      std::string color = "") {
    color = color.empty()? generateColor() : color;
    for (auto value: values) {
      CHECK(value.time.defined());
      timeSpan.extend(value.time);
      CHECK(timeSpan.initialized());
      valueSpan.extend(value.value);
      CHECK(valueSpan.initialized());
    }
    _data.push_back(SignalToPlot{
      type, colors.back(), values
    });
  }

  std::string generateColor() {
    if (!colors.empty()) {
      auto c = colors.back();
      colors.pop_back();
      return c;
    } else {
      std::uniform_int_distribution<int> distrib(0, 359);
      std::stringstream code;
      code << "hsl(" << distrib(_rng) << ", 100%, 50%)";
      return code.str();
    }
  }

  void renderTo(HtmlNode::Ptr dst) const {
    if (!(valueSpan.initialized() && timeSpan.initialized())) {
      HtmlTag::tagWithData(dst, "p", "Nothing to plot");
      return;
    }
    auto vpy = ValuesPerPixel<T>::get();
    auto vpx = ValuesPerPixel<Duration<double>>::get();
    auto dur = duration();
    double pixelHeight = valueSpan.width()/vpy;
    HtmlTag::tagWithData(dst, "p",
        stringFormat("Duration: %s", dur.str().c_str()));
    double pixelWidth = dur/vpx;
    auto svg = HtmlTag::make(dst, "svg", {
        {"height", int(round(pixelHeight))},
        {"width", int(round(pixelWidth))}
    });
    auto xmap = LineKM::identity();
    auto ymap = LineKM(
        valueSpan.minv()/vpy, valueSpan.maxv()/vpy,
        pixelHeight, 0.0);

    auto canvas = HtmlTag::make(svg, "g", {
      {"transform",
          stringFormat("matrix(%.3g, %.3g, %.3g, %.3g, %.3g, %.3g)",
              xmap.getK(), 0.0,
              0.0, ymap.getK(),
              xmap.getM(), ymap.getM())}
    });
    auto &stream = canvas->stream();
    //auto &stream = svg->stream();
    for (const SignalToPlot &curve: _data) {
      /*if (curve.type == StrokeType::Line)*/ {
        stream << "<polyline points=\"";
        for (auto pt: curve.values) {
          stream << (pt.time - timeSpan.minv())/vpx
                 << "," << (pt.value/vpy) << " ";
        }
        stream << "\" style=\"fill: none; stroke:" << curve.color << "; stroke-width: 2\" />";
      }
    }
  }

  Duration<double> duration() const {
    return timeSpan.maxv() - timeSpan.minv();
  }
private:
  std::default_random_engine _rng;
  std::vector<std::string> colors{
    "red", "green", "blue",
  };
  Span<TimeStamp> timeSpan;
  Span<T> valueSpan;
  std::vector<SignalToPlot> _data;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_DEBUGPLOT_H_ */
