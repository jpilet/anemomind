var fs = require('fs');
var anemonode = require('./build/Release/anemonode');

// Inspect the anemonode object
console.warn(anemonode.dispatcher.values);

for (var i in anemonode.dispatcher.values) {
  console.log(i + ': \"' + anemonode.dispatcher.values[i].description + '\"');
}

var logger = new anemonode.Logger();
var estimator = new anemonode.Estimator();

var calibFile = '../../Arduino/NMEAStats/test/boat.dat';
if (!estimator.loadCalibration(calibFile)) {
  console.log('Failed to load: ' + calibFile);
} else {
  console.log(calibFile + ': calibration loaded');
}

// Try subscribe/unsubscribe functions
var numCalls = 0;
var subsIndex = anemonode.dispatcher.values.awa.subscribe(function(val) {
  var description = anemonode.dispatcher.values.awa.description;
  console.log(description + ' changed to: ' + val);
  if (++numCalls == 3) {
    // stop notifying.
    console.log('Unsubscribing from ' + description);
    anemonode.dispatcher.values.awa.unsubscribe(subsIndex);
  }
});
console.log('Subscribed to awa: ' + subsIndex);

var sourceLow = "js test low prio";
var sourceMedium = "js test medium prio";
var sourceHigh = "js test high prio";

anemonode.dispatcher.setSourcePriority("NMEA0183", 20);
anemonode.dispatcher.setSourcePriority(sourceHigh, 10);
anemonode.dispatcher.setSourcePriority(sourceMedium, 5);
anemonode.dispatcher.setSourcePriority(sourceLow, 0);
anemonode.dispatcher.values.awa.setValue(sourceMedium, 1);
anemonode.dispatcher.values.awa.setValue(sourceLow, -10);
console.log('awa ' + anemonode.dispatcher.values.awa.value()
            + ' from source: ' + anemonode.dispatcher.values.awa.source());
anemonode.dispatcher.values.awa.setValue(sourceHigh, 10);
anemonode.dispatcher.values.awa.setValue(sourceHigh, 11);
anemonode.dispatcher.values.awa.setValue(sourceHigh, 12);
console.log('awa ' + anemonode.dispatcher.values.awa.value()
            + ' from source: ' + anemonode.dispatcher.values.awa.source());

var nmeaSource = new anemonode.Nmea0183Source();
console.warn(nmeaSource.process);

function format(x) {
  return JSON.stringify(x);
}

function printHistory(field) {
  for (var i = 0; i < anemonode.dispatcher.values[field].length(); ++i) {
    var dispatchData = anemonode.dispatcher.values[field];
    console.log(dispatchData.description
                + ': ' + format(dispatchData.value(i))
                + ' ' + dispatchData.unit
                + ' set on: ' + dispatchData.time(i).toLocaleString());
  }
}
// Read some NMEA data.
fs.readFile("../../../../datasets/tinylog.txt", function (err, data ) {
  if (err) {
    console.warn(err);
  } else {
    nmeaSource.process(data);
  }

  // List all available values
  for (var i in anemonode.dispatcher.values) {
    console.log(anemonode.dispatcher.values[i].description
                + ": " + anemonode.dispatcher.values[i].value());
  }

  printHistory('awa');
  printHistory('twa');
  printHistory('pos');
  printHistory('dateTime');


  logger.logText("test", "this text is logged from javascript");

  estimator.compute();
  printHistory('vmg');
  printHistory('targetVmg');

  logger.flush("./", function(path, err) {
    if (err) {
      console.log(err);
    } else {
      console.log('log written to: ' + path);
    }
  });
});

anemonode.dispatcher.values.orient.setValue("test", {heading: 128, roll: 12.2, pitch: -3.1});
anemonode.dispatcher.values.orient.setValue("test", {heading: 120, roll: 10.2, pitch: -3.8});
printHistory('orient');
