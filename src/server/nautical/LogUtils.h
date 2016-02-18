/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_LOGLOADERUTILS_H_
#define SERVER_NAUTICAL_LOGLOADERUTILS_H_

#include <string>

namespace sail {
class Dispatcher;
namespace LogUtils {

struct LoadStatus {
  int successCount;
  int failureCount;
};
std::ostream &operator<<(std::ostream &s, const LoadStatus &x);

LoadStatus loadAll(const std::string &datasetPath, Dispatcher *dst);

}
}

#endif /* SERVER_NAUTICAL_LOGLOADERUTILS_H_ */
