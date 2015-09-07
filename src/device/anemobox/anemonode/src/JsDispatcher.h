#ifndef ANEMONODE_JSDISPATCHER_H
#define ANEMONODE_JSDISPATCHER_H

#include <node.h>
#include <nan.h>

namespace sail {

class Dispatcher;

class JsDispatcher : public node::ObjectWrap {
 public:
  JsDispatcher() : _dispatcher(0) { }

  static v8::Local<v8::FunctionTemplate> functionTemplate(); 
   
  static JsDispatcher* obj(const v8::Handle<v8::Object>& obj) {
    return node::ObjectWrap::Unwrap<JsDispatcher>(obj);
  }

  static void Init(Dispatcher* dispatcher, v8::Handle<v8::Object> target);

  static void setDispatcher(
      v8::Handle<v8::Object> object, Dispatcher* dispatcher);

 protected:
  static NAN_METHOD(New);
  static NAN_METHOD(setSourcePriority);
  static NAN_METHOD(sourcePriority);

 private:
  Dispatcher* _dispatcher;
};

}  // namespace sail
#endif  // ANEMONODE_JSDISPATCHER_H
