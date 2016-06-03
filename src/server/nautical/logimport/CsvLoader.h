/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_CSVLOADER_H_
#define SERVER_NAUTICAL_LOGIMPORT_CSVLOADER_H_

#include <string>

namespace sail {

class LogAccumulator;

void loadCsv(const std::string &filename, LogAccumulator *dst);

}

#endif /* SERVER_NAUTICAL_LOGIMPORT_CSVLOADER_H_ */
