/*
 * TimedValueDiagram.h
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_HTML_TIMEDVALUEDIAGRAM_H_
#define SERVER_HTML_TIMEDVALUEDIAGRAM_H_

#include <server/common/TimeStamp.h>
#include <server/common/Unmovable.h>

namespace sail {

class TimedValueDiagram {
public:
  struct Settings {
    Duration<double> margin = 1.0_m;
    double width;
  };

  TimedValueDiagram(
      const std::string &outputFilename,
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

  ~TimedValueDiagram();
private:
  struct TimeBarPlot {
    PlotUtils::RGB color;
    Array<Span<TimeStamp>> timeSpans;
  };

  MAKE_UNMOVABLE(TimedValueDiagram);
  std::vector<TimeBarPlot> _toPlot;
};

}


#endif /* SERVER_HTML_TIMEDVALUEDIAGRAM_H_ */
