/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ArgMap.h"
#include <iostream>
#include <server/common/logging.h>
#include <server/common/string.h>

namespace sail {

bool ArgMap::instantiated = false;

ArgMap::ArgMap(int argc0, const char **argv0) {
  assert(!instantiated);
  instantiated = true;
  _keywordPrefix = "--";
  _severity = LOGLEVEL_WARNING;

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
    if (ptr->isKeyword(_keywordPrefix)) {
      _map[value] = ptr;
    }
  }

  registerKeyword("--help", 0, 0, "Displays information about available commands.");
  registerHelpInfo("(no help or usage information specified)");
}

ArgMap::~ArgMap() {
  if (hasKeyword("--help")) {
    dispHelp(&std::cout);
  }
}

bool ArgMap::hasKeyword(const std::string &arg) {
  bool retval = !(_map.find(arg) == _map.end());
  if (retval) {
    _map[arg]->setWasRead();
  }
  if (_keywords.find(arg) == _keywords.end()) {
    internal::LogFinisher() = internal::LogMessage(_severity, __FILE__, __LINE__) << stringFormat("The keyword %s has not been registered", arg.c_str());
  }
  return retval;
}

Array<ArgMap::Entry*> ArgMap::argsAfterKeyword(const std::string &arg) {
  assert(hasKeyword(arg));
  return _keywords[arg].trim(_map[arg]->after(), _keywordPrefix);
}

Array<ArgMap::Entry*> ArgMap::unreadArgs() const {
  return _args.slice([=](const Entry *e) {return !e->wasRead() && !e->isKeyword(_keywordPrefix);});
}

void ArgMap::registerKeyword(std::string keyword, int minArgs, int maxArgs, std::string helpString) {
  // We cannot register the same keyword twice.
  CHECK(_keywords.find(keyword) == _keywords.end());

  _keywords[keyword] = KeywordInfo(keyword, minArgs, maxArgs, helpString);
}

Array<ArgMap::Entry*> ArgMap::KeywordInfo::trim(Array<Entry*> args, const std::string &kwPref) const {
  int len = std::min(args.size(), _maxArgs);
  for (int i = 0; i < len; i++) {
    if (args[i]->isKeyword(kwPref)) {
      len = i;
      break;
    }
  }
  if (len < _minArgs) {
    LOG(FATAL) << stringFormat("Less than %d arguments available to the keyword %s", _minArgs, _keyword.c_str());
  }
  return args.sliceTo(len);
}

void ArgMap::KeywordInfo::dispHelp(std::ostream *out) const {
  *out << "   " << _keyword << "  (expects ";
  if (_minArgs == 0) {
    if (_maxArgs == 0) {
      *out << "no arguments";
    } else {
      *out << "at most " << _maxArgs << " argument";
      if (_maxArgs > 1) {
        *out << "s";
      }
    }
  } else if (_minArgs == _maxArgs) {
    *out << "exactly " << _minArgs << " argument";
    if (_minArgs > 1) {
      *out << "s";
    }
  } else {
    *out << "from " << _minArgs << " to " << _maxArgs << " arguments";
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
