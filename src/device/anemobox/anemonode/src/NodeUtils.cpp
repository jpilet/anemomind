#include <device/anemobox/anemonode/src/NodeUtils.h>

bool tryExtract(const v8::Local<v8::Value>& val, 
                       std::string* dst) {
  if (val->IsString()) {
    v8::String::Utf8Value utf8(val->ToString());
    *dst = std::string(*utf8);
    return true;
  }
  return false;
}

bool tryExtract(const v8::Local<v8::Value>& val,
                       int32_t* dst) {
  if (val->IsNumber()) {
    *dst = val->Int32Value();
    return true;
  }
  return false;
}

bool tryExtract(const v8::Local<v8::Value>& val,
                       double* dst) {
  if (val->IsNumber()) {
    *dst = val->NumberValue();
    return true;
  }
  return false;
}
