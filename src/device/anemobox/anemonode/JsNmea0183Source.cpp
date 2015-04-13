#include <device/anemobox/anemonode/JsNmea0183Source.h>
#include <device/anemobox/Dispatcher.h>

#include <iostream>
using namespace std;

using namespace v8;

namespace sail {

namespace {
v8::Persistent<v8::FunctionTemplate> nmea0183_constructor;
}  // namespace

JsNmea0183Source::JsNmea0183Source() : _nmea0183(Dispatcher::global()) { }

void JsNmea0183Source::Init(v8::Handle<v8::Object> target) {
  NanScope();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew<String>("Nmea0183Source"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  NODE_SET_METHOD(proto, "process", JsNmea0183Source::process);

  NanAssignPersistent<FunctionTemplate>(nmea0183_constructor, tpl);

  target->Set(NanNew<String>("Nmea0183Source"), tpl->GetFunction());
}

NAN_METHOD(JsNmea0183Source::New) {
  NanScope();
  JsNmea0183Source *obj = new JsNmea0183Source();
  obj->Wrap(args.This());
  NanReturnValue(args.This());
}

NAN_METHOD(JsNmea0183Source::process) {
  NanScope();

  if(!args[0]->IsObject() || !node::Buffer::HasInstance(args[0])) {
    NanThrowTypeError("First argument must be a buffer");
    NanReturnUndefined();
  }
  
  JsNmea0183Source* obj = ObjectWrap::Unwrap<JsNmea0183Source>(args.This());
  if (!obj) {
    NanThrowTypeError("This is not a Nmea0183Source");
    NanReturnUndefined();
  }

  v8::Local<v8::Object> buffer = args[0]->ToObject();
  unsigned char* bufferData = (unsigned char *)node::Buffer::Data(buffer);
  size_t bufferLength = node::Buffer::Length(buffer);

  obj->_nmea0183.process(bufferData, bufferLength);
  NanReturnUndefined();
}

}  // namespace sail
