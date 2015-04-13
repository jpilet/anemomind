#ifndef ANEMONODE_JSDISPATCHDATA_H
#define ANEMONODE_JSDISPATCHDATA_H

#include <node.h>
#include <nan.h>

#include <map>

namespace sail {

class DispatchData;

namespace {
class JsListener;
}  // namespace

class JsDispatchData : public node::ObjectWrap {
 public:
  JsDispatchData() : _dispatchData(0) { }

  static v8::Local<v8::FunctionTemplate> functionTemplate(); 
   
  static JsDispatchData* obj(const v8::Handle<v8::Object>& obj) {
    return node::ObjectWrap::Unwrap<JsDispatchData>(obj);
  }

  static void setDispatchData(
      v8::Handle<v8::Object> object, DispatchData* data);

 protected:
  static NAN_METHOD(New);
  static NAN_METHOD(length);
  static NAN_METHOD(value);
  static NAN_METHOD(time);
  static NAN_METHOD(setValue);
  static NAN_METHOD(unsubscribe);
  static NAN_METHOD(subscribe);

 private:
  DispatchData* _dispatchData;

  static std::map<int, JsListener *> registeredCallbacks;
};

}  // namespace sail

#endif // ANEMONODE_JSDISPATCHDATA_H
