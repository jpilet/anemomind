#include <device/anemobox/anemonode/src/JsEstimator.h>

using namespace v8;

namespace sail {

namespace {

v8::Persistent<v8::FunctionTemplate> estimator_constructor;

}  // namespace

JsEstimator::JsEstimator()
  : _estimator(Dispatcher::global()) {
}

void JsEstimator::Init(v8::Handle<v8::Object> target) {
  NanScope();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew<String>("Estimator"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  NODE_SET_METHOD(proto, "loadCalibration", JsEstimator::loadCalibration);
  NODE_SET_METHOD(proto, "compute", JsEstimator::compute);

  NanAssignPersistent<FunctionTemplate>(estimator_constructor, tpl);

  target->Set(NanNew<String>("Estimator"), tpl->GetFunction());
}

NAN_METHOD(JsEstimator::New) {
  NanScope();
  JsEstimator *obj = new JsEstimator();
  obj->Wrap(args.This());
  NanReturnValue(args.This());
}

NAN_METHOD(JsEstimator::loadCalibration) {
  NanScope();

  JsEstimator* obj = ObjectWrap::Unwrap<JsEstimator>(args.This());
  if (!obj) {
    NanThrowTypeError("This is not a Logger");
    NanReturnUndefined();
  }

  if (args.Length() < 1 || !args[0]->IsString()) {
    NanThrowTypeError(
        "Bad arguments. "
        "Usage: loadCalibration('path to boat.dat')");
    NanReturnUndefined();
  }

  v8::String::Utf8Value folder(args[0]->ToString());

  // The loading is synchronous. This is not a very node-ish way to
  // do. But we do not care since it is a small file loaded just once.
  NanReturnValue(NanNew(obj->_estimator.loadCalibration(*folder)));
}

NAN_METHOD(JsEstimator::compute) {
  NanScope();
  JsEstimator* obj = ObjectWrap::Unwrap<JsEstimator>(args.This());
  if (!obj) {
    NanThrowTypeError("This is not a Logger");
    NanReturnUndefined();
  }
  obj->_estimator.compute();
  NanReturnUndefined();
}

}  // namespace sail
