
#include <device/anemobox/anemonode/src/JsDispatchData.h>

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
      auto val = values[index_];
      value_ = NanNew(val.value.degrees());
      timestamp_ = val.time;
    }
  }
  virtual void run(DispatchVelocityData *velocity) {
    auto values = velocity->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values[index_];
      value_ = NanNew(val.value.knots());
      timestamp_ = val.time;
    }
  }
  virtual void run(DispatchLengthData *velocity) {
    auto values = velocity->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values[index_];
      value_ = NanNew(val.value.nauticalMiles());
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchGeoPosData *pos) {
    auto values = pos->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values[index_];
      Local<Object> obj = NanNew<Object>();
      obj->Set(NanNew("lon"), NanNew(val.value.lon().degrees()));
      obj->Set(NanNew("lat"), NanNew(val.value.lat().degrees()));
      value_ = obj;
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchTimeStampData *dateTime) {
    auto values = dateTime->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values[index_];
      value_ = NanNew<Date>(double(val.value.toMilliSecondsSince1970()));
      timestamp_ = val.time;
    }
  }

  virtual void run(DispatchAbsoluteOrientationData *orient) {
    auto values = orient->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values[index_];
      Local<Object> obj = NanNew<Object>();
      obj->Set(NanNew("heading"), NanNew(val.value.heading.degrees()));
      obj->Set(NanNew("roll"), NanNew(val.value.roll.degrees()));
      obj->Set(NanNew("pitch"), NanNew(val.value.pitch.degrees()));
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
  virtual void run(DispatchGeoPosData *) {
    error_ = "set GeoPos from javascript is not implemented yet in " __FILE__;
    success_ = false;
  }
  virtual void run(DispatchTimeStampData *) {
    error_ = "set TimeStamp from javascript is not implemented yet in " __FILE__;
    success_ = false;
  }

  virtual void run(DispatchAbsoluteOrientationData *orientDispatch) {
    auto headingKey = NanNew("heading");
    auto rollKey = NanNew("roll");
    auto pitchKey = NanNew("pitch");

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
  public Listener<GeographicPosition<double>>,
  public Listener<TimeStamp>,
  public Listener<AbsoluteOrientation> {
 public:
  JsListener(DispatchData *dispatchData,
             Local<Function> callback,
             Duration<> minInterval)
    : Listener<Angle<double>>(minInterval),
    Listener<Velocity<double>>(minInterval),
    dispatchData_(dispatchData) { NanAssignPersistent(callback_, callback); }
  ~JsListener() {
    NanDisposePersistent(callback_);
  }

  virtual void onNewValue(const ValueDispatcher<Angle<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<Velocity<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<Length<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<GeographicPosition<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<TimeStamp> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<AbsoluteOrientation> &) { valueChanged(); }

  void valueChanged() {
    GetValueVisitor getValue(0);
    dispatchData_->visit(&getValue);

    Handle<Value> argv[1] = { getValue.value() };
    NanMakeCallback(
        NanGetCurrentContext()->Global(), NanNew<Function>(callback_), 1, argv);
  }

 private:
  DispatchData *dispatchData_;
  Persistent<Function> callback_;
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

  const std::string& type() const { return type_; }
  const std::string& unit() const { return unit_; }

 private:
  std::string type_;
  std::string unit_;
};

}  // namespace

Local<FunctionTemplate> JsDispatchData::functionTemplate() {
  Local<FunctionTemplate> t = NanNew<FunctionTemplate>(New);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  Local<ObjectTemplate> proto = t->PrototypeTemplate();

  NODE_SET_METHOD(proto, "length", JsDispatchData::length);
  NODE_SET_METHOD(proto, "value", JsDispatchData::value);
  NODE_SET_METHOD(proto, "time", JsDispatchData::time);
  NODE_SET_METHOD(proto, "setValue", JsDispatchData::setValue);
  NODE_SET_METHOD(proto, "subscribe", JsDispatchData::subscribe);
  NODE_SET_METHOD(proto, "unsubscribe", JsDispatchData::unsubscribe);
  NODE_SET_METHOD(proto, "source", JsDispatchData::source);
  return t;
}
   
void JsDispatchData::setDispatchData(
    Handle<Object> object, DispatchData* data, Dispatcher* dispatcher) {
  JsDispatchData* zis = obj(object);

  zis->_dispatchData = data;
  zis->_dispatcher = dispatcher;
  GetTypeAndUnitVisitor typeAndUnit;
  zis->_dispatchData->visit(&typeAndUnit);

  object->Set(NanNew("unit"), NanNew<String>(typeAndUnit.unit().c_str()));
  object->Set(NanNew("type"), NanNew<String>(typeAndUnit.type().c_str()));
  object->Set(NanNew("description"),
              NanNew<String>(zis->_dispatchData->description()));
  object->Set(NanNew("dataCode"),
              NanNew<Integer>(zis->_dispatchData->dataCode()));
}

NAN_METHOD(JsDispatchData::New) {
  NanScope();
  JsDispatchData* obj = new JsDispatchData();
  obj->Wrap(args.This());
  NanReturnValue(args.This());
}

NAN_METHOD(JsDispatchData::length) {
  NanScope();
  DispatchData* dispatchData = obj(args.This())->_dispatchData;

  CountValuesVisitor countValues;
  dispatchData->visit(&countValues);

  NanReturnValue(NanNew<Number>(countValues.numValues()));
}

NAN_METHOD(JsDispatchData::value) {
  NanScope();

  DispatchData* dispatchData = obj(args.This())->_dispatchData;

  unsigned index = 0;
  if (args.Length() >= 1) {
    if (!args[0]->IsNumber() || args[0]->ToInteger()->Value() < 0) {
      return NanThrowTypeError("the index should be a positive integer");
    }
    index = args[0]->ToInteger()->Value();
  }

  GetValueVisitor getValue(index);

  dispatchData->visit(&getValue);

  if (getValue.valid()) {
    NanReturnValue(getValue.value());
  } else {
    NanReturnUndefined();
  }
}

NAN_METHOD(JsDispatchData::time) {
  NanScope();

  DispatchData* dispatchData = obj(args.This())->_dispatchData;
  unsigned index = 0;
  if (args.Length() >= 1) {
    if (!args[0]->IsNumber() || args[0]->ToInteger()->Value() < 0) {
      return NanThrowTypeError("the index should be a positive integer");
    }
    index = args[0]->ToInteger()->Value();
  }

  GetValueVisitor getValue(index);

  dispatchData->visit(&getValue);

  if (getValue.valid()) {
    NanReturnValue(NanNew<Date>(
            double(getValue.time().toMilliSecondsSince1970())));
  } else {
    NanReturnUndefined();
  }
}

NAN_METHOD(JsDispatchData::setValue) {
  NanScope();
  JsDispatchData* zis = obj(args.This());
  DispatchData* dispatchData = zis->_dispatchData;

  if (args.Length() != 2) {
    return NanThrowTypeError("setValue expects 2 argument: source name and value");
  }

  if (!args[0]->IsString()) {
    return NanThrowTypeError("setValue a source name string as first argument");
  }

  v8::String::Utf8Value sourceName(args[0]->ToString());
  SetValueVisitor setValue(zis->_dispatcher, *sourceName, args[1]);
  dispatchData->visit(&setValue);

  if (!setValue.success()) {
    return NanThrowError(setValue.error().c_str());
  }

  NanReturnUndefined();
}

NAN_METHOD(JsDispatchData::unsubscribe) {
  NanScope();
  const char* exception = "First argument must be a subscribe index";

  if (args.Length() < 1) {
    return NanThrowTypeError(exception);
  }

  int index = args[0]->ToInteger()->Value();
  auto iterator = registeredCallbacks.find(index);
  if (iterator == registeredCallbacks.end()) {
    return NanThrowTypeError(exception);
  }

  delete iterator->second;
  registeredCallbacks.erase(iterator);

  NanReturnUndefined();
}

NAN_METHOD(JsDispatchData::subscribe) {
  NanScope();

  DispatchData* dispatchData = obj(args.This())->_dispatchData;
  if (args.Length() < 1 || !args[0]->IsFunction()) {
    return NanThrowTypeError("First argument must be a function");
  }

  Local<Function> cb = Local<Function>::Cast(args[0]);

  Duration<> minInterval = Duration<>::seconds(0);
  if (args.Length() >= 2) {
    minInterval = Duration<>::seconds(args[1]->ToNumber()->Value());
  }
  JsListener *listener = new JsListener(
      dispatchData, cb, minInterval);
  int index = registeredCallbacks.size() + 1;
  registeredCallbacks[index] = listener;

  SubscribeVisitor<JsListener> subscriber(listener);
  dispatchData->visit(&subscriber);
  NanReturnValue(NanNew<Integer>(index));
}

NAN_METHOD(JsDispatchData::source) {
  NanScope();

  DispatchData* dispatchData = obj(args.This())->_dispatchData;
  NanReturnValue(NanNew<String>(dispatchData->source()));
}

}  // namespace sail
