/*
 * ProtobufLogLoader.h
 *
 *  Created on: May 27, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_PROTOBUFLOGLOADER_H_
#define SERVER_NAUTICAL_LOGIMPORT_PROTOBUFLOGLOADER_H_

#include <device/anemobox/logger/Logger.h>
#include <server/nautical/logimport/LogLoaderSettings.h>

namespace sail {
struct LogAccumulator;
namespace ProtobufLogLoader {

void load(const LogFile &data, LogAccumulator *dst,
    const LogLoaderSettings &settings);
bool load(const std::string &filename, LogAccumulator *dst,
    const LogLoaderSettings &settings);

}
} /* namespace sail */

#endif /* SERVER_NAUTICAL_LOGIMPORT_PROTOBUFLOGLOADER_H_ */
