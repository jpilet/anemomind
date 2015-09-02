var anemonode = require('./build/Release/anemonode');

// Internal GPS with output to NMEA0183
var gps = require('./components/gps');

setInterval(function() {
  gps.readGps(function(buffer) {
    console.log(buffer.toString('ascii'));
  });
}, 50);

require('./components/settime.js');
anemonode.dispatcher.values.dateTime.subscribe(function(val) {
  console.log('Got time: ' + val);
});
