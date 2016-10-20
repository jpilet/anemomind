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



class AbstractInputForm {
public:
  typedef std::shared_ptr<AbstractInputForm> Ptr;
  virtual int argCount() const = 0;
  virtual ~AbstractInputForm() {}
};

template <typename... T>
class InputForm : public AbstractInputForm {
public:
  InputForm(
      const Array<std::string> &argNames,
      const std::function<bool(T...)> &handler) :
        _argNames(argNames), _handler(handler) {}
private:
  Array<std::string> _argNames;
  std::function<bool(T...)> _handler;
};


template <typename... T>
AbstractInputForm::Ptr inputForm(
    const Array<std::string> &argNames,
        std::function<bool(T...)> f) {
  return AbstractInputForm::Ptr(
      new InputForm<T...>(argNames,
          [=](T ... args) -> bool {
            return f(args...);
          }));
}

template <typename... T>
AbstractInputForm::Ptr inputForm(
    const Array<std::string> &argNames,
        std::function<void(T...)> f) {
  return AbstractInputForm::Ptr(
      new InputForm<T...>(argNames,
          [=](T ... args) -> bool {
            f(args...);
            return true;
          }));
}

class CmdArg {
public:
  CmdArg();

  struct Entry {
    std::string description;
    Array<std::string> commands;
    Array<AbstractInputForm::Ptr> forms;

    std::shared_ptr<Entry> Ptr;

    // "Builder"-style method
    Entry &describe(const std::string &d) {
      description = d;
      return *this;
    }
  };

  Entry &bind(
      const Array<std::string> &commands,
      const Array<AbstractInputForm::Ptr> &inputForms);
private:
  std::vector<Entry> _entries;
};


}


#endif /* SERVER_COMMON_CMDARG_H_ */
