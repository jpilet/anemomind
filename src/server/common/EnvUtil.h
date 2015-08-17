/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_COMMON_ENVUTIL_H_
#define SERVER_COMMON_ENVUTIL_H_

#include <Poco/Path.h>

namespace sail {

Poco::Path getDatasetPath(std::string subFolderName);

}

#endif /* SERVER_COMMON_ENVUTIL_H_ */
