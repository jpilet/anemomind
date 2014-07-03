'use strict';

var express = require('express'),
    path = require('path'),
    fs = require('fs'),
    mongoose = require('mongoose');
var morgan  = require('morgan');

/**
 * Main application file
 */

// Set default node environment to development
process.env.NODE_ENV = process.env.NODE_ENV || 'production';

// Application Config
var config = require('./lib/config/config');

// Connect to database
var db = mongoose.connect(config.mongo.uri, config.mongo.options);

// Bootstrap models
var modelsPath = path.join(__dirname, 'lib/models');
fs.readdirSync(modelsPath).forEach(function (file) {
  if (/(.*)\.(js$|coffee$)/.test(file)) {
    require(modelsPath + '/' + file);
  }
});

// Populate empty DB with sample data
// require('./lib/config/dummydata');
  
// Passport Configuration
var passport = require('./lib/config/passport');

var app = express();

function initAfterLog() {
  // Hack for psaros33
  app.use('/sui300', express.static(__dirname + '/sui300'));

  app.use(express.bodyParser({uploadDir:__dirname + '/uploads'}));

  // Express settings
  require('./lib/config/express')(app);

  // Routing
  require('./lib/routes')(app);

  // Start server
  app.listen(config.port, function () {
      console.log('Express server listening on port %d in %s mode', config.port, app.get('env'));
  });
}

// Log accesses to file
var logFile = fs.createWriteStream(config.logfile, {flags: 'a'});
logFile.on('error', function(err) {
  console.log(config.logfile + ": " + err);
  logFile.end();
  if (config.env == "development") {
    initAfterLog();
  }
});
logFile.once('open', function(fd) {
  console.log('Logging to: ' + config.logfile);
  app.use(morgan({stream: logFile}));
  initAfterLog();
});


// Expose app
exports = module.exports = app;
