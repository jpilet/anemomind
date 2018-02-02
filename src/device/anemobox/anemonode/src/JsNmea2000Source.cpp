#include <device/anemobox/anemonode/src/JsNmea2000Source.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/src/NodeNmea2000.h>
#include <device/anemobox/anemonode/src/anemonode.h>
#include <iostream>

using namespace std;

using namespace v8;

namespace sail {

namespace {

#define CHECK_CONDITION(expr, str) if(!(expr)) return Nan::ThrowError(str);

Nan::Persistent<v8::FunctionTemplate> nmea2000_constructor;
}  // namespace

JsNmea2000Source::JsNmea2000Source(tNMEA2000* nmea2000)
  : _nmea2000(nmea2000, globalAnemonodeDispatcher) { }

void JsNmea2000Source::Init(v8::Handle<v8::Object> target) {
  Nan::HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Nmea2000Source").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();

  nmea2000_constructor.Reset(tpl);

  target->Set(Nan::New<String>("Nmea2000Source").ToLocalChecked(), tpl->GetFunction());

  Nan::SetPrototypeMethod(tpl, "send", send);
}

NAN_METHOD(JsNmea2000Source::New) {
  Nan::HandleScope scope;

  CHECK_CONDITION(info.IsConstructCall(), "Must be called with new");
  CHECK_CONDITION(info.Length() >= 1 && info[0]->IsObject(),
                  "Must be called with a native NMEA2000 object as argument.");

  NodeNmea2000* nmea2000 = ObjectWrap::Unwrap<NodeNmea2000>(info[0]->ToObject());

  JsNmea2000Source *obj = new JsNmea2000Source(nmea2000);

  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

/*
Usage:
  First argument: Array of pieces of data to send

  See NodeNmea2000::New for another example of how to parse an
  array of Node objects.
*/
NAN_METHOD(JsNmea2000Source::send) {
  Nan::HandleScope scope;
  info.GetReturnValue().Set(true);
  if (info.Length() != 1) {
    return Nan::ThrowTypeError(
      "'send' accepts one argument: An array of messages to send");
  }
  auto arg = info[0];
  if (!arg->IsArray()) {
    return Nan::ThrowTypeError(
      "'send' expects the first argument to be an array of messages to send");
  }
  v8::Local<v8::Array> msgArray = v8::Local<v8::Array>::Cast(info[0]);
  
}

}  // namespace sail
