/*
 * TimedValueDiagram.cpp
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#include "TimedValueDiagram.h"

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

}


}


