var anemonode = require('./build/Release/anemonode');

// Internal GPS with output to NMEA0183
require('./components/gps').init(function(buffer) {
  console.log(buffer.toString('ascii'));
});

require('./components/settime.js');
anemonode.dispatcher.values.dateTime.subscribe(function(val) {
  console.log('Got time: ' + val);
});
