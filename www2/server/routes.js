/**
 * Main application routes
 */

'use strict';

var errors = require('./components/errors');

module.exports = function(app) {

  // Insert routes below
  app.use('/api/updates', require('./api/update'));
  app.use('/api/boxexecs', require('./api/boxexec'));
  app.use('/api/boatstats', require('./api/boatstat'));
  app.use('/api/contact', require('./api/contact'));
  app.use('/api/events', require('./api/event'));
  app.use('/api/boats', require('./api/boat'));
  app.use('/api/map', require('./api/map'));
  app.use('/api/mailrpc', require('./api/mailrpc')); // old
  app.use('/api/endpoint', require('./api/endpoint')); // new
  app.use('/api/session', require('./api/session'));  
  app.use('/api/timeset', require('./api/timeset'));  
  app.use('/api/tiles', require('./api/tiles'));
  app.use('/api/users', require('./api/user'));
  app.use('/api/chart', require('./api/chart'));
  app.use('/api/export', require('./api/export'));
  app.use('/api/files', require('./api/files'));
  app.use('/api/perfstat', require('./api/perfstats'));
  app.use('/api/pricing', require('./api/pricing'));

  app.use('/auth', require('./auth'));
  app.use('/map', require('./social')(app));
  
  // All undefined asset or api routes should return a 404
  app.route('/:url(api|auth|components|app|bower_components|assets|.well-known)/*')
   .get(errors[404]);

  // All other routes should redirect to the index.html
  app.route('/*').get(function(req, res) {
    res.sendfile(app.get('appPath') + '/index.html');
  });
};
