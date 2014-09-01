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

namespace sail {

class ArgMap {
 public:
  static bool instantiated;
  ArgMap(int argc, const char **argv);
  ~ArgMap();

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
    bool isKeyword(const std::string &prefix) const {
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
   * To check if a keyword exists, e.g. '--descend' if we were to call a sorting algorithm.
   */
  bool hasKeyword(const std::string &arg);

  /*
   * For instance, use this function to retrieve the filename succeeding a keyword, e.g.
   * '--outfile /home/alan/nmea.txt'
   */
  Array<Entry*> argsAfterKeyword(const std::string &arg);

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
   Array<Entry*> unreadArgs() const;

   /*
    * Information about a keyword that can be given on the command line, e.g.
    *
    *  --out-filename
    *
    * If this command for instance only accepts one argument such as the filename,
    * then call
    *
    *   registerKeyword("--out-filename", 1, 1, "Specify the filename");
    */
   void registerKeyword(std::string keyword, int minArgs, int maxArgs, std::string helpString);

   void registerHelpInfo(std::string helpInfo) {_helpInfo = helpInfo;}

   void dispHelp(std::ostream *out);
   std::string dispHelp();
 private:
  std::string _keywordPrefix;
  Array<Entry> _argStorage;
  Array<Entry*> _args;
  std::map<std::string, Array<Entry*> > _map;

  class Option {
   public:
    Option() : _minArgs(-1), _maxArgs(-1) {}
    Option(std::string keyword, int minArgs, int maxArgs, std::string helpString) :
      _keyword(keyword), _minArgs(minArgs), _maxArgs(maxArgs), _helpString(helpString) {}

    Array<Entry*> trim(Array<Entry*> args, const std::string &kwPref) const;
    void dispHelp(std::ostream *out) const;
   private:
    std::string _keyword;
    int _minArgs, _maxArgs;
    std::string _helpString;
  };

  std::map<std::string, Option> _keywords;
  std::string _helpInfo;
};

}

#endif /* ARGMAP_H_ */
