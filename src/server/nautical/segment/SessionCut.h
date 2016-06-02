/*
 * SessionCut.h
 *
 *  Created on: May 20, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_SESSIONCUT_H_
#define SERVER_NAUTICAL_SESSIONCUT_H_

#include <server/common/TimeStamp.h>
#include <server/common/Span.h>
#include <server/common/Array.h>
#include <server/common/AbstractArray.h>

namespace sail {
namespace SessionCut {

/*
 * Explanation:
 *
 * The goal of this code is to split all the recorded data into sessions that
 * the user can understand and appreciate.
 *
 *
 */

struct Settings {
  Settings();

  // If there were no regularization, then
  // this threshold determines how the timestamps
  // should be grouped into sessions...
  Duration<double> cuttingThreshold;

  // ...but since the data is not perfect, we can have holes that
  // are larger than the above threshold but should still not result into
  // a split between two sessions, or maybe the device was turned on for just
  // a few seconds during the winter season to test it. By adding some
  // regularization we can account for this imperfection of the data,
  // either by ignoring short bursts of samples or accepting holes.
  double regularization;
};

Array<Span<TimeStamp> > cutSessions(
    const AbstractArray<sail::TimeStamp> &timeStamps,
    const Settings &settings);

}
}


#endif /* SERVER_NAUTICAL_SESSIONCUT_H_ */
