/* julien@anemomind.com, 4.2015
 *
 * This node module declares a 'dispatcher' object with:
 * dispatcher{
 *   entries: {
 *     dataMeaning (Number) : {
 *       description (String),
 *       unit (String)
 *       type (String: 'angle' or 'velocity')
 *       value(index) (returns a Number. index defaults to 0: the last measure)
 *       timestamp(index) (returns a Date. index defaults to 0, the last measure)
 *       length() (returns the number of stored measures)
 *       setValue(source, x) (source: string, x: Number)
 *       subscribe(function(value))  // adds a listener, returns an index
 *       unsubscribe(index)  // the argument is the returned value of subscribe()
 *       dataCode
 *     }
 *   },
 *   run()
 * }
 */
#include <node.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/Nmea0183Source.h>

#include <iostream>
#include <vector>

#include <fcntl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>

using namespace sail;
using namespace v8;

namespace {

bool run(const char *filename, Duration<> timeout) {
  Nmea0183Source nmea0183(Dispatcher::global());

  if (!nmea0183.open(filename)) {
    perror(filename);
    return false;
  }

  TimeStamp startTime = TimeStamp::now();

  struct pollfd pfds[1];
  pfds[0].fd = nmea0183.fd();
  pfds[0].events = POLLIN;

  while (timeout.seconds() == 0 || (TimeStamp::now() - startTime) <= timeout) {
    int r = poll(pfds, sizeof(pfds) / sizeof(pfds[0]), -1);
    if (r > 0) {
      nmea0183.poll();
    } else {
      if (errno != EAGAIN) {
        perror("poll");
        break;
      }
      break;
    }
  }
  return true;
}

Handle<Value> Run(const Arguments& args) {
  String::Utf8Value param1(args[0]->ToString());
  std::string path = std::string(*param1); 
  Duration<> timeout = Duration<>::seconds(0);
  if (args.Length() > 1 && args[1]->IsNumber()) {
    timeout = Duration<>::seconds(args[1]->ToNumber()->Value());
  }

  return Boolean::New(run(path.c_str(), timeout));
}

class GetValueVisitor : public DispatchDataVisitor {
 public:
  GetValueVisitor(unsigned index) : index_(index), valid_(false), value_(0) { }
  virtual void run(DispatchAngleData *angle) {
    auto values = angle->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values[index_];
      value_ = val.value.degrees();
      timestamp_ = val.time;
    }
  }
  virtual void run(DispatchVelocityData *velocity) {
    auto values = velocity->dispatcher()->values();
    valid_ = values.size() > index_;
    if (valid_) {
      auto val = values[index_];
      value_ = val.value.knots();
      timestamp_ = val.time;
    }
  }

  double value() const { return value_; }
  TimeStamp time() const { return timestamp_; }
  bool valid() const { return valid_; }

 private:
  unsigned index_;
  bool valid_;
  double value_;
  TimeStamp timestamp_;
};

Handle<Value> evalEntry(const Arguments& args) {
  DispatchData *dispatchData = (DispatchData*)External::Unwrap(args.Data());
  unsigned index = 0;
  if (args.Length() >= 1) {
    if (!args[0]->IsNumber() || args[0]->ToInteger()->Value() < 0) {
      return ThrowException(Exception::TypeError(
              String::New("the index should be a positive integer")));
    }
    index = args[0]->ToInteger()->Value();
  }

  GetValueVisitor getValue(index);

  dispatchData->visit(&getValue);

  if (getValue.valid()) {
    return Number::New(getValue.value());
  } else {
    return Undefined();
  }
}

Handle<Value> entryTime(const Arguments& args) {
  DispatchData *dispatchData = (DispatchData*)External::Unwrap(args.Data());
  unsigned index = 0;
  if (args.Length() >= 1) {
    if (!args[0]->IsNumber() || args[0]->ToInteger()->Value() < 0) {
      return ThrowException(Exception::TypeError(
              String::New("the index should be a positive integer")));
    }
    index = args[0]->ToInteger()->Value();
  }

  GetValueVisitor getValue(index);

  dispatchData->visit(&getValue);

  if (getValue.valid()) {
    return Date::New(double(getValue.time().toMilliSecondsSince1970()));
  } else {
    return Undefined();
  }
}


