var http = require('./http/api/live/live.controller.js');
var rpcble = require('./rpcble');

exports.call = function(func, args, callback) {
  if (http.isConnected()) {
    http.callRpc(func, args, callback);
  } else if (rpcble.isConnected()) {
    rpcble.call(func, args, callback);
  } else {
    console.log('Can\'t call rpc ' + func + ': nobody is connected.');
  }
};

// Useful for debugging RPC pipeline.
exports.ping = function() {
  setInterval(function() {
    var now = new Date();
    exports.call('ping', {data: now}, function(answer) {
      if (answer && 'data' in answer) {
        var delta = (new Date()) - new Date(answer.data);
        console.log('RPC ping reply in ' + delta + ' ms');
      } else {
        console.log('bad ping answer:');
        console.warn(answer);
      }
   });
  }, 900);
} 
