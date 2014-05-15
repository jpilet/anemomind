'use strict';

var util = require('util'),
    path = require('path'),
    async = require('async'),
    mongoose = require('mongoose'),
    Upload = mongoose.model('Upload'),
    winston = require('winston'),
    fs = require('fs'),
    mkdirp = require('mkdirp'),
    exec = require('child_process').exec;

/**
 * TODO: replace ansyc with $q promise for better error handling
 * TODO: verify json schema
 */

exports.upload = function(req, res) {

  var tempPath = req.files.file.path,
      tempFile = path.basename(tempPath),
      targetPath = path.resolve('./uploads/' +
        req.session.passport.user + '/' +
        tempFile);

  async.series([
    //create folder if not present
    function (callback) {
      mkdirp('./uploads/' +
        req.session.passport.user, function (err) {
        if (err) {
          winston.error('could not create folder: ' + err);
        } else {
          callback();
        }
      });
    },
    //move upload
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
    //run data crunching app
    function (callback) {
      exec('./scripts/crunch.sh ' +
        req.session.passport.user +
        ' ' +
        tempFile, function (error, stdout, stderr) {
        console.log('stdout: ' + stdout);
        console.log('stderr: ' + stderr);
        if (error) {
          console.log('error executing script: ' + error);
        }
        // console.log(stdout);
        callback();
      });
    }
  ], function() {
    res.writeHead(200, {'content-type': 'text/plain'});
    res.write('received upload:\n\n');
    res.end(util.inspect(req.files));
    return;
  });
};

exports.storeUpload = function(req, res) {
  console.dir(req.params);
  var id = req.body.id;
  var filename = req.body.filename;
  console.log(id + filename);
  var targetPath = path.resolve('./data/' +
        id + '/' +
        filename);

  fs.readFile(targetPath, 'utf8', function (err, data) {
    if (err) {
      console.log('Error: ' + err);
      return;
    }
    data = JSON.parse(data);
    var date = new Date(data[0]["time_milliseconds_since_1970"]);
    var day = date.getDate();
    var month = date.getMonth()+1;
    var year = date.getFullYear();

    var title = day + '/' + month + '/' + year;
    
    console.log('storing ' + id + '/' + filename + ' in db..');
    var newUpload = new Upload({
      user:id,
      file:targetPath,
      title: title
    });
    newUpload.provider = 'local';
    newUpload.save(function(err) {
      if (err) {
        res.json(400, err);
      } else {
        winston.info('db updated');
        res.send(200);
      }
    });
  });
};
