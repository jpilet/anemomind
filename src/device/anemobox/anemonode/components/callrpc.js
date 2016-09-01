var http = require('./http/api/live/live.controller.js');

exports.WITH_BT = false;
exports.WITH_HTTP = false;

exports.call = function(func, args, callback) {
  if (exports.WITH_BT) {
    var rpcble = require('./rpcble');
  }
  if (exports.WITH_HTTP && http.isConnected()) {
    http.callRpc(func, args, callback);
  } else if (exports.WITH_BT && rpcble.isConnected()) {
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
