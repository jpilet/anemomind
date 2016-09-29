/*
 * Channels.cpp
 *
 *  Created on: 29 Sep 2016
 *      Author: jonas
 */

#include <anemobox/Channels.h>

namespace sail {

const char* descriptionForCode(DataCode code) {
  switch (code) {
#define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    case HANDLE : return DESCRIPTION;

  FOREACH_CHANNEL(CASE_ENTRY)
#undef CASE_ENTRY
  }
  return nullptr;
}

const char* wordIdentifierForCode(DataCode code) {
  switch (code) {
#define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    case HANDLE : return SHORTNAME;

  FOREACH_CHANNEL(CASE_ENTRY)
#undef CASE_ENTRY
  }
  return nullptr;
}

}
