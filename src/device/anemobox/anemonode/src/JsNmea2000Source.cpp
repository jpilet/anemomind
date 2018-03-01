#include <device/anemobox/anemonode/src/JsNmea2000Source.h>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/src/NodeNmea2000.h>
#include <device/anemobox/anemonode/src/anemonode.h>
#include <device/anemobox/anemonode/src/NodeUtils.h>
#include <iostream>

using namespace std;

using namespace v8;
using namespace PgnClasses;

// Helper type when parsing physical quantities
struct TaggedValue {
  double value = 0.0;
  std::string tag;

  TaggedValue() {}
  TaggedValue(double x, const std::string& s = "") 
    : value(x), tag(s) {}
};

bool tryExtract(const v8::Local<v8::Value>& val,
                TaggedValue* dst) {
  if (val->IsNumber()) { // No tag, just a number.
    *dst = TaggedValue(val->NumberValue());
    return true;

    
  } else if (val->IsArray()) { // On this format: 
                               // [value: number, tag: string]?
    v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(val);
    if (arr->Length() == 2) {
      double x = 0.0;
      std::string tag;
      bool success = tryExtract(arr->Get(0), &x)
        && tryExtract(arr->Get(1), &tag);
      if (success) {
        *dst = TaggedValue(x, tag);
        return true;
      }
    }
  }
  return false;
}

