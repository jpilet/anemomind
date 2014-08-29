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


  _args = Array<Entry>(argc0-1);
  for (int i = 1; i < argc0; i++) {
    std::string value(argv0[i]);
    int index = i - 1;
    Entry e(index, value, _args);
    _args[index] = e;
    _map[value] = _args.ptr(index);
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
    _map[arg]->setWasRead();
  }
  return retval;
}

Array<ArgMap::Entry> ArgMap::argsAfter(const std::string &arg) {
  assert(hasArg(arg));
  return _map[arg]->after();
}

Array<ArgMap::Entry> ArgMap::unreadArgs() const {
  return _args.slice([=](const Entry &e) {return !e.wasRead();});
}

}
