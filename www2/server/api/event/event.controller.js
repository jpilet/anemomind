'use strict';

var Q = require('q');
var _ = require('lodash');
var Event = require('./event.model');
var mongoose = require('mongoose');
var multer  = require('multer');
var fs = require('fs');
var config = require('../../config/environment');
var mkdirp = require('mkdirp');

var boatAccess = require('../boat/access.js');
var backup = require('../../components/backup');

// Encoding hex mongoid in urls is too long.
// we should switch to https://www.npmjs.com/package/hashids
// at some point.
// In the mean time, this function can still be used.
function stringIsObjectId(a) {
  return /^[0-9a-fA-F]{24}$/.test(a);
}

var canRead = function(req, event) {
  // A user can access a note if he/she is the author
  if (req.user && req.user.id && event.author == req.user.id) {
    return Q.Promise(function(resolve, reject) {
      resolve(req.user.id, event);
    });
  }

  // Otherwise, the user needs write access to the boat the note is
  // attached to.
  return boatAccess.userCanReadBoatId(req.user.id, event.boat);
}

var canWrite = function(req, event) {
  return boatAccess.userCanWriteBoatId(req.user.id, event.boat);
}

function sendEventsWithQuery(res, query) {
  Event.find(query, function (err, events) {
    if(err) { return handleError(res, err); }
    return res.status(200).json(events);
  });
}

// Get the latest readable events
exports.index = function(req, res) {
  try {
    if (!req.user) {
      return res.sendStatus(401);
    }

    if (req.query.b) {
      boatAccess.userCanReadBoatId(req.user.id, req.query.b)
      .then(function() { sendEventsWithQuery(res, { boat: req.query.b }); })
      .catch(function(err) { res.sendStatus(403); });
    } else {
      boatAccess.readableBoats(req.user.id)
        .then(function(boats) {
          if (boats.length == 0) {
            return res.status(200).json([]);
          }
          sendEventsWithQuery({ boat: { $in : _.map(boats, '_id') } });
        })
        .catch(function(err) { res.sendStatus(403); });
    }
  } catch(err) {
    console.warn(err);
    console.warn(err.stack);
    res.sendStatus(500);
  }
};

// Get a single event
exports.show = function(req, res) {
  Event.findById(req.params.id, function (err, event) {
    if(err) { return handleError(res, err); }
    if(!event) { return res.sendStatus(404); }

    canRead(req, event)
    .then(function() { res.json(event); })
    .catch(function(err) { res.sendStatus(403); });
  });
};

// Creates a new event in the DB.
exports.create = function(req, res) {
  try {
  if (!req.user) {
    return res.sendStatus(401);
  }
  if (!req.body.boat) {
    return res.sendStatus(400);
  }

  if (!stringIsObjectId(req.body.boat)) {
    return res.sendStatus(400);
  }

  var user = mongoose.Types.ObjectId(req.user.id);
  var event = req.body;
  event.author = user;
  event.boat = mongoose.Types.ObjectId(req.body.boat);

  canWrite(req, event)
    .then(function() {
      // User is allowed to add a note for this boat.

      Event.create(event, function(err, event) {
                   if(err) { return handleError(res, err); }
                   return res.status(201).json(event);
                   });
    })
    .catch(function(err) {
      res.sendStatus(403);
    });
  } catch(err) {
    console.warn(err);
    console.warn(err.stack);
    res.sendStatus(500);
  }
};

exports.remove = function(req, res) {
  if (!req.user) {
    return res.sendStatus(401);
  }
  if (!req.body.boat) {
    return res.sendStatus(400);
  }
  boatAccess.userCanWriteBoatId(req.user.id, req.body.boat)
    .then(function() {
      // User is allowed to add a note for this boat.
      var user = mongoose.Types.ObjectId(req.user.id);
      var event = req.body;
      event.author = user;
      event.boat = mongoose.Types.ObjectId(req.body.boat);

      Event.create(event, function(err, event) {
                   if(err) { return handleError(res, err); }
                   return res.status(201).json(event);
                   });
    })
    .catch(function(err) { res.sendStatus(403); });
};

function handleError(res, err) {
  console.log('error: ' + err);
  return res.sendStatus(500, err);
}

var checkAccess = function(checkFunc, req, res, next) {
  if (!req.user || !req.user.id) {
    return res.sendStatus(401);
  }

  if (!req.params.boatId) {
    return res.sendStatus(400);
  }

  checkFunc(req.user.id, req.params.boatId)
    .then(next)
    .catch(function() {
      return res.sendStatus(403);
    });
};

exports.boatWriteAccess = function(req, res, next) {
  return checkAccess(boatAccess.userCanWriteBoatId, req, res, next);
}

exports.boatReadAccess = function(req, res, next) {
  return checkAccess(boatAccess.userCanReadBoatId, req, res, next);
};

var photoUploadPath = fs.realpathSync(config.uploadDir) + '/photos/';
console.log('Uploading photos to: ' + photoUploadPath);

// mkdirp is async, so we take the opportunity to run before multer with
// express chained handlers
exports.createUploadDirForBoat = function(req, res, next) {
  if (req.params.boatId && req.params.boatId.match(/^[0-9a-zA-Z_-]+$/)) {
    mkdirp(photoUploadPath + '/' + req.params.boatId, next);
  } else {
    next();
  }
};

// This handler can assume that the user is authentified, it has write access,
// and the upload folder for the boat has been created.
exports.postPhoto = multer({
  dest: photoUploadPath,

  rename: function (fieldname, filename) {
    // Validation is done in onFileUploadStart
    return filename;
  },

  onFileUploadStart: function (file, req, res) {
    // Recognizes a UUID as generated by the app
    var uuidRegexp = 
      /^[A-F0-9]{8}-[A-F0-9]{4}-4[A-F0-9]{3}-[89aAbB][A-F0-9]{3}-[A-F0-9]{12}.jpg$/;

    if (file.originalname.match(uuidRegexp)) {
      if (!fs.existsSync(file.path)) {
        console.log('photoHandler: accepting file: ' + file.originalname);
        return true;
      } else {
        console.log('photoHandler: rejecting existing photo: '
                    + file.originalname);
        res.sendStatus(403);
        return false;
      }
    }
    console.log('photoHandler: rejecting bad filename: '
                + file.originalname);
    res.sendStatus(400);
    return false;
  },

  changeDest: function(dest, req, res) {
    if (req.params.boatId && req.params.boatId.match(/^[0-9a-zA-Z_-]+$/)) {
      var dir = dest + '/' + req.params.boatId;
      return dir;
    }
    return dest;
  },

  onFileUploadComplete: function(file, req, res) {
    res.sendStatus(201);
    backup.backupPhotos();
  }
});

exports.photoUploadPath = photoUploadPath;
