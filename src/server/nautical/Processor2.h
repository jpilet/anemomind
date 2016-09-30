/*
 * Processor2.h
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_PROCESSOR2_H_
#define SERVER_NAUTICAL_PROCESSOR2_H_

#include <server/common/ArrayBuilder.h>
#include <server/common/Span.h>
#include <server/nautical/segment/SessionCut.h>
#include <device/anemobox/Dispatcher.h>
#include <server/common/HtmlLog.h>

namespace sail {

class NavDataset;
namespace Processor2 {


struct Settings {
  Settings();

  bool debug;

  Duration<double> mainSessionCut;
  Duration<double> subSessionCut;
  Duration<double> minCalibDur;
  Duration<double> calibWindowSize;

  SessionCut::Settings sessionCutSettings;
  std::string logRoot;
};

// Used for cutting the sessions.
Array<TimeStamp> getAllGpsTimeStamps(const Dispatcher *d);
Array<TimeStamp> getAllTimeStampsFiltered(
    std::function<bool(DataCode)> pred,
    const Dispatcher *d);

template <typename TimedValueIterator>
Array<TimeStamp> getTimeStamps(
    TimedValueIterator begin,
    TimedValueIterator end) {
  ArrayBuilder<TimeStamp> timeStamps;
  for (auto at = begin; at != end; at++) {
    timeStamps.add(at->time);
  }
  return timeStamps.get();
}

Array<Span<TimeStamp> > segmentSubSessions(
    const Array<TimeStamp> &times,
    Duration<double> threshold);

Array<TimedValue<GeographicPosition<double> > >
  filterAllGpsData(const NavDataset &ds, const Settings &settings);

void outputTimeSpans(
    const Array<Span<TimeStamp> > &timeSpans,
    HtmlNode::Ptr dst);

void outputGroups(
      const Array<Spani> &groups,
      const Array<Span<TimeStamp> > sessions,
      HtmlNode::Ptr dst);

Array<Spani> groupSessionsByThreshold(
    const Array<Span<TimeStamp> > &timeSpans,
    const Duration<double> &threshold);

Array<Spani> computeCalibrationGroups(
    Array<Span<TimeStamp> > timeSpans,
    Duration<double> minCalibDur);

void runDemoOnDataset(
    NavDataset &dataset,
    HtmlNode::Ptr logNode);

}
}

#endif /* SERVER_NAUTICAL_PROCESSOR2_H_ */
