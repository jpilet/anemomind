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
  "tiles/VectorTileLayer.js",
  "tiles/ClearLayer.js"
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

var preparedCanvases = {};
function getCanvas(width, height, name, cb) {
    if (preparedCanvases[name]) {
      var prepared = preparedCanvases[name];
      prepared.canvas.width = width;
      prepared.canvas.height = height;
      prepared.context = prepared.canvas.getContext('2d');
      cb(prepared);
      return;
    }

    var canvas;
    var renderer;
    var pathLayer;
    var context;

    canvas = new Canvas(width, height);
    canvas.width = width;
    canvas.height = height;
    context = canvas.getContext('2d');

    renderer = new OffscreenTileRenderer({
      canvas: canvas,
      forceDevicePixelRatio: 1,
      url: function(scale, x, y) { 
        var s = [ 'a', 'b', 'c' ][(scale + x + y) % 3];
        return "http://stamen-tiles-" + s + ".a.ssl.fastly.net/toner-lite/"
          + scale + "/" + x + "/" + y + ".png";
      },

      // Allow direct disc access instead of going through a http call
      localImagePath: config.root
          + (config.env == 'production' ? '/public' : '/client'),

      initialLocation: {x:0.5,y:0.5,scale:0.01},
      maxNumCachedTiles: 256
    });

    if (name == 'noMap') {
      renderer.layers[0] = new ClearLayer({color: '#d9d9d9'});
    }

    pathLayer = new VectorTileLayer({
      maxNumCachedTiles: 512,
      maxSimultaneousLoads: 64,
      maxUpLevel: 1
    }, renderer);
    renderer.addLayer(pathLayer);

    var prepared = {
      canvas: canvas,
      renderer: renderer,
      pathLayer: pathLayer,
      context: context
    };
    preparedCanvases[name] = prepared;

    cb(prepared);
}


function parseLocation(encoded) {
  var entries = encoded.split(',');
  return {
          x: parseFloat(entries[0]),
          y: parseFloat(entries[1]),
          scale: parseFloat(entries[2])
  };
}

function generateMapImage(boat, start, end, location, width, height, style,
                          cb) {
  ensureNotBusy(function(done) {
    getCanvas(width,height, style, function(prepared) {
      prepared.renderer.setLocation(location);
      var pathLayer = prepared.pathLayer;

      // Make sure to reset / clear loaded tiles by changing the URL
      pathLayer.setUrl(undefined);
      pathLayer.setUrl(function(scale, x, y) { return [scale, x, y, boat].join('/'); });
      pathLayer.fetchTile = function(url, success, error) {
        var urlAsArray = url.split('/');
        var scale = urlAsArray[0];
        var x = urlAsArray[1];
        var y = urlAsArray[2];
        prepared.renderer.addLoading();
        fetchTiles(boat, scale, x, y, start, end, function(err, data) {
          if (err) {
            error(err);
          } else {
            success(data);
          }
          prepared.renderer.doneLoading(err);
        });
      }
      try {
        prepared.renderer.render(function(err) {
          if (err) {
            cb(err);
            done();
            return;
          }
          prepared.canvas.toBuffer(function(err, buffer) {
            if (err) {
              cb(err);
              done();
              return;
            }
            cb(undefined, buffer);
            done();
          });
        });
      } catch (err) {
        console.log('Error: ' + err);
        console.log(err.stack);
        cb(err);
        done();
      }
    });
  });
};

function sendPngWithCache(res, buffer, seconds) {
  res.header('Cache-Control', 'public, max-age=' + seconds);
  res.type('image/png');
  res.send(buffer);
}

module.exports.getMapPng = function(req, res, next) {
  var boat = req.params.boat;
  var start = new Date(req.params.timeStart +'Z');
  var end = new Date(req.params.timeEnd + 'Z');
  var location = parseLocation(req.params.location);
  var width = parseInt(req.params.width);
  var height = parseInt(req.params.height);

  generateMapImage(boat, start, end, location, width, height, 'map',
                   function(err, buffer) {
    if (err) {
      // something went wrong. Try again without the map.
      generateMapImage(boat, start, end, location, width, height, 'noMap',
                       function(err, buffer) {
        if (err || !buffer) {
          // Does not work without the map :(
          console.log(err);
          res.status(500).send(err);
          return;
        } else {
          var minutes = 60;
          sendPngWithCache(res, buffer, 30 * minutes);
        }
      });
    } else {
      // First rendering attempt succeeded
      var day = 24 * 60 * 60;
      sendPngWithCache(res, buffer, 7 * day);
    }
  });
};
