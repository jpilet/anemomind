#include <device/anemobox/anemonode/src/JsNmea0183Source.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/src/anemonode.h>
#include <iostream>

using namespace std;

using namespace v8;

namespace sail {

namespace {
Nan::Persistent<v8::FunctionTemplate> nmea0183_constructor;
}  // namespace

JsNmea0183Source::JsNmea0183Source(const std::string& sourceName )
  : _nmea0183(globalAnemonodeDispatcher, sourceName) { }

void JsNmea0183Source::Init(v8::Handle<v8::Object> target) {
  Nan::HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Nmea0183Source").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  Nan::SetMethod(proto, "process", JsNmea0183Source::process);

  nmea0183_constructor.Reset(tpl);

  target->Set(Nan::New<String>("Nmea0183Source").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(JsNmea0183Source::New) {
  Nan::HandleScope scope;
  std::string name = "NMEA0183";

  if (info.Length() >= 1) {
    if (info[0]->IsString()) {
      v8::String::Utf8Value nameArg(info[0]->ToString());
      name = *nameArg;
    } else {
      Nan::ThrowTypeError("Source name must be a string");
    }
  }
  JsNmea0183Source *obj = new JsNmea0183Source(name);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(JsNmea0183Source::process) {
  Nan::HandleScope scope;

  if(!info[0]->IsObject() || !node::Buffer::HasInstance(info[0])) {
    Nan::ThrowTypeError("First argument must be a buffer");
    return;
  }
  
  JsNmea0183Source* obj = Nan::ObjectWrap::Unwrap<JsNmea0183Source>(info.This());
  if (!obj) {
    Nan::ThrowTypeError("This is not a Nmea0183Source");
    return;
  }

  v8::Local<v8::Object> buffer = info[0]->ToObject();
  unsigned char* bufferData = (unsigned char *)node::Buffer::Data(buffer);
  size_t bufferLength = node::Buffer::Length(buffer);

  obj->_nmea0183.process(bufferData, bufferLength);
  return;
}

}  // namespace sail
