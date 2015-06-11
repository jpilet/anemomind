/**
 * Main application routes
 */

'use strict';

var errors = require('./components/errors');

module.exports = function(app) {

  // Insert routes below
  app.use('/api/boxexecs', require('./api/boxexec'));
  app.use('/api/events', require('./api/event'));
  app.use('/api/boats', require('./api/boat'));
  app.use('/api/mailrpc', require('./api/mailrpc'));
  app.use('/api/tiles', require('./api/tiles'));
  app.use('/api/upload', require('./api/upload'));
  app.use('/api/users', require('./api/user'));

  app.use('/auth', require('./auth'));
  
  // All undefined asset or api routes should return a 404
  app.route('/:url(api|auth|components|app|bower_components|assets)/*')
   .get(errors[404]);

  // All other routes should redirect to the index.html
  app.route('/*')
    .get(function(req, res) {
      res.sendfile(app.get('appPath') + '/index.html');
    });
};
