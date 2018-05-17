'use strict';

var multer  = require('multer');
var fs = require('fs');
var config = require('../../config/environment');
var mkdirp = require('mkdirp');

var uploadPath = fs.realpathSync(config.uploadDir) + '/anemologs/boat';
console.log('Uploading log files to: ' + uploadPath);

function fileDir(req) {
  if (req.params.boatId && req.params.boatId.match(/^[0-9a-zA-Z_-]+$/)) {
    return uploadPath + '/' + req.params.boatId + '/files';
  }
  return undefined;
};

exports.listFiles = function(req, res, next) {
  var dir = fileDir(req);
  if (!dir) {
    req.status(400).send();
  } else {
    fs.readdir(dir, function(err, files) {
      if (err) {
        console.warn(err);
        req.status(500).send();
      } else {
        res.json(files).send();
      }
    });
  }
};

// This handler can assume that the user is authentified, it has write access,
// and the upload folder for the boat has been created.
exports.postFile = multer({storage: multer.diskStorage({
  destination: function(req, file, cb) {
    var dir = fileDir(req);
    if (dir) {
      mkdirp(dir, cb);
    } else {
      return cb('no boatId', undefined);
    }
  },

  filename: function (req, file, cb) {
    var regexp = /^[a-zA-F0-9_-][a-zA-F0-9_., -]+$/;

    if (file.originalname.match(regexp)) {
      console.log('file handler: accepting file: ' + file.originalname);
      cb(null, file.originalname);
    } else {
      console.log('file handler: rejecting bad filename: '
                  + file.originalname);
      cb('Bad filename', undefined);
    }
  }
})}).single('log');

exports.handleUploadedFile = function(req, res, next) {
  console.log(req.log.filename + ' uploaded, size: ' + req.log.size);
};


