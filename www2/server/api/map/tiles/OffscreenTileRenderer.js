/*
 * Use several node tricks to pretend to be a browser to render a map using
 * CanvasTilesRenderer.
 *
 * Usage example:

var canvas = new Canvas(800, 600, 'pdf');

var renderer = new OffscreenTileRenderer({
  canvas: canvas,
  forceDevicePixelRatio: 1,
  initialLocation: {x:0.5137435332434739,y:0.32862555455727877,scale:0.0002892879213498567},
  url: "https://tiles.wmflabs.org/bw-mapnik/$scale/$x/$y.png",
  localImagePath: '../images'
});

renderer.render(function(err) {
  if (err) {
    console.log(err);
  } else {
    fs.writeFile('out.pdf', canvas.toBuffer());
  }
});

 */

var Canvas = require('canvas');
var fs = require('fs');
var vm = require('vm');
var request = require('request');
var cachedRequest = require('cached-request')(request)

// These are necessary to fake the browser
window = { setTimeout: setTimeout };
document = { createElement: function() { return {}; } };
addWheelListener = function() { };

var sources = [
  "utils.js",
  "affinetransform.js",
  "pinchzoom.js",
  "CanvasTilesRenderer.js",
  "TileLayer.js"
];

for (var i in sources) {
  vm.runInThisContext(fs.readFileSync(__dirname + '/' + sources[i]) + '',
                      { filename: sources[i] });
}


cachedRequest.setCacheDirectory('/tmp/imgcache');
cachedRequest.setValue('ttl', 7 * 24 * 60 * 60 * 1000);

function OffscreenTileRenderer(params) {
  if (!params.canvas) {
    throw(new Error('params.canvas has to be set'));
  }
  params.canvas.addEventListener = function() { };
  params.maxSimultaneousLoads = 500;
  params.downgradeIfSlowerFPS = .0001;
  params.maxUpLevels = 1;

  CanvasTilesRenderer.call(this, params);

  this.numLoading = 0;
  this.numFailedLoading = 0;
  this.failedImages = [];
}

OffscreenTileRenderer.prototype = Object.create(CanvasTilesRenderer.prototype);
OffscreenTileRenderer.prototype.constructor = OffscreenTileRenderer;

OffscreenTileRenderer.prototype.addLoading = function() {
  ++this.numLoading;
};

OffscreenTileRenderer.prototype.doneLoading = function(err) {
  --this.numLoading;
  if (err) {
    ++this.numFailedLoading;
  }
  if (this.numLoading == 0) {
    setTimeout(this.numFailedLoading == 0 ? this.renderingDone
               : this.renderingFailed, 0);
  }
};

OffscreenTileRenderer.prototype.failedLoading = function() {
  this.doneLoading(true);
};

// Overloads 'refresh' to do nothing.
// We will manually draw() when everything is loaded, no need
// to draw intermediate states as when in interactive mode
OffscreenTileRenderer.prototype.refresh = function() { };


OffscreenTileRenderer.prototype.loadImage = function(url, success, error) {
  this.addLoading();

  var image = new Canvas.Image();
  var me = this;

  var reportError = function(err) {
    var err = url + ': ' + err;
    console.error(err);
    me.failedImages.push(url);
    me.failedLoading();
    if (error) {
      error(err);
    }
  };
  image.onerror = reportError;

  image.onload = function() {
      //console.log('loaded image: ' + url + '(' + numLoading + ' loading)');
      success(image);
      me.doneLoading();
  };

  if (url.substr(0,4) == 'http') {
    cachedRequest.get({ url: url, encoding: null }, function(err, res, body) {
        if (err) {
          return reportError(err);
        }
        if (!body) {
          return reportError(url + ': empty response from server');
        }
        image.src = new Buffer(body, 'binary');
    });
  } else {
    // local file
    image.src = (this.params.localImagePath || __dirname) + '/' + url;
  }
};

OffscreenTileRenderer.prototype.render = function(cb) {
    // If images have to be loaded, the loadImage function will call
    // this function when they are all here.
    var me = this;
    this.renderingDone = function() {
      me.draw();
      cb();
    };
    this.renderingFailed = function() {
      cb("Failed to load:\n" + me.failedImages.join('\n'));
    };

    if (this.numLoading != 0) {
      console.log('PARALLEL CALLS TO render ARE NOT SUPPORTED');
      console.log('numLoading: ' + this.numLoading + ', numFailedLoading: '
                  + this.numFailedLoading);
      cb(new Error('render() called while previous call was not terminated.'));
      return;
    }
    this.numFailedLoading = 0;
    this.failedImages = [];

    // the first call to draw() will queue all image load queries
    this.addLoading();
    this.draw();
    this.doneLoading();
  };

module.exports.OffscreenTileRenderer = OffscreenTileRenderer;
