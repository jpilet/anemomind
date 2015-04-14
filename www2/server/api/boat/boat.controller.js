'use strict';

var _ = require('lodash');
var Boat = require('./boat.model');
var User = require('../user/user.model');
var mongoose = require('mongoose');

var userCanRead = function(user, boat) {
  if (!user || !user.id || !boat) {
    return false;
  }
  var userId = mongoose.Types.ObjectId(user.id);

  return (boat.admins && (_.findIndex(boat.admins, userId) >= 0))
    || (boat.readers && (_.findIndex(boat.readers, userId) >= 0));
}
var userCanWrite = function(user, boat) {
  if (!user || !user.id || !boat || !boat.admins) {
    return false;
  }
  var userId = mongoose.Types.ObjectId(user.id);
  return _.findIndex(boat.admins, userId) >= 0;
}

var validateBoatForUser = function(user, boat) {
  // Make sure the following arrays contain unique values.
  var fields = ['admins', 'readers', 'invited'];
  for (var i = 0; i < fields.length; ++i) {
    var field = fields[i];
    boat[field] = _.uniq(boat[field], function(x) { return JSON.stringify(x); });
  }

  // We do not want to let a user remove himself as admin.
  if (!userCanWrite(user, boat)) {
    var userId = mongoose.Types.ObjectId(user.id);
    boat.admins.push(userId);
  }
}

// Get list of boats
exports.index = function(req, res) {
  if (!req.user) { return res.send(403); }
  var user = mongoose.Types.ObjectId(req.user.id);
  var query = {$or: [{admins: { $in: [user] } }, {readers: { $in: [user]}}]};

  Boat.find(query, function (err, boats) {
    if(err) { return handleError(res, err); }
    return res.json(200, boats);
  });
};

// Get a single boat
exports.show = function(req, res) {
  Boat.findById(req.params.id, function (err, boat) {
    if(err) { return handleError(res, err); }
    if (!userCanRead(req.user, boat)) { return res.send(403); }
    if(!boat) { return res.send(404); }
    return res.json(boat);
  });
};

// Creates a new boat in the DB.
exports.create = function(req, res) {
  if (!req.user) { return res.send(403); }
  var user = mongoose.Types.ObjectId(req.user.id);
  var boat = req.body;

  // Make sure the user who creates a boat has
  // administrative rights.
  if (!userCanWrite(req.user, boat)) {
    if (boat.admins && boat.admins instanceof Array) {
      boat.admins.push(user);
    } else {
      boat.admins = [ user ];
    }
  }

  Boat.create(boat, function(err, boat) {
    if(err) { return handleError(res, err); }
    return res.json(201, boat);
  });
};

// Updates an existing boat in the DB.
exports.update = function(req, res) {
  var userId = mongoose.Types.ObjectId(req.user.id);
  if(req.body.id) { delete req.body.id; }
  Boat.findById(req.params.id, function (err, boat) {
    if (err) { return handleError(res, err); }
    if(!boat) { return res.send(404); }
    if (!userCanWrite(req.user, boat)) { return res.send(403); }

    var toRemove;
    if (req.body.toRemove) {
      toRemove = req.body.toRemove;
      delete req.body.toRemove;
    }
    _.assign(boat, req.body);

    validateBoatForUser(req.user, boat);

    boat.save(function (err) {
      if (err) { return handleError(res, err); }
      return res.json(200, boat);
    });
  });
};

// Invite a user to join a boat.
exports.inviteUser = function(req, res) {

  if (req.body._id) { delete req.body._id; }
  if (!req.body.email) { return res.send(401); }

  var invitedEmail = req.body.email;
  var invitedAdmin = (req.body.admin?true:false);

  if (invitedEmail == req.user.email) { return res.send(200); }

  Boat.findById(req.params.id, function (err, boat) {
    if (err) { return handleError(res, err); }
    if(!boat) { return res.send(404); }
    if (!userCanWrite(req.user, boat)) { return res.send(403); }

    // Is the guest already invited?
    for (var i in boat.invited) {
      if (boat.invited[i].email == invitedEmail) {
        return res.json(200, { message: 'user already invited: ' + invitedEmail});
      }
    }

    // The boat owner has write access.
    // Let's search for the guest.
    User.find({email: invitedEmail}, function (err, users) {
      if(err) { return handleError(res, err); }
      if (users.length == 0) {
        // The user does not exist yet. Let's invite him/her.
        if (!boat.invited) { boat.invited = []; }
        boat.invited.push({email: invitedEmail, admin: invitedAdmin});

        validateBoatForUser(req.user, boat);

        boat.save(function (err) {
          if (err) { return handleError(res, err); }
          return res.json(200, {
             message: 'user invited at address: ' + req.body.email,
             boat: boat
          });
        });
      } else {
        // The invited user is already registered.
        var invitedId = mongoose.Types.ObjectId(users[0].id);

        // Has it access to the boat already?
        if (userCanWrite(users[0], boat)) {
          return res.json(200, { message: 'Guest is already a member of the team.' });
        }
        _.remove(boat.admins, function(a) { return invitedId.equals(a); });
        _.remove(boat.readers, function(a) { return invitedId.equals(a); });

        if (invitedAdmin) {
          boat.admins.push(invitedId);
        } else {
          boat.readers.push(invitedId);
        }
        validateBoatForUser(req.user, boat);
        boat.save(function (err) {
          if (err) { return handleError(res, err); }
          return res.json(200, {
            message: 'user ' + users[0].name + ' added as ' + (invitedAdmin ? 'admin' : 'reader'),
            user: users[0].profile,
            boat: boat
          });
        });
      }
    }); // User.find
  }); // Boat.findById
};

function handleError(res, err) {
  return res.send(500, err);
}
