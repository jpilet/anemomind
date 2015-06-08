var assert = require('assert');

function getTimeMillis() {
  return (new Date()).getTime();
}

// A class to help delaying the call to a specific function.
// If a new delayed call is initiated while there is already
// a pending call, the the new call with be ignored if it
// occurs sooner than the already pending one. If it occurs
// later, the pending call will be rescheduled.
function DelayedCall(functionToCall) {
  assert(typeof functionToCall == 'function');
  this.functionToCall = functionToCall;
  this.whenToCall = null;
}

DelayedCall.prototype.initiateTimeout = function() {
  var self = this;
  if (self.whenToCall) {
    var delayMillis = Math.max(0, getTimeMillis() - self.whenToCall);
    setTimeout(function() {
      if (self.whenToCall <= getTimeMillis()) {
        // Perform the call
        self.whenToCall = null;
        self.functionToCall();
      } else {
        // Wait longer. Maybe the timer was inaccurate, or the call was rescheduled.
        self.initiateTimeout();
      }
    }, delayMillis);
  }
}

DelayedCall.prototype.callDelayed = function(delayMillis) {
  var whenToCall = getTimeMillis() + delayMillis;
  if (this.whenToCall) { // <-- If there is a call pending.
    if (this.whenToCall < whenToCall) { // <-- We can only reschedule a call to happen later.
      this.whenToCall = whenToCall;
    }
  } else {
    this.whenToCall = whenToCall;
    // Apparently no call is pending, so we need to initiate the timeout.
    this.initiateTimeout();
  }
}

module.exports = DelayedCall;
