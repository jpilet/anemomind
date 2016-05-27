
#include <device/anemobox/logger/Logger.h>
#include <server/common/TimeStamp.h>
#include <server/common/string.h>
#include <server/common/logging.h>

#include <iostream>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <google/protobuf/io/gzip_stream.h>

using namespace google::protobuf::io;
using namespace boost::iostreams;
using namespace boost::iostreams::gzip;
using namespace std;

namespace sail {

Logger::Logger(Dispatcher* dispatcher) :
    _dispatcher(dispatcher) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  _newDispatchDataListener = dispatcher->newDispatchData.connect(
      [=](DispatchData* ptr) { this->subscribeToDispatcher(ptr); });
  subscribe();
}

void Logger::flushTo(LogFile* container) {
  // Clear content.
  *container = LogFile();
 
  for (auto ptr : _listeners) {
    if (ptr->valueSet().timestamps_size() > 0) {
      // the priority is set every time flushTo is called, since the 
      // priority in the dispatcher can change.
      // We do not log exactly when the priority changed, though.
      // Thus, it is wise to flush right before changing a priority.
      ptr->setPriority(_dispatcher);
      container->add_stream()->Swap(ptr->mutable_valueSet());
    }
    ptr->clear();
  }
  for (auto it = _textLoggers.begin();  it != _textLoggers.end(); ++it) {
    if (it->second.valueSet().timestamps_size() > 0) {
      container->add_text()->Swap(it->second.mutable_valueSet());
    }
    it->second.clear();
  }
}

std::string Logger::nextFilename(const std::string& folder) {
  return folder + int64ToHex(TimeStamp::now().toSecondsSince1970()) + ".log";
}

bool Logger::flushAndSave(const std::string& folder,
                          std::string *savedFilename) {
  LogFile container;

  flushTo(&container);

  if (container.stream_size() == 0 && container.text_size() == 0) {
    // nothing to save.
    return false;
  }

  std::string filename = nextFilename(folder);

  if (!save(filename, container)) {
    LOG(ERROR) << "Failed to save log file.";
    return false;
  }
  if (savedFilename) {
    *savedFilename = filename;
  }
  return true;
}

void Logger::subscribe() {
  _listeners.clear();

  for (auto sourcesForCode : _dispatcher->allSources()) {
    for (auto sourceAndDispatcher : sourcesForCode.second) {
      subscribeToDispatcher(sourceAndDispatcher.second.get());
    }
  }
}

void Logger::subscribeToDispatcher(DispatchData *d) {
  LoggerValueListener* listener =
    new LoggerValueListener(d->wordIdentifier(), d->source());
  SubscribeVisitor<LoggerValueListener> subscriber(listener);
  d->visit(&subscriber);

  _listeners.push_back(std::shared_ptr<LoggerValueListener>(listener));
}

void Logger::logText(const std::string& streamName, const std::string& content) {
  auto it = _textLoggers.find(streamName);
  if (it == _textLoggers.end()) {
    it = _textLoggers.insert(
        make_pair(streamName, LoggerValueListener("text", streamName))).first;
  }
  it->second.addText(content);
}

bool Logger::save(const std::string& filename, const LogFile& data) {
  filtering_ostream out; 
  out.push(gzip_compressor(9)); 
  out.push(file_sink(filename, ios_base::out | ios_base::binary));
  return data.SerializeToOstream(&out);
}

bool Logger::read(const std::string& filename, LogFile *dst) {
    ifstream file(filename, ios_base::in | ios_base::binary);
    filtering_istream in;
    in.push(gzip_decompressor());
    in.push(file);
    dst->Clear();
    return dst->ParseFromIstream(&in);
}

void Logger::unpack(const AngleValueSet& values, std::vector<Angle<double>>* angles) {
  angles->clear();
  angles->reserve(values.deltaangle_size());

  int accumulate = 0;
  for (int i = 0; i < values.deltaangle_size(); ++i) {
    accumulate += values.deltaangle(i);
    angles->push_back(Angle<double>::degrees(accumulate / 100.0));
  }
}

void Logger::unpack(const VelocityValueSet& values,
                    std::vector<Velocity<double>>* result) {
  result->clear();
  result->reserve(values.deltavelocity_size());

  int accumulate = 0;
  for (int i = 0; i < values.deltavelocity_size(); ++i) {
    accumulate += values.deltavelocity(i);
    result->push_back(Velocity<double>::knots(accumulate / 100.0));
  }
}

void Logger::unpack(const LengthValueSet& values,
                    std::vector<Length<double>>* result) {
  result->clear();
  result->reserve(values.deltalength_size());

  int accumulate = 0;
  for (int i = 0; i < values.deltalength_size(); ++i) {
    accumulate += values.deltalength(i);
    result->push_back(Length<double>::meters(accumulate));
  }
}

void Logger::unpack(const GeoPosValueSet& values,
                    std::vector<GeographicPosition<double>>* result) {
  result->clear();
  result->reserve(values.pos_size());

  for (int i = 0; i < values.pos_size(); ++i) {
    const GeoPosValueSet_Pos& pos = values.pos(i);
    result->push_back(GeographicPosition<double>(
            Angle<double>::degrees(pos.lon()),
            Angle<double>::degrees(pos.lat())));
  }
}

void Logger::unpack(const AbsOrientValueSet& values,
                    std::vector<AbsoluteOrientation>* result) {
  std::vector<Angle<double>> heading;
  std::vector<Angle<double>> roll;
  std::vector<Angle<double>> pitch;
  
  unpack(values.heading(), &heading);
  unpack(values.roll(), &roll);
  unpack(values.pitch(), &pitch);

  int size = values.heading().deltaangle_size();
  result->resize(size);
  for (int i = 0; i < size; ++i) {
    AbsoluteOrientation *entry = &((*result)[i]);
    entry->heading = heading[i];
    entry->pitch = pitch[i];
    entry->roll = roll[i];
  }
}

void Logger::unpack(const google::protobuf::RepeatedField<long int> &times,
                        std::vector<TimeStamp>* result) {
  result->clear();
  result->reserve(times.size());

  int64_t time = 0;
  for (int i = 0; i < times.size(); ++i) {
    time += times.Get(i);
    result->push_back(TimeStamp::fromMilliSecondsSince1970(time));
  }
}

void Logger::unpackTime(const ValueSet& valueSet,
                        std::vector<TimeStamp>* result) {
  result->clear();
  result->reserve(valueSet.timestamps_size());

  int64_t time = 0;
  for (int i = 0; i < valueSet.timestamps_size(); ++i) {
    time += valueSet.timestamps(i);
    result->push_back(TimeStamp::fromMilliSecondsSince1970(time));
  }
}


}  // namespace sail
