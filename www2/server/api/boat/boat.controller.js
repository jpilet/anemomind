'use strict';

var _ = require('lodash');
var Boat = require('./boat.model');
var User = require('../user/user.model');
var mongoose = require('mongoose');

var access = require('./access.js');
var userCanRead = access.userCanRead;
var userCanWrite = access.userCanWrite;

var winston = require('winston');
var mailer = require('../../components/mailer');
var transporter = mailer.transporter;

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

var sendInvitationEmail = function(inviter, email, boat, hasAnAccount) {
  var messageBody;
  if (hasAnAccount) {
    messageBody = 'Hello!\n'
     + inviter.name + ' invites you to see the navigation data ' +
    'of the boat ' + boat.name + '.\nPlease go to anemolab.com and log in ' +
    'with this email address: ' + email + '\n\nBest regards,\nAnemobot';
  } else {
    messageBody = 'Hello!\n'
     + inviter.name + ' invites you to see the navigation data ' +
    'of the boat ' + boat.name + '.\n'
    + 'Please create your account here: http://anemolab.com/signup?email=' + email
    + '\n\nBest regards,\nAnemobot';
  }
  transporter.sendMail({
    from: mailer.from,
    to: email,
    subject: 'You have been invited to see ' + boat.name + ' navigation data ' +
    'on anemolab.com',
    text: messageBody
  }, function(err, info) {
    if (err) {
      // TODO: update the info page and notify the inviter
      return winston.log('error', 'Failed to send invitation to: '+ email
                         + (err.stack ? '\n' + err.stack : '.'));
    }
    winston.log('info', inviter.name + '<' + inviter.email +'> sent an Invitation sent to: ' + email);
  });
}

// Get list of boats
exports.index = function(req, res) {
  access.readableBoats(req)
  .then(function (boats) {
    res.status(200).json(boats);
  })
  .catch(function (err) {
    handleError(res, err);
  });
};

// Get a single boat
exports.show = function(req, res) {
  Boat.findById(req.params.boatId, function (err, boat) {
    if(err) { return handleError(res, err); }
    if (!userCanRead(req.user, boat)) { return res.sendStatus(403); }
    if(!boat) { return res.sendStatus(404); }
    return res.json(boat);
  });
};

// Creates a new boat in the DB.
exports.create = function(req, res) {
  if (!req.user) { return res.sendStatus(403); }
  var user = mongoose.Types.ObjectId(req.user.id);
  var boat = req.body;

  if (boat._id) {
    try {
      var id = mongoose.Types.ObjectId(boat._id);
    } catch (e) {
      res.sendStatus(400);
      return;
    }
  }

  // Make sure the user who creates a boat has
  // administrative rights.
  if (!userCanWrite(req.user, boat)) {
    if (boat.admins && boat.admins instanceof Array) {
      boat.admins.push(user);
    } else {
      boat.admins = [ user ];
    }
  }

  Boat.create(boat, function(err, createdBoat) {
    if (err) {
      if (err.code == 11000) {
        // Duplicate key error.
        // Check if boatid, boat name and boxid match. If so, grant access.
        Boat.findById(id, function(err, dbBoat) {
          if (err) {
            // that should not happen
            console.warn('Duplication error when creating a boat, but can\'t fetch'
              + ' the duplicated boat! id: ' + id);
            res.sendStatus(500);
          } else {
            if ('' + dbBoat._id == '' + id
                && dbBoat.anemobox
                && dbBoat.anemobox === boat.anemobox
                && dbBoat.name === boat.name) {
              // it is a match. So we assume the user got all these information
              // by connection to the boat anemobox, and it should have access.
              if (!userCanWrite(req.user, dbBoat)) {
                dbBoat.admins.push(user);
                console.warn('Grant-on-create succeeded for user ' + user
                  + ' on boat ' + dbBoat._id + ' (' + dbBoat.name + ')');
                dbBoat.save(function (err) {
                  if (err) { return handleError(res, err); }
                  return res.status(201).json(dbBoat);
                });
              } else {
                // a user tries to create a boat twice. Well, that should not occur,
                // but it is no big deal. Let's pretend it worked.
                console.warn('User: ' + user + ' re-created boat ' + id
                             + ', it could be a bug in the app');
                return res.status(201).json(dbBoat);
              }
            } else {
              // The boat already exists but does not match.
              // We can't grant access.
              console.log('Grant-on-create refused because '
                + id + ' != ' + dbBoat._id
                + ' || ' + boat.name + ' != ' + dbBoat.name
                + ' || ' + boat.anemobox + ' != ' + dbBoat.anemobox);
              return res.sendStatus(400);
            }
          }
        });
      } else {
        return handleError(res, err);
      }
    } else {
      // creation worked.
      return res.status(201).json(createdBoat);
    }
  });
};

// Updates an existing boat in the DB.
exports.update = function(req, res) {
  var userId = mongoose.Types.ObjectId(req.user.id);
  if(req.body.id) { delete req.body.id; }
  if(req.body._id) { delete req.body._id; }
  Boat.findById(req.params.id, function (err, boat) {
    if (err) { return handleError(res, err); }
    if(!boat) { return res.sendStatus(404); }
    if (!userCanWrite(req.user, boat)) { return res.sendStatus(403); }

    var toRemove;
    if (req.body.toRemove) {
      toRemove = req.body.toRemove;
      delete req.body.toRemove;
    }
    _.assign(boat, req.body);

    validateBoatForUser(req.user, boat);

    boat.save(function (err) {
      if (err) { return handleError(res, err); }
      return res.status(200).json(boat);
    });
  });
};

// Invite a user to join a boat.
exports.inviteUser = function(req, res) {

  if (req.body._id) { delete req.body._id; }
  if (!req.body.email) { return res.sendStatus(401); }

  var invitedEmail = req.body.email.toLowerCase();
  var invitedAdmin = (req.body.admin?true:false);

  if (invitedEmail == req.user.email) { return res.sendStatus(200); }

  Boat.findById(req.params.id, function (err, boat) {
    if (err) { return handleError(res, err); }
    if(!boat) { return res.sendStatus(404); }
    if (!userCanWrite(req.user, boat)) { return res.sendStatus(403); }

    // Is the guest already invited?
    for (var i in boat.invited) {
      if (boat.invited[i].email == invitedEmail) {
        return res.status(200).json({ message: 'user already invited: ' + invitedEmail});
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

          sendInvitationEmail(req.user, req.body.email, boat, false);

          return res.status(200).json({
             message: 'user invited at address: ' + req.body.email,
             boat: boat
          });
        });
      } else {
        // The invited user is already registered.
        var invitedId = mongoose.Types.ObjectId(users[0].id);

        // Has it access to the boat already?
        if (userCanWrite(users[0], boat)) {
          return res.status(200).json({ message: 'Guest is already a member of the team.' });
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

          sendInvitationEmail(req.user, req.body.email, boat, true);

          return res.status(200).json({
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
  console.warn(err);
  return res.status(500).send(err);
}
