/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ArgMap.h"
#include <iostream>
#include <server/common/logging.h>

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

  registerKeyword("--help", 0, "Displays information about available commands.");
  registerHelpInfo("(no help or usage information specified)");
}

ArgMap::~ArgMap() {
  if (hasArg("--help")) {
    dispHelp(&std::cout);
  }
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
  return _args.slice([=](const Entry *e) {return !e->wasRead() && !e->isKeyword(_keywordPrefix);});
}

void ArgMap::registerKeyword(std::string keyword, int maxArgs, std::string helpString) {
  // We cannot register the same keyword twice.
  CHECK(_keywords.find(keyword) == _keywords.end());

  _keywords[keyword] = KeywordInfo(keyword, maxArgs, helpString);
}

Array<ArgMap::Entry*> ArgMap::KeywordInfo::trim(Array<Entry*> args, const std::string &kwPref) const {
  int len = std::min(args.size(), _maxArgs);
  for (int i = 0; i < len; i++) {
    if (args[i]->isKeyword(kwPref)) {
      return i;
    }
  }
  return len;
}

void ArgMap::KeywordInfo::dispHelp(std::ostream *out) const {
  *out << "   " << _keyword << "  (expects ";
  if (_maxArgs == 0) {
    *out << "no arguments)";
  } else {
    *out << "at most " << _maxArgs << " argument";
    if (_maxArgs > 1) {
      *out << "s";
    }
  }
  *out << "):\n      " << _helpString << "\n" << std::endl;
}

void ArgMap::dispHelp(std::ostream *out) {
  *out << _helpInfo << "\n" << std::endl;
  *out << "Available commands:\n";
  typedef std::map<std::string, KeywordInfo>::iterator I;
  for (I i = _keywords.begin(); i != _keywords.end(); i++) {
    i->second.dispHelp(out);
  }
}

}
