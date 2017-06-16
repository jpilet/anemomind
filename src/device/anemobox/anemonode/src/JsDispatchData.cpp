
#include <device/anemobox/anemonode/src/JsDispatchData.h>

#include <device/anemobox/BinarySignal.h>
#include <device/anemobox/Dispatcher.h>

using namespace v8;
using namespace node;

namespace sail {

std::map<int, JsListener *> JsDispatchData::registeredCallbacks;

// We declare a couple of visitors used to handle the different types
// stored in the Dispatcher.
namespace {

class GetValueVisitor : public DispatchDataVisitor {
 public:
  GetValueVisitor(unsigned index) : index_(index), valid_(false) { }
  virtual void run(DispatchAngleData *angle) {
    auto values = angle->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values.back(index_);
      value_ = Nan::New(val.value.degrees());
      timestamp_ = val.time;
    }
  }
  virtual void run(DispatchVelocityData *velocity) {
    auto values = velocity->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values.back(index_);
      value_ = Nan::New(val.value.knots());
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchLengthData *velocity) {
    auto values = velocity->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values.back(index_);
      value_ = Nan::New(val.value.nauticalMiles());
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchBinaryEdge *edge) {
    auto values = edge->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values.back(index_);
      value_ = Nan::New(val.value == BinaryEdge::ToOn ? true : false);
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchGeoPosData *pos) {
    auto values = pos->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values.back(index_);
      Local<Object> obj = Nan::New<Object>();
      obj->Set(Nan::New("lon").ToLocalChecked(), Nan::New(val.value.lon().degrees()));
      obj->Set(Nan::New("lat").ToLocalChecked(), Nan::New(val.value.lat().degrees()));
      value_ = obj;
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchTimeStampData *dateTime) {
    auto values = dateTime->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values.back(index_);
      value_ = Nan::New<Date>(double(val.value.toMilliSecondsSince1970())).ToLocalChecked();
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchAbsoluteOrientationData *orient) {
    auto values = orient->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values.back(index_);
      Local<Object> obj = Nan::New<Object>();
      obj->Set(Nan::New("heading").ToLocalChecked(), Nan::New(val.value.heading.degrees()));
      obj->Set(Nan::New("roll").ToLocalChecked(), Nan::New(val.value.roll.degrees()));
      obj->Set(Nan::New("pitch").ToLocalChecked(), Nan::New(val.value.pitch.degrees()));
      value_ = obj;
      timestamp_ = val.time;
    }
  }

  Local<Value> value() const { return value_; }
  TimeStamp time() const { return timestamp_; }
  bool valid() const { return valid_; }

 private:
  unsigned index_;
  bool valid_;
  Local<Value> value_;
  TimeStamp timestamp_;
};

class CountValuesVisitor : public DispatchDataVisitor {
 public:
  CountValuesVisitor() : count_(0) { }

  virtual void run(DispatchAngleData *angle) {
    count_ = angle->dispatcher()->values().size();
  }
  virtual void run(DispatchVelocityData *v) {
    count_ = v->dispatcher()->values().size();
  }
  virtual void run(DispatchLengthData *v) {
    count_ = v->dispatcher()->values().size();
  }
  virtual void run(DispatchGeoPosData *v) {
    count_ = v->dispatcher()->values().size();
  }
  virtual void run(DispatchTimeStampData *v) {
    count_ = v->dispatcher()->values().size();
  }
  virtual void run(DispatchAbsoluteOrientationData *v) {
    count_ = v->dispatcher()->values().size();
  }
  virtual void run(DispatchBinaryEdge *v) {
    count_ = v->dispatcher()->values().size();
  }
  int numValues() const { return count_; }

 private:
  int count_;
};

class SetValueVisitor : public DispatchDataVisitor {
 public:
  SetValueVisitor(Dispatcher* dispatcher, std::string src, Handle<Value> val)
    : dispatcher_(dispatcher), source_(src), value_(val), success_(false) { }

