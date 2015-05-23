#include <device/anemobox/Dispatcher.h>

namespace sail {

Dispatcher *Dispatcher::_globalInstance = 0;

Dispatcher::Dispatcher() :
  _awa(&_data, AWA, "awa", "apparent wind angle", this),
  _aws(&_data, AWS, "aws", "apparent wind speed", this),
  _twa(&_data, TWA, "twa", "true wind angle", this),
  _tws(&_data, TWS, "tws", "true wind speed", this),
  _twdir(&_data, TWDIR, "twdir", "true wind direction", this),
  _gpsBearing(&_data, GPS_BEARING, "gpsBearing", "GPS bearing", this),
  _gpsSpeed(&_data, GPS_SPEED, "gpsSpeed", "GPS speed", this),
  _magHeading(&_data, MAG_HEADING, "magHdg", "magnetic heading", this),
  _watSpeed(&_data, WAT_SPEED, "watSpeed", "water speed", this),
  _watDist(&_data, WAT_DIST, "watDist", "distance over water", this),
  _pos(&_data, GPS_POS, "pos", "GPS position", this),
  _dateTime(&_data, DATE_TIME, "dateTime", "GPS date and time (UTC)", this)
{
}

Dispatcher *Dispatcher::global() {
  if (!_globalInstance) {
    _globalInstance = new Dispatcher();
  }
  return _globalInstance;
}

}  // namespace sail
