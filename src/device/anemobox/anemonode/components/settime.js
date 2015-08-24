var anemonode = require('../build/Release/anemonode');

function adjustTimeFromDispatcher() {
  var sysTime = anemonode.dispatcher.values.dateTime.time(0);
  var gpsTime = anemonode.dispatcher.values.dateTime.value(0);
  if (!sysTime || !gpsTime) {
    console.log('time undefined');
    return;
  }
  // getTime returns milliseconds, we want seconds.
  var delta = (gpsTime.getTime() - sysTime.getTime()) / 1000.0;
  if (Math.abs(delta) > .2) {
    console.log('Calling adjtime with delta: ' + delta);
    var r= anemonode.adjTime(delta);
  }
}
anemonode.dispatcher.values.dateTime.subscribe(adjustTimeFromDispatcher);

