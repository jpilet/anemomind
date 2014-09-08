'use strict';

var mongoose = require('mongoose');
var Tile = mongoose.model('Tile');

var makeQuery = function(boatId, s, x, y, startsAfter, endsBefore) {
  var search = {
    boat: boatId,
    key: "s" + s + "x" + x + "y" + y
  };

  if (startsAfter) {
    search.startTime = { $gte: startsAfter };
  }
  if (endsBefore) {
    search.endTime = { $lte: endsBefore };
  }

  return search;
};

exports.retrieve = function(req, res, next) {
  var query = makeQuery("exocet", req.params.scale, req.params.x, req.params.y);
  console.log('get tile: ' + query.key);
  Tile.find(query, function(err, tiles) {
    if (err) {
      return next(err);
    }
    if (!tiles) return res.send(404);
    res.send(tiles);
  });
};
