#include <device/anemobox/Dispatcher.h>

namespace sail {

Dispatcher::Dispatcher() :
  _awa(&_data, AWA, "apparent wind angle"),
  _aws(&_data, AWS, "apparent wind speed"),
  _twa(&_data, TWA, "true wind angle"),
  _tws(&_data, TWS, "true wind speed"),
  _twdir(&_data, TWDIR, "true wind direction"),
  _gpsBearing(&_data, GPS_BEARING, "GPS bearing"),
  _gpsSpeed(&_data, GPS_SPEED, "GPS speed"),
  _magHeading(&_data, MAG_HEADING, "magnetic heading"),
  _watSpeed(&_data, WAT_SPEED, "water speed")
{
}

}  // namespace sail
