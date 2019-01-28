// Modified from
// https://github.com/Vaidas737/express-thumbnail

var fs = require('fs');
var path = require('path');
var mkdirp = require('mkdirp');
var imageMagick = require('gm').subClass({ imageMagick: true });

var expressThumbnail = module.exports;

// Register middleware.
expressThumbnail.register = function(rootDir, options) {

  rootDir = path.normalize(rootDir);

  options = options || {};

  // cache folder, default to {root dir}/.thumb
  options.cacheDir = options.cacheDir || path.join(rootDir, 'thumbnails');
 
  // compression level, default to 80
  options.quality = options.quality || 80;

  // thumbnail gravity, default to Center
  options.gravity = options.gravity || 'Center';

  return function (req, res, next) {
    var filename = path.join(req.params.boatId, req.params.photo);
    var filepath = path.join(rootDir, filename);

    // wanted thumbnail dimensions
    var dimension = req.query.s || '';

    // file location in cache
    var location = path.join(options.cacheDir, dimension, filename);

    // send converted file from cache
    function sendConverted() {
      var dimensions = dimension.split('x');
      var convertOptions = {
        filepath: filepath,
        location: location,
        width: dimensions[0],
        height: dimensions[1],
        quality: options.quality,
        gravity: options.gravity
      };

      expressThumbnail.convert(convertOptions, function (err) {
        if (err) {
          console.log(err);
          return next();
        }
        return res.sendFile(location);
      });
    }

    fs.stat(filepath, function (err, stats) {

      // go forward
      if (err || !stats.isFile()) { return next(); }

      // send original file
      if (!dimension) { return res.sendFile(filepath); }

      // send converted file
      fs.exists(location, function (exists) {

        // file was found in cache
        if (exists) { return res.sendFile(location); }

        // convert and send
        sendConverted();
      });
    });
  };
};

// Convert the image and store in the cache.
expressThumbnail.convert = function(options, callback) {
  mkdirp(path.dirname(options.location), function(err) {
    if (err) { return callback(err); }
    var img = imageMagick(options.filepath).autoOrient().gravity(options.gravity);
    img.size(function(err, size) {
      if (err) {
        callback(err);
        return;
      }
      // If either 'width' or 'height' is missing, we'll simply preserve the
      // original image aspect ratio.
      if (options.width == '_') {
        options.width = size.width * (parseInt(options.height) / size.height);
      }
      if (options.height == '_') {
        options.height = size.height * (parseInt(options.width) / size.width);
      }
      img.thumb(options.width, options.height, options.location, options.quality, function(err, stdout, stderr, command) {
        return err ? callback(err) : callback(null);
      });
    });
  });
};
