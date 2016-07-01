'use strict';

var mongoose = require('mongoose');
var passport = require('passport');
var config = require('../config/environment');
var jwt = require('jsonwebtoken');
var expressJwt = require('express-jwt');
var compose = require('composable-middleware');
var User = require('../api/user/user.model');
var validateJwt = expressJwt({ secret: config.secrets.session });

function validateToken(req, res, next) {
  // allow access_token to be passed through query parameter as well
  if(req.query
     && req.query.hasOwnProperty('access_token')
     && req.query.access_token != "undefined") {
    req.headers.authorization = 'Bearer ' + req.query.access_token;
  }
  validateJwt(req, res, next);
}

function attachUserToRequest(req, res, next) {
  User.findById(req.user._id, function (err, user) {
    if (err) return next(err);
    if (!user) return res.send(401);

    req.user = user;
    next();
  });
}

function isAuthRequest(req) {
  return (req.query
          && req.query.hasOwnProperty('access_token')
          && req.query.access_token != "undefined")
     || (req.headers && req.headers.hasOwnProperty('authorization'));
}

// For content that can be either public or protected, we need a way
// to verify user identity if provided, or let through unauthorized queries.
// Of course, we reject bad authentications.
function maybeAuthenticated() {
  return compose().use(function(req, res, next) {
    if (isAuthRequest(req)) {
      return validateToken(req, res, next);
    } else {
      next();
    }
  }).use(function(req, res, next) {
    if (isAuthRequest(req)) {
      attachUserToRequest(req, res, next);
    } else {
      // make sure 'user' is not defined.
      if (req.user) {
        delete req.user;
      }
      next();
    }
  });
}

/**
 * Attaches the user object to the request if authenticated
 * Otherwise returns 403
 */
function isAuthenticated() {
  return compose()
    .use(validateToken)
    // Attach user to request
    .use(attachUserToRequest);
}

/**
 * Checks if the user role meets the minimum requirements of the route
 */
function hasRole(roleRequired) {
  if (!roleRequired) throw new Error('Required role needs to be set');

  return compose()
    .use(isAuthenticated())
    .use(function meetsRequirements(req, res, next) {
      if (config.userRoles.indexOf(req.user.role) >= config.userRoles.indexOf(roleRequired)) {
        next();
      }
      else {
        res.send(403);
      }
    });
}

/**
 * Returns a jwt token signed by the app secret
 */
function signToken(id) {
  return jwt.sign({ _id: id }, config.secrets.session, { expiresInMinutes: 60*5 });
}

/**
 * Set token cookie directly for oAuth strategies
 */
function setTokenCookie(req, res) {
  if (!req.user) return res.json(404, { message: 'Something went wrong, please try again.'});
  var token = signToken(req.user._id, req.user.role);
  res.cookie('token', JSON.stringify(token));
  res.redirect('/');
}

exports.maybeAuthenticated = maybeAuthenticated;
exports.isAuthenticated = isAuthenticated;
exports.hasRole = hasRole;
exports.signToken = signToken;
exports.setTokenCookie = setTokenCookie;
