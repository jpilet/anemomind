/*
 * CmdArg.h
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_CMDARG_H_
#define SERVER_COMMON_CMDARG_H_

#include <server/common/Array.h>
#include <string>
#include <map>
#include <server/common/Unmovable.h>

namespace sail {

// Result of applying the handler to this input form
class Result {
public:
  Result(bool success, const std::string &e = "Unspecified") :
    _success(success), _error(e) {};
  static Result success() {return Result(true);}
  static Result failure(const std::string &e) {
    return Result(false, e);
  }

  bool succeeded() const {return _success;}
  bool failed() const {return !_success;}
  operator bool() {return _success;}
  std::string toString() const {return _error;}
private:
  bool _success;
  std::string _error;
};

class InputForm {
public:
  InputForm() {}

  typedef std::function<Result(Array<std::string>)> Handler;
  InputForm(const Handler &handler) : _handler(handler) {}
  Result parse(const Array<std::string> &remainingArgs) const;
  InputForm &describe(const std::string &d);
  int argCount () const {return _argSpecs.size();}
private:
  Array<std::string> _argSpecs;
  Handler _handler;
  std::string _desc;
};

template <typename T>
class Arg {
public:
  typedef Arg<T> ThisType;
  typedef T type;

  Arg(const std::string &name) : _name(name) {}
  ThisType &describe(const std::string &d);

  T parseAndProceed(std::string **s) const;

  std::string description() const;
private:
  std::string _name, _desc;
};


// Rename to InputFormGroup
class Entry {
public:
  Entry(
      const Array<std::string> &commands,
      const Array<InputForm> &forms);
  Entry &describe(const std::string &d);
  Entry &setMinCount(int i);
  Entry &setMaxCount(int i);
  Entry &required();

  bool parse(
      std::vector<Result> *failureReasons,
      Array<std::string> *remaingArgsInOut);

  const Array<std::string> &commands() const;

  typedef std::shared_ptr<Entry> Ptr;
private:
  int _callCount;
  int _minCount;
  int _maxCount;
  std::string _description;
  Array<std::string> _commands;
  Array<InputForm> _forms;
};

class Parser {
public:
  Parser(const std::string &desc);

  enum Status {
    Done,
    Continue,
    Error
  };

  Entry &bind(
      const Array<std::string> &commands,
      const Array<InputForm> &inputForms);

  Status parse(int argc, const char **argv);
private:
  MAKE_UNMOVABLE(Parser);
  std::vector<std::string> _freeArgs;
  bool _initialized, _helpDisplayed;
  std::string _description;
  std::vector<Entry::Ptr> _entries;
  std::map<std::string, Entry::Ptr> _map;

  Entry::Ptr addEntry(const Entry::Ptr &e);
  Entry &addAndRegisterEntry(const Entry::Ptr &e);
  void displayHelpMessage() const;
  void initialize();
};


template <typename Function, typename... Arg>
InputForm inputForm(
    Function f, Arg ... arg) {
  return InputForm(
      [=](const Array<std::string> &args) -> Result {
    if (args.size() < sizeof...(Arg)) {
      return Result::failure("Too few arguments provided");
    }
    std::string *s0 = args.ptr();
    auto s = &s0;
    try {
      return Result(f(arg.parseAndProceed(s)...));
    } catch (const std::exception &e) {
      return Result::failure(e.what());
    }
  });
}


}


#endif /* SERVER_COMMON_CMDARG_H_ */
