'use strict';

var User = require('./user.model');
var Boat = require('../boat/boat.model');
var mongoose = require('mongoose');
var passport = require('passport');
var config = require('../../config/environment');
var jwt = require('jsonwebtoken');
var winston = require('winston');
var generatePassword = require('password-generator');
var transporter = require('../../components/mailer').transporter;

var validationError = function(res, err) {
  return res.status(422).json(err);
};

var checkForInvite = function(user) {
  Boat.find({invited: {$elemMatch: {email: user.email}}}, function (err, boats) {
    for (var i in boats) {
      for (var j in boats[i].invited) {
        if (boats[i].invited[j].email === user.email) {

          var pushAction = boats[i].invited[j].admin ? {"admins": user._id} : {"readers": user._id};
          var action = {$push: pushAction, $pull: {"invited": {"email": user.email}}};
          Boat.findByIdAndUpdate(
            boats[i]._id,
            action,
            {safe: true, upsert: true, new : true},
            function(err, model) {
              if (err) return winston.log('err', err);
            }
          );
        }
      }
    }
  });
}

/**
 * Get list of users
 * restriction: 'admin'
 */
exports.index = function(req, res) {
  User.find({}, '-salt -hashedPassword', function (err, users) {
    if(err) return res.status(500).send(err);
    res.status(200).json(users);
  });
};

/**
 * Creates a new user
 */
exports.create = function (req, res, next) {
  var newUser = new User(req.body);
  newUser.provider = 'local';
  newUser.role = 'user';
  newUser.save(function(err, user) {
    if (err) return validationError(res, err);
    checkForInvite(user);
    var token = jwt.sign({_id: user._id }, config.secrets.session, { expiresInMinutes: 60*5 });
    res.json({ token: token, user: user.profile });
  });
};

/**
 * Get a single user
 */
exports.show = function (req, res, next) {
  var userId = req.params.id;

  User.findById(userId, function (err, user) {
    if (err) return next(err);
    if (!user) return res.sendStatus(401);
    if (!req.user) {
      // client not authenticated, serving a restricted object for
      // a public page.
      res.json({
               '_id': user.profile._id,
               'name': user.profile.name
               });
    } else {
      res.json(user.profile);
    }
  });
};

/**
 * Deletes a user
 * restriction: 'admin'
 */
exports.destroy = function(req, res) {
  User.findByIdAndRemove(req.params.id, function(err, user) {
    if(err) return res.status(500).send(err);
    return res.sendStatus(204);
  });
};

/**
 * Change a users password
 */
exports.changePassword = function(req, res, next) {
  var userId = req.user._id;
  var oldPass = String(req.body.oldPassword);
  var newPass = String(req.body.newPassword);

  User.findById(userId, function (err, user) {
    if(user.authenticate(oldPass)) {
      user.password = newPass;
      user.save(function(err) {
        if (err) return validationError(res, err);
        res.sendStatus(200);
      });
    } else {
      res.sendStatus(403);
    }
  });
};

/**
 * Resets a users password
 */
exports.resetPassword = function(req, res, next) {

  var newPass = generatePassword();
  User.findOne({
    email: req.body.email
  }, function (err, user) {
    if (err) return next(err);
    if (!user) {
      return validationError(res, err);
    }
    transporter.sendMail({
      from: 'anemobot@gmail.com',
      to: req.body.email,
      subject: 'Your new password for anemolab.com',
      text: 'A new password has been generated for you: ' + newPass +
            '\n\nBest regards,\nAnemobot'
    }, function(err, info) {
      if (err) {
        winston.log('err', err);
        return res.json(401);
      }
      user.password = newPass;
      user.save(function(err) {
        if (err) return validationError(res, err);
        winston.log('info', 'User with email ' + req.body.email + ' resetted his password');
        res.sendStatus(200);
      });
    });
  });
};

/**
 * Get my info
 */
exports.me = function(req, res, next) {
  var userId = req.user._id;
  User.findOne({
    _id: userId
  }, '-salt -hashedPassword', function(err, user) { // don't ever give out the password or salt
    if (err) return next(err);
    if (!user) return res.sendStatus(401);
    res.json(user);
  });
};

/**
 * Authentication callback
 */
exports.authCallback = function(req, res, next) {
  res.redirect('/');
};
