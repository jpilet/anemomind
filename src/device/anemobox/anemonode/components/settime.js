var anemonode = require('../build/Release/anemonode');

var maxSetTime = 1;
var numSetTime = 0;

var subscribers = [];

function notifySubscribers() {
  for (var i = 0; i < subscribers.length; i++) {
    console.log("Notify subscriber %d/%d", i+1, subscribers.length);
    subscribers[i]();
  } 
  if (subscribers.length == 0) {
    console.log("No subscribers");
  }
}

function adjustTimeFromDispatcher() {
  if (numSetTime >= maxSetTime) {
    return;
  }

  // We have 3 time references:
  // - gps: the time measurement from the GPS receiver
  // - sys: the system clock, real time
  // - mono: the monotonic clock used by the dispatcher
  //
  // and we have 2 instants to work with:
  // - measure: the time at which the dateTime measurement has been received
  // - now: when we are execution this function

  var monoMeasureTime = anemonode.dispatcher.values.dateTime.time(0);
  var gpsMeasureTime = anemonode.dispatcher.values.dateTime.value(0);

  if (!monoMeasureTime || !gpsMeasureTime) {
    console.log('time undefined');
    return;
  }

  var monoNow = anemonode.currentTime();
  var sysNow = new Date();

  // The current difference between the system clock and the monotonic clock
  var sysMonoDelta = sysNow.getTime() - monoNow.getTime();

  var sysMeasureTime = new Date(monoMeasureTime.getTime() + sysMonoDelta);

  // getTime returns milliseconds, we want seconds.
  var delta = (gpsMeasureTime - sysMeasureTime) / 1000;
  if (Math.abs(delta) > .2) {
    console.log('Calling adjtime with delta: ' + delta);
    var r= anemonode.adjTime(delta);
    ++numSetTime;
    if (numSetTime >= maxSetTime) {
      anemonode.dispatcher.values.dateTime.unsubscribe(subscriptionIndex);
    }
    notifySubscribers();
  }
}

var subscriptionIndex =
  anemonode.dispatcher.values.dateTime.subscribe(adjustTimeFromDispatcher);

module.exports.subscribe = function(cb) {
  subscribers.push(cb);
}

