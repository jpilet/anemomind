'use strict'
var factoryReset = require('../../../factoryReset.js');
var reboot = require('../../../reboot.js').reboot;

module.exports.reset = function(req, res) {
  factoryReset(function(err) {
    if (err) {
      res.status(500).send("Factory reset error: " + err);
    } else {
      res.status(200).end();
      
      // Leave some time for the HTTP-response to get away
      // before rebooting.
      setTimeout(function() {
        reboot();
      }, 3000);
    }
  });
};
