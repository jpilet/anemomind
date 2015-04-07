var anemonode = require('./build/Release/anemonode');

// Inspect the anemonode object
console.warn(anemonode);

var numCalls = 0;
var subsIndex = anemonode.entries.awa.subscribe(function(val) {
  var description = anemonode.entries.awa.description;
  console.log(description + ' changed to: ' + val);
  if (++numCalls == 3) {
    // stop notifying.
    console.log('Unsubscribing from ' + description);
    anemonode.entries.awa.unsubscribe(subsIndex);
  }
});

anemonode.entries.awa.setValue("js test", 1);
console.log('awa ' + anemonode.entries.awa.value());

// Read NMEA for 1 second
anemonode.run("../../../../datasets/tinylog.txt", 1);

// List all available values
for (var i in anemonode.entries) {
  console.log(anemonode.entries[i].description
              + ": " + anemonode.entries[i].value());
}

// Go back in time
for (var i = 0; i < anemonode.entries.awa.length(); ++i) {
  var awa = anemonode.entries.awa;
  console.log(' awa ' + awa.value(i)
              + ' set on: ' + awa.time(i).toLocaleString());
}
