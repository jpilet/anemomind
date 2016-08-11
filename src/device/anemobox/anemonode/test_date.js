var anemonode = require('./build/Release/anemonode');
var now = require('./components/timeest.js').now;

var sysTime = new Date();
var monotonic = anemonode.currentTime();

console.log('System time: ' + sysTime);
console.log('Monotonic time: ' + monotonic);
