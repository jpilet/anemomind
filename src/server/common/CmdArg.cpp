/*
 * CmdArg.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/common/CmdArg.h>
#include <server/common/logging.h>
#include <iostream>

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

  static char apply(
      const std::string &input) {
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

template <typename T>
T Arg<T>::parseAndProceed(std::string **s) const {
  auto v = **s;
  (*s)++;
  return ParseArgument<T>::apply(v);
}

template <typename T>
std::string Arg<T>::description() const {
  return _name + " [" + ParseArgument<T>::type() + "]: " + _desc;
}

template class Arg<int>;
template class Arg<double>;
template class Arg<std::string>;
template class Arg<bool>;
template class Arg<char>;


InputForm::Result InputForm::parse(
    const Array<std::string> &remainingArgs) const {
  return _handler(remainingArgs);
}

InputForm &InputForm::describe(const std::string &d) {
  _desc = d;
  return *this;
}

Entry::Entry(
    const Array<std::string> &commands,
    const Array<InputForm> &forms) :
        _commands(commands),
        _forms(forms) {}

Entry &Entry::describe(const std::string &d) {
  _description = d;
  return *this;
}

bool Entry::parse(
    std::vector<InputForm::Result> *failureReasons,
    Array<std::string> *remainingArgsInOut) {
  for (auto f: _forms) {
    auto result = f.parse(*remainingArgsInOut);
    if (result.succeeded()) {
      *remainingArgsInOut = remainingArgsInOut->sliceFrom(
          f.argCount());
      return true;
    } else {
      // TODO: Append metadata (such as which form failed to the result)
      failureReasons->push_back(result);
    }
  }
  return false;
}

const Array<std::string> &Entry::commands() const {
  return _commands;
}

Entry &CmdArg::bind(
    const Array<std::string> &commands,
    const Array<InputForm> &inputForms) {
  CHECK(!_initialized);
}

Entry *CmdArg::addEntry(const Entry &e0) {
  _entries.push_back(e0);
  return &(_entries.back());
}

void CmdArg::addAndRegisterEntry(const Entry &e0) {
  auto e = addEntry(e0);
  for (auto cmd: e0.commands()) {

  }
}

void CmdArg::displayHelpMessage() const {
  std::cout << _description << "\n\n";
}

void CmdArg::initialize() {
  CHECK(!_initialized);
  auto frm = inputForm([this]() {
              displayHelpMessage();
              return true;
            });
  auto hcmds = Array<std::string>{"-h", "--help"};
  auto e = addEntry(Entry(
      hcmds, {
          frm
      }));
  for (auto h: hcmds) {
    _map[h] = e;
  }
}

CmdArg::Status CmdArg::parse(int argc, const char **argv) {
  CHECK(!_initialized);
  initialize();
  CHECK(_initialized);

}


CmdArg::CmdArg(const std::string &desc) :
    _description(desc),
    _initialized(false) {
  // Add place holders
  _map["-h"] = nullptr;
  _map["--help"] = nullptr;
}







}
