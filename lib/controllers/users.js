'use strict';

var mongoose = require('mongoose'),
    User = mongoose.model('User'),
    passport = require('passport');

/**
 * Create user
 */
exports.create = function (req, res, next) {
  var newUser = new User(req.body);
  newUser.provider = 'local';
  newUser.save(function(err) {
    if (err) return res.json(400, err);
    
    req.logIn(newUser, function(err) {
      if (err) return next(err);

      return res.json(req.user.userInfo);
    });
  });
};

/**
 *  Get profile of specified user
 */
exports.show = function (req, res, next) {
  var userId = req.params.id;

  User.findById(userId, function (err, user) {
    if (err) return next(err);
    if (!user) return res.send(404);

    res.send({ profile: user.profile });
  });
};

/**
 * Change preferences
 */
exports.changePreferences = function(req, res) {
  var userId = req.user._id;
  User.findById(userId, function (err, user) {
    for (var prop in req.body) {
      console.log('updating req.body.' + prop + ' = ' + req.body[prop] + '\n');
      user[prop] = req.body[prop];
    }
    console.dir(user);
    user.save(function(err) {
      if (err) return res.send(400);
      res.send(200);
    });
  });
};

/**
 * Get current user
 */
exports.me = function(req, res) {
  res.json(req.user || null);
};