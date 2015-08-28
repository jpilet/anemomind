/**
 * Main application routes
 */

'use strict';

module.exports = function(app) {

  app.use('/api/id', require('./api/id'));
  app.use('/api/live', require('./api/live'));
  app.use('/api/mailrpc', require('./api/mailrpc'));

  app.route('/')
    .get(function(req, res) {
      res.send(':)');
    });
};
