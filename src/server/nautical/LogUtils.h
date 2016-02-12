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
  std::string localPath;
  std::string resolvedPath;
};
std::ostream &operator<<(std::ostream &s, const LoadStatus &x);

LoadStatus loadAll(const std::string &path, Dispatcher *dst);

}
}

#endif /* SERVER_NAUTICAL_LOGLOADERUTILS_H_ */
