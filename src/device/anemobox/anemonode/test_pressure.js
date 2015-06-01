pressureSensor = require('./components/pressure.js');

pressureSensor.init();

setInterval(
    function() {
    console.log('Temperature: ' + pressureSensor.temperature(10)
                + ' pressure: ' + pressureSensor.pressure(10));
    }, 1000);
