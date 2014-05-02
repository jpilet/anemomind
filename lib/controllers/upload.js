'use strict';

var util = require('util'),
    path = require('path'),
    async = require('async'),
    mongoose = require('mongoose'),
    Upload = mongoose.model('Upload'),
    winston = require('winston'),
    fs = require('fs');

/**
 * TODO: replace ansyc with $q promise for better error handling
 * TODO: verify json schema
 */

exports.upload = function(req, res) {

  var tempPath = req.files.file.path,
      tempFile = path.basename(tempPath),
      targetPath = path.resolve('./uploads/' + tempFile),
      title;

  async.series([
    function (callback) {
      fs.rename(tempPath, targetPath, function (err) {
        if (err) {
          res.json(400, err);
          throw err;
        } else {
          winston.info('file saved');
          callback();
        }
      });

    },
    function (callback) {
      fs.readFile(targetPath, 'utf8', function (err, data) {
        if (err) {
          console.log('Error: ' + err);
          return res.send(500);
        }
        data = JSON.parse(data);
        console.log(data);
        var date = new Date(data[0]["time_milliseconds_since_1970"]);
        var day = date.getDate();
        var month = date.getMonth()+1;
        var year = date.getFullYear();

        title = day + '/' + month + '/' + year;
        callback();
      });
    },
    function (callback) {

      console.log(title);
      var newUpload = new Upload({
        user:req.session.passport.user,
        file:targetPath,
        title: title
      });
      newUpload.provider = 'local';
      newUpload.save(function(err) {
        if (err) {
          res.json(400, err);
        } else {
          winston.info('db updated');
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