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

namespace sail {

class InputForm {
public:

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
  private:
    bool _success;
    std::string _error;
  };

  typedef std::function<Result(Array<std::string>)> Handler;
  InputForm(const Handler &handler) : _handler(handler) {}
  typedef std::shared_ptr<InputForm> Ptr;
  int argCount() const;

  Result parse(const Array<std::string> &remainingArgs) const;
private:
  Array<std::string> _argSpecs;
  Handler _handler;
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


class CmdArg {
public:
  CmdArg(const std::string &desc);

  struct Entry {
    std::string description;
    Array<std::string> commands;
    Array<InputForm::Ptr> forms;

    std::shared_ptr<Entry> Ptr;

    // "Builder"-style method
    Entry &describe(const std::string &d) {
      description = d;
      return *this;
    }
  };

  Entry &bind(
      const Array<std::string> &commands,
      const Array<InputForm::Ptr> &inputForms);
private:
  std::string _desc;
  std::vector<Entry> _entries;
  std::map<std::string, Entry*> _map;
};


template <typename Function, typename... Arg>
InputForm::Ptr inputForm(
    Function f, Arg ... arg) {
  return std::make_shared<InputForm>(
      [=](const Array<std::string> &args) -> InputForm::Result {
    if (args.size() < sizeof...(Arg)) {
      return InputForm::Result::failure("Too few arguments provided");
    }
    std::string *s0 = args.ptr();
    auto s = &s0;
    try {
      return InputForm::Result(f(arg.parseAndProceed(s)...));
    } catch (const std::exception &e) {
      return InputForm::Result(false);
    }
  });
}


}


#endif /* SERVER_COMMON_CMDARG_H_ */
