'use strict';

var mongoose = require('mongoose');
var Tile = mongoose.model('Tile');

var makeQuery = function(boatId, s, x, y, startsAfter, endsBefore) {
  var search = {
    boat: boatId,
    key: "s" + s + "x" + x + "y" + y
  };

  if (startsAfter) {
    search.startTime = { $gte: new Date(startsAfter) };
  }
  if (endsBefore) {
    search.endTime = { $lte: new Date(endsBefore) };
  }

  return search;
};

exports.retrieve = function(req, res, next) {
  var query = makeQuery(req.params.boat,
                        req.params.scale,
                        req.params.x,
                        req.params.y,
                        req.params.startsAfter,
                        req.params.endsBefore);

  console.log('get tile: ' + query.key);
  Tile.find(query, function(err, tiles) {
    if (err) {
      return next(err);
    }
    if (!tiles) return res.send(404);

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
              raceName: curve.curveId
            }
          };
        for (var p in curve.points) {
          if (curve.points[p].pos) {
            var coords = tile2LonLat(curve.points[p].pos);
            feature.geometry.coordinates.push(coords);
          }
        }
        if (feature.geometry.coordinates.length > 1) {
          json.features.push(feature);
        }
      }
    }

    res.send(json);
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
