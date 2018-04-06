/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *  Reinterprets NMEA0183 messages to our format.
 */

#ifndef DEVICE_ANEMOBOX_NMEA0183ADAPTOR_H_
#define DEVICE_ANEMOBOX_NMEA0183ADAPTOR_H_

#ifdef ENABLE_HACKS
#include <server/nautical/BoatSpecificHacks.h>
#endif

#include <iostream>

class NmeaParser;

namespace sail {

inline TimeStamp getTime(const NmeaParser& parser) {
  return parser.timestamp();
}

inline GeographicPosition<double> getPos(const NmeaParser& parser) {
  return GeographicPosition<double>(
      Angle<double>::degrees(parser.pos().lon.toDouble()),
      Angle<double>::degrees(parser.pos().lat.toDouble()));
}

template <typename Handler>
void Nmea0183ProcessByte(const std::string &sourceName,
    unsigned char b, NmeaParser *parser, Handler *handler) {

  switch (parser->processByte(b)) {
    // Nothing
    case NmeaParser::NMEA_NONE: break;

    // These are handled by callbacks, nothing to do.
    case NmeaParser::NMEA_HDM:
    case NmeaParser::NMEA_RUDDER:
    case NmeaParser::NMEA_MWD:
    case NmeaParser::NMEA_ROLL:
    case NmeaParser::NMEA_PITCH:
    case NmeaParser::NMEA_RSA: break;

    case NmeaParser::NMEA_UNKNOWN: break; // Valid sentence, but unknown to us.
    case NmeaParser::NMEA_RMC:
      handler->template add<DATE_TIME>(sourceName, getTime(*parser));
      handler->template add<GPS_BEARING>(sourceName,
                                static_cast<Angle<double>>(parser->gpsBearing()));
      handler->template add<GPS_SPEED>(sourceName,
                                static_cast<Velocity<double>>(parser->gpsSpeed()));
      handler->template add<GPS_POS>(sourceName, getPos(*parser));
      break;
    case NmeaParser::NMEA_AW:
      handler->template add<AWA>(sourceName, static_cast<Angle<double>>(parser->awa()));
      handler->template add<AWS>(sourceName, static_cast<Velocity<double>>(parser->aws()));
      break;
    case NmeaParser::NMEA_TW:
      handler->template add<TWA>(
          sourceName, static_cast<Angle<double>>(parser->twa()));
      handler->template add<TWS>(
          sourceName, static_cast<Velocity<double>>(parser->tws()));
      break;
    case NmeaParser::NMEA_WAT_SP_HDG:
      handler->template add<MAG_HEADING>(
          sourceName, static_cast<Angle<double>>(parser->magHdg()));
      handler->template add<WAT_SPEED>(
          sourceName, static_cast<Velocity<double>>(parser->watSpeed()));
      break;
    case NmeaParser::NMEA_VLW:
      handler->template add<WAT_DIST>(
          sourceName, static_cast<Length<double>>(parser->watDist()));
      break;
    case NmeaParser::NMEA_GLL:
      handler->setTimeOfDay(parser->hour(), parser->min(), parser->sec());
#ifdef ENABLE_HACKS
#ifdef ON_SERVER
      if (hack::forceDateForGLL) {
        TimeStamp t = TimeStamp::tryUTC(
            2017, 9, 9, parser->hour(), parser->min(), parser->sec())
          + double(hack::bootCount)*1.0_days;
        if (t.defined()) {
          handler->template add<DATE_TIME>(sourceName, t);
        }
      }
#endif
#endif
      handler->template add<GPS_POS>(sourceName, getPos(*parser));
      break;
    case NmeaParser::NMEA_VTG:
      handler->template add<GPS_BEARING>(
          sourceName, static_cast<Angle<double>>(parser->gpsBearing()));
      handler->template add<GPS_SPEED>(
          sourceName, static_cast<Velocity<double>>(parser->gpsSpeed()));
      break;
    case NmeaParser::NMEA_ZDA:
      handler->template add<DATE_TIME>(sourceName, getTime(*parser));
      break;
  }
}

}



#endif /* DEVICE_ANEMOBOX_NMEA0183ADAPTOR_H_ */
