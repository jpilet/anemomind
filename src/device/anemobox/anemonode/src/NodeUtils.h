#ifndef __NODE_UTILS_H_
#define __NODE_UTILS_H_

#include <node.h>
#include <nan.h>
#include <server/common/Optional.h>

#define CHECK_CONDITION(expr, str) if(!(expr)) return Nan::ThrowError(str);
#define CHECK_CONDITION_BOOL(expr, str) if(!(expr)) {Nan::ThrowError(str); return false;}

#define TRY_LOOK_UP(src, key, dst) CHECK_CONDITION_BOOL(tryLookUp(src, key, dst), "Missing or malformed '" key "'")

bool tryExtract(const v8::Local<v8::Value>& val, 
                std::string* dst);

bool tryExtract(const v8::Local<v8::Value>& val,
                int32_t* dst);

bool tryExtract(const v8::Local<v8::Value>& val,
                double* dst);

bool tryExtract(const v8::Local<v8::Value>& val,
                uint64_t* dst);

bool tryExtract(const v8::Local<v8::Value>& val,
                int64_t* dst);

template <typename T>
bool tryExtract(const v8::Local<v8::Value>& val,
                Optional<T>* dst) {
  T x;
  if (tryExtract(val, &x)) {
    *dst = x;
    return true;
  } else if (val->IsNull() || val->IsUndefined()) {
    *dst = Optional<T>();
    return true;
  }
  return false;
}


template <typename T>
bool tryLookUp(v8::Local<v8::Object> obj,
            const char* key, 
            T* dst) {
  v8::Local<v8::Value> val = Nan::Get(
      obj, Nan::New<v8::String>(key)
      .ToLocalChecked()).ToLocalChecked();
  return tryExtract(val, dst);
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
