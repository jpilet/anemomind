/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ArgMap.h"
#include <iostream>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>
#include <sstream>

namespace sail {

bool ArgMap::instantiated = false;

ArgMap::ArgMap() {
  _successfullyParsed = false;
  assert(!instantiated);
  instantiated = true;
  _optionPrefix = "--";
  registerOption("--help", "Displays information about available commands.").minArgs(0).maxArgs(0);
  setHelpInfo("(no help or usage information specified)");
}

namespace {
  void fillArgs(int argc, const char **argv,
      Array<ArgMap::Entry> *storageOut, Array<ArgMap::Entry*> *argsOut) {
    *storageOut = Array<ArgMap::Entry>::fill(argc-1, [&](int i) {
      return ArgMap::Entry(i, argv[i+1]);
    });

    *argsOut = Array<ArgMap::Entry*>::fill(storageOut->size(), [&](int i) {
      return &((*storageOut)[i]);
    });
  }
}

bool ArgMap::parse(int argc0, const char **argv0) {
  CHECK(!_successfullyParsed);
  int argc = argc0 - 1;

  Array<Entry*> args;
  fillArgs(argc0, argv0, &_argStorage, &args);

  for (int i = 0; i < argc; i++) {
    std::string value = args[i]->valueUntraced();
    Entry *ptr = _argStorage.ptr(i);
    if (ptr->isOption(_optionPrefix)) {
      CHECK(hasRegisteredOption(value));
      _map[value] = _options[value].trim(args.sliceFrom(i), _optionPrefix);
    }
  }
  _successfullyParsed = true;

  if (hasOption("--help")) {
    dispHelp(&std::cout);
  }
  return _successfullyParsed;
}


bool ArgMap::hasRegisteredOption(const std::string &arg) {
  return _options.find(arg) != _options.end();
}

bool ArgMap::hasOption(const std::string &arg) {
  CHECK(_successfullyParsed);
  bool retval = !(_map.find(arg) == _map.end());
  if (retval) {
    _map[arg][0]->setWasRead();
  }
  CHECK(hasRegisteredOption(arg)) << stringFormat("hasOption called with unregistered option: %s", arg.c_str());
  return retval;
}

Array<ArgMap::Entry*> ArgMap::argsAfterOption(const std::string &arg) {
  CHECK(_successfullyParsed);
  assert(hasOption(arg));
  return _map[arg];
}

Array<ArgMap::Entry*> ArgMap::freeArgs() {
  CHECK(_successfullyParsed);
  int count = _argStorage.size();
  ArrayBuilder<ArgMap::Entry*> args(count);
  for (int i = 0; i < count; i++) {
    Entry *e = &(_argStorage[i]);
    if (!e->wasRead() && !e->isOption(_optionPrefix)) {
      args.add(e);
    }
  }
  return args.get();
}

ArgMap::Option &ArgMap::registerOption(std::string option, std::string helpString) {
  // Registering options must be done before any parsing.
  CHECK(!_successfullyParsed);

  // We cannot register the same option twice.
  CHECK(_options.find(option) == _options.end());

  Option &at = _options[option];
  at = Option(option, helpString);
  return at;
}

Array<ArgMap::Entry*> ArgMap::Option::trim(Array<Entry*> optionAndArgs, const std::string &kwPref) const {
  Array<Entry*> args = optionAndArgs.sliceFrom(1);
  int len = std::min(args.size(), _maxArgs);
  {
    for (int i = 0; i < len; i++) {
      if (args[i]->isOption(kwPref)) {
        len = i;
        break;
      }
    }
    if (len < _minArgs) {
      LOG(FATAL) << stringFormat("Less than %d arguments available to the option %s", _minArgs, _option.c_str());
    }
  }

  // Include the option
  return optionAndArgs.sliceTo(len + 1);
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
