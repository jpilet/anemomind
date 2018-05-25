'use strict';

const { execFile } = require('child_process');

var multer  = require('multer');
var fs = require('fs');
var config = require('../../config/environment');
var mkdirp = require('mkdirp');


var uploadPath = fs.realpathSync(config.uploadDir) + '/anemologs/boat';
console.log('Uploading log files to: ' + uploadPath);

function isFileNameOk(filename) {
  if (!filename || typeof(filename) != 'string') {
    return false;
  }

  var regexp = /^[a-z0-9_-][a-z0-9_., -]+$/i;
  return !!filename.match(regexp);
}

function fileDir(req) {
  if (req.params.boatId && req.params.boatId.match(/^[0-9a-zA-Z_-]+$/)) {
    return uploadPath + '/' + req.params.boatId + '/files';
  }
  return undefined;
}

function fileName(req) {
  var file = req.params.file;
  if (!isFileNameOk(file)) {
    return undefined;
  }
  return file;
}

function getDetailsForFiles(dir, files) {
  return new Promise((resolve, reject) => {
    execFile(config.tryLoadBin, [ '-C', dir ].concat(files),
             (error, stdout, stderr) => {
      if (error) {
        reject(error);
      }
      resolve(JSON.parse(stdout));
    });
  });
}

exports.getSingle = function(req, res, next) {
  const dir = fileDir(req);
  const name = fileName(req);
  if (!dir || !name) {
    res.status(400).send();
    return;
  }

  getDetailsForFiles(dir, [name])
  .then((details) => {
    res.status(200).json(details[0]);
  })
  .catch((err) => {
    res.status(404).send();
  });
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
    })
    .then((files) => { return getDetailsForFiles(dir, files); })
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
    if (isFileNameOk(filename)) {
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

exports.delete = function(req, res, next) {
  const dir = fileDir(req);
  const name = fileName(req);
  if (!dir || !name) {
    res.status(400).send();
    return;
  }
  fs.unlink(dir + '/' + name, function(err) {
    if (err) {
      console.warn(err);
      res.status(500).send(err);
    } else {
      res.status(204 /* no content */).send();
    }
  });
};

