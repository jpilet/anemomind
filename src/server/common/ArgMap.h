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

namespace sail {

class ArgMap {
 public:
  static bool instantiated;
  ArgMap();


  void parse(int argc, const char **argv);

  class Entry {
   public:
    Entry() : _index(-1), _wasRead(false) {}
    Entry(int index, std::string arg) : _index(index),
        _arg(arg), _wasRead(false) {}

    const std::string &value() {
      _wasRead = true;
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




  /*
   * To check if a option exists, e.g. '--descend' if we were to call a sorting algorithm.
   */
  bool hasOption(const std::string &arg);

  /*
   * For instance, use this function to retrieve the filename succeeding a option, e.g.
   * '--outfile /home/alan/nmea.txt'
   */
  Array<Entry*> argsAfterOption(const std::string &arg);

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
   Array<Entry*> freeArgs() const;



   class Option {
    public:
     Option() : _minArgs(0), _maxArgs(0) {}
     Option(std::string option, std::string helpString) :
       _option(option), _minArgs(0), _maxArgs(0), _helpString(helpString) {}

     Array<Entry*> trim(Array<Entry*> args, const std::string &optPref) const;
     void dispHelp(std::ostream *out) const;

     Option &minArgs(int ma) {
       _minArgs = ma;
       return *this;
     }

     Option &maxArgs(int ma) {
       _maxArgs = ma;
       return *this;
     }

     Option &argCount(int ac) {
       _minArgs = ac;
       _maxArgs = ac;
       return *this;
     }
    private:
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

   void setHelpInfo(std::string helpInfo) {_helpInfo = helpInfo;}

   void dispHelp(std::ostream *out);
   std::string helpMessage();
 private:
  std::string _optionPrefix;
  Array<Entry> _argStorage;
  Array<Entry*> _args;
  std::map<std::string, Array<Entry*> > _map;



  std::map<std::string, Option> _options;
  std::string _helpInfo;
};

}

#endif /* ARGMAP_H_ */
