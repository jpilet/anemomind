'use strict';

var boxId = require('../../../boxId.js');
var config = require('../../../config.js');
var anemoIdBuffer;

// Get list of boats
exports.index = function(req, res) {

  boxId.getAnemoId(function(id) {
    anemoIdBuffer = new Buffer(id, 'utf8');
    config.get(function(err, config) {
      res.json({
        boxId: id,
        boatId: config.boatId,
        boatName: config.boatName
      });
    });
  });
};

function handleError(res, err) {
  return res.status(500).send(err);
}
