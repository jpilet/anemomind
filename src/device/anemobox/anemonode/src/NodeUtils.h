#ifndef __NODE_UTILS_H_
#define __NODE_UTILS_H_

inline bool tryExtract(const v8::Local<v8::Value>& val, 
                       std::string* dst) {
  if (val->IsString()) {
    v8::String::Utf8Value utf8(val->ToString());
    *dst = std::string(*utf8);
    return true;
  }
  return false;
}

inline bool tryExtract(const v8::Local<v8::Value>& val,
                       int32_t* dst) {
  if (val->IsNumber()) {
    *dst = val->Int32Value();
    return true;
  }
  return false;
}

inline bool tryExtract(const v8::Local<v8::Value>& val,
                       double* dst) {
  if (val->IsNumber()) {
    *dst = val->NumberValue();
    return true;
  }
  return false;
}

template <typename T>
T lookUpOrDefault(v8::Local<v8::Object> obj,
                  const char* key, 
                  const T& def) {
  T result = def;
  v8::Local<v8::Value> val = Nan::Get(
      obj, Nan::New<v8::String>(key)
      .ToLocalChecked()).ToLocalChecked();
  tryExtract(val, &result);
  return result;
}


#endif // __NODE_UTILS_H_
