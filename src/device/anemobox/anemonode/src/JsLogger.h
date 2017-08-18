#ifndef ANEMONODE_JSLOGGER_H
#define ANEMONODE_JSLOGGER_H

#include <device/anemobox/logger/Logger.h>

#include <node.h>
#include <nan.h>

namespace sail {

class JsLogger : public Nan::ObjectWrap {
 public:
  JsLogger();

  static void Init(v8::Handle<v8::Object> target);

 protected:
  static NAN_METHOD(New);
  static NAN_METHOD(flush);
  static NAN_METHOD(logText);
  static NAN_METHOD(logRawNmea2000);

 private:
  Logger _logger;
};

}  // namespace sail

#endif  // ANEMONODE_JSLOGGER_H
