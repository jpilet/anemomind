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


class FailedToParseArgument : public std::exception {};
template <typename T>
struct ParseArgument {
  static const char *type() {return "unknown type";}

  static T apply(const std::string &input) {
    throw FailedToParseArgument();
    return T();
  }
};

// TODO: Do something nice that will
// work with any integer, not just 'int'
template <>
struct ParseArgument<int> {
  static const char *type() {return "int";}

  static int apply(const std::string &input) {
    return std::stoi(input);
  }
};

template <>
struct ParseArgument<double> {
  static const char *type() {return "double";}

  static double apply(const std::string &input) {
    return std::stod(input);
  }
};

template <>
struct ParseArgument<std::string> {
  static const char *type() {return "string";}

  static std::string apply(const std::string &input) {
    return input;
  }
};

template <>
struct ParseArgument<bool> {
  static bool apply(const std::string &input) {
    if (input == "true" || input == "on" || input == "1"
        || input == "enabled" || input == "enable") {
      return true;
    } else if (input == "false" || input == "off" || input == "0"
        || input == "disabled" || input == "disable") {
      return false;
    }

    return false;
  }
};



class InputForm {
public:

  class Result {
  public:
    Result(bool success) :
      _success(success), _error(success? "" : "Unspecified") {};

    static Result success() {return Result(true);}
  private:
    bool _success;
    std::string _error;
  };
  InputForm(const std::function<Result(Array<std::string>)>
    &handler) {}
  typedef std::shared_ptr<InputForm> Ptr;
  int argCount() const;
};

template <typename T>
class Arg {
public:
  typedef Arg<T> ThisType;
  typedef T type;

  Arg(const std::string &name) : _name(name) {}
  T get() const {return _value;}
  ThisType &describe(const std::string &d);
  bool canParse(const std::string &s) const;
  T parse(const std::string &s) const;

  T parseAndProceed(std::string *s) const {return T();}
private:
  std::string _name, _desc;
  T _value;
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
    std::string *s = args.ptr();
    try {
      return f(arg.parseAndProceed(s)...);
    } catch (const std::exception &e) {
      return InputForm::Result(false);
    }
  });
}


}


#endif /* SERVER_COMMON_CMDARG_H_ */
