#include <device/anemobox/anemonode/src/JsDispatcher.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/src/JsDispatchData.h>

using namespace sail;
using namespace v8;
using namespace node;

void JsDispatcher::Init(Dispatcher* dispatcher, Handle<Object> target) {
  static Nan::Persistent<FunctionTemplate> persistentConstructor;

  Nan::HandleScope scope;

  Local<FunctionTemplate> constructor = JsDispatcher::functionTemplate();
  persistentConstructor.Reset(constructor);

  Handle<Object> jsDispatcher = constructor->GetFunction()->NewInstance();
  setDispatcher(jsDispatcher, dispatcher);

  target->Set(Nan::New("dispatcher").ToLocalChecked(), jsDispatcher);
}

Local<FunctionTemplate> JsDispatcher::functionTemplate() {
  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  Local<ObjectTemplate> proto = t->PrototypeTemplate();

  Nan::SetMethod(proto, "setSourcePriority", JsDispatcher::setSourcePriority);
  Nan::SetMethod(proto, "sourcePriority", JsDispatcher::sourcePriority);
  Nan::SetMethod(proto, "allSources", JsDispatcher::allSources);
  return t;
}
   
void JsDispatcher::setDispatcher(Handle<Object> object, Dispatcher* dispatcher) {
  JsDispatcher* zis = obj(object);
  zis->_dispatcher = dispatcher;

  Local<FunctionTemplate> constructor = JsDispatchData::functionTemplate();
  zis->persistentConstructor.Reset(constructor);

  Handle<Object> entries = Nan::New<Object>();
  for (auto entry : dispatcher->dispatchers()) {
    Handle<Object> jsentry = constructor->GetFunction()->NewInstance();
    JsDispatchData::setDispatchData(jsentry, entry.second, dispatcher);

    entries->Set(Nan::New<String>(entry.second->wordIdentifier()).ToLocalChecked(), jsentry);
  }
  object->Set(Nan::New("values").ToLocalChecked(), entries);
}

NAN_METHOD(JsDispatcher::New) {
  Nan::HandleScope scope;
  JsDispatcher* obj = new JsDispatcher();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(JsDispatcher::setSourcePriority) {
  Nan::HandleScope scope;
  Dispatcher* dispatcher = obj(info.This())->_dispatcher;

  if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsNumber()) {
      return Nan::ThrowTypeError(
          "setSourcePriority expects a source name (string)"
          " and a priority (int) as info.");
  }

  v8::String::Utf8Value source(info[0]->ToString());

  dispatcher->setSourcePriority(*source, info[1]->ToInteger()->Value());

  return;
}

NAN_METHOD(JsDispatcher::sourcePriority) {
  Nan::HandleScope scope;
  Dispatcher* dispatcher = obj(info.This())->_dispatcher;

  if (info.Length() < 1 || !info[0]->IsString()) {
      return Nan::ThrowTypeError(
          "sourcePriority expects a source name (string) as arg.");
  }

  v8::String::Utf8Value source(info[0]->ToString());
  info.GetReturnValue().Set(Nan::New(dispatcher->sourcePriority(*source)));
}

NAN_METHOD(JsDispatcher::allSources) {
  Nan::HandleScope scope;
  JsDispatcher* me = obj(info.This());
  Dispatcher* dispatcher = me->_dispatcher;

  Local<Object> sources = Nan::New<Object>();
  const auto& sourceMap = dispatcher->allSources();

  Local<FunctionTemplate> constructor = Nan::New(me->persistentConstructor);

  for (auto channel : sourceMap) {
    Local<Object> sourceDict = Nan::New<Object>();
    for (auto source : channel.second) {
      Handle<Object> jsentry = constructor->GetFunction()->NewInstance();
      JsDispatchData::setDispatchData(jsentry, source.second, dispatcher);
      sourceDict->Set(Nan::New(source.first).ToLocalChecked(), jsentry);
    }
    sources->Set(Nan::New(wordIdentifierForCode(channel.first)).ToLocalChecked(), sourceDict);
  }

  info.GetReturnValue().Set(sources);
}
