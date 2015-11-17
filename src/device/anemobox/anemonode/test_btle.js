var btle = require('./components/AnemoServiceBTLE');
var anemonode = require('./build/Release/anemonode');
var rpcble = require('./components/rpcble');
require('./components/RpcAssignBoat').register(rpcble.rpcFuncTable);

btle.startBTLE();

setInterval(function() {
  var tws = Math.random()*16;
  anemonode.dispatcher.values.tws.setValue(__filename, tws);
  console.log('tws: ' + tws);
}, 2000);


setInterval(function() {
  var now = new Date();
  rpcble.call('ping', {data: now}, function(answer) {
    if ('data' in answer) {
      var delta = (new Date()) - new Date(answer.data);
      console.log('RPC ping reply in ' + delta + ' ms');
    } else {
      console.log('bad ping answer:');
      console.warn(answer);
    }
 });
}, 1300);
  
rpcble.rpcFuncTable.echo = function(data, cb) {
  console.log('RPC echo called with: ' + data.text); 
  cb({ echo: 'I got your message: ' + data.text + '. Thanks!'});
};
