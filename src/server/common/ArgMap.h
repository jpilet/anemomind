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

namespace sail {

class ArgMap {
 public:
  static bool instantiated;
  ArgMap(int argc, const char **argv);
  ~ArgMap();

  class Entry {
   public:
    Entry() : _index(-1), _wasRead(false) {}
    Entry(int index, std::string arg, Array<Entry*> allArgs) : _index(index),
        _arg(arg), _allArgs(allArgs), _wasRead(false) {}

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

    Array<Entry*> after() const {
      return _allArgs.sliceFrom(_index + 1);
    }

    bool wasRead() const {
      return _wasRead;
    }
   private:
    bool _wasRead;
    std::string _arg;
    int _index;
    Array<Entry*> _allArgs;
  };




  /*
   * To check if an argument exists, e.g. '--descend' if we were to call a sorting algorithm.
   */
  bool hasArg(const std::string &arg);

  /*
   * For instance, use this function to retrieve the filename succeeding a keyword, e.g.
   * '--outfile /home/alan/nmea.txt'
   */
  Array<Entry*> argsAfter(const std::string &arg);

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
    * If this command for instance only accepts one argument, e.g. the filename,
    * then call for instance
    *
    *   registerKeyword("--out-filename", 1, "Specifi
    */
   void registerKeyword(std::string keyword, int maxArgs, std::string helpString);

   void registerHelpInfo(std::string helpInfo) {_helpInfo = helpInfo;}
 private:
  void dispHelp(std::ostream *out);
  std::string _keywordPrefix;
  Array<Entry> _argStorage;
  Array<Entry*> _args;
  std::map<std::string, Entry*> _map;


  class KeywordInfo {
   public:
    KeywordInfo() : _maxArgs(-1) {}
    KeywordInfo(std::string keyword, int maxArgs, std::string helpString) :
      _keyword(keyword), _maxArgs(maxArgs), _helpString(helpString) {}

    Array<Entry*> trim(Array<Entry*> args, const std::string &kwPref) const;
    void dispHelp(std::ostream *out) const;
   private:
    std::string _keyword;
    int _maxArgs;
    std::string _helpString;
  };

  std::map<std::string, KeywordInfo> _keywords;
  std::string _helpInfo;
};

}

#endif /* ARGMAP_H_ */
