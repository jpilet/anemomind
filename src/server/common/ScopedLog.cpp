/*
 *  Created on: 2014-02-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ScopedLog.h"

// To verify that ScopedLog is only used in a single thread.
#include <thread>
#include <sstream>

namespace sail {

void ScopedLog::enter(const char* filename, int line, const std::string &message) {
  _filename = filename;
  _line = line;
  _message = message;
  verifyThread();
  _finalScope = _depth == _depthLimit-1;

  if (_finalScope) {
    disp(_filename, _line, LOGLEVEL_INFO, _message);
  } else {
    dispScopeLimit("BEGIN: ");
  }
  _depth++;
}

ScopedLog::ScopedLog(const ScopedLog &x) :
    _filename(nullptr), _line(-1), _finalScope(false), _message(nullptr) {
}

void ScopedLog::operator= (const ScopedLog &x) {
}


ScopedLog::~ScopedLog() {
  if (initialized()) {
    _depth--;
    if (!_finalScope) {
      dispScopeLimit("END:   ");
    }
  }
}

void ScopedLog::setDepthLimit(int l) {
  _depthLimit = l;
}

void ScopedLog::dispScopeLimit(const char *label) {
  disp(_filename, _line, LOGLEVEL_INFO, std::string(label) + _message);
}

bool ScopedLog::shouldBeDisplayed(LogLevel level) {
  return (_depth < _depthLimit) || (level != LOGLEVEL_INFO);
}

void ScopedLog::dispSub(const char *filename, int line, LogLevel level, std::string s) {
  std::string data = makeIndentation() + s;
  internal::LogFinisher() = internal::LogMessage(level, filename, line) << data;
}

void ScopedLog::disp(const char *filename, int line, LogLevel level, std::string s) {
  if (initialized() && shouldBeDisplayed(level)) {
    dispSub(filename, line, level, s);
  }
}

std::string ScopedLog::makeIndentation() {
  return std::string(2*_depth, ' ');
}

int ScopedLog::_depth = 0;
int ScopedLog::_depthLimit = 3000;


namespace {
  ScopedLog::ThreadId getThreadId() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
  }
}

void ScopedLog::verifyThread() {
  if (!(getThreadId() == _commonThreadId)) {
    LOG(FATAL) << "Only use the ScopedLog class within a single thread.";
  }
}

ScopedLog::ThreadId ScopedLog::_commonThreadId = getThreadId();

} /* namespace mmm */
