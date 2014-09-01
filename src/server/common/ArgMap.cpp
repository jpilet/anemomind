/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ArgMap.h"
#include <iostream>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <sstream>

namespace sail {

bool ArgMap::instantiated = false;

ArgMap::ArgMap(int argc0, const char **argv0) {
  assert(!instantiated);
  instantiated = true;
  _optionPrefix = "--";

  int argc = argc0 - 1;
  _argStorage = Array<Entry>(argc);
  _args = Array<Entry*>(argc);
  for (int i0 = 1; i0 < argc0; i0++) {
    std::string value(argv0[i0]);
    int i = i0 - 1;
    Entry e(i, value);
    _argStorage[i] = e;
    Entry *ptr = _argStorage.ptr(i);
    _args[i] = ptr;
    if (ptr->isOption(_optionPrefix)) {
      _map[value] = _args.sliceFrom(i);
    }
  }

  registerOption("--help", "Displays information about available commands.").minArgs(0).maxArgs(0);
  setHelpInfo("(no help or usage information specified)");
}

ArgMap::~ArgMap() {
  if (hasOption("--help")) {
    dispHelp(&std::cout);
  }
}

bool ArgMap::hasOption(const std::string &arg) {
  bool retval = !(_map.find(arg) == _map.end());
  if (retval) {
    _map[arg][0]->setWasRead();
  }
  CHECK(_options.find(arg) != _options.end()) << stringFormat("hasOption called with unregistered option: %s", arg.c_str());
  return retval;
}

Array<ArgMap::Entry*> ArgMap::argsAfterOption(const std::string &arg) {
  assert(hasOption(arg));
  return _options[arg].trim(_map[arg].sliceFrom(1), _optionPrefix);
}

Array<ArgMap::Entry*> ArgMap::freeArgs() const {
  return _args.slice([=](const Entry *e) {return !e->wasRead() && !e->isOption(_optionPrefix);});
}

ArgMap::Option &ArgMap::registerOption(std::string option, std::string helpString) {
  // We cannot register the same option twice.
  CHECK(_options.find(option) == _options.end());

  Option &at = _options[option];
  at = Option(option, helpString);
  return at;
}

Array<ArgMap::Entry*> ArgMap::Option::trim(Array<Entry*> args, const std::string &kwPref) const {
  int len = std::min(args.size(), _maxArgs);
  for (int i = 0; i < len; i++) {
    if (args[i]->isOption(kwPref)) {
      len = i;
      break;
    }
  }
  if (len < _minArgs) {
    LOG(FATAL) << stringFormat("Less than %d arguments available to the option %s", _minArgs, _option.c_str());
  }
  return args.sliceTo(len);
}

void ArgMap::Option::dispHelp(std::ostream *out) const {
  *out << "   " << _option << "  (expects ";
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
  typedef std::map<std::string, Option>::iterator I;
  for (I i = _options.begin(); i != _options.end(); i++) {
    i->second.dispHelp(out);
  }
}

std::string ArgMap::helpMessage() {
  std::stringstream ss;
  dispHelp(&ss);
  return ss.str();
}

}
