
var btle = require('./components/AnemoServiceBTLE');
var anemonode = require('./build/Release/anemonode');

btle.startBTLE();

setInterval(function() {
  var tws = Math.random()*16;
  anemonode.dispatcher.tws.setValue(__filename, tws);
  console.log('tws: ' + tws);
}, 2000);


require('./components/rpcble').rpcFuncTable.echo = function(data, cb) {
  console.log('RPC echo called with: ' + data.text); 
  cb({ echo: 'I got your message: ' + data.text + '. Thanks!'});
};
