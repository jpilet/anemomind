/*
 * TimedValueDiagram.cpp
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#include <server/html/TimedValueDiagram.h>
#include <server/common/logging.h>
#include <server/plot/CairoUtils.h>
#include <iostream>

namespace sail {

TimedValueDiagram::TimedValueDiagram(
    cairo_t *dstContext,
    TimeStamp fromTime,
    TimeStamp toTime,
    const Settings &s) :
        _dstContext(dstContext), _settings(s),
        _fromTime(fromTime), _toTime(toTime) {}

void TimedValueDiagram::addTimes(
    const std::string &label,
    const Array<TimeStamp> &times) {
  CHECK(std::is_sorted(times.begin(), times.end()));
  _y += _settings.verticalStep;

  {
    Cairo::WithLocalContext wlc(_dstContext);
    cairo_set_line_width(_dstContext, 1.0);
    cairo_set_source_rgb(_dstContext, 0.0, 0.0, 0.0);
    drawLine(_fromTime, _toTime);
  }

  if (times.empty()) {
    return;
  }
  TimeStamp from = latest(times.first(), _fromTime);

  if (_toTime < from) {
    return;
  }

  TimeStamp to = from;
  for (auto t: times.sliceFrom(1)) {
    if (_toTime < t) { // Reached the end of the plot?
      drawLine(from, _toTime); // Finish off the last line
      break;
    }

    if (_settings.margin < t - to) { // Sufficiently large gap?
      drawLine(from, to); // Complete the line we were about to make...
      from = t; // ...and start a new one.
    }
    to = t;

    /*{
      using namespace Cairo;
      WithLocalDeviceScale with(
          _dstContext, WithLocalDeviceScale::Identity);
      cairo_move_to(_dstContext, timeToX(_toTime), _y);
      cairo_show_text(_dstContext, label.c_str());
    }*/
  }
}

void TimedValueDiagram::drawLine(TimeStamp a, TimeStamp b) const {
  using namespace Cairo;
  if (a < b) {
    std::cout << "Line from " << a.toString()
        << " to " << b.toString() << std::endl;
    double a0 = timeToX(a);
    double b0 = timeToX(b);
    double extra = 0.5*std::max(_settings.minWidth - (b0 - a0), 0.0);
    std::cout << "Extra = " << extra << std::endl;
    cairo_move_to(_dstContext, a0 - extra, _y);
    cairo_line_to(_dstContext, b0 + extra, _y);
    WithLocalDeviceScale with(_dstContext,
        WithLocalDeviceScale::Identity);
    cairo_stroke(_dstContext);
  }
}

double TimedValueDiagram::timeToX(TimeStamp t) const {
  return _settings.timeWidth*((t - _fromTime)/(_toTime - _fromTime));
}


}


