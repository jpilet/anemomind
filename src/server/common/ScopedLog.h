/*
 *  Created on: 2014-02-24
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

  ScopedLog() : _filename(nullptr), _line(-1), _finalScope(true) {}
  void enter(const char* filename, int line, const std::string &message);

  ~ScopedLog();
  static void setDepthLimit(int l);
  void disp(const char *file, int line, LogLevel level, std::string s);

  // These methods are public so that they can be called by the SCOPEDMESSAGE macro
  static bool shouldBeDisplayed(LogLevel level);
  void dispSub(const char *filename, int line, LogLevel level, std::string s);

  bool initialized() const {
    return _line != -1;
  }
 private:
  void init();
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
//
// The SCOPENAME argument, which should be something from which a std::string
// can be constructed, will only be evaluated if it needs to be displayed.
#define ENTERSCOPE(SCOPENAME) ScopedLog _slog; \
  if (ScopedLog::shouldBeDisplayed(LOGLEVEL_INFO)) {_slog.enter(__FILE__, __LINE__, SCOPENAME);}

#define ENTER_FUNCTION_SCOPE ENTERSCOPE(__FUNCTION__)

// TODO: Provide stream-like syntax, so that we have
//    LOG(INFO) << ...
//  and
//    SCOPED(INFO) << ...

// Rather than directly calling
//
//   _slog.dispSub(__FILE__, __LINE__, LOGLEVEL_##LEVEL, MESSAGE);
//
// we first call _slog.shouldBeDisplayed. This way,
// MESSAGE is only evaluated if it is needed. MESSAGE
// could for instance be a call to the stringFormat function...
#define SCOPEDMESSAGE(LEVEL, MESSAGE) \
  if (_slog.initialized() && ScopedLog::shouldBeDisplayed(LOGLEVEL_##LEVEL)) \
  {_slog.dispSub(__FILE__, __LINE__, LOGLEVEL_##LEVEL, MESSAGE);}

} /* namespace mmm */

#endif /* SCOPEDLOG_H_ */
