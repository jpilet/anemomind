
var btle = require('./components/AnemoServiceBTLE');
var anemonode = require('./build/Release/anemonode');

btle.startBTLE();

setInterval(function() {
   anemonode.dispatcher.awa.setValue(__filename, Math.random()*360);
}, 1000);