class CountValuesVisitor : public DispatchDataVisitor {
 public:
  CountValuesVisitor() : count_(0) { }

  virtual void run(DispatchAngleData *angle) {
    count_ = angle->dispatcher()->values().size();
  }
  virtual void run(DispatchVelocityData *v) {
    count_ = v->dispatcher()->values().size();
  }
  int numValues() const { return count_; }

 private:
  int count_;
};

Handle<Value> length(const Arguments& args) {
  DispatchData *dispatchData = (DispatchData*)External::Unwrap(args.Data());
  CountValuesVisitor countValues;
  dispatchData->visit(&countValues);

  return Number::New(countValues.numValues());
}

class SetValueVisitor : public DispatchDataVisitor {
 public:
  SetValueVisitor(std::string src, Handle<Value> val)
    : source_(src), value_(val), success_(false) { }

  virtual void run(DispatchAngleData *angle) {
    if (checkNumberAndSetSuccess()) {
      angle->publishValue(
          source_.c_str(),
          Angle<double>::degrees(value_->ToNumber()->Value()));
    }
  }
  virtual void run(DispatchVelocityData *velocity) {
    if (checkNumberAndSetSuccess()) {
      velocity->publishValue(
          source_.c_str(),
          Velocity<double>::knots(value_->ToNumber()->Value()));
    }
  }

  bool checkNumberAndSetSuccess() {
    success_ = value_->IsNumber();
    return success_;
  }
    
  bool success() const { return success_; }
 private:
  std::string source_;
  Handle<Value> value_;
  bool success_;
};

Handle<Value> setValue(const Arguments& args) {
  DispatchData *dispatchData = (DispatchData*)External::Unwrap(args.Data());

  if (args.Length() != 2) {
    return ThrowException(
        Exception::TypeError(
            String::New("setValue expects 2 argument: source name and value")));
  }

  if (!args[0]->IsString()) {
    return ThrowException(
        Exception::TypeError(
            String::New("setValue a source name string as first argument")));
  }

  v8::String::Utf8Value sourceName(args[0]->ToString());
  SetValueVisitor setValue(*sourceName, args[1]);
  dispatchData->visit(&setValue);

  if (!setValue.success()) {
    return ThrowException(
        Exception::TypeError(
            String::New("failed to convert or set value")));
  }

  return Undefined();
}

class JsListener: public Listener<Angle<double>>, public Listener<Velocity<double>> {
 public:
  JsListener(DispatchData *dispatchData,
             Persistent<Function> callback,
             Duration<> minInterval)
    : Listener<Angle<double>>(minInterval),
    Listener<Velocity<double>>(minInterval),
    dispatchData_(dispatchData), callback_(callback) { }

  virtual void onNewValue(const ValueDispatcher<Angle<double>> &) { valueChanged(); }
  virtual void onNewValue(const ValueDispatcher<Velocity<double>> &) { valueChanged(); }

  void valueChanged() {
    GetValueVisitor getValue(0);
    dispatchData_->visit(&getValue);

    Local<Value> argv[1] = { Local<Value>::New(Number::New(getValue.value())) };
    callback_->Call(Context::GetCurrent()->Global(), 1, argv);
  }

 private:
  DispatchData *dispatchData_;
  Persistent<Function> callback_;
};

std::map<int, JsListener *> registeredCallbacks;

class SubscribeVisitor : public DispatchDataVisitor {
 public:
  SubscribeVisitor(JsListener *listener) : listener_(listener) { }

  virtual void run(DispatchAngleData *angle) {
    angle->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchVelocityData *velocity) {
    velocity->dispatcher()->subscribe(listener_);
  }

 private:
  JsListener *listener_;
};

