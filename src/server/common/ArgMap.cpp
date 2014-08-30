/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ArgMap.h"

namespace sail {

bool ArgMap::instantiated = false;

ArgMap::ArgMap(int argc0, const char **argv0) {
  assert(!instantiated);
  instantiated = true;
  _keywordPrefix = "--";


  int argc = argc0 - 1;
  _argStorage = Array<Entry>(argc);
  _args = Array<Entry*>(argc);
  for (int i0 = 1; i0 < argc0; i0++) {
    std::string value(argv0[i0]);
    int i = i0 - 1;
    Entry e(i, value, _args);
    _argStorage[i] = e;
    Entry *ptr = _argStorage.ptr(i);
    _args[i] = ptr;
    _map[value] = ptr;
  }
}

ArgMap::~ArgMap() {
}

bool ArgMap::hasArg(const std::string &arg) {
  bool retval = !(_map.find(arg) == _map.end());
  if (retval) {
    _map[arg]->setWasRead();
  }
  return retval;
}

Array<ArgMap::Entry*> ArgMap::argsAfter(const std::string &arg) {
  assert(hasArg(arg));
  return _map[arg]->after();
}

Array<ArgMap::Entry*> ArgMap::unreadArgs() const {
  return _args.slice([=](const Entry *e) {return !e->wasRead();});
}

}
