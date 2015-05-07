#ifndef ANEMONODE_JSNMEA0183SOURCE_H
#define ANEMONODE_JSNMEA0183SOURCE_H

#include <node.h>
#include <nan.h>

#include <device/anemobox/Nmea0183Source.h>

namespace sail {

class JsNmea0183Source : public node::ObjectWrap {
 public:
  JsNmea0183Source(const std::string& sourceName);

  static void Init(v8::Handle<v8::Object> target);

 protected:
  static NAN_METHOD(New);
  static NAN_METHOD(process);

 private:
  sail::Nmea0183Source _nmea0183;
};

}  // namespace sail

#endif  // ANEMONODE_JSNMEA0183SOURCE_H
