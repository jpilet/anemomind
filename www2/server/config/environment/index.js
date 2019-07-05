'use strict';

var path = require('path');
var _ = require('lodash');
var assert = require('assert');

function requiredProcessEnv(name) {
  if(!process.env[name]) {
    throw new Error('You must set the ' + name + ' environment variable');
  }
  return process.env[name];
}

// All configurations will extend these options
// ============================================
var all = {
  env: process.env.NODE_ENV,

  // Root path of server
  root: path.normalize(__dirname + '/../../..'),

  // Server port
  port: process.env.PORT || 8080,

  // Should we populate the DB with sample data?
  seedDB: false,

  secrets: {
    session: process.env.JWT_SECRET || 'www2-secret'
  },

  // List of user roles
  userRoles: ['guest', 'user', 'admin'],

  // MongoDB connection options
  mongo: {
    options: {
      safe: true
    }
  },

  facebook: {
    clientID:     process.env.FACEBOOK_ID || '1090167944444670',
    clientSecret: process.env.FACEBOOK_SECRET || 'secret',
    callbackURL:  (process.env.DOMAIN || '') + '/auth/facebook/callback'
  },

  twitter: {
    clientID:     process.env.TWITTER_ID || 'id',
    clientSecret: process.env.TWITTER_SECRET || 'secret',
    callbackURL:  (process.env.DOMAIN || '') + '/auth/twitter/callback'
  },

  google: {
    clientID:     process.env.GOOGLE_ID || 'id',
    clientSecret: process.env.GOOGLE_SECRET || 'secret',
    callbackURL:  (process.env.DOMAIN || '') + '/auth/google/callback'
  },

  // Would it make sense to have absolute paths here to?
  uploadDir: 'uploads',
  endpointDir: '/tmp/mail2',
  backupDestination: '/tmp/anemobackup',

  smtp: {
    host: 'mail.infomaniak.ch',
    port: 587,
    requireTLS: true,
    auth: { user: 'anemolab@calodox.org', pass: process.env.SMTP_PASSWORD || '' }
  },

  tryLoadBin: path.normalize(__dirname +
                               '/../../../../build/src/server/nautical/logimport/logimport_try_load')
};

// Export the config object based on the NODE_ENV
// ==============================================
assert(process.env.NODE_ENV);
module.exports = _.merge(
  all,
  require('./' + process.env.NODE_ENV + '.js') || {});
