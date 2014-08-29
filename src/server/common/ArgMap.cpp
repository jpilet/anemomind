/*
 *  Created on: 2014-
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
  const char **argv = argv0 + 1;

  _args = Array<Entry>(argc);
  for (int i = 0; i < argc; i++) {
    std::string value = argv[i];
    Entry e(i, value, _args);
    _args[i] = e;
    _map[value] = e;
  }
}

ArgMap::~ArgMap() {
  for (auto &arg: _args) {
    arg.uncycle(); //Break cyclic references before destruction.
  }
}

bool ArgMap::hasArg(const std::string &arg) {
  bool retval = !(_map.find(arg) == _map.end());
  if (retval) {
    _map[arg].setWasRead();
  }
  return retval;
}

Array<ArgMap::Entry> ArgMap::argsAfter(const std::string &arg) {
  assert(hasArg(arg));
  return _map[arg].after();
}

Array<ArgMap::Entry> ArgMap::unreadArgs() const {
  return _args.slice([=](const Entry &e) {return !e.wasRead();});
}

}