Handle<Value> subscribe(const Arguments& args) {
  HandleScope scope;

  DispatchData *dispatchData = (DispatchData*)External::Unwrap(args.Data());
  if (args.Length() < 1 || !args[0]->IsFunction()) {
    return ThrowException(
        Exception::TypeError(String::New("First argument must be a function"))
    );
  }

  Local<Function> cb = Local<Function>::Cast(args[0]);

  Duration<> minInterval = Duration<>::seconds(0);
  if (args.Length() >= 2) {
    minInterval = Duration<>::seconds(args[1]->ToNumber()->Value());
  }
  JsListener *listener = new JsListener(
      dispatchData, Persistent<Function>::New(cb), minInterval);
  int index = registeredCallbacks.size() + 1;
  registeredCallbacks[index] = listener;

  SubscribeVisitor subscriber(listener);
  dispatchData->visit(&subscriber);
  return scope.Close(Integer::New(index));
}

Handle<Value> unsubscribe(const Arguments& args) {
  auto exception = Exception::TypeError(
      String::New("First argument must be a subscribe index"));

  if (args.Length() < 1) {
    return ThrowException(exception);
  }

  int index = args[0]->ToInteger()->Value();
  auto iterator = registeredCallbacks.find(index);
  if (iterator == registeredCallbacks.end()) {
    return ThrowException(exception);
  }

  delete iterator->second;
  registeredCallbacks.erase(iterator);

  return Undefined();
}

class GetTypeAndUnitVisitor : public DispatchDataVisitor {
 public:
  virtual void run(DispatchAngleData *) {
    type_ = "angle";
    unit_ = "degrees";
  }
  virtual void run(DispatchVelocityData *) {
    type_ = "velocity";
    unit_ = "knots";
  }

  const std::string& type() const { return type_; }
  const std::string& unit() const { return unit_; }

 private:
  std::string type_;
  std::string unit_;
};

void registerDispatcher(Dispatcher *dispatcher, Handle<Object> target) {

  Handle<Object> entries = Object::New();
  for (auto entry : dispatcher->data()) {
    Handle<Object> jsentry = Object::New();
    jsentry->Set(
        String::NewSymbol("description"),
        String::New(entry.second->description().c_str()));

    GetTypeAndUnitVisitor typeAndUnit;
    entry.second->visit(&typeAndUnit);

    jsentry->Set(String::NewSymbol("type"), String::New(typeAndUnit.type().c_str()));
    jsentry->Set(String::NewSymbol("unit"), String::New(typeAndUnit.unit().c_str()));

    jsentry->Set(String::NewSymbol("dataCode"), Integer::New(entry.second->dataCode()));

    jsentry->Set(
        String::NewSymbol("length"),
        FunctionTemplate::New(length, External::Wrap(entry.second))->GetFunction());

    jsentry->Set(
        String::NewSymbol("value"),
        FunctionTemplate::New(evalEntry, External::Wrap(entry.second))->GetFunction());

    jsentry->Set(
        String::NewSymbol("time"),
        FunctionTemplate::New(entryTime, External::Wrap(entry.second))->GetFunction());

    jsentry->Set(
        String::NewSymbol("setValue"),
        FunctionTemplate::New(setValue, External::Wrap(entry.second))->GetFunction());

    jsentry->Set(
        String::NewSymbol("subscribe"),
        FunctionTemplate::New(subscribe, External::Wrap(entry.second))->GetFunction());

    jsentry->Set(
        String::NewSymbol("unsubscribe"),
        FunctionTemplate::New(unsubscribe, External::Wrap(entry.second))->GetFunction());

    entries->Set(String::New(entry.second->wordIdentifier().c_str()), jsentry);
  }
    
  target->Set(String::NewSymbol("entries"), entries);

  target->Set(String::NewSymbol("run"),
        FunctionTemplate::New(Run)->GetFunction());
}

}  // namespace

void RegisterModule(Handle<Object> target) {
  registerDispatcher(Dispatcher::global(), target);
}

// Register the module with node. Note that "modulename" must be the same as
// the basename of the resulting .node file. You can specify that name in
// binding.gyp ("target_name"). When you change it there, change it here too.
NODE_MODULE(anemonode, RegisterModule);
