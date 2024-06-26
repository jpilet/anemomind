#ifndef ANEMONODE_JSNMEA2000SOURCE_H
#define ANEMONODE_JSNMEA2000SOURCE_H

#include <node.h>
#include <nan.h>

#include <device/anemobox/Nmea2000Source.h>

namespace sail {

class JsNmea2000Source : public Nan::ObjectWrap {
 public:
  JsNmea2000Source(tNMEA2000* nmea2000);

  static void Init(v8::Handle<v8::Object> target);

 protected:
  static NAN_METHOD(New);
  static NAN_METHOD(send);
 private:
  sail::Nmea2000Source _nmea2000;
};

}  // namespace sail

#endif  // ANEMONODE_JSNMEA2000SOURCE_H
