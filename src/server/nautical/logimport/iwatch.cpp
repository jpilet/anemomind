
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
      double rawLat, rawLon, rawSpeed, rawCog;
      try {
        rawLat = obj->getValue<double>("latitude");
        rawLon = obj->getValue<double>("longitude");
        rawSpeed = obj->getValue<double>("speedMps");
        rawCog = obj->getValue<double>("course");
      } catch (Poco::Exception& ex) {
        LOG(ERROR) << "Bad value when parsing latitude, longitude, sog or cog." << ex.displayText();
        return false;
      }
      if (rawLat != -1 && rawLon != -1) {
        Angle<> latitude = Angle<>::degrees(rawLat);
        Angle<> longitude = Angle<>::degrees(rawLon);
        pushBack(created, GeographicPosition<double>(longitude, latitude),
                 &dst->_GPS_POSsources[iwatchSource]);
        foundAnObject = true;
      }
      if (rawSpeed != -1) {
        Velocity<> sog = Velocity<>::metersPerSecond(rawSpeed);
        pushBack(created, sog, &dst->_GPS_SPEEDsources[iwatchSource]);
        foundAnObject = true;
      }
      if (rawCog != -1) {
        Angle<> cog = Angle<>::degrees(rawCog);
        pushBack(created, cog, &dst->_GPS_BEARINGsources[iwatchSource]);
        foundAnObject = true;
      }
    } else {
     // unknown object type...
    } 
  }
  return foundAnObject;
}

}  // namespace sail
