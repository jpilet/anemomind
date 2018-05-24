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
  console.log('fetching file list in ' + dir);
  if (!dir) {
    req.status(400).send();
  } else {
    const p = new Promise((resolve, reject) => {
      fs.readdir(dir, function(err, files) {
        if (err) {
          reject(err);
        } else {
          resolve(files);
        }
      });
    });
    p.then((files) => {
      const promises = [];
      files.forEach((file) => {
        promises.push(new Promise((resolve, reject) => {
          fs.stat(dir + '/' + file, (err, stat) => {
            if (err) {
              console.warn(err);
              reject(err);
            } else {
              resolve({ name: file, date: stat.ctime, size: stat.size });
            }
          });
        }));
      });
      return Promise.all(promises);
    })
    .then((data) => {
      res.json(data);
    })
    .catch((err) => {
        console.warn(err);
        req.status(500).send(err).send();
    });
  }
};

// This handler can assume that the user is authentified, it has write access,
// and the upload folder for the boat has been created.
exports.postFile = multer({storage: multer.diskStorage({
  destination: function(req, file, cb) {
    var dir = fileDir(req);
    if (dir) {
      mkdirp(dir, () => { cb(null, dir); });
    } else {
      return cb('no boatId', undefined);
    }
  },

  filename: function (req, file, cb) {
    var filename = file.originalname
      .replace(/[^a-z0-9_.-]/gi, '_')
      .replace(/^[.]/, '_'); // initial dot is not allowed

    // For paranoia, let's check the result.
    var regexp = /^[a-z0-9_-][a-z0-9_., -]+$/i;
    if (filename.match(regexp)) {
      console.log('file handler: accepting file: ' + file.originalname
        + ' as: ' + filename);
      file.newname = filename;
      cb(null, filename);
    } else {
      console.log('file handler: rejecting bad filename: '
                  + file.originalname);
      cb('Bad filename', undefined);
    }
  }
})}).single('file');

exports.handleUploadedFile = function(req, res, next) {
  console.log(req.file.filename + ' uploaded, size: ' + req.file.size);
  res.status(201).json({ result: 'OK', file: req.file.newname });
};


