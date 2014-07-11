'use strict';

var util = require('util'),
    path = require('path'),
    async = require('async'),
    mongoose = require('mongoose'),
    RaceData = mongoose.model('RaceData'),
    winston = require('winston'),
    fs = require('fs'),
    mkdirp = require('mkdirp'),
    exec = require('child_process').exec,
    traverse = require('traverse');

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
      winston.info('running: ./scripts/crunch.sh ' +
        req.session.passport.user +
        ' ' + tempFile);
      exec('./scripts/crunch.sh ' +
        req.session.passport.user + ' ' +
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

exports.storeData = function(req, res) {
  var id = req.body.id;
  var filename = req.body.filename.slice(0, -4);
  console.log(filename);
  var targetPath = path.resolve('./data/' +
        id + '/processed/' + filename);

  // read the 3 generated files in parallel
  async.parallel({
    navs: function(callback){
      fs.readFile(targetPath + '_navs.js', 'utf8', function (err, data) {
        if (err) {
          console.log('Error: ' + err);
          callback(err);
        }
        data = JSON.parse(data);
        callback(null, data);

      });
    },
    tree: function(callback){
      fs.readFile(targetPath + '_tree.js', 'utf8', function (err, data) {
        if (err) {
          console.log('Error: ' + err);
          callback(err);
        }
        data = JSON.parse(data);
        callback(null, data);
      });
    },
    nodeInfo: function(callback){
      fs.readFile(targetPath + '_tree_node_info.js', 'utf8', function (err, data) {
        if (err) {
          console.log('Error: ' + err);
          callback(err);
        }
        data = JSON.parse(data);
        callback(null, data);
      });
    }
  },
  // and do something with the result
  function(err, results) {
    if (err) {
      console.dir(err);
      return res.json(400, err);
    }

    // get code for the "Sailing" part
    var code;
    for (var i = 0; i < results.nodeInfo.length; i++) {
      if (results.nodeInfo[i].description === 'Sailing'){
        code = results.nodeInfo[i].code;
        break;
      }
    }

    //get left and right ids for all sailing periods
    var left = [];
    var right = [];
    var j = 0;
    traverse(results.tree).map(function (x) {
      if (this.key === 'code') {
        if (x === code) {
          left[j] = this.parent.node.left;
          right[j] = this.parent.node.right;
          j++;
        }
      }
    });

    //collect coords for all sailing periods
    var sailingPeriods = [];
    for (var k = 0; k < left.length; k++) {

      for(var l = 0; l < results.navs.length; l++) {
        if (results.navs[l].id >= right[k]) {
          console.log((l+1) + ' coords found for sailing period ' + (k+1));
          break;
        }
        if (results.navs[l].id >= left[k]) {
          if (!sailingPeriods[k]) {
            var date = new Date(results.navs[l]['time_ms_1970']);
            var title = date.getDate() + '/' + (date.getMonth()+1) + '/' + date.getFullYear();
            sailingPeriods[k] = {
              title: title,
              boatId: results.navs[0]['boat-id'],
              items: []
            };
          }
          var tmpItem = {
            latRad: results.navs[l]['lat_rad'],
            lonRad: results.navs[l]['lon_rad'],
            awaRad: results.navs[l]['awa_rad'],
            awsMps: results.navs[l]['aws_mps'],
            externalTwaRad: results.navs[l]['externalTwa_rad'],
            externalTwsMps: results.navs[l]['externalTws_mps'],
            gpsBearingRad: results.navs[l]['gpsbearing_rad'],
            gpsSpeedMps: results.navs[l]['gpsspeed_mps'],
            magHdgRad: results.navs[l]['maghdg_rad'],
            timeMs: results.navs[l]['time_ms_1970'],
            watSpeedMps: results.navs[l]['watspeed_mps'],
            twdirRad: results.navs[l]['twdir_rad'],
            twsMps: results.navs[l]['tws_mps'],
          };
          sailingPeriods[k].items.push(tmpItem);
        }
      }
    }

    //store coords for all sailing periods in DB
    async.each(sailingPeriods, function( period, callback) {
      var newRaceData = new RaceData(period);
      newRaceData.save(function(err) {
        if (err) {
          winston.error('failed saving data: ' + err);
          callback(err);
        } else {
          callback();
        }
      });
    }, function(err){
      if( err ) {
        console.log('Could not store data in DB: ' + err);
        res.send(400);
      } else {
        res.send(200);
      }
    });
  });
};
