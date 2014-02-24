/*
 *  Created on: 24 févr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SCOPEDLOG_H_
#define SCOPEDLOG_H_

#include <server/common/logging.h>

namespace sail {

// Construct this object on the stack in the beginning of a new scope.
// It helps you follow the program flow.
class ScopedLog {
 public:
  typedef std::string ThreadId;

  ScopedLog(const char* filename, int line, std::string message);
  ~ScopedLog();
  static void setDepthLimit(int l);
  void disp(const char *file, int line, LogLevel level, std::string s);
 private:
  ScopedLog(const ScopedLog &x);
  void operator= (const ScopedLog &x);
  const char *_filename;
  int _line;
  std::string _message;
  static int _depth;      // <-- How much indentation there should be
  static int _depthLimit; // <-- Upper non-inclusive limit beyond which nothing will be displayed.
  void dispScopeLimit(const char *label);
  static std::string makeIndentation();
  bool _finalScope;


  static ThreadId _commonThreadId;
  static void verifyThread();
};

// This macro is flawed in the sense that the
// name of the resulting object is not auto-generated
// to be unique. Be careful.
#define ENTERSCOPE(SCOPENAME) ScopedLog _slog(__FILE__, __LINE__, SCOPENAME)

// TODO: Provide stream-like syntax, so that we have
//    LOG(INFO) << ...
//  and
//    SCOPED(INFO) << ...
#define SCOPEDMESSAGE(LEVEL, MESSAGE) _slog.disp(__FILE__, __LINE__, LOGLEVEL_##LEVEL, MESSAGE)

} /* namespace mmm */

#endif /* SCOPEDLOG_H_ */
