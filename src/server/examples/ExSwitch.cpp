/*
 *  Created on: 2014-02-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ExSwitch.h"
#include <assert.h>
#include <iostream>

namespace sail {

void ExEntry::run() {
  assert(_example != nullptr);
  _example();
}

ExSwitch::ExSwitch() : _wasRun(false) {
}

ExSwitch::~ExSwitch() {
  assert(_wasRun);
}

void ExSwitch::add(std::string key, const ExEntry &entry) {
  _last = entry;
  assert(!hasExample(key));
  _map[key] = entry;
}

void ExSwitch::list(std::ostream *out) {
  *out << "\n\n\n=== Possible switches ===" << std::endl;
  *out << "list:\n  Display this list" << std::endl;
  *out << "(none):\n  Run the last example" << std::endl;
  for (std::map<std::string, ExEntry>::iterator i = _map.begin(); i != _map.end(); i++) {
    *out << i->first << ":\n  " << i->second.description() << std::endl;
  }
  *out << "\n\n\n";
}

void ExSwitch::dispList() {
  list(&std::cout);
}

bool ExSwitch::hasExample(std::string key) {
  return _map.find(key) != _map.end();
}

void ExSwitch::run(int argc, char **argv) {
  _wasRun = true;
  if (argc < 2) {
    std::cout << "Running last example." << std::endl;
    _last.run();
    std::cout << "Ran last example because no switch was provided. Here is a list of possible switches." << std::endl;
    dispList();
  } else {
    std::string cmd(argv[1]);
    if (cmd == "list") {
      list(&std::cout);
    } else {
      if (hasExample(cmd)) {
        _map[cmd].run();
      } else {
        std::cout << "No such example: " << cmd << std::endl;
        dispList();
      }
    }
  }
}


} /* namespace sail */
