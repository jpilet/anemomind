//var callrpc = require('./callrpc.js');

// Will try to tell the iPhone that it should perform a
// full sync with the server.
function triggerSync(cb) {
  console.log("UNDO THIS!!! in components/sync.js");
  // PUT BACK LATER!!! callrpc.call('fullSync', {data: null}, cb);
}

module.exports.triggerSync = triggerSync;
