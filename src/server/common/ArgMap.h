/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ARGMAP_H_
#define ARGMAP_H_

#include <map>
#include <string>
#include <server/common/Array.h>
#include <memory>
#include <ostream>
#include <server/common/logging.h>
#include <limits>
#include <server/common/ArrayBuilder.h>
#include <set>

namespace sail {

class ArgMap {
 public:
  ArgMap();


  enum ParseStatus {
    Error = 0,    // parsing error or something.
    Continue, // no error, continue with the rest of the program
    Done // If the user displayed help, this is not an error, but the program should be done.
  };

  ParseStatus parse(int argc, const char **argv);

  // TODO: Maybe ArgMap could provide some predefined
  // exit codes, such as: enum ExitCodes {OK = 0 /*when no error occurred*/, Failure = -1 /*Something went wrong*/};

  class Arg {
   public:
    Arg() : _wasRead(false), _index(-1) {}
    Arg(int index, std::string arg) : _wasRead(false), _arg(arg), _index(index) {}

    const std::string &value() {
      _wasRead = true;
      return valueUntraced();
    }

    bool tryParseInt(int *dst);
    bool tryParseDouble(double *dst);
    int parseIntOrDie();
    double parseDoubleOrDie();

    // Reads the value without leaving any trace, that would be settings _wasRead = true.
    const std::string &valueUntraced() const {
      return _arg;
    }

    void setWasRead() {
      _wasRead = true;
    }

    // This function DOESN'T change the _wasRead flag.
    bool isOption(const std::string &prefix) const {
      const int vlen = _arg.length();
      const int plen = prefix.length();
      return (vlen >= plen? _arg.substr(0, plen) == prefix : false);
    }

    bool wasRead() const {
      return _wasRead;
    }

   private:
    bool _wasRead;
    std::string _arg;
    int _index;
  };



  bool helpAsked();

  /*
   * To check if a option exists, e.g. '--descend' if we were to call a sorting algorithm.
   */
  bool optionProvided(const std::string &arg);

  /*
   * For instance, use this function to retrieve the filename succeeding a option, e.g.
   * '--outfile /home/alan/nmea.txt'
   */
  Array<Arg*> optionArgs(const std::string &arg);

  /*
   * When all functions that need to read arguments from the command line have been called,
   * use this method to suck up the remaining, unread, arguments.
   *
   * For instance, if we pass the args
   *
   *   --slice 10 50 rulle.txt
   *
   * and one function already read '--slice 10 50',
   * then the argument 'rulle.txt' will remain the single
   * unread argument.
   */
   Array<Arg*> freeArgs();



   class Option {
    public:
     Option() :  _unique(false), _required(false), _minArgs(0), _maxArgs(0) { }
     Option(std::string key, std::string helpString) :
       _unique(false), _required(false),
       _key(key), _minArgs(0), _maxArgs(-1), _helpString(helpString) { }

     Array<Arg*> trim(Array<Arg*> optionAndArgs, const std::string &optPref) const;
     void dispHelp(std::ostream *out) const;

     Option &setMinArgCount(int ma) {
       _minArgs = ma;
       if (hasMaxArgCount()) {
         _maxArgs = std::max(_minArgs, _maxArgs);
       }
       return *this;
     }

     Option &setMaxArgCount(int ma) {
       _maxArgs = ma;
       _minArgs = std::min(_minArgs, _maxArgs);
       return *this;
     }

     Option &setArgCount(int ac) {
       _minArgs = ac;
       _maxArgs = ac;
       return *this;
     }

     Option &setFlag() {
       return setArgCount(0);
     }

     Option &setUnique() {
       _unique = true;
       return *this;
     }

     Option &setRequired() {
       _required = true;
       return *this;
     }

     Option &callback(std::function<void(const Array<Arg*>&)> callback) {
       _callback = callback;
       return *this;
     }

     Option &callback(std::function<void()> callback) {
       _callback = [=](const Array<Arg*> &) {callback();};
       return *this;
     }

