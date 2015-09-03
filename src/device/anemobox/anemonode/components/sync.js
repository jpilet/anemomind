var callrpc = require('./callrpc.js');

// Will try to tell the iPhone that it should perform a
// full sync with the server.
function triggerSync(cb) {
  callrpc.call('fullSync', {data: null}, cb);
}

module.exports.triggerSync = triggerSync;
