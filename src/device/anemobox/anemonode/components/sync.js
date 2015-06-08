var rpcble = require('./rpcble.js');

// Will try to tell the iPhone that it should perform a
// full sync with the server.
function triggerSync(cb) {

  // cb is optional.
  cb = cb || function() {};
  
  // See https://github.com/jpilet/anemomind-ios/commit/071e5897be61e80563a2ea999c77421bd69babb3#diff-88087bfe37e7b54974e3e116ade952aaR265, in ANMBluetoothManager.
  // There is an rpc command: "fullSync".
  rpcble.call('fullSync', {data: null}, function(answer) {
    // Do we care?
    cb();
  });
}

module.exports.triggerSync = triggerSync;
