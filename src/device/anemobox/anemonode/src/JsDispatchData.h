#ifndef ANEMONODE_JSDISPATCHDATA_H
#define ANEMONODE_JSDISPATCHDATA_H

#include <node.h>
#include <nan.h>

#include <map>
#include <memory>

namespace sail {

class DispatchData;
class Dispatcher;

namespace {
class JsListener;
}  // namespace

class JsDispatchData : public Nan::ObjectWrap {
 public:
  JsDispatchData() : _dispatchData(0) { }

  static v8::Local<v8::FunctionTemplate> functionTemplate(); 
   
  static JsDispatchData* obj(const v8::Handle<v8::Object>& obj) {
    return Nan::ObjectWrap::Unwrap<JsDispatchData>(obj);
  }

  static void setDispatchData(
      v8::Handle<v8::Object> object, std::shared_ptr<DispatchData> data, Dispatcher* dispatcher);

 protected:
  static NAN_METHOD(New);
  static NAN_METHOD(length);
  static NAN_METHOD(value);
  static NAN_METHOD(time);
  static NAN_METHOD(setValue);
  static NAN_METHOD(unsubscribe);
  static NAN_METHOD(subscribe);
  static NAN_METHOD(source);

 private:
  std::shared_ptr<DispatchData> _dispatchData;
  Dispatcher* _dispatcher;

  static int64_t subscriptionIndex;
  static std::map<int, JsListener *> registeredCallbacks;
};

}  // namespace sail

#endif // ANEMONODE_JSDISPATCHDATA_H
