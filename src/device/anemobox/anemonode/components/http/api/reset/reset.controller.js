'use strict'
var factoryReset = require('../../../factoryReset.js');

module.exports.reset = function(req, res) {
  factoryReset(function(err) {
    if (err) {
      res.status(500).send("Factory reset error: " + err);
    } else {
      res.status(200).end();
    }
  });
};
