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
    return (1.0/200.0)*60.0_min;
  }
};

template <typename T>
class TemporalSignalPlot {
public:
  enum StrokeType {Line, Dot};

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
      timeSpan.extend(value.time);
      valueSpan.extend(value.value);
    }
    _data.push_back(SignalToPlot{
      type, colors.back()
    });
  }

  std::string generateColor() const {
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
    auto svg = HtmlTag::make(dst, "svg", {
        {"width", int(round(2.0*_margin
            + valueSpan.width()/ValuesPerPixel<T>::get()))},
        {"height", int(round(2.0*_margin
            + duration()/ValuesPerPixel<Duration<double>>::get()))}
    });

  }

  Duration<double> duration() const {
    return timeSpan.maxv() - timeSpan.minv();
  }
private:
  double _margin = 30;
  std::default_random_engine _rng;
  std::vector<std::string> colors{
    "red", "green", "blue", "cyan", "yellow", "magenta",
  };
  Span<TimeStamp> timeSpan;
  Span<T> valueSpan;
  std::vector<SignalToPlot> _data;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_DEBUGPLOT_H_ */
