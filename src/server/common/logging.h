/*! Tools to output debug/log messages.
\author Julien Pilet <julien.pilet@gmail.com>
\date 2012
*/

#ifndef _LOGGING_H
#define _LOGGING_H

#include <string>

//! Defines the severity of log messages.
enum LogLevel {
  //! The message provides information about a normal case.
  LOGLEVEL_INFO,

  //! Warns about issues that, although not technically a
  //  problem now, could cause problems in the future.
  LOGLEVEL_WARNING,

  //! An error occurred which should never happen during normal use.
  LOGLEVEL_ERROR,

  //! An error occurred from which the caller cannot
  // recover. This usually indicates a programming error.
  LOGLEVEL_FATAL,

  //! A DFATAL error is considered fatal when the code is compiled in debug
  // mode. In release mode, it is considered as ERROR.
  LOGLEVEL_DFATAL =
#ifdef NDEBUG
    LOGLEVEL_ERROR
#else
    LOGLEVEL_FATAL
#endif
};

namespace internal {

class LogFinisher;

// Provides stream-like API for message logging.
class LogMessage {
 public:
  LogMessage(LogLevel level, const char* filename, int line);
  ~LogMessage();

  LogMessage& operator<<(const std::string& value);
  LogMessage& operator<<(const char* value);
  LogMessage& operator<<(char value);
  LogMessage& operator<<(int value);
  LogMessage& operator<<(unsigned int value);
  LogMessage& operator<<(long value);
  LogMessage& operator<<(unsigned long value);
  LogMessage& operator<<(double value);
  LogMessage& operator<<(long long value);

 private:
  friend class LogFinisher;
  void Finish();

  LogLevel level_;
  const char* filename_;
  int line_;

  std::string message_;
};

// Thrown by  LogMessage::Finish() if level_ is LOGLEVEL_FATAL
class LogMessageException {
 public:
  LogMessageException(LogLevel level, const char* filename, int line, const std::string &message);
 private:
  LogLevel level_;
  const char* filename_;
  int line_;
  std::string message_;
};


// Used to make the entire "LOG(BLAH) << etc." expression have a void return
// type and print a newline after each message.
class LogFinisher {
 public:
  void operator=(LogMessage& other);
};

template <class T>
T CheckNotNull(T x, const char *expr, const char* file, int line) {
  if (x != nullptr) return x;
  internal::LogFinisher() = internal::LogMessage(LOGLEVEL_FATAL, file, line)
                            << expr << " is NULL, which is unexpected.";
  return x;
}

}  // namespace internal

/*! Log messages.
  Example:
  LOG(WARNING) << "this is a bad sign";
  Note: LOG(FATAL) will terminate execution of the program.
  See \ref LogLevel for the list of log levels.
*/
#define LOG(LEVEL)                                                 \
    internal::LogFinisher() = internal::LogMessage(                 \
	    LOGLEVEL_##LEVEL, __FILE__, __LINE__)

//! Conditionally outputs a log message.
#define LOG_IF(LEVEL, CONDITION) \
    !(CONDITION) ? (void)0 : LOG(LEVEL)

/*!
  If EXPRESSION is evaluates to false, outputs a debug message and crash the
  program. EXPRESSION is evaluated once, in debug mode and release mode.
  Example: CHECK(file = fopen(filename, "r")) << "can't open: " << filename;
*/
#define CHECK(EXPRESSION) \
    LOG_IF(FATAL, !(EXPRESSION)) << "CHECK failed: " #EXPRESSION ": "

/*! CHECK_NOTNULL(EXPRESSION) checks that EXPRESSION does not evaluate to 0. The
 macro can be used as an inplace replacement for EXPRESSION. For example:
 char* array = CHECK_NOTNULL(malloc(size));
 */
#define CHECK_NOTNULL(EXPRESSION) \
    internal::CheckNotNull((EXPRESSION), #EXPRESSION, __FILE__, __LINE__)

void SetLogHandler(void (*log_handler)(LogLevel level, const char* filename, int line,
                                       const std::string& message));


typedef std::string ThreadId;

// Construct this object on the stack in the beginning of a new scope.
// It helps you follow the program flow.
class ScopedLog {
 public:
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
#define SCOPEDLOG(SCOPENAME) ScopedLog _slog(__FILE__, __LINE__, SCOPENAME)

// TODO: Provide stream-like syntax, so that we have
//    LOG(INFO) << ...
//  and
//    SCOPED(INFO) << ...
#define SCOPEDMESSAGE(LEVEL, MESSAGE) _slog.disp(__FILE__, __LINE__, LOGLEVEL_##LEVEL, MESSAGE)

#endif  // _LOGGING_H
