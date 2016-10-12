'use strict';

var _ = require('lodash');
var Tiles = require('../tiles/tiles.model');
var fetchTiles = require('../tiles/tiles.controller').fetchTiles;
var mongoose = require('mongoose');
var Canvas = require('canvas');
var OffscreenTileRenderer =
    require('./tiles/OffscreenTileRenderer.js').OffscreenTileRenderer;
var vm = require('vm');
var fs = require('fs');
var config = require('../../config/environment');

var sources = [
  "tiles/VectorTileLayer.js"
];

for (var i in sources) {
  vm.runInThisContext(fs.readFileSync(__dirname + '/' + sources[i]) + '',
                      { filename: sources[i] });
}

// Because creating a canvas and its context takes around 300ms,
// we keep them loaded and recycle it for multiple queries.
// It implies that we can only process one query at a time.
// So ensureNotBusy waits for the canvas to be available.
var busy;
var waiting = [];
function ensureNotBusy(cb) {
  if (busy) {
    waiting.push(cb);
  } else {
    busy = true;
    cb(function() {
       busy = false;
       if (waiting.length > 0) {
         ensureNotBusy(waiting.shift());
       }
    });
  }
};

var canvas;
var renderer;
var pathLayer;
var context;

function getCanvas(width, height, cb) {
    if (!canvas) {
      canvas = new Canvas(width, height);
    }
    canvas.width = width;
    canvas.height = height;
    context = canvas.getContext('2d');

    if (!renderer) {
      renderer = new OffscreenTileRenderer({
        canvas: canvas,
        forceDevicePixelRatio: 1,
        url: "http://stamen-tiles-a.a.ssl.fastly.net/toner-lite/$scale/$x/$y.png",

        // Allow direct disc access instead of going through a http call
        localImagePath: config.root
            + (config.env == 'production' ? '/public' : '/client'),

        initialLocation: {x:0.5,y:0.5,scale:0.01},
        maxNumCachedTiles: 256
      });

      pathLayer = new VectorTileLayer({
        maxNumCachedTiles: 512,
        maxSimultaneousLoads: 64,
        maxUpLevel: 1
      }, renderer);
      renderer.addLayer(pathLayer);

    }

    cb(canvas, renderer);
}


function parseLocation(encoded) {
  var entries = encoded.split(',');
  return {
          x: parseFloat(entries[0]),
          y: parseFloat(entries[1]),
          scale: parseFloat(entries[2])
  };
}

module.exports.getMapPng = function(req, res, next) {
  var boat = req.params.boat;
  var start = new Date(req.params.timeStart +'Z');
  var end = new Date(req.params.timeEnd + 'Z');
  var location = req.params.location;

  ensureNotBusy(function(done) {
    getCanvas(parseInt(req.params.width), parseInt(req.params.height),
              function(canvas, renderer) {
      renderer.setLocation(parseLocation(location));

      pathLayer.setUrl(function(scale, x, y) { return [scale, x, y].join('/'); });
      pathLayer.fetchTile = function(url, success, error) {
        var urlAsArray = url.split('/');
        var scale = urlAsArray[0];
        var x = urlAsArray[1];
        var y = urlAsArray[2];
        renderer.addLoading();
        fetchTiles(boat, scale, x, y, start, end, function(err, data) {
          if (err) {
            error(err);
          } else {
            success(data);
          }
          renderer.doneLoading(err);
        });
      }
      try {
        renderer.render(function(err) {
          if (err) {
            console.log(err);
            res.status(500).send(err);
            return;
          }
          canvas.toBuffer(function(err, buffer) {
            if (err) {
              next(err);
              done();
              return;
            }
            
            res.header('Cache-Control', 'public, max-age=' + (7 *24 * 60 * 60));
            res.type('image/png');
            res.send(buffer);
            done();
          });
        });
      } catch (err) {
        console.log('Error: ' + err);
        console.log(err.stack);
        next(err);
        done();
      }
    });
  });
};
