/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/logimport/SourceGroup.h>
#include <server/nautical/logimport/LogAccumulator.h>

namespace sail {

SourceGroup::SourceGroup() {
#define INIT_NULL(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  HANDLE = nullptr;

  FOREACH_CHANNEL(INIT_NULL)
#undef INIT_NULL
}

SourceGroup::SourceGroup(const std::string &srcName, LogAccumulator *dst) {
#define ALLOC_IF_NEEDED(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  HANDLE = allocateSourceIfNeeded<TYPE >(srcName, dst->get ## HANDLE ## sources());

  FOREACH_CHANNEL(ALLOC_IF_NEEDED)
#undef ALLOC_IF_NEEDED
}

}  // namespace sail
