#include <device/anemobox/Dispatcher.h>

namespace sail {

Dispatcher *Dispatcher::_globalInstance = 0;

Dispatcher::Dispatcher() :
  _awa(&_data, AWA, "awa", "apparent wind angle"),
  _aws(&_data, AWS, "aws", "apparent wind speed"),
  _twa(&_data, TWA, "twa", "true wind angle"),
  _tws(&_data, TWS, "tws", "true wind speed"),
  _twdir(&_data, TWDIR, "twdir", "true wind direction"),
  _gpsBearing(&_data, GPS_BEARING, "gpsBearing", "GPS bearing"),
  _gpsSpeed(&_data, GPS_SPEED, "gpsSpeed", "GPS speed"),
  _magHeading(&_data, MAG_HEADING, "magHdg", "magnetic heading"),
  _watSpeed(&_data, WAT_SPEED, "watSpeed", "water speed"),
  _watDist(&_data, WAT_DIST, "watDist", "distance over water"),
  _pos(&_data, GPS_POS, "pos", "GPS position"),
  _dateTime(&_data, DATE_TIME, "dateTime", "GPS date and time (UTC)")
{
}

Dispatcher *Dispatcher::global() {
  if (!_globalInstance) {
    _globalInstance = new Dispatcher();
  }
  return _globalInstance;
}

}  // namespace sail
