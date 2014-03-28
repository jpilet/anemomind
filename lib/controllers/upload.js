'use strict';

var util = require('util'),
    path = require('path'),
    async = require('async'),
    mongoose = require('mongoose'),
    Upload = mongoose.model('Upload'),
    fs = require('fs');

/**
 * TODO: replace ansyc with $q promise for better error handling
 */

exports.upload = function(req, res) {

  var tempPath = req.files.file.path,
      tempFile = path.basename(tempPath),
      targetPath = path.resolve('./uploads/' + tempFile);

      console.log(tempPath);

  async.series([
    function(callback) {
      fs.rename(tempPath, targetPath, function (err) {
        if (err) throw err;
        console.log('file saved');
        callback();
      });

    },
    function(callback) {

      var newUpload = new Upload({user:req.session.passport.user, file:targetPath});
      newUpload.provider = 'local';
      newUpload.save(function(err) {
        if (err) {
          res.json(400, err);
        } else {
          console.log('db updated');
          callback();
        }
      });
    }
  ], function() {
    res.writeHead(200, {'content-type': 'text/plain'});
    res.write('received upload:\n\n');
    res.end(util.inspect(req.files));
    return;
  });
};