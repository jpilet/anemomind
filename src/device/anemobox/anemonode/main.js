var nmea0183PortPath = '/dev/ttyMFD1';

var btle = require('./components/AnemoServiceBTLE');
var config = require('./components/config');
var nmea0183port = require('./components/nmea0183port');

btle.startBTLE();

nmea0183port.init(nmea0183PortPath);
