/*
 * DispatcherJson.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <device/anemobox/DispatcherJson.h>
#include <device/anemobox/DispatcherUtils.h>
#include <Poco/JSON/Stringifier.h>


namespace sail {
namespace json {

class DispatcherChannelJsonVisitor {
 public:

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
      // TODO: write your code here
  }
};

Poco::Dynamic::Var makeEmptyObject() {
  return Poco::Dynamic::Var(Poco::JSON::Object::Ptr(
      new Poco::JSON::Object()));
}

template <typename T>
struct TypeSerializer {
  static bool const implemented = false;
  static const char *unitTag() {return "notImplemented";}
  static Poco::Dynamic::Var serialize(T) {return makeEmptyObject();}
};

template <>
struct TypeSerializer<Angle<double> > {
  static bool const implemented = true;
  static const char *unitTag() {return "radians";}
  static Poco::Dynamic::Var serialize(Angle<double> x) {
    return x.radians();
  }
};

template <>
struct TypeSerializer<Velocity<double> > {
  static bool const implemented = true;
  static const char *unitTag() {return "metersPerSecond";}
  static Poco::Dynamic::Var serialize(Velocity<double> x) {
    return x.metersPerSecond();
  }
};

template <>
struct TypeSerializer<Length<double> > {
  static bool const implemented = true;
  static const char *unitTag() {return "meters";}
  static Poco::Dynamic::Var serialize(Length<double> x) {
    return x.meters();
  }
};

// TODO: The other datatypes too.


template <typename T>
Poco::Dynamic::Var serializeTimedValue(const TimedValue<T> &x) {
  Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
  arr->add(x.time.toIso8601String());
  arr->add(TypeSerializer<T>::serialize(x.value));
  return Poco::Dynamic::Var(arr);
}

template <typename T>
Poco::Dynamic::Var serializeData(const TimedSampleCollection<T> &samples) {
  Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
  for (auto x: samples) {
    arr->add(serializeTimedValue<T>(x));
  }
  return Poco::Dynamic::Var(arr);
}

template <typename T>
Poco::Dynamic::Var serializeForType(std::shared_ptr<DispatchData> d) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
  x->set("unit", std::string(TypeSerializer<T>::unitTag()));
  if (TypeSerializer<T>::implemented) {
    x->set("data", serializeData<T>(dynamic_cast<TypedDispatchData<T>*>(d.get())
        ->dispatcher()->values()));
  }
  return Poco::Dynamic::Var(x);
}

Poco::Dynamic::Var serialize(DataCode code, std::shared_ptr<DispatchData> d) {
  switch (code) {
#define SERIALIZE_FOR_TYPE(handle, code, shortname, type, description) \
  case handle: return serializeForType<type>(d);
  FOREACH_CHANNEL(SERIALIZE_FOR_TYPE)
#undef SERIALIZE_FOR_TYPE
  default:
    return makeEmptyObject();
  };
  return makeEmptyObject();
}

Poco::Dynamic::Var serialize(
    DataCode code,
    const std::map<std::string, std::shared_ptr<DispatchData> > &m) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
  for (auto kv: m) {
    x->set(kv.first, serialize(code, kv.second));
  }
  return Poco::Dynamic::Var(x);
}

Poco::Dynamic::Var serialize(const Dispatcher *d) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
  auto m = d->allSources();
  for (auto ch: m) {
    x->set(wordIdentifierForCode(ch.first), serialize(ch.first, ch.second));
  }
  return Poco::Dynamic::Var(x);
}

void outputJson(const Dispatcher *d, std::ostream *dst) {
  Poco::JSON::Stringifier::stringify(
      serialize(d),
      *dst, 0, 0);
}


}
}