namespace sail {

namespace {

Nan::Persistent<v8::FunctionTemplate> nmea2000_constructor;
}  // namespace

JsNmea2000Source::JsNmea2000Source(tNMEA2000* nmea2000)
  : _nmea2000(nmea2000, globalAnemonodeDispatcher) { }

void JsNmea2000Source::Init(v8::Handle<v8::Object> target) {
  Nan::HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Nmea2000Source").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();

  Nan::SetPrototypeMethod(tpl, "send", send);

  nmea2000_constructor.Reset(tpl);
  target->Set(Nan::New<String>("Nmea2000Source").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(JsNmea2000Source::New) {
  Nan::HandleScope scope;

  CHECK_CONDITION(info.IsConstructCall(), "Must be called with new");
  CHECK_CONDITION(info.Length() >= 1 && info[0]->IsObject(),
                  "Must be called with a native NMEA2000 object as argument.");

  NodeNmea2000* nmea2000 = ObjectWrap::Unwrap<NodeNmea2000>(info[0]->ToObject());

  JsNmea2000Source *obj = new JsNmea2000Source(nmea2000);

  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

bool extractTypedValue(
    const TaggedValue& src, Angle<double>* dst) {
  if (src.tag == "deg" 
      || src.tag == "degrees" 
      || src.tag == "degree" 
      || src.tag == "" /*<-- For backward compability and conventions, 
                         we will assume it is degrees if there is
                         no tag*/) {
    *dst = Angle<double>::degrees(src.value);
    return true;
  } else if (src.tag == "rad" 
             || src.tag == "radians"
             || src.tag == "radian") {
    *dst = Angle<double>::radians(src.value);
    return true;
  }
  return false;
}

bool extractTypedValue(
    const TaggedValue& src, Velocity<double>* dst) {
  if (src.tag == "kt" 
      || src.tag == "kn" 
      || src.tag == "knots"
      || src.tag == "knot"
      || src.tag == "" /*<-- For backward compability and conventions, 
                         we will assume it is knots if there is
                         no tag*/) {
    *dst = Velocity<double>::knots(src.value);
    return true;
  } else if (src.tag == "m/s" || src.tag == "mps") {
    *dst = Velocity<double>::metersPerSecond(src.value);
    return true;
  }
  return false;
}


bool extractTypedValue(
    const TaggedValue& src, Duration<double>* dst) {
  if (src.tag == "days" || src.tag == "day") {
    *dst = Duration<double>::seconds(src.value);
    return true;
  } else if (src.tag == "hours" 
             || src.tag == "hour" 
             || src.tag == "h") {
    *dst = Duration<double>::hours(src.value);
    return true;
  } else if (src.tag == "minutes" 
             || src.tag == "min" 
             || src.tag == "minutes") {
    *dst = Duration<double>::minutes(src.value);
    return true;
  } else if (src.tag == "seconds" 
             || src.tag == "second" 
             || src.tag == "s") {
    *dst = Duration<double>::seconds(src.value);
    return true;
  }
  return false;
}

bool extractTypedValue(
    const TaggedValue& src, Length<double>* dst) {
  if (src.tag == "m" 
      || src.tag == "meters" 
      || src.tag == "meter"
      || src.tag == "metre"
      || src.tag == "metres") {
    *dst = Length<double>::meters(src.value);
    return true;
  }
  return false;
}

  
  template <typename T>
  bool tryExtractTypedFromTagged(
     const v8::Local<v8::Value>& val,
     T* dst) {
    TaggedValue tmp;
    T x;
    if (tryExtract(val, &tmp) && extractTypedValue(tmp, &x)) {
      *dst = x;
      return true;
    }
    return false;
  }

  bool tryExtract(const v8::Local<v8::Value>& val,
                  Angle<double>* dst) {
    return tryExtractTypedFromTagged(val, dst);
  }

  bool tryExtract(const v8::Local<v8::Value>& val,
                  Duration<double>* dst) {
    return tryExtractTypedFromTagged(val, dst);
  }

  bool tryExtract(const v8::Local<v8::Value>& val,
                  Velocity<double>* dst) {
    return tryExtractTypedFromTagged(val, dst);
  }
  
  bool tryExtract(const v8::Local<v8::Value>& val,
                  Length<double>* dst) {
    return tryExtractTypedFromTagged(val, dst);
  }

bool sendPositionRapidUpdate(
    int32_t deviceIndex,
    const v8::Local<v8::Object>& val,
    Nmea2000Source* dst) {
  
  PgnClasses::PositionRapidUpdate x;
  TRY_LOOK_UP(val, "longitude", &x.longitude);
  TRY_LOOK_UP(val, "latitude", &x.latitude);
  CHECK_CONDITION_BOOL(dst->send(deviceIndex, x), "Failed to send message");
  return true;
}

  template <typename Q>
  struct QuantityAsNumber {
    Q unit;

    QuantityAsNumber(Q u) : unit(u) {}

    typedef Optional<double> in_type;
    typedef Optional<Q> out_type;

    Optional<Q> apply(in_type x) const {
      return x.defined()? 
        Optional<Q>(x.get()*unit) 
        : Optional<Q>();
    }
  };

  template <typename Q>
  QuantityAsNumber<Q> quantityAsNumber(Q unit) {
    return {unit};
  }

  template <typename E>
  struct EnumAsInt {
    typedef uint64_t in_type;
    typedef Optional<E> out_type;

    Optional<E> apply(in_type x) const {
      return static_cast<E>(x);
    }
  };

  template <typename T>
  bool tryLookUpTyped(
    const T& conv,                 
    v8::Local<v8::Object> obj,
    const char* key, 
    typename T::out_type* dst) {
    typename T::in_type x;
    if (tryLookUp(obj, key, &x)) {
      *dst = conv.apply(x);
      return true;
    }
    return false;
  }

#define TRY_LOOK_UP_TYPED(conv, obj, key, dst) \
  CHECK_CONDITION_BOOL(tryLookUpTyped(conv, obj, key, dst), "Missing or malformed '" key "'")

  bool readRepeatingField(const v8::Local<v8::Value>& src,
                          GnssPositionData::Repeating* dst) {
      CHECK_CONDITION_BOOL(
         src->IsObject(), 
         "GnssPositionData repeating field not an object");

      v8::Local<v8::Object> obj = src->ToObject();
      
      CHECK_CONDITION_BOOL(
         tryLookUpTyped(
            EnumAsInt<GnssPositionData::ReferenceStationType>(),
            obj, "referenceStationType", &(dst->referenceStationType)),
         "Malformed referenceStationType");
      
      CHECK_CONDITION_BOOL(
         tryLookUp(
            obj, "referenceStationId", &(dst->referenceStationId)),
         "Malformed referenceStationId");
      
      CHECK_CONDITION_BOOL(
         tryLookUp(
            obj, "ageOfDgnssCorrections", 
            &(dst->ageOfDgnssCorrections)),
         "Malformed age");
      return true;
  }

  template <typename T>
  bool readRepeating(const v8::Local<v8::Object>& obj,
                     const char* key,
                     std::vector<T>* dst) {
    v8::Local<v8::Value> val = Nan::Get(
      obj, Nan::New<v8::String>(key)
      .ToLocalChecked()).ToLocalChecked();
    
    CHECK_CONDITION_BOOL(val->IsArray(), 
                         "Repeating field not an array");

    v8::Local<v8::Array> arr= Local<v8::Array>::Cast(val);
    
    for (size_t i = 0; i < arr->Length(); i++) {
      T field;
      if (!readRepeatingField(arr->Get(i), &field)) {
        return false;
      }
      dst->push_back(field);
    }
    return true;
  }

bool sendGnssPositionData(
   int32_t deviceIndex,
   const v8::Local<v8::Object>& obj,
   Nmea2000Source* dst) {
  using namespace PgnClasses;

  GnssPositionData x;

  x.sid = lookUpOrDefault<uint64_t>(obj, "sid", 0);

  // I am not sure about these ones. Should we require
  // the unit to be provided from the JS side, that
  // is [3.4, "days"] and prohibit just 3.4?
  TRY_LOOK_UP_TYPED(quantityAsNumber(1.0_days), 
                    obj, "date", &x.date);
  TRY_LOOK_UP_TYPED(quantityAsNumber(1.0_seconds), 
                    obj, "time", &x.time);

  // Should we assume the altitude is in meters,
  // or require the user to provide the unit explicitly
  // from the JS side, that is [1.0, "meters"]?
  TRY_LOOK_UP_TYPED(quantityAsNumber(Length<double>::meters(1.0)),
                    obj, "altitude", &x.altitude);

  TRY_LOOK_UP(obj, "latitude", &x.latitude);

  TRY_LOOK_UP(obj, "longitude", &x.longitude);

  TRY_LOOK_UP_TYPED(EnumAsInt<GnssPositionData::GnssType>(),
                    obj, "gnssType", &x.gnssType);

  TRY_LOOK_UP_TYPED(EnumAsInt<GnssPositionData::Method>(),
                    obj, "method", &x.method);

  TRY_LOOK_UP_TYPED(EnumAsInt<GnssPositionData::Integrity>(),
                    obj, "integrity", &x.integrity);

  TRY_LOOK_UP(obj, "numberOfSvs", &x.numberOfSvs);
  
  TRY_LOOK_UP(obj, "hdop", &x.hdop);

  TRY_LOOK_UP(obj, "pdop", &x.pdop);

  TRY_LOOK_UP(obj, "geoidalSeparation", &x.geoidalSeparation);

  if (readRepeating(obj, "referenceStations", &x.repeating)) {
    x.referenceStations = x.repeating.size();
    CHECK_CONDITION_BOOL(dst->send(deviceIndex, x), 
                         "Failed to send GnssPositionData");
    return true;
  } else {
    return false;
  }
}

bool sendWindData(
   int32_t deviceIndex,
   const v8::Local<v8::Object>& obj,
   Nmea2000Source* dst) {
  using namespace PgnClasses;

  WindData x;
  x.sid = lookUpOrDefault<uint64_t>(obj, "sid", 0);
  TRY_LOOK_UP(obj, "windSpeed", &(x.windSpeed));
  TRY_LOOK_UP(obj, "windAngle", &(x.windAngle));
  TRY_LOOK_UP_TYPED(EnumAsInt<WindData::Reference>(), obj, "reference", &(x.reference));
  CHECK_CONDITION_BOOL(dst->send(deviceIndex, x), "Failed to send WindData");
  return true;
}

bool sendTimeDate(
   int32_t deviceIndex,
   const v8::Local<v8::Object>& obj,
   Nmea2000Source* dst) {
  using namespace PgnClasses;

  TimeDate x;
  TRY_LOOK_UP_TYPED(quantityAsNumber(1.0_days), obj, "date", &(x.date));
  TRY_LOOK_UP_TYPED(quantityAsNumber(1.0_seconds), obj, "time", &(x.time));
  TRY_LOOK_UP_TYPED(quantityAsNumber(1.0_minutes), obj, "localOffset", &(x.localOffset));
  CHECK_CONDITION_BOOL(dst->send(deviceIndex, x), "Failed to send TimeDate");
  return true;
}

bool sendCogSogRapidUpdate(
  int32_t deviceIndex,
  const v8::Local<v8::Object>& obj,
  Nmea2000Source* dst) {
  using namespace PgnClasses;
  CogSogRapidUpdate x;
  x.sid = lookUpOrDefault<uint64_t>(obj, "sid", 0);
  TRY_LOOK_UP_TYPED(EnumAsInt<CogSogRapidUpdate::CogReference>(),
                    obj, "cogReference", &x.cogReference);
  TRY_LOOK_UP(obj, "cog", &x.cog);
  TRY_LOOK_UP(obj, "sog", &x.sog);
  auto result = dst->send(deviceIndex, x);
  CHECK_CONDITION_BOOL(result, "Failed to send CogSogRapidUpdate");
  return true;
}

bool dispatchPgn(
   int64_t pgn,
   const v8::Local<v8::Object>& obj,
   Nmea2000Source* dst) {

  // NOTE: deviceIndex is an implementation detail of 
  // the tNMEA2000 class. Would it be useful to specify the device
  // in some other way?
  int32_t deviceIndex = 0;
  CHECK_CONDITION_BOOL(tryLookUp(obj, "deviceIndex", &deviceIndex), 
                       "Missing deviceIndex field");

  switch (pgn) {
  case PgnClasses::PositionRapidUpdate::ThisPgn:
    return sendPositionRapidUpdate(deviceIndex, obj, dst);
  case PgnClasses::GnssPositionData::ThisPgn:
      return sendGnssPositionData(deviceIndex, obj, dst);
  case PgnClasses::WindData::ThisPgn:
    return sendWindData(deviceIndex, obj, dst);
  case PgnClasses::TimeDate::ThisPgn:
    return sendTimeDate(deviceIndex, obj, dst);
  case PgnClasses::CogSogRapidUpdate::ThisPgn:
    return sendCogSogRapidUpdate(deviceIndex, obj, dst);
  default: 
    break;
  };
  {
    std::stringstream ss;
    ss << "PGN not supported: " << pgn;
    Nan::ThrowError(ss.str().c_str());
    return false;
  }
}

// These codes are ordered, so that the
// closer we get towards sending a message,
// the lower the code.
bool parseAndSendMessage(
    const v8::Local<v8::Value>& val,
    Nmea2000Source* dst) {
  CHECK_CONDITION_BOOL(val->IsObject(), "Message not an object");
  v8::Local<v8::Object> obj =  val->ToObject();

  int32_t pgn = 0;
  CHECK_CONDITION_BOOL(tryLookUp(obj, "pgn", &pgn), 
                  "PGN missing or not an integer");
  return dispatchPgn(pgn, obj, dst);
}


/*
Usage:
  First argument: Array of pieces of data to send

  See NodeNmea2000::New for another example of how to parse an
  array of Node objects.
*/
NAN_METHOD(JsNmea2000Source::send) {
  Nan::HandleScope scope;
  JsNmea2000Source* zis = 
    ObjectWrap::Unwrap<JsNmea2000Source>(
      info.Holder());
  info.GetReturnValue().Set(true);
  if (info.Length() != 1) {
    return Nan::ThrowTypeError(
      "'send' accepts one argument: An array of messages to send");
  }
  auto arg = info[0];
  if (!arg->IsArray()) {
    return Nan::ThrowTypeError(
      "'send' expects the first argument to be an array of messages to send");
  }
  v8::Local<v8::Array> msgArray = v8::Local<v8::Array>::Cast(info[0]);
  size_t n = msgArray->Length();
  for (size_t i = 0; i < n; i++) {
    if (!parseAndSendMessage(msgArray->Get(i), &(zis->_nmea2000))) {
      return;
    }
  }
}

} // namespace sail
