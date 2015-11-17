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
#include <set>

namespace sail {

std::shared_ptr<ArgMap::Option> findOptionInMap(ArgMap::OptionMap &options, const std::string &key) {
  if (options.find(key) != options.end()) {
    return options[key];
  }
  return std::shared_ptr<ArgMap::Option>();
}


ArgMap::ArgMap() {
  _successfullyParsed = false;
  _optionPrefix = "-";
  _allowFreeArgs = true;

  registerOption("--help", "Displays information about available commands.")
    .setArgCount(0)
    .alias("-h");

  registerOption("--noinfo", "Mute loglevel INFO.")
    .setArgCount(0)
    .callback(
      [=](const Array<Arg*>&) { SetLogLevelThreshold(LOGLEVEL_WARNING); });

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


bool ArgMap::readOptionAndParseSub(TempArgMap &tempmap, std::shared_ptr<Option> info,
    Arg *opt, Array<Arg*> rest,
    ArrayBuilder<ArgMap::Arg*> &acc) {
  int count = acc.size()-1; // ignore first argument.
  if (count >= info->maxArgCount()) { // Max number of arguments reached?
    return parseSub(tempmap, rest); // Continue parsing...
  } else if (rest.empty() || rest[0]->isOption(_optionPrefix)) { // nothing more to read

    // Check if there are not enough arguments for this option.
    if (count < info->minArgCount()) {
      LOG(ERROR) << "Too few values provided to the " << opt->valueUntraced() << " option.";
      LOG(ERROR) << "You provided " << count << " values, but "
          << info->minArgCount() << " required.";
      return false;
    }

    return parseSub(tempmap, rest);
  } else { // Still data to read and max number of arguments not reached.
    auto x = rest[0];
    x->setWasRead();
    acc.add(x);
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
      auto info = findOption(s);
      if (info) {
        TempArgs &targs = tempmap[s];
        ArrayBuilder<Arg*> &acc = targs.getArgsForNewOption(first);
        if (info->unique() && targs.optionCount() > 1) {
          LOG(ERROR) << "You can provide the " << s << " option at most once.";
          return false;
        }
        return readOptionAndParseSub(tempmap, info, first, rest, acc);
      } else {
        LOG(ERROR) << "No such option: " << s;
        return false;
      }
    } else {
      return parseSub(tempmap, rest);
    }
  }
}


std::map<std::string, Array<ArgMap::Arg*> > ArgMap::buildMap(TempArgMap &src) {
  std::map<std::string, Array<ArgMap::Arg*> > dst;
  for (TempArgMap::iterator
      i = src.begin(); i != src.end(); i++) {
    dst[mainKey(i->first)] = i->second.get();
  }
  return dst;
}


bool ArgMap::hasAllRequiredArgs(TempArgMap &tempmap) {
  std::set<std::string> optionsProvided;
  for (auto x: tempmap) {
    auto y = findOptionInMap(_options, x.first);
    if (y) {
      optionsProvided.insert(y->key());
    }
  }

  for (auto pair: _options) {
    auto opt = pair.second;
    if (opt->required()) {
      if (optionsProvided.find(opt->key()) == optionsProvided.end()) {
        LOG(ERROR) << "Missing required option " << opt->key();
        return false;
      }
    }
  }
  return true;
}

std::shared_ptr<ArgMap::Option> ArgMap::findOption(const std::string &key) {
  return findOptionInMap(_options, key);
}

void ArgMap::registerAliases() {
  for (auto opt: _optionSet) {
    for (auto a: opt->aliases()) {
      _options[a] = opt;
    }
  }
}

bool ArgMap::parseSub(int argc0, const char **argv0) {
  registerAliases();
  CHECK(!_successfullyParsed);
  Array<Arg*> args;
  fillArgs(argc0, argv0, &_argStorage, &args);
  TempArgMap tempmap;
  if (!parseSub(tempmap, args)) {
    return false;
  }

  if (!hasAllRequiredArgs(tempmap)) {
    return false;
  }

  _successfullyParsed = true;
  _map = buildMap(tempmap);

  auto fargs = freeArgs();
  if (!_allowFreeArgs && !fargs.empty()) {
    std::stringstream ss;
    ss << "Free arguments not allowed:";
    for (auto arg: fargs) {
      ss << " " << arg->value();
    }
    LOG(ERROR) << ss.str();
    return false;
  }

  for (auto opt: _map) {
    auto callback = _options[opt.first]->callback();
    if (callback) {
      callback(optionArgs(opt.first));
    }
  }
  return true;
}

std::string ArgMap::mainKey(std::string alternativeKey) {
  auto opt = findOption(alternativeKey);
  if (opt) {
    return opt->key();
  }
  return "";
}


ArgMap::ParseStatus ArgMap::parse(int argc, const char **argv) {
  bool success = parseSub(argc, argv);
  if (success) {
    if (helpAsked()) {
      dispHelp(&(std::cout));
      return Done;
    }
    return Continue;
  }
  dispHelp(&(std::cout));
  return Error;
}

bool ArgMap::Arg::tryParseInt(int *dst) {
  return sail::tryParseInt(value(), dst);
}

bool ArgMap::Arg::tryParseDouble(double *dst) {
  return sail::tryParseDouble(value(), dst);
}

int ArgMap::Arg::parseIntOrDie() {
  int dst = -1;
  CHECK(this->tryParseInt(&dst)) << "Failed to convert " << value() << " to int.";
  return dst;
}

bool ArgMap::helpAsked() {
  return optionProvided("--help");
}

double ArgMap::Arg::parseDoubleOrDie() {
  double dst = -1;
  CHECK(this->tryParseDouble(&dst)) << "Failed to convert " << value() << " to double.";
  return dst;
}


bool ArgMap::hasRegisteredOption(const std::string &arg) {
  return _options.find(arg) != _options.end();
}

bool ArgMap::optionProvided(const std::string &arg0) {
  auto arg = mainKey(arg0);
  CHECK(_successfullyParsed);
  bool retval = !(_map.find(arg) == _map.end());
  if (retval) {
    _map[arg][0]->setWasRead();
  }
  CHECK(hasRegisteredOption(arg)) << stringFormat("hasOption called with unregistered option: %s", arg.c_str());
  return retval;
}

Array<ArgMap::Arg*> ArgMap::optionArgs(const std::string &arg) {
  CHECK(_successfullyParsed);
  if (optionProvided(arg)) {
    return _map[mainKey(arg)].sliceFrom(1);
  } else {
    return Array<ArgMap::Arg*>();
  }
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

  auto newOption = std::shared_ptr<Option>(new Option(option, helpString));
  _options[option] = newOption;
  _optionSet.insert(newOption);
  return *(_options[option]);
}

Array<ArgMap::Arg*> ArgMap::Option::trim(Array<Arg*> optionAndArgs, const std::string &kwPref) const {
  Array<Arg*> args = optionAndArgs.sliceFrom(1);
  int len = std::min(args.size(), maxArgCount());
  {
    for (int i = 0; i < len; i++) {
      if (args[i]->isOption(kwPref)) {
        len = i;
        break;
      }
    }
    if (len < _minArgs) {
      LOG(FATAL) << stringFormat("Less than %d arguments available to the option %s", _minArgs, _key.c_str());
    }
  }

  // Include the option
  return optionAndArgs.sliceTo(len + 1);
}

void ArgMap::Option::dispHelp(std::ostream *out) const {
  *out << "   " << _key;
  for (auto alias: _aliases) {
    *out << ", " << alias;
  }
  *out << " (expects ";
  if (_minArgs == 0) {
    if (_maxArgs == 0) {
      *out << "no arguments";
    } else if (hasMaxArgCount()) {
      *out << "at most " << _maxArgs << " argument";
      if (_maxArgs > 1) {
        *out << "s";
      }
    } else {
      *out << "any number of arguments";
    }
  } else if (_minArgs == _maxArgs) {
    *out << "exactly " << _minArgs << " argument";
    if (_minArgs > 1) {
      *out << "s";
    }
  } else if (hasMaxArgCount()) {
    *out << "from " << _minArgs << " to " << _maxArgs << " arguments";
  } else {
    *out << "at least " << _minArgs << " arguments";
  }
  *out << "):\n      " << _helpString << "\n" << std::endl;
}

void ArgMap::dispHelp(std::ostream *out) {
  *out << _helpInfo << "\n" << std::endl;
  *out << "Available commands:\n";
  for (auto opt: _optionSet) {
    opt->dispHelp(out);
  }
}

std::string ArgMap::helpMessage() {
  std::stringstream ss;
  dispHelp(&ss);
  return ss.str();
}


}
