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

namespace sail {

class ArgMap {
 public:
  static bool instantiated;
  ArgMap();


  bool parse(int argc, const char **argv);

  class Arg {
   public:
    Arg() : _index(-1), _wasRead(false) {}
    Arg(int index, std::string arg) : _index(index),
        _arg(arg), _wasRead(false) {}

    const std::string &value() {
      _wasRead = true;
      return valueUntraced();
    }

    // Reads the value without leaving any trace, that is settings _wasRead = true.
    const std::string &valueUntraced() {
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
 public:



  /*
   * To check if a option exists, e.g. '--descend' if we were to call a sorting algorithm.
   */
  bool hasOption(const std::string &arg);

  /*
   * For instance, use this function to retrieve the filename succeeding a option, e.g.
   * '--outfile /home/alan/nmea.txt'
   */
  Array<Arg*> argsAfterOption(const std::string &arg);

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
     Option() : _minArgs(0), _maxArgs(0), _unique(false), _required(false) {}
     Option(std::string option, std::string helpString) :
       _option(option), _minArgs(0), _maxArgs(0), _helpString(helpString),
       _unique(false), _required(false) {}

     Array<Arg*> trim(Array<Arg*> optionAndArgs, const std::string &optPref) const;
     void dispHelp(std::ostream *out) const;

     Option &setMinArgCount(int ma) {
       _minArgs = ma;
       return *this;
     }

     Option &setMaxArgCount(int ma) {
       _maxArgs = ma;
       return *this;
     }

     Option &setArgCount(int ac) {
       _minArgs = ac;
       _maxArgs = ac;
       return *this;
     }

     Option &setUnique() {
       _unique = true;
       return *this;
     }

     Option &setRequired() {
       _required = true;
       return *this;
     }

     int minArgCount() const {
       return _minArgs;
     }

     int maxArgCount() const {
       return _maxArgs;
     }

     bool unique() const {
       return _unique;
     }

     bool required() const {
       return _required;
     }
    private:
     bool _unique, _required;
     std::string _option;
     int _minArgs, _maxArgs;
     std::string _helpString;
   };
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
 private:
  class TempArgs {
   public:
    TempArgs() : _optionCounter(0) {}

    // Whenever we encounter an option, call this method to retrieve the builder.
    ArrayBuilder<ArgMap::Arg*> &getArgsForNewOption() {
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
  static std::map<std::string, Array<ArgMap::Arg*> > buildMap(TempArgMap &src);

  bool parseSub(TempArgMap &tempmap, Array<Arg*> args);
  bool readOptionAndParseSub(TempArgMap &tempmap,
      Option info, Arg *opt, Array<Arg*> rest,
      ArrayBuilder<Arg*> &acc);

  bool _successfullyParsed;
  std::string _optionPrefix;
  Array<Arg> _argStorage;
  //Array<Entry*> _args;
  std::map<std::string, Array<Arg*> > _map;



  std::map<std::string, Option> _options;
  std::string _helpInfo;
};

}

#endif /* ARGMAP_H_ */
