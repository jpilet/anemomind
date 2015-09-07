var bno055 = require('./components/bno055');

bno055.init(function() {
  console.log('BNO055 init done.');
  setInterval(function() {
    bno055.saveCalib(function() {
      console.log('Calibration saved');
    });
  }, 5000);

  setInterval(function() {
    var angles = bno055.getAngles();
    angles.calibrated = bno055.isCalibrated();
    console.warn(angles);
  }, 500);
});
