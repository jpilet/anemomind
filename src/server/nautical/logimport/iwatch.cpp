
#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Exception.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <fstream>
#include <server/common/logging.h>
#include <server/nautical/logimport/LogAccumulator.h>
#include <server/nautical/logimport/SourceGroup.h>


using Poco::Dynamic::Var;
using Poco::JSON::Object;

namespace sail {

bool parseIwatch(const std::string& filename, LogAccumulator* dst) {
  std::ifstream stream(filename);

  if (!stream) {
    LOG(ERROR) << filename << ": can't read file\n";
    return false;
  }

  // quickly fail if the filetype is wrong.
  char header[2];
  stream.read(header, 2);
  if (!stream.good() || header[0] != '{' || header[1] != '"') {
    return false;
  }
  stream.seekg(0, stream.beg);

  Poco::JSON::Parser parser;
  Var json = parser.parse(stream);
  Object::Ptr object = json.extract<Object::Ptr>();
  if (!object) {
    return false;
  }

  Poco::JSON::Array::Ptr arr = object->getArray("datalogs");
  if (!arr) {
    return false;
  }

  std::string iwatchSource("iWatch");

  bool foundAnObject = false;

  for (size_t i = 0; i < arr->size(); ++i) {
    Object::Ptr obj = arr->getObject(i);
    if (!obj) {
      continue;
    }

    TimeStamp created;
    {
      std::string str;
      try {
        str = obj->getValue<std::string>("createdAt");
      } catch (Poco::Exception& ex) {
        LOG(ERROR) << "Missing or bad \"createdAt\" field:" << ex.displayText();
        return false; // that's a fatal error.
      }

      Poco::DateTime dateTime;
      int tz;
      if (!Poco::DateTimeParser::tryParse(str, dateTime, tz)) {
        LOG(ERROR) << "Can't parse date: " << str;
        return false;
      }
      dateTime.makeUTC(tz);
      created = TimeStamp::fromMilliSecondsSince1970(
          dateTime.timestamp().epochMicroseconds() / 1000);
    }

    if (obj->has("awa")) {
      // this is a wind object
      Angle<> awa;
      Velocity<> aws;
      try {
        awa = Angle<>::degrees(obj->getValue<double>("awa"));
        aws = Velocity<>::metersPerSecond(obj->getValue<double>("aws"));
      } catch (Poco::Exception& ex) {
        LOG(ERROR) << "Bad value when parsing awa or aws: " << ex.displayText();
        continue;
      }

      pushBack(created, awa, &dst->_AWAsources[iwatchSource]);
      pushBack(created, aws, &dst->_AWSsources[iwatchSource]);

      foundAnObject = true;

      Angle<> roll, pitch;
      try {
        roll = Angle<>::degrees(obj->getValue<double>("roll"));
        pitch = Angle<>::degrees(obj->getValue<double>("pitch"));
      } catch (Poco::Exception& ex) {
        LOG(ERROR) << "Bad/missing values for roll and pitch";
        continue;
      }
      pushBack(created, roll, &dst->_ROLLsources[iwatchSource]);
      pushBack(created, pitch, &dst->_PITCHsources[iwatchSource]);

      Angle<> compass;
      try {
        compass = Angle<>::degrees(obj->getValue<double>("compass"));
        pushBack(created, compass, &dst->_MAG_HEADINGsources[iwatchSource]);
      } catch (Poco::Exception& ex) { }
    } else if (obj->has("longitude")) {
      // this is a GPS object
      Angle<> latitude, longitude;
      Velocity<> sog;
      Angle<> cog;
      try {
        latitude = Angle<>::degrees(obj->getValue<double>("latitude"));
        longitude = Angle<>::degrees(obj->getValue<double>("longitude"));
        sog = Velocity<>::metersPerSecond(obj->getValue<double>("speedMps"));
        cog = Angle<>::degrees(obj->getValue<double>("course"));
      } catch (Poco::Exception& ex) {
        LOG(ERROR) << "Bad value when parsing latitude, longitude, sog or cog." << ex.displayText();
        return false;
      }
      foundAnObject = true;
      pushBack(created, sog, &dst->_GPS_SPEEDsources[iwatchSource]);
      pushBack(created, cog, &dst->_GPS_BEARINGsources[iwatchSource]);
      pushBack(created, GeographicPosition<double>(latitude, longitude),
               &dst->_GPS_POSsources[iwatchSource]);
    } else {
     // unknown object type...
    } 
  }
  return foundAnObject;
}

}  // namespace sail
