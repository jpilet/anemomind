#include <device/anemobox/anemonode/src/JsLogger.h>

using namespace v8;

namespace sail {

namespace {

class FlushWorker : public NanAsyncWorker {
 public:
  FlushWorker(NanCallback *callback, std::string folder)
    : NanAsyncWorker(callback),
      _folder(folder),
      _result(false) { }

  LogFile* dataContainer() { return &_data; }

  void Execute () {
    _filename = Logger::nextFilename(_folder);
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
  std::string _folder;
  LogFile _data;
  bool _result;
  std::string _filename;
};

v8::Persistent<v8::FunctionTemplate> logger_constructor;

}  // namespace

JsLogger::JsLogger() : _logger(Dispatcher::global()) { }

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

  NanAssignPersistent<FunctionTemplate>(logger_constructor, tpl);

  target->Set(NanNew<String>("Logger"), tpl->GetFunction());
}

NAN_METHOD(JsLogger::New) {
  NanScope();
  JsLogger *obj = new JsLogger();
  obj->Wrap(args.This());
  NanReturnValue(args.This());
}

NAN_METHOD(JsLogger::flush) {
  NanScope();

  JsLogger* obj = ObjectWrap::Unwrap<JsLogger>(args.This());
  if (!obj) {
    NanThrowTypeError("This is not a Logger");
    NanReturnUndefined();
  }

  if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
    NanThrowTypeError(
        "Bad arguments. "
        "Usage: flush('path', function(writtenFilename, error) {})");
    NanReturnUndefined();
  }

  v8::String::Utf8Value folder(args[0]->ToString());
  NanCallback *callback = new NanCallback(args[1].As<Function>());
  FlushWorker* worker = new FlushWorker(callback, *folder);
  obj->_logger.flushTo(worker->dataContainer());

  NanAsyncQueueWorker(worker);
  NanReturnUndefined();
}

NAN_METHOD(JsLogger::logText) {
  NanScope();

  JsLogger* obj = ObjectWrap::Unwrap<JsLogger>(args.This());
  if (!obj) {
    NanThrowTypeError("This is not a Logger");
    NanReturnUndefined();
  }

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

}  // namespace sail