  virtual void run(DispatchAngleData *angle) {
    if (checkNumberAndSetSuccess()) {
      dispatcher_->publishValue(
          angle->dataCode(),
          source_.c_str(),
          Angle<double>::degrees(value_->ToNumber()->Value()));
    }
  }
  virtual void run(DispatchVelocityData *velocity) {
    if (checkNumberAndSetSuccess()) {
      dispatcher_->publishValue(
          velocity->dataCode(),
          source_.c_str(),
          Velocity<double>::knots(value_->ToNumber()->Value()));
    }
  }
  virtual void run(DispatchLengthData *velocity) {
    if (checkNumberAndSetSuccess()) {
      dispatcher_->publishValue(
          velocity->dataCode(),
          source_.c_str(),
          Length<double>::nauticalMiles(value_->ToNumber()->Value()));
    }
  }
  virtual void run(DispatchBinaryEdge *edge) {
    success_ = value_->IsBoolean();
    if (!success_) {
      error_ = "a boolean is expected.";
    } else {
      dispatcher_->publishValue(
          edge->dataCode(),
          source_.c_str(),
          value_->ToBoolean()->Value() ? BinaryEdge::ToOn : BinaryEdge::ToOff);
    }
  }
  virtual void run(DispatchGeoPosData *) {
    error_ = "set GeoPos from javascript is not implemented yet in " __FILE__;
    success_ = false;
  }
  virtual void run(DispatchTimeStampData *dateTime) {
    if (!value_->IsDate()) {
      error_ = "Expecting a Date object";
      success_ = false;
    } else {
      double millisSinceEpoch = value_->ToNumber()->Value();
      dispatcher_->publishValue(
          dateTime->dataCode(),
          source_.c_str(),
          TimeStamp::fromMilliSecondsSince1970(millisSinceEpoch));
      success_ = true;
    }
  }

  virtual void run(DispatchAbsoluteOrientationData *orientDispatch) {
    auto headingKey = Nan::New("heading").ToLocalChecked();
    auto rollKey = Nan::New("roll").ToLocalChecked();
    auto pitchKey = Nan::New("pitch").ToLocalChecked();

    if (!value_->IsObject()) {
      error_ = "an object is expected";
      success_ = false;
      return;
    }

    auto obj = value_->ToObject();

    if (!obj->Has(headingKey) || !obj->Has(rollKey) || !obj->Has(pitchKey)) {
      error_ = "expect an AbsoluteOrientation object with heading, roll, pitch";
      success_ = false;
      return;
    }

    AbsoluteOrientation orient;
    orient.heading = Angle<double>::degrees(
        obj->Get(headingKey)->ToNumber()->Value());
    orient.roll = Angle<double>::degrees(
        obj->Get(rollKey)->ToNumber()->Value());
    orient.pitch = Angle<double>::degrees(
        obj->Get(pitchKey)->ToNumber()->Value());

    dispatcher_->publishValue(orientDispatch->dataCode(), source_.c_str(), orient);

    success_ = true;
  }

  bool checkNumberAndSetSuccess() {
    success_ = value_->IsNumber();
    if (!success_) {
      error_ = "a number is expected.";
    }
    return success_;
  }
    
  bool success() const { return success_; }
  const std::string& error() const { return error_; }
 private:
  Dispatcher* dispatcher_;
  std::string source_;
  std::string error_;
  Handle<Value> value_;
  bool success_;
};

class JsListener:
  public Listener<Angle<double>>,
  public Listener<Velocity<double>>,
  public Listener<Length<double>>,
  public Listener<BinaryEdge>,
  public Listener<GeographicPosition<double>>,
  public Listener<TimeStamp>,
  public Listener<AbsoluteOrientation> {
 public:
  JsListener(std::shared_ptr<DispatchData> dispatchData,
             Local<Function> callback,
             Duration<> minInterval)
    : Listener<Angle<double>>(minInterval),
    Listener<Velocity<double>>(minInterval),
    dispatchData_(dispatchData) { callback_.Reset(callback); }
  ~JsListener() {
    callback_.Reset();
  }

  virtual void onNewValue(const ValueDispatcher<Angle<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<Velocity<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<Length<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<GeographicPosition<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<TimeStamp> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<AbsoluteOrientation> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<BinaryEdge> &) { valueChanged(); }

  void valueChanged() {
    GetValueVisitor getValue(0);
    dispatchData_->visit(&getValue);

    Handle<Value> argv[1] = { getValue.value() };
    Nan::MakeCallback(
        Nan::GetCurrentContext()->Global(), 
	Nan::New<Function>(callback_), 1, argv);
  }

 private:
  std::shared_ptr<DispatchData> dispatchData_;
  Nan::Persistent<Function> callback_;
};

