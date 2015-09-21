var anemonode = require('../build/Release/anemonode');

var estimator = new anemonode.Estimator();

function calibFilePath() {
  return (process.env.ANEMOBOX_CONFIG_PATH || ".") + "/boat.dat";
}

function loadCalib(path) {
  var r = estimator.loadCalibration(path || calibFilePath());
  if (r) {
    console.log('Calibration loaded.');
  }
  return r;
}

function start() {
  var triggeringFields =
    ['awa', 'aws', 'gpsSpeed', 'gpsBearing', 'watSpeed', 'magHdg'];

  var timer = false;
  function update() {
    if (!timer) {
      timer = true;
      setTimeout(function() {
        timer = false;
        estimator.compute();
      }, 20);
    }
  }
  for (var i in triggeringFields) {
    anemonode.dispatcher.values[triggeringFields[i]].subscribe(update);
  }
}

module.exports.calibFilePath = calibFilePath;
module.exports.loadCalib = loadCalib;
module.exports.start = start;
