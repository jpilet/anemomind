/**
 * Main application routes
 */

'use strict';

module.exports = function(app) {

  app.use('/api/id', require('./api/id'));
  app.use('/api/live', require('./api/live'));

  app.route('/')
    .get(function(req, res) {
      res.send(':)');
    });
};