class GetTypeAndUnitVisitor : public DispatchDataVisitor {
 public:
  virtual void run(DispatchAngleData *) {
    type_ = "angle";
    unit_ = "degrees";
  }
  virtual void run(DispatchVelocityData *) {
    type_ = "velocity";
    unit_ = "knots";
  }
  virtual void run(DispatchLengthData *) {
    type_ = "distance";
    unit_ = "nautical miles";
  }
  virtual void run(DispatchGeoPosData *) {
    type_ = "geographic position";
    unit_ = "WGS84 latitude and longitude, in degrees";
  }
  virtual void run(DispatchTimeStampData *) {
    type_ = "Date and time";
    unit_ = "seconds since 1.1.1970, UTC";
  }
  virtual void run(DispatchAbsoluteOrientationData *) {
    type_ = "Absolute orientation";
    unit_ = "heading, roll, and pitch, in degrees";
  }
  virtual void run(DispatchBinaryEdge *) {
    type_ = "binary signal";
    unit_ = "boolean";
  }

  const std::string& type() const { return type_; }
  const std::string& unit() const { return unit_; }

 private:
  std::string type_;
  std::string unit_;
};

}  // namespace

Local<FunctionTemplate> JsDispatchData::functionTemplate() {
  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  Local<ObjectTemplate> proto = t->PrototypeTemplate();

  Nan::SetMethod(proto, "length", JsDispatchData::length);
  Nan::SetMethod(proto, "value", JsDispatchData::value);
  Nan::SetMethod(proto, "time", JsDispatchData::time);
  Nan::SetMethod(proto, "setValue", JsDispatchData::setValue);
  Nan::SetMethod(proto, "subscribe", JsDispatchData::subscribe);
  Nan::SetMethod(proto, "unsubscribe", JsDispatchData::unsubscribe);
  Nan::SetMethod(proto, "source", JsDispatchData::source);
  return t;
}
   
void JsDispatchData::setDispatchData(
    Handle<Object> object, std::shared_ptr<DispatchData> data,
    Dispatcher* dispatcher) {
  JsDispatchData* zis = obj(object);

  zis->_dispatchData = data;
  zis->_dispatcher = dispatcher;
  GetTypeAndUnitVisitor typeAndUnit;
  zis->_dispatchData->visit(&typeAndUnit);

  object->Set(Nan::New("unit").ToLocalChecked(), 
	      Nan::New<String>(typeAndUnit.unit().c_str()))
    .ToLocalChecked();
  object->Set(Nan::New("type").ToLocalChecked(), 
	      Nan::New<String>(typeAndUnit.type().c_str()))
    .ToLocalChecked();
  object->Set(Nan::New("description").ToLocalChecked(),
              Nan::New<String>(zis->_dispatchData->description()))
    .ToLocalChecked();
  object->Set(Nan::New("dataCode").ToLocalChecked(),
              Nan::New<Integer>(zis->_dispatchData->dataCode()));
}

