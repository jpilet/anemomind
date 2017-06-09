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
//   timestampMillisecondsSinceBoot: A number
//   id: Id of the message (a number)
//   data: The data of the message (a string (not a buffer!))
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
  NanScope();
  GET_TYPED_THIS(JsLogger, obj);
  
  if (args.Length() < 3) {
    NanThrowTypeError("Too few arguments. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  } 
  if (args.Length() > 3) {
    NanThrowTypeError("Too many arguments. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }
  if (!args[0]->IsNumber()) {
    NanThrowTypeError("'timestampMillisecondsSinceBoot' is not a number. " 
		      LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }
  if (!args[1]->IsNumber()) {
    NanThrowTypeError("'id' is not a number. " LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }
  if (!args[2]->IsObject()) {
    NanThrowTypeError("'data' is not a buffer. " 
		      LOG_RAW_NMEA2000_USAGE);
    NanReturnUndefined();
  }

  double tsMs = args[0]->ToNumber()->Value();
  double id = args[1]->ToNumber()->Value();

  Local<Object> bufferObj = args[2]->ToObject();
  char* bufferData = node::Buffer::Data(bufferObj);
  size_t bufferLength = node::Buffer::Length(bufferObj);

  obj->_logger.logRawNmea2000(tsMs, id, bufferLength, bufferData);

  NanReturnUndefined();
}

}  // namespace sail
