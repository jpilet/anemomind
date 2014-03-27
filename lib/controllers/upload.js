'use strict';

var util = require('util'),
    path = require('path'),
    fs = require('fs');

/**
 * Get awesome things
 */
exports.upload = function(req, res) {
  console.dir(req.files);
  res.writeHead(200, {'content-type': 'text/plain'});
  res.write('received upload:\n\n');
  res.end(util.inspect(req.files));

  return;
};

exports.upload = function(req, res) {

  var tempPath = req.files.file.path,
      targetPath = path.resolve('./uploads/image.png');
  if (path.extname(req.files.file.name).toLowerCase() === '.txt') {
      fs.rename(tempPath, targetPath, function (err) {
          if (err) throw err;
          console.log('Upload completed!');
          res.send(200);
      });
  } else {
      fs.unlink(tempPath, function (err) {
          if (err) throw err;
          console.error('Only .txt files are allowed!');
      });
  }


  return;
};