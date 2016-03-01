#include <device/anemobox/anemonode/src/JsNmea2000Source.h>
#include <device/anemobox/Dispatcher.h>

#include <iostream>
using namespace std;

using namespace v8;

namespace sail {

namespace {
v8::Persistent<v8::FunctionTemplate> nmea2000_constructor;
}  // namespace

JsNmea2000Source::JsNmea2000Source()
  : _nmea2000(Dispatcher::global()) { }

void JsNmea2000Source::Init(v8::Handle<v8::Object> target) {
  NanScope();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew<String>("Nmea2000Source"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  NODE_SET_METHOD(proto, "process", JsNmea2000Source::process);

  NanAssignPersistent<FunctionTemplate>(nmea2000_constructor, tpl);

  target->Set(NanNew<String>("Nmea2000Source"), tpl->GetFunction());
}

NAN_METHOD(JsNmea2000Source::New) {
  NanScope();

  JsNmea2000Source *obj = new JsNmea2000Source();
  obj->Wrap(args.This());
  NanReturnValue(args.This());
}

// Args: data, timestamp, srcName, pgn
NAN_METHOD(JsNmea2000Source::process) {
  NanScope();

  if (args.Length() < 5) {
    NanThrowTypeError(
        "Usage: process(data, timestamp, srcName, pgn, srcAddr)");
    NanReturnUndefined();
  }

  if(!args[0]->IsObject() || !node::Buffer::HasInstance(args[0])) {
    NanThrowTypeError("First argument must be a buffer");
    NanReturnUndefined();
  }
  
  if(!args[2]->IsString()) {
    NanThrowTypeError("3rd argument (srcName) must be a string");
    NanReturnUndefined();
  }

  if(!args[3]->IsNumber()) {
    NanThrowTypeError("4th argument (pgn) must be a number");
    NanReturnUndefined();
  }

  if(!args[4]->IsNumber()) {
    NanThrowTypeError("5th argument (srcAddr) must be a number");
    NanReturnUndefined();
  }
  
  JsNmea2000Source* obj = ObjectWrap::Unwrap<JsNmea2000Source>(args.This());
  if (!obj) {
    NanThrowTypeError("This is not a Nmea2000Source");
    NanReturnUndefined();
  }

  v8::Local<v8::Object> buffer = args[0]->ToObject();
  unsigned char* bufferData = (unsigned char *)node::Buffer::Data(buffer);
  size_t bufferLength = node::Buffer::Length(buffer);

  v8::String::Utf8Value srcNameStr(args[2]->ToString());
  obj->_nmea2000.process(
      *srcNameStr,
      args[3]->ToNumber()->Value(),
      bufferData,
      bufferLength,
      args[4]->ToNumber()->Value());
  NanReturnUndefined();
}

}  // namespace sail
