#include <device/anemobox/anemonode/JsDispatcher.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/JsDispatchData.h>

using namespace sail;
using namespace v8;
using namespace node;

void JsDispatcher::Init(Dispatcher* dispatcher, Handle<Object> target) {
  static Persistent<FunctionTemplate> persistentConstructor;

  NanScope();

  Local<FunctionTemplate> constructor = JsDispatcher::functionTemplate();
  NanAssignPersistent(persistentConstructor, constructor);

  Handle<Object> jsDispatcher = constructor->GetFunction()->NewInstance();
  setDispatcher(jsDispatcher, dispatcher);

  target->Set(NanNew("dispatcher"), jsDispatcher);
}

Local<FunctionTemplate> JsDispatcher::functionTemplate() {
  Local<FunctionTemplate> t = NanNew<FunctionTemplate>(New);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  Local<ObjectTemplate> proto = t->PrototypeTemplate();

  NODE_SET_METHOD(proto, "setSourcePriority", JsDispatcher::setSourcePriority);
  NODE_SET_METHOD(proto, "sourcePriority", JsDispatcher::sourcePriority);
  return t;
}
   
void JsDispatcher::setDispatcher(Handle<Object> object, Dispatcher* dispatcher) {
  JsDispatcher* zis = obj(object);
  zis->_dispatcher = dispatcher;

  static Persistent<FunctionTemplate> persistentConstructor;

  Local<FunctionTemplate> constructor = JsDispatchData::functionTemplate();
  NanAssignPersistent(persistentConstructor, constructor);

  Handle<Object> entries = NanNew<Object>();
  for (auto entry : dispatcher->dispatchers()) {
    Handle<Object> jsentry = constructor->GetFunction()->NewInstance();
    JsDispatchData::setDispatchData(jsentry, entry.second.get(), dispatcher);

    entries->Set(NanNew<String>(entry.second->wordIdentifier().c_str()), jsentry);
  }
  object->Set(NanNew("values"), entries);
}

NAN_METHOD(JsDispatcher::New) {
  NanScope();
  JsDispatcher* obj = new JsDispatcher();
  obj->Wrap(args.This());
  NanReturnValue(args.This());
}

NAN_METHOD(JsDispatcher::setSourcePriority) {
  NanScope();
  Dispatcher* dispatcher = obj(args.This())->_dispatcher;

  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsNumber()) {
      return NanThrowTypeError(
          "setSourcePriority expects a source name (string)"
          " and a priority (int) as args.");
  }

  v8::String::Utf8Value source(args[0]->ToString());

  dispatcher->setSourcePriority(*source, args[1]->ToInteger()->Value());

  NanReturnUndefined();
}

NAN_METHOD(JsDispatcher::sourcePriority) {
  NanScope();
  Dispatcher* dispatcher = obj(args.This())->_dispatcher;

  if (args.Length() < 1 || !args[0]->IsString()) {
      return NanThrowTypeError(
          "sourcePriority expects a source name (string) as arg.");
  }

  v8::String::Utf8Value source(args[0]->ToString());
  NanReturnValue(NanNew(dispatcher->sourcePriority(*source)));
}

