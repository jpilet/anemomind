/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *  Reinterprets NMEA0183 messages to our format.
 */

#ifndef DEVICE_ANEMOBOX_NMEA0183ADAPTOR_H_
#define DEVICE_ANEMOBOX_NMEA0183ADAPTOR_H_

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
void Nmea0183ProcessByte(unsigned char b, NmeaParser *parser, Handler *handler) {
  switch (parser->processByte(b)) {
    case NmeaParser::NMEA_NONE:
    case NmeaParser::NMEA_UNKNOWN: break;
    case NmeaParser::NMEA_RMC:
      handler->template add<DATE_TIME>(*parser, getTime(*parser));
      handler->template add<GPS_BEARING>(*parser,
                                static_cast<Angle<double>>(parser->gpsBearing()));
      handler->template add<GPS_SPEED>(*parser,
                                static_cast<Velocity<double>>(parser->gpsSpeed()));
      handler->template add<GPS_POS>(*parser, getPos(*parser));
      break;
    case NmeaParser::NMEA_AW:
      handler->template add<AWA>(*parser, static_cast<Angle<double>>(parser->awa()));
      handler->template add<AWS>(*parser, static_cast<Velocity<double>>(parser->aws()));
      break;
    case NmeaParser::NMEA_TW:
      handler->template add<TWA>(*parser,
          static_cast<Angle<double>>(parser->twa()));
      handler->template add<TWS>(*parser,
          static_cast<Velocity<double>>(parser->tws()));
      break;
    case NmeaParser::NMEA_WAT_SP_HDG:
      handler->template add<MAG_HEADING>(*parser,
          static_cast<Angle<double>>(parser->magHdg()));
      handler->template add<WAT_SPEED>(*parser,
          static_cast<Velocity<double>>(parser->watSpeed()));
      break;
    case NmeaParser::NMEA_VLW: break;
      handler->template add<WAT_DIST>(*parser,
          static_cast<Length<double>>(parser->watDist()));
    case NmeaParser::NMEA_GLL:
      handler->template add<GPS_POS>(*parser, getPos(*parser));
      break;
    case NmeaParser::NMEA_VTG:
      handler->template add<GPS_BEARING>(*parser,
          static_cast<Angle<double>>(parser->gpsBearing()));
      handler->template add<GPS_SPEED>(*parser,
          static_cast<Velocity<double>>(parser->gpsSpeed()));
      break;
    case NmeaParser::NMEA_ZDA:
      handler->template add<DATE_TIME>(*parser, getTime(*parser));
      break;
  }
}

}



#endif /* DEVICE_ANEMOBOX_NMEA0183ADAPTOR_H_ */
