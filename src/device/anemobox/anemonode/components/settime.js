var anemonode = require('../build/Release/anemonode');

var maxSetTime = 1;
var numSetTime = 0;

function adjustTimeFromDispatcher() {
  if (numSetTime >= maxSetTime) {
    return;
  }
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
    ++numSetTime;
    if (numSetTime >= maxSetTime) {
      anemonode.dispatcher.values.dateTime.unsubscribe(subscriptionIndex);
    }
  }
}
var subscriptionIndex =
  anemonode.dispatcher.values.dateTime.subscribe(adjustTimeFromDispatcher);

