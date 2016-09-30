/*
 * Channels.cpp
 *
 *  Created on: 29 Sep 2016
 *      Author: jonas
 */

#include <device/anemobox/Channels.h>

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

std::vector<DataCode> getAllDataCodes() {
  return std::vector<DataCode>{
#define DATA_CODE_LINE(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  HANDLE,
FOREACH_CHANNEL(DATA_CODE_LINE)
#undef DATA_CODE_LINE
  };
}

}
