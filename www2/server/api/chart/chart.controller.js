'use strict';

var _ = require('lodash');
var ChartTile = require('./charttile.model');
var ChartSource = require('./chartsource.model');
var mongoose = require('mongoose');

var makeQuery = function(boatId, zoom, tile, channel, source) {
  return {
    boat : mongoose.Types.ObjectId(boatId),
    zoom: parseInt(zoom),
    tileno: parseInt(tile),
    what: channel || '',
    source: source || ''
  };
  return obj;
};

exports.retrieve = function(req, res, next) {
  var query = makeQuery(req.params.boat,
                        req.params.zoom,
                        req.params.tile,
                        req.params.channel,
                        req.params.source);

  // The query bypasses mongoose.
  ChartTile.collection.find(query).toArray(function(err, tiles) {
    if (err) {
      return next(err);
    }
    if (!tiles) return res.sendStatus(404);

    res.contentType('application/json');
    return res.send(JSON.stringify(tiles));
  });
};

function sampleTime(tile, sampleno) {
  return new Date((tile.tileno << tile.zoom)
                  + (sampleno / tile.samples.length) * (1 << tile.zoom));
}



exports.index = function(req, res, next) {
  var boat = req.params.boat;
  if (!boat) {
    return res.sendStatus(400);
  }

  console.log('findById(' + boat + ')');
  ChartSource.findById(boat, function(err, sources) {
      if (err) {
        console.log(err);
        return res.sendStatus(500);
      }
      res.json(sources);
    });
};
