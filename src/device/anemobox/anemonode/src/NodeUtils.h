#ifndef __NODE_UTILS_H_
#define __NODE_UTILS_H_

#include <node.h>
#include <nan.h>

bool tryExtract(const v8::Local<v8::Value>& val, 
                std::string* dst);

bool tryExtract(const v8::Local<v8::Value>& val,
                int32_t* dst);

bool tryExtract(const v8::Local<v8::Value>& val,
                double* dst);

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
