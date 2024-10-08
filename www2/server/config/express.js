/**
 * Express configuration
 */

'use strict';

var express = require('express');
var favicon = require('serve-favicon');
var morgan = require('morgan');
// var compression = require('compression');
var bodyParser = require('body-parser');
var methodOverride = require('method-override');
var cookieParser = require('cookie-parser');
var errorHandler = require('errorhandler');
var path = require('path');
var config = require('./environment');
var passport = require('passport');
var session = require('express-session');
var mongoStore = require('connect-mongo')(session);
var mongoose = require('mongoose');

module.exports = function(app) {
  var env = app.get('env');

  app.set('views', config.root + '/server/views');
  app.engine('html', require('ejs').renderFile);
  app.set('view engine', 'html');
  // app.use(compression());
  app.use(bodyParser.urlencoded({limit: '2mb',  extended: false }));
  app.use(bodyParser.json({ limit: '2mb' }));
  app.use(bodyParser.raw({type: 'application/octet-stream', limit: '10mb'}));
  app.use(methodOverride());
  app.use(cookieParser());
  app.use(passport.initialize());

  // Persist sessions with mongoStore
  // We need to enable sessions for passport twitter because its an oauth 1.0 strategy
  app.use(session({
    secret: config.secrets.session,
    resave: true,
    saveUninitialized: true,
    store: new mongoStore({ mongooseConnection: mongoose.connection })
  }));
  
  if ('production' === env) {
    app.use(favicon(path.join(config.root, 'public', 'favicon.ico')));
    app.use(express.static(path.join(config.root, 'public')));
    app.set('appPath', config.root + '/public');
    app.use(morgan('dev'));
  }

  if ('development' === env || 'test' === env) {
    app.use(require('connect-livereload')());

    const vhost = process.env.VHOST || 'client';
    const serveCss = express.static(path.join(config.root, vhost));
    app.use((req, res, next) => {
      if (req.path.match(/css$/)) {
        serveCss(req, res, next);
      } else {
	next();
      }
    });
    app.use(express.static(path.join(config.root, '.tmp')));
    app.use(express.static(path.join(config.root, vhost)));
    app.set('appPath', vhost);
    app.use(morgan('dev'));
    app.use(errorHandler()); // Error handler - has to be last
  }
};
