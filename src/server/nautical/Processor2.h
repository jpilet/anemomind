/*
 * Processor2.h
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_PROCESSOR2_H_
#define SERVER_NAUTICAL_PROCESSOR2_H_

#include <server/nautical/segment/SessionCut.h>
#include <device/anemobox/Dispatcher.h>

namespace sail {
namespace Processor2 {

struct Settings {
  Settings();

  Duration<double> mainSessionCut;
  Duration<double> subSessionCut;
  Duration<double> minCalibDur;

  SessionCut::Settings sessionCutSettings;
  std::string logRoot;

  std::string makeLogFilename(const std::string &s);
};

// Used for cutting the sessions.
Array<TimeStamp> getAllGpsTimeStamps(const Dispatcher *d);

Array<Span<TimeStamp> > segmentSubSessions(
    const Array<TimeStamp> &times,
    const Settings &settings);

void outputTimeSpansToFile(
    const std::string &filename,
    const Array<Span<TimeStamp> > &timeSpans);

void outputGroupsToFile(
      const std::string &filename,
      const Array<Spani> &groups,
      const Array<Span<TimeStamp> > sessions);

Array<Spani> groupSessionsByThreshold(
    const Array<Span<TimeStamp> > &timeSpans,
    const Duration<double> &threshold);

Array<Spani> computeCalibrationGroups(
    Array<Span<TimeStamp> > timeSpans,
    Duration<double> minCalibDur);

}
}

#endif /* SERVER_NAUTICAL_PROCESSOR2_H_ */
