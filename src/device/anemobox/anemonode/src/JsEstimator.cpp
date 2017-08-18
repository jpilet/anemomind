#include <device/anemobox/anemonode/src/JsEstimator.h>

#include <device/anemobox/anemonode/src/anemonode.h>

using namespace v8;

namespace sail {

namespace {

Nan::Persistent<v8::FunctionTemplate> estimator_constructor;

}  // namespace

JsEstimator::JsEstimator()
  : _estimator(globalAnemonodeDispatcher) {
}

void JsEstimator::Init(v8::Handle<v8::Object> target) {
  Nan::HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Estimator").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  Nan::SetMethod(proto, "loadCalibration", JsEstimator::loadCalibration);
  Nan::SetMethod(proto, "compute", JsEstimator::compute);

  estimator_constructor.Reset(tpl);

  target->Set(Nan::New<String>("Estimator").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(JsEstimator::New) {
  Nan::HandleScope scope;
  JsEstimator *obj = new JsEstimator();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(JsEstimator::loadCalibration) {
  Nan::HandleScope scope;

  JsEstimator* obj = Nan::ObjectWrap::Unwrap<JsEstimator>(info.This());
  if (!obj) {
    Nan::ThrowTypeError("This is not a Logger");
    return;
  }

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError(
        "Bad arguments. "
        "Usage: loadCalibration('path to boat.dat')");
    return;
  }

  v8::String::Utf8Value folder(info[0]->ToString());

  // The loading is synchronous. This is not a very node-ish way to
  // do. But we do not care since it is a small file loaded just once.
  info.GetReturnValue().Set(Nan::New(obj->_estimator.loadCalibration(*folder)));
}

NAN_METHOD(JsEstimator::compute) {
  Nan::HandleScope scope;
  JsEstimator* obj = Nan::ObjectWrap::Unwrap<JsEstimator>(info.This());
  if (!obj) {
    Nan::ThrowTypeError("This is not a Logger");
    return;
  }
  obj->_estimator.compute();
  return;
}

}  // namespace sail
