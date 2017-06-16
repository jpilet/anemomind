#include <device/anemobox/anemonode/src/JsLogger.h>

#include <device/anemobox/anemonode/src/anemonode.h>

using namespace v8;

namespace sail {

namespace {

class FlushWorker : public Nan::AsyncWorker {
 public:
  FlushWorker(Nan::Callback *callback, std::string filename)
    : Nan::AsyncWorker(callback),
      _result(false),
      _filename(filename) { }

  LogFile* dataContainer() { return &_data; }

  void Execute () {
    _result = Logger::save(_filename, _data);
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;
    Handle<Value> argv[2];

    argv[0] = Nan::New(_filename);
    if (!_result) {
      argv[1] = Nan::New("Logger::save failed to write " + _filename);
    } else {
      argv[1] = Nan::Undefined();
    }

    callback->Call(2, argv);
  }

 private:
  LogFile _data;
  bool _result;
  std::string _filename;
};

Nan::Persistent<v8::FunctionTemplate> logger_constructor;

}  // namespace

JsLogger::JsLogger() : _logger(globalAnemonodeDispatcher) { }

void JsLogger::Init(v8::Handle<v8::Object> target) {
  Nan::HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Logger").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  Nan::SetMethod(proto, "flush", JsLogger::flush);
  Nan::SetMethod(proto, "logText", JsLogger::logText);
  Nan::SetMethod(proto, "logRawNmea2000", 
		  JsLogger::logRawNmea2000);

  logger_constructor.Reset(tpl);

  target->Set(Nan::New<String>("Logger").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(JsLogger::New) {
  Nan::HandleScope scope;
  JsLogger *obj = new JsLogger();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

#define GET_TYPED_THIS(TYPE, OBJ)		     \
  TYPE* OBJ = Nan::ObjectWrap::Unwrap<TYPE>(info.This()); \
  if (!OBJ) {					     \
    Nan::ThrowTypeError("This is not a " #TYPE);	     \
    return;			     \
  }						     

NAN_METHOD(JsLogger::flush) {
  Nan::HandleScope scope;
  GET_TYPED_THIS(JsLogger, obj);

  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsFunction()) {
    Nan::ThrowTypeError(
        "Bad arguments. "
        "Usage: flush('path', function(writtenFilename, error) {})");
    return;
  }

  v8::String::Utf8Value filename(info[0]->ToString());
  Nan::Callback *callback = new Nan::Callback(info[1].As<Function>());
  FlushWorker* worker = new FlushWorker(callback, *filename);
  obj->_logger.flushTo(worker->dataContainer());

  Nan::AsyncQueueWorker(worker);
  return;
}

NAN_METHOD(JsLogger::logText) {
  Nan::HandleScope scope;
  GET_TYPED_THIS(JsLogger, obj);

  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsString()) {
    Nan::ThrowTypeError(
        "Bad arguments. "
        "Usage: logText('source', 'content')");
    return;
  }

  v8::String::Utf8Value source(info[0]->ToString());
  v8::String::Utf8Value content(info[1]->ToString());

  obj->_logger.logText(*source, *content);

  return;
}

// Expects as input these arguments:
//   timestampMillisecondsSinceBoot: A number
//   id: Id of the message (a number)
//   data: The data of the message (a buffer)
// https://github.com/jpilet/node-can/commit/4d4019b2b7a7b6c14f550ff02ab99db5e0c148ea

#define LOG_RAW_NMEA2000_USAGE "Usage: logRawNmea2000(timestampMillisecondsSinceBoot: Number, id: Number, data: String)"

  /*
    
    The incoming sentences come with the system time, call it S0, 
    of when the packet arrived to the kernel. Then we receive
    the packet a bit later, when the system time is S1 and the 
    monotonic clock time (time since boot) is M1. To compute the 
    monotonic clock time M0 when the packet arrived (which is the
    value expected by this function), we need to solve w.r.t. M0:

       M1 - M0 = S1 - S0 ,

    which has the solution M0 = M1 - (S1 - S0).

   */

NAN_METHOD(JsLogger::logRawNmea2000) {
  Nan::HandleScope scope;
  GET_TYPED_THIS(JsLogger, obj);
  
  if (info.Length() < 3) {
    Nan::ThrowTypeError("Too few arguments. " LOG_RAW_NMEA2000_USAGE);
    return;
  } 
  if (info.Length() > 3) {
    Nan::ThrowTypeError("Too many arguments. " LOG_RAW_NMEA2000_USAGE);
    return;
  }
  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("'timestampMillisecondsSinceBoot' is not a number. " 
		      LOG_RAW_NMEA2000_USAGE);
    return;
  }
  if (!info[1]->IsNumber()) {
    Nan::ThrowTypeError("'id' is not a number. " LOG_RAW_NMEA2000_USAGE);
    return;
  }
  if (!info[2]->IsObject()) {
    Nan::ThrowTypeError("'data' is not a buffer. " 
		      LOG_RAW_NMEA2000_USAGE);
    return;
  }

  double tsMs = info[0]->ToNumber()->Value();
  double id = info[1]->ToNumber()->Value();

  Local<Object> bufferObj = info[2]->ToObject();
  char* bufferData = node::Buffer::Data(bufferObj);
  size_t bufferLength = node::Buffer::Length(bufferObj);

  obj->_logger.logRawNmea2000(tsMs, id, bufferLength, bufferData);

  return;
}

}  // namespace sail
