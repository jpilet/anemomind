/*
 * LogLoaderSettings.h
 *
 *  Created on: Jul 14, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_LOGLOADERSETTINGS_H_
#define SERVER_NAUTICAL_LOGIMPORT_LOGLOADERSETTINGS_H_

namespace sail {
  struct LogLoaderSettings {
    bool ignoreNmea0183Checksum = false;

    static LogLoaderSettings relaxed() {
      LogLoaderSettings dst;
      dst.ignoreNmea0183Checksum = true;
      return dst;
    }
  };
}



#endif /* SERVER_NAUTICAL_LOGIMPORT_LOGLOADERSETTINGS_H_ */
