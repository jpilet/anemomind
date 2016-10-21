/*
 * CmdArg.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/common/logging.h>
#include <iostream>
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


Result InputForm::parse(
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
        _forms(forms),
        _callCount(0) {}

Entry &Entry::describe(const std::string &d) {
  _description = d;
  return *this;
}

bool Entry::parse(
    std::vector<Result> *failureReasons,
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

Entry &Parser::bind(
    const Array<std::string> &commands,
    const Array<InputForm> &inputForms) {
  CHECK(!_initialized);
  return addAndRegisterEntry(std::make_shared<Entry>(
      commands, inputForms));
}

Entry::Ptr Parser::addEntry(const Entry::Ptr &e0) {
  _entries.push_back(e0);
  return e0;
}

Entry &Parser::addAndRegisterEntry(const Entry::Ptr &e0) {
  auto e = addEntry(e0);
  for (auto cmd: e0->commands()) {
    CHECK(_map.count(cmd) == 0);
    _map[cmd] = e;
  }
  return *e;
}

void Parser::displayHelpMessage() const {
  std::cout << _description << "\n\n";
}

void Parser::initialize() {
  CHECK(!_initialized);
  auto frm = inputForm([this]() {
              displayHelpMessage();
              return true;
            });
  auto hcmds = Array<std::string>{"-h", "--help"};
  Array<InputForm> frms{
            frm
        };
  auto e = addEntry(std::make_shared<Entry>(
      hcmds, frms));
  e->describe("Display help message");
  for (auto h: hcmds) {
    _map[h] = e;
  }
  _initialized = true;
}

namespace {
  Array<std::string> wrapArgs(int argc, const char **argv) {
    Array<std::string> dst(argc-1);
    for (int i = 1; i < argc; i++) {
      dst[i-1] = argv[i];
    }
    return dst;
  }
}


Parser::Status Parser::parse(int argc, const char **argv) {
  CHECK(!_initialized);
  initialize();
  CHECK(_initialized);
  auto args = wrapArgs(argc, argv);
  while (!args.empty()) {
    if (_helpDisplayed) {
      return Parser::Status::Done;
    }
    auto first = args[0];
    std::cout << "Parsing " << first << std::endl;
    args = args.sliceFrom(1);
    auto f = _map.find(first);
    if (f == _map.end()) {
      std::cout << "Push back " << first << std::endl;
      _freeArgs.push_back(first);
    } else {
      std::vector<Result> reasons;
      if (!f->second->parse(&reasons, &args)) {
        std::cout << "Failed to parse command "
            << first << " because\n";
        for (auto r: reasons) {
          std::cout << "  * " << r.toString() << std::endl;
        }
        return Parser::Status::Error;
      }
    }
  }
  return Parser::Status::Continue;
}


Parser::Parser(const std::string &desc) :
    _description(desc),
    _initialized(false),
    _helpDisplayed(false) {
  // Add place holders
  _map["-h"] = nullptr;
  _map["--help"] = nullptr;
}







}
