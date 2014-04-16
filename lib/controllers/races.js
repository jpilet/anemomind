'use strict';

var mongoose = require('mongoose'),
    Upload = mongoose.model('Upload'),
    fs = require('fs');
    // passport = require('passport');



/**
 * Get list of races for specific user
 */
exports.list = function (req, res) {
  var userId = req.session.passport.user;
  console.log(userId);

  Upload.find({user: userId}, function (err, raceList) {
    res.send(raceList);
  });
};

/**
 * Get race details
*/
exports.raceDetail = function(req, res) {
  var raceId = req.params.id;

  Upload.findById(raceId, function (err, race) {
    if (err) return res.send(500);
    if (!race) return res.send(404);
    
    fs.readFile(race.file, 'utf8', function (err, data) {
      if (err) {
        console.log('Error: ' + err);
        return res.send(500);
      }
     
      data = JSON.parse(data);
      res.send(data);
    });
  });
};
