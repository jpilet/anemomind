/*
 *  Created on: Jul 1, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "VersionedEnum.h"
#include <Poco/Checksum.h>

namespace sail {

unsigned int calcChecksum(const char *data) {
  Poco::Checksum cs;
  cs.update(std::string(data));
  return cs.checksum();
}

} /* namespace mmm */
