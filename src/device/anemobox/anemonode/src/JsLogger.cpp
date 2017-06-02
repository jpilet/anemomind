#include <device/anemobox/anemonode/src/JsLogger.h>

#include <device/anemobox/anemonode/src/anemonode.h>

using namespace v8;

namespace sail {

namespace {

class FlushWorker : public NanAsyncWorker {
 public:
  FlushWorker(NanCallback *callback, std::string filename)
    : NanAsyncWorker(callback),
      _result(false),
      _filename(filename) { }

  LogFile* dataContainer() { return &_data; }

  void Execute () {
    _result = Logger::save(_filename, _data);
  }

  void HandleOKCallback() {
    NanScope();
    Handle<Value> argv[2];

    argv[0] = NanNew(_filename);
    if (!_result) {
      argv[1] = NanNew("Logger::save failed to write " + _filename);
    } else {
      argv[1] = NanUndefined();
    }

    callback->Call(2, argv);
  }

 private:
  LogFile _data;
  bool _result;
  std::string _filename;
};

v8::Persistent<v8::FunctionTemplate> logger_constructor;

}  // namespace

JsLogger::JsLogger() : _logger(globalAnemonodeDispatcher) { }

void JsLogger::Init(v8::Handle<v8::Object> target) {
  NanScope();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew<String>("Logger"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  NODE_SET_METHOD(proto, "flush", JsLogger::flush);
  NODE_SET_METHOD(proto, "logText", JsLogger::logText);
  NODE_SET_METHOD(proto, "logRawNmea2000", 
		  JsLogger::logRawNmea2000);

  NanAssignPersistent<FunctionTemplate>(logger_constructor, tpl);

  target->Set(NanNew<String>("Logger"), tpl->GetFunction());
}

NAN_METHOD(JsLogger::New) {
  NanScope();
  JsLogger *obj = new JsLogger();
  obj->Wrap(args.This());
  NanReturnValue(args.This());
}

#define GET_TYPED_THIS(TYPE, OBJ)		     \
  TYPE* OBJ = ObjectWrap::Unwrap<TYPE>(args.This()); \
  if (!OBJ) {					     \
    NanThrowTypeError("This is not a " #TYPE);	     \
    NanReturnUndefined();			     \
  }						     

NAN_METHOD(JsLogger::flush) {
  NanScope();
  GET_TYPED_THIS(JsLogger, obj);

  if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
    NanThrowTypeError(
        "Bad arguments. "
        "Usage: flush('path', function(writtenFilename, error) {})");
    NanReturnUndefined();
  }

  v8::String::Utf8Value filename(args[0]->ToString());
  NanCallback *callback = new NanCallback(args[1].As<Function>());
  FlushWorker* worker = new FlushWorker(callback, *filename);
  obj->_logger.flushTo(worker->dataContainer());

  NanAsyncQueueWorker(worker);
  NanReturnUndefined();
}

NAN_METHOD(JsLogger::logText) {
  NanScope();
  GET_TYPED_THIS(JsLogger, obj);

  if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
    NanThrowTypeError(
        "Bad arguments. "
        "Usage: logText('source', 'content')");
    NanReturnUndefined();
  }

  v8::String::Utf8Value source(args[0]->ToString());
  v8::String::Utf8Value content(args[1]->ToString());

  obj->_logger.logText(*source, *content);

  NanReturnUndefined();
}

// Expects as input these arguments:
//   ts_sec: Time stamp in seconds (a number)
//   ts_usec: Additional microseconds, to add to timestamp (a number)
//   id: Id of the message (a number)
//   data: The data of the message (a string (not a buffer!))
// https://github.com/jpilet/node-can/commit/4d4019b2b7a7b6c14f550ff02ab99db5e0c148ea

#define LOG_RAW_NMEA2000_USAGE "Usage: logRawNmea2000(ts_sec: Number, ts_usec: Number, id: Number, data: String)"

NAN_METHOD(JsLogger::logRawNmea2000) {
  NanScope();
  GET_TYPED_THIS(JsLogger, obj);
  
  if (args.Length() < 4) {
    NanThrowTypeError("Too few arguments. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }
  if (!args[0]->IsNumber()) {
    NanThrowTypeError("'ts_sec' is not a number. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }
  if (!args[1]->IsNumber()) {
    NanThrowTypeError("'ts_usec' is not a number. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }
  if (!args[2]->IsNumber()) {
    NanThrowTypeError("'id' is not a number. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }
  if (!args[3]->IsString()) {
    NanThrowTypeError("'data' is not a string. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }

  double ts_sec = args[0]->ToNumber()->Value();
  double ts_usec = args[1]->ToNumber()->Value();
  double id = args[2]->ToNumber()->Value();
  v8::String::Utf8Value data(args[3]->ToString());

  obj->_logger.logRawNmea2000(ts_sec,
			      ts_usec,
			      id, *data);

  NanReturnUndefined();
}

}  // namespace sail
