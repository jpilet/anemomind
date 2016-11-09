#include <device/anemobox/anemonode/src/JsNmea0183Source.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/src/anemonode.h>
#include <iostream>

using namespace std;

using namespace v8;

namespace sail {

namespace {
v8::Persistent<v8::FunctionTemplate> nmea0183_constructor;
}  // namespace

JsNmea0183Source::JsNmea0183Source(const std::string& sourceName )
  : _nmea0183(globalAnemonodeDispatcher, sourceName) { }

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
  std::string name = "NMEA0183";

  if (args.Length() >= 1) {
    if (args[0]->IsString()) {
      v8::String::Utf8Value nameArg(args[0]->ToString());
      name = *nameArg;
    } else {
      NanThrowTypeError("Source name must be a string");
    }
  }
  JsNmea0183Source *obj = new JsNmea0183Source(name);
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
