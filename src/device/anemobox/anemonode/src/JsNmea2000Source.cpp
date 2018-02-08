#include <device/anemobox/anemonode/src/JsNmea2000Source.h>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/anemonode/src/NodeNmea2000.h>
#include <device/anemobox/anemonode/src/anemonode.h>
#include <device/anemobox/anemonode/src/NodeUtils.h>
#include <iostream>

using namespace std;

using namespace v8;

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
  std::cerr << "Failed to extract tagged value" << std::endl;
  return false;
}

namespace sail {

namespace {

#define CHECK_CONDITION(expr, str) if(!(expr)) return Nan::ThrowError(str);

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

// These codes are ordered, so that the
// closer we get towards sending a message,
// the lower the code.
enum class N2kSendResult {
  Success,
  N2kError,
  MissingPgn,
  PgnNotSupported,
  BadMessageFormat
};

const char* n2kSendResultToString(N2kSendResult r) {
  switch (r) {
  case N2kSendResult::Success: return "Success";
  case N2kSendResult::N2kError: return "NMEA2000-related error";
  case N2kSendResult::MissingPgn: return "Missing PGN";
  case N2kSendResult::PgnNotSupported: return "PGN not supported";
  case N2kSendResult::BadMessageFormat: return "Bad message format";
  };
  CHECK(false);
  return nullptr;
}

bool extractTypedValue(
    const TaggedValue& src, Angle<double>* dst) {
  if (src.tag == "deg" || src.tag == "" || src.tag == "degrees") {
    *dst = Angle<double>::degrees(src.value);
    return true;
  } else if (src.tag == "rad" || src.tag == "radians") {
    *dst = Angle<double>::radians(src.value);
    return true;
  }
  std::cerr << "Bad angle unit: " << src.tag << std::endl;
  return false;
}

  bool tryExtract(const v8::Local<v8::Value>& val,

                  // Most fields in the PgnClasses
                  // are optional.
                  Optional<Angle<double>>* dst) {
    TaggedValue tmp;
    Angle<double> angle;
    if (tryExtract(val, &tmp) && extractTypedValue(tmp, &angle)) {
      *dst = angle;
      return true;
    }
    return false;
  }

N2kSendResult sendPositionRapidUpdate(
    int32_t deviceIndex,
    const v8::Local<v8::Object>& val,
    Nmea2000Source* dst) {
  
  PgnClasses::PositionRapidUpdate x;
  if (!tryLookUp(val, "longitude", &x.longitude)) {
    return N2kSendResult::BadMessageFormat;
  }
  if (!tryLookUp(val, "latitude", &x.latitude)) {
    return N2kSendResult::BadMessageFormat;
  }

  return dst->send(deviceIndex, x)? 
    N2kSendResult::Success 
    : N2kSendResult::N2kError;
}


N2kSendResult dispatchPgn(
   int64_t pgn,
   const v8::Local<v8::Object>& obj,
   Nmea2000Source* dst) {

  // NOTE: deviceIndex is an implementation detail of 
  // the tNMEA2000 class. Would it be useful to specify the device
  // in some other way?
  int32_t deviceIndex = 0;
  if (!tryLookUp(obj, "deviceIndex", &deviceIndex)) {
    std::cerr << "Missing deviceIndex in message" << std::endl;
    return N2kSendResult::BadMessageFormat;
  }

  switch (pgn) {
  case PgnClasses::PositionRapidUpdate::ThisPgn:
    return sendPositionRapidUpdate(deviceIndex, obj, dst);
  default: return N2kSendResult::PgnNotSupported;
  };
}

// These codes are ordered, so that the
// closer we get towards sending a message,
// the lower the code.
N2kSendResult parseAndSendMessage(
    const v8::Local<v8::Value>& val,
    Nmea2000Source* dst) {
  if (!val->IsObject()) {
    std::cerr << "Message is not an object";
    return N2kSendResult::BadMessageFormat;
  }
  v8::Local<v8::Object> obj =  val->ToObject();

  int32_t pgn = 0;
  return tryLookUp(obj, "pgn", &pgn)?
    dispatchPgn(pgn, obj, dst)
    : N2kSendResult::MissingPgn;
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
    auto result = parseAndSendMessage(msgArray->Get(i), &(zis->_nmea2000));
    if (result != N2kSendResult::Success) {
      std::string emsg = std::string("Failed to send message: ")
        + n2kSendResultToString(result);
      return Nan::ThrowTypeError(emsg.c_str());
    }
  }
}

}  // namespace sail