NAN_METHOD(JsDispatchData::New) {
  Nan::HandleScope scope;
  JsDispatchData* obj = new JsDispatchData();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(JsDispatchData::length) {
  Nan::HandleScope scope;
  std::shared_ptr<DispatchData> dispatchData = obj(info.This())->_dispatchData;

  CountValuesVisitor countValues;
  dispatchData->visit(&countValues);

  info.GetReturnValue().Set(Nan::New<Number>(countValues.numValues()));
}

NAN_METHOD(JsDispatchData::value) {
  Nan::HandleScope scope;

  std::shared_ptr<DispatchData> dispatchData = obj(info.This())->_dispatchData;

  unsigned index = 0;
  if (info.Length() >= 1) {
    if (!info[0]->IsNumber() || info[0]->ToInteger()->Value() < 0) {
      return Nan::ThrowTypeError("the index should be a positive integer");
    }
    index = info[0]->ToInteger()->Value();
  }

  GetValueVisitor getValue(index);

  dispatchData->visit(&getValue);

  if (getValue.valid()) {
    info.GetReturnValue().Set(getValue.value());
  } else {
    return;
  }
}

NAN_METHOD(JsDispatchData::time) {
  Nan::HandleScope scope;

  std::shared_ptr<DispatchData> dispatchData = obj(info.This())->_dispatchData;

  unsigned index = 0;
  if (info.Length() >= 1) {
    if (!info[0]->IsNumber() || info[0]->ToInteger()->Value() < 0) {
      return Nan::ThrowTypeError("the index should be a positive integer");
    }
    index = info[0]->ToInteger()->Value();
  }

  GetValueVisitor getValue(index);

  dispatchData->visit(&getValue);

  if (getValue.valid()) {
    info.GetReturnValue().Set(Nan::New<Date>(
            double(getValue.time().toMilliSecondsSince1970())));
  } else {
    return;
  }
}

NAN_METHOD(JsDispatchData::setValue) {
  Nan::HandleScope scope;
  JsDispatchData* zis = obj(info.This());
  std::shared_ptr<DispatchData> dispatchData = zis->_dispatchData;

  if (info.Length() != 2) {
    return Nan::ThrowTypeError("setValue expects 2 argument: source name and value");
  }

  if (!info[0]->IsString()) {
    return Nan::ThrowTypeError("setValue a source name string as first argument");
  }

  v8::String::Utf8Value sourceName(info[0]->ToString());
  SetValueVisitor setValue(zis->_dispatcher, *sourceName, info[1]);
  dispatchData->visit(&setValue);

  if (!setValue.success()) {
    return Nan::ThrowError(setValue.error().c_str());
  }

  return;
}

NAN_METHOD(JsDispatchData::unsubscribe) {
  Nan::HandleScope scope;
  const char* exception = "First argument must be a subscribe index";

  if (info.Length() < 1) {
    return Nan::ThrowTypeError(exception);
  }

  int index = info[0]->ToInteger()->Value();
  auto iterator = registeredCallbacks.find(index);
  if (iterator == registeredCallbacks.end()) {
    return Nan::ThrowTypeError(exception);
  }

  delete iterator->second;
  registeredCallbacks.erase(iterator);

  return;
}

NAN_METHOD(JsDispatchData::subscribe) {
  Nan::HandleScope scope;

  std::shared_ptr<DispatchData> dispatchData = obj(info.This())->_dispatchData;

  if (info.Length() < 1 || !info[0]->IsFunction()) {
    return Nan::ThrowTypeError("First argument must be a function");
  }

  Local<Function> cb = Local<Function>::Cast(info[0]);

  Duration<> minInterval = Duration<>::seconds(0);
  if (info.Length() >= 2) {
    minInterval = Duration<>::seconds(info[1]->ToNumber()->Value());
  }
  JsListener *listener = new JsListener(
      dispatchData, cb, minInterval);
  int index = registeredCallbacks.size() + 1;
  registeredCallbacks[index] = listener;

  SubscribeVisitor<JsListener> subscriber(listener);
  dispatchData->visit(&subscriber);
  info.GetReturnValue().Set(Nan::New<Integer>(index));
}

NAN_METHOD(JsDispatchData::source) {
  Nan::HandleScope scope;

  std::shared_ptr<DispatchData> dispatchData = obj(info.This())->_dispatchData;
  info.GetReturnValue().Set(Nan::New<String>(dispatchData->source())).ToLocalChecked();
}

}  // namespace sail
