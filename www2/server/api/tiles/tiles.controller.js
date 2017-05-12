'use strict';

var _ = require('lodash');
var Tiles = require('./tiles.model');
var mongoose = require('mongoose');

var makeQuery = function(boatId, s, x, y, startsAfter, endsBefore) {
  var search = {
    boat: mongoose.Types.ObjectId(boatId),
    key: "s" + s + "x" + x + "y" + y
  };

  if (startsAfter) {
    search.endTime  = { $gte: new Date(startsAfter) };
  }
  if (endsBefore) {
    search.startTime = { $lte: new Date(endsBefore) };
  }

  return search;
};

function fetchTiles(boatId, scale, x, y, startsAfter, endsBefore, callback) {
  var query = makeQuery(boatId,
                        scale,
                        x,
                        y,
                        startsAfter,
                        endsBefore);
  // This query might return many tiles. The lean() call tells mongoose to
  // pass raw objects instead of trying to apply magic to them. There is a
  // significant speed difference!
  Tiles.find(query).lean().exec(function(err, tiles) {
    if (err) {
      return callback(err, null);
    }
    return callback(null, tiles);
  });
}
module.exports.fetchTiles = fetchTiles;

exports.retrieveRaw = function(req, res, next) {
  fetchTiles(req.params.boat,
             req.params.scale,
             req.params.x,
             req.params.y,
             req.params.startsAfter,
             req.params.endsBefore,
             function(err, tiles) {
    if (err) {
      return next(err);
    }
    if (!tiles) return res.sendStatus(404);

    res.contentType('application/json');
    return res.send(JSON.stringify(tiles));
  });
};

exports.retrieveGeoJson = function(req, res, next) {
  var query = makeQuery(req.params.boat,
                        req.params.scale,
                        req.params.x,
                        req.params.y,
                        req.params.startsAfter,
                        req.params.endsBefore);

  console.log('get tile: ' + query.key);
  Tiles.find(query, function(err, tiles) {
    if (err) {
      return next(err);
    }
    if (!tiles) return res.sendStatus(404);

    var json = {
      type: "FeatureCollection",
      features: []
    };

    for (var i in tiles) {
      for (var c in tiles[i].curves) {
        var curve = tiles[i].curves[c];
        var feature = {
              geometry: {
                type: "LineString",
                coordinates: []
              },
              type: "Feature",
              id: curve.curveId,
              clipped: false,
              properties: {
                raceName: curve.curveId,
                time: []
            }
          };
        for (var p in curve.points) {
          if (curve.points[p].pos) {
            var coords = tile2LonLat(curve.points[p].pos);
            feature.geometry.coordinates.push(coords);
            feature.properties.time.push(curve.points[p].time.getTime());
          }
        }
        if (feature.geometry.coordinates.length > 1) {
          json.features.push(feature);
        }
      }
    }
    res.contentType('application/json');
    return res.send(JSON.stringify(json));
    // crazy bug, when doing res.json(json), browsers receive content twice (but only once with curl)
    // res.json(json);
  });
};

function tile2long(x) {
  return (x*360-180);
}

function tile2lat(y) {
  var n=Math.PI-2*Math.PI*y;
  return (180/Math.PI*Math.atan(0.5*(Math.exp(n)-Math.exp(-n))));
}

function tile2LonLat(point) {
  return [ tile2long(point[0]), tile2lat(point[1]) ];
}

function handleError(res, err) {
  return res.status(500).send(err);
}


module.exports.boatInfoAtTime = function(boat, time, callback) {
  var query = {
    $and: [
      {boat: mongoose.Types.ObjectId(boat) },
      {startTime: {$lte: time, $gte: new Date(time.getTime() - 60 * 60 * 1000)} },
      {endTime: {$gt: time} },
      {key: {$regex : /^s18/}} // only scale 18
    ]
  };
  Tiles.find(query, function(err, tiles) {
     if (err) {
       callback(err, undefined);
       return;
     }

     console.log('Searching for boat info at time ' + time
                 + ': ' + tiles.length + ' tiles returned by mongo');
     var closest;
     tiles.forEach(function(tile) {
       tile.curves.forEach(function(c) {
         c.points.forEach(function(p) {
           if (closest == undefined
               || Math.abs(p.time - time) < Math.abs(closest.time - time)) {
             closest = p;
           }
         });
       });
     });

     callback(undefined, closest);
  });
};
