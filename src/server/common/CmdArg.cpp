/*
 * CmdArg.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/common/CmdArg.h>

namespace sail {


class FailedToParseArgument : public std::exception {
public:
  FailedToParseArgument(const std::string &r) : _reason(r),
    _what("FailedToParseArgument: " + r) {}
  const char* what() const noexcept {
    return _what.c_str();
  }
private:
  std::string _reason, _what;
};

template <typename T>
struct ParseArgument {
  static const char *type() {return "unknown type";}

  static T apply(const std::string &input) {
    throw FailedToParseArgument("Unknown type");
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
  static const char *type() {return "bool";}

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

template <>
struct ParseArgument<char> {
  static const char *type() {return "char";}

  static char apply(const std::string &input) {
    if (input.size() == 1) {
      return input[0];
    } else {
      throw FailedToParseArgument("'" + input + "' is not a character");
    }
    return '_';
  }
};


template <typename T>
Arg<T> &Arg<T>::describe(const std::string &d) {
  _desc = d;
  return *this;
}

template class Arg<int>;
template class Arg<double>;
template class Arg<std::string>;
template class Arg<bool>;
template class Arg<char>;

CmdArg::CmdArg(const std::string &desc) : _desc(desc) {}

}

