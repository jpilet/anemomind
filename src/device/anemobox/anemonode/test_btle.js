
var btle = require('./components/AnemoServiceBTLE');
var anemonode = require('./build/Release/anemonode');

btle.startBTLE();

setInterval(function() {
  var tws = Math.random()*16;
  anemonode.dispatcher.tws.setValue(__filename, tws);
  console.log('tws: ' + tws);
}, 2000);
