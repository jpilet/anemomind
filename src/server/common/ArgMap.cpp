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

ArgMap::ArgMap() {
  _successfullyParsed = false;
  _optionPrefix = "--";
  registerOption("--help", "Displays information about available commands.").setMinArgCount(0).setMaxArgCount(0);
  setHelpInfo("(no help or usage information specified)");
}

namespace {
  void fillArgs(int argc, const char **argv,
      Array<ArgMap::Arg> *storageOut, Array<ArgMap::Arg*> *argsOut) {
    *storageOut = Array<ArgMap::Arg>::fill(argc-1, [&](int i) {
      return ArgMap::Arg(i, argv[i+1]);
    });

    *argsOut = Array<ArgMap::Arg*>::fill(storageOut->size(), [&](int i) {
      return &((*storageOut)[i]);
    });
  }
}


bool ArgMap::readOptionAndParseSub(TempArgMap &tempmap, Option info, Arg *opt, Array<Arg*> rest,
    ArrayBuilder<ArgMap::Arg*> &acc) {
  if (acc.size() >= info.maxArgCount()) { // Max number of arguments reached?
    return parseSub(tempmap, rest); // Continue parsing...
  } else if (rest.empty() || rest[0]->isOption(_optionPrefix)) { // nothing more to read

    // Check if there are not enough arguments for this option.
    if (acc.size() < info.minArgCount()) {
      std::cout << "Too few values provided to the " << opt->valueUntraced() << "-option." << std::endl;
      std::cout << "You provided " << acc.size() << " values, but "
          << info.minArgCount() << " required." << std::endl;
      return false;
    }

    return parseSub(tempmap, rest);
  } else { // Still data to read and max number of arguments not reached.
    acc.add(rest[0]);
    return readOptionAndParseSub(tempmap, info, opt, rest.sliceFrom(1), acc);
  }
}


bool ArgMap::parseSub(TempArgMap &tempmap, Array<Arg*> args) {
  if (args.empty()) {
    return true;
  } else {
    Arg *first = args[0];
    Array<Arg*> rest = args.sliceFrom(1);
    if (first->isOption(_optionPrefix)) {
      const std::string &s = first->valueUntraced();
      Option info = _options[s];
      TempArgs targs = tempmap[s];
      ArrayBuilder<Arg*> &acc = targs.getArgsForNewOption();
      if (info.unique() && targs.optionCount() > 1) {
        std::cout << "You can provide the " << s << " option at most once." << std::endl;
        return false;
      }
      return readOptionAndParseSub(tempmap, info, first, rest, acc);
    } else {
      return parseSub(tempmap, rest);
    }
  }
}


std::map<std::string, Array<ArgMap::Arg*> > ArgMap::buildMap(TempArgMap &src) {
  std::map<std::string, Array<ArgMap::Arg*> > dst;
  for (TempArgMap::iterator
      i = src.begin(); i != src.end(); i++) {
    dst[i->first] = i->second.get();
  }
  return dst;
}


bool ArgMap::hasAllRequiredArgs(std::map<std::string, Option> &options, TempArgMap &tempmap) {
  for (std::map<std::string, Option>::iterator i = options.begin();
      i != options.end(); i++) {
    if (i->second.required()) {
      if (tempmap.find(i->first) == tempmap.end()) {
        std::cout << "Missing required option " << i->first << std::endl;
        return false;
      }
    }
  }
  return true;
}

bool ArgMap::parse(int argc0, const char **argv0) {
  CHECK(!_successfullyParsed);
  int argc = argc0 - 1;
  Array<Arg*> args;
  fillArgs(argc0, argv0, &_argStorage, &args);
  TempArgMap tempmap;
  if (!parseSub(tempmap, args)) {
    return false;
  }

  if (!hasAllRequiredArgs(_options, tempmap)) {
    return false;
  }

  _successfullyParsed = true;
  _map = buildMap(tempmap);
  return true;
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

Array<ArgMap::Arg*> ArgMap::argsAfterOption(const std::string &arg) {
  CHECK(_successfullyParsed);
  assert(hasOption(arg));
  return _map[arg];
}

Array<ArgMap::Arg*> ArgMap::freeArgs() {
  CHECK(_successfullyParsed);
  int count = _argStorage.size();
  ArrayBuilder<ArgMap::Arg*> args(count);
  for (int i = 0; i < count; i++) {
    Arg *e = &(_argStorage[i]);
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

  _options[option] = Option(option, helpString);
  return _options[option];
}

Array<ArgMap::Arg*> ArgMap::Option::trim(Array<Arg*> optionAndArgs, const std::string &kwPref) const {
  Array<Arg*> args = optionAndArgs.sliceFrom(1);
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
