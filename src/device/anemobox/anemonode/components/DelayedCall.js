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
  this.timeout = null;
}

DelayedCall.prototype.callDelayed = function(delayMillis) {
  if (this.timeout) {
    clearTimeout(this.timeout);
  }
  var self = this;
  this.timeout = setTimeout(self.functionToCall, delayMillis);
}

module.exports = DelayedCall;
