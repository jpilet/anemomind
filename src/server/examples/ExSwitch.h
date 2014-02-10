/*
 *  Created on: 2014-02-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef EXSWITCH_H_
#define EXSWITCH_H_

#include <map>
#include <string>
#include <iosfwd>

namespace sail {

class ExEntry {
  public:

  typedef void (*ExPtr)();

  ExEntry() : _example(nullptr) {}
  ExEntry(ExPtr ex, std::string desc) : _example(ex), _description(desc) {}

  void run();
  std::string description() const {return _description;}
  private:
    ExPtr _example;
    std::string _description;
};

class ExSwitch {
public:

  ExSwitch();
  virtual ~ExSwitch();

  void add(std::string key, const ExEntry &entry);
  void list(std::ostream *out);
  void dispList();
  void run(int argc, char **argv);
  bool hasExample(std::string key);
private:
  bool _wasRun;
  ExEntry _last;
  std::map<std::string, ExEntry> _map;
};

#define ADDEXAMPLE(exswitch, function, desc) (exswitch).add(#function, ExEntry(&function, (desc)))

} /* namespace sail */

#endif /* EXSWITCH_H_ */
