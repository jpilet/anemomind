/*
 * TimedValueDiagram.h
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_HTML_TIMEDVALUEDIAGRAM_H_
#define SERVER_HTML_TIMEDVALUEDIAGRAM_H_

#include <cairo/cairo.h>
#include <server/common/TimeStamp.h>
#include <server/common/Unmovable.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Span.h>
#include <server/plot/PlotUtils.h>

namespace sail {

class TimedValueDiagram {
public:
  struct Settings {
    Duration<double> margin = 1.0_minutes;
    double timeWidth = 1024;
    double verticalStep = 1.0;
  };

  TimedValueDiagram(
      cairo_t *dstContext,
      TimeStamp fromTime,
      TimeStamp toTime,
      const Settings &s);

  template <typename Iterator>
  void addTimedValues(
      const std::string &label,
      Iterator begin, Iterator end) {
    int n = std::distance(begin, end);
    ArrayBuilder<TimeStamp> dst;
    for (auto x = begin; x != end; x++) {
      dst.add(x->time);
    }
    addTimes(label, dst.get());
  }

  void addTimes(
      const std::string &label,
      const Array<TimeStamp> &times);
private:
  Settings _settings;
  double _y = 0.0;
  cairo_t *_dstContext;
  TimeStamp _fromTime, _toTime;
  MAKE_UNMOVABLE(TimedValueDiagram);
};

}


#endif /* SERVER_HTML_TIMEDVALUEDIAGRAM_H_ */
