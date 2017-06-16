#include <device/anemobox/anemonode/src/JsNmea2000Source.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/src/anemonode.h>
#include <iostream>

using namespace std;

using namespace v8;

namespace sail {

namespace {
Nan::Persistent<v8::FunctionTemplate> nmea2000_constructor;
}  // namespace

JsNmea2000Source::JsNmea2000Source()
  : _nmea2000(globalAnemonodeDispatcher) { }

void JsNmea2000Source::Init(v8::Handle<v8::Object> target) {
  Nan::HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Nmea2000Source").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  Nan::SetMethod(proto, "process", JsNmea2000Source::process);

  nmea2000_constructor.Reset(tpl);

  target->Set(Nan::New<String>("Nmea2000Source").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(JsNmea2000Source::New) {
  Nan::HandleScope scope;

  JsNmea2000Source *obj = new JsNmea2000Source();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

// Args: data, timestamp, srcName, pgn
NAN_METHOD(JsNmea2000Source::process) {
  Nan::HandleScope scope;

  if (info.Length() < 5) {
    Nan::ThrowTypeError(
        "Usage: process(data, timestamp, srcName, pgn, srcAddr)");
    return;
  }

  if(!info[0]->IsObject() || !node::Buffer::HasInstance(info[0])) {
    Nan::ThrowTypeError("First argument must be a buffer");
    return;
  }
  
  if(!info[2]->IsString()) {
    Nan::ThrowTypeError("3rd argument (srcName) must be a string");
    return;
  }

  if(!info[3]->IsNumber()) {
    Nan::ThrowTypeError("4th argument (pgn) must be a number");
    return;
  }

  if(!info[4]->IsNumber()) {
    Nan::ThrowTypeError("5th argument (srcAddr) must be a number");
    return;
  }
  
  JsNmea2000Source* obj = Nan::ObjectWrap::Unwrap<JsNmea2000Source>(info.This());
  if (!obj) {
    Nan::ThrowTypeError("This is not a Nmea2000Source");
    return;
  }

  v8::Local<v8::Object> buffer = info[0]->ToObject();
  unsigned char* bufferData = (unsigned char *)node::Buffer::Data(buffer);
  size_t bufferLength = node::Buffer::Length(buffer);

  v8::String::Utf8Value srcNameStr(info[2]->ToString());
  obj->_nmea2000.process(
      *srcNameStr,
      info[3]->ToNumber()->Value(),
      bufferData,
      bufferLength,
      info[4]->ToNumber()->Value());
  return;
}

}  // namespace sail
