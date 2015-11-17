#ifndef ANEMONODE_JS_ESTIMATOR_H
#define ANEMONODE_JS_ESTIMATOR_H

#include <device/anemobox/DispatcherTrueWindEstimator.h>

#include <node.h>
#include <nan.h>

namespace sail {

class JsEstimator : public node::ObjectWrap {
 public:
  JsEstimator();

  static void Init(v8::Handle<v8::Object> target);

 protected:
  static NAN_METHOD(New);
  static NAN_METHOD(loadCalibration);
  static NAN_METHOD(compute);

 private:
  DispatcherTrueWindEstimator _estimator;
};

}  // namespace sail

#endif  // ANEMONODE_JS_ESTIMATOR_H