     Option &store(std::string* destination) {
       setArgCount(1);
       callback([=](const Array<Arg*>& args) { *destination = args[0]->value(); });
       return *this;
     }

     Option &store(int* destination) {
       setArgCount(1);
       callback([=](const Array<Arg*>& args) { *destination = args[0]->parseIntOrDie(); });
       return *this;
     }

     Option &store(double* destination) {
       setArgCount(1);
       callback([=](const Array<Arg*>& args) { *destination = args[0]->parseDoubleOrDie(); });
       return *this;
     }

     int minArgCount() const {
       return _minArgs;
     }

     bool hasMaxArgCount() const {
       return _maxArgs != -1;
     }

     int maxArgCount() const {
       if (hasMaxArgCount()) {
         return _maxArgs;
       } else {
         return std::numeric_limits<int>::max();
       }
     }

     bool unique() const {
       return _unique;
     }

     bool required() const {
       return _required;
     }

     std::function<void(const Array<Arg*>&)> callback() { return _callback; }

     void addAlias(const std::string &alias) {
       _aliases.push_back(alias);
     }

     const std::string &key() const {
       return _key;
     }
    private:
     bool _unique, _required;
     std::string _key;
     int _minArgs, _maxArgs;
     std::string _helpString;
     std::function<void(const Array<Arg*>&)> _callback;

     // Other aliases for this option. For instance
     // A --file option might have the alias "-f".
     std::vector<std::string> _aliases;
   };
   typedef std::map<std::string, std::shared_ptr<Option> > OptionMap;

   /*
    * Information about a option that can be given on the command line, e.g.
    *
    *  --out-filename
    *
    * If this command for instance only accepts one argument such as the filename,
    * then call
    *
    *   registerOption("--out-filename", "Specify the filename").argCount(1);
    */
   ArgMap::Option &registerOption(std::string option, std::string helpString);

   bool hasRegisteredOption(const std::string &arg);

   void setHelpInfo(std::string helpInfo) {_helpInfo = helpInfo;}

   void dispHelp(std::ostream *out);
   std::string helpMessage();

   void alias(std::string mainName, std::string alternativeName);

   void disableFreeArgs() {
     _allowFreeArgs = false;
   }
 private:
  class TempArgs {
   public:
    TempArgs() : _optionCounter(0) {}

    // Whenever we encounter an option, call this method to retrieve the builder.
    ArrayBuilder<ArgMap::Arg*> &getArgsForNewOption(ArgMap::Arg *opt) {
      if (_optionCounter == 0) {
        _args.add(opt);
      }
      _optionCounter++;
      return _args;
    }

    Array<ArgMap::Arg*> get() {
      return _args.get();
    }

    int optionCount() const {
      return _optionCounter;
    }
   private:
    ArrayBuilder<ArgMap::Arg*> _args;
    int _optionCounter;
  };

  typedef std::map<std::string, TempArgs> TempArgMap;
  std::map<std::string, Array<ArgMap::Arg*> > buildMap(TempArgMap &src);
  bool hasAllRequiredArgs(TempArgMap &tempmap);

  bool parseSub(TempArgMap &tempmap, Array<Arg*> args);
  bool readOptionAndParseSub(TempArgMap &tempmap,
      std::shared_ptr<Option> info, Arg *opt, Array<Arg*> rest,
      ArrayBuilder<Arg*> &acc);


  bool _successfullyParsed;
  std::string _optionPrefix;
  Array<Arg> _argStorage;
  //Array<Entry*> _args;
  std::map<std::string, Array<Arg*> > _map;

  std::shared_ptr<Option> findOption(const std::string &key);
  OptionMap _options;

  std::set<std::shared_ptr<Option> > _optionSet;

  std::string _helpInfo;
  bool parseSub(int argc, const char **argv);

  std::string mainKey(std::string alternativeKey);

  bool _allowFreeArgs;
};

}

#endif /* ARGMAP_H_ */
