/**
 * Main application routes
 */

'use strict';

module.exports = function(app) {

  app.use('/api/id', require('./api/id'));

  app.route('/')
    .get(function(req, res) {
      res.send(':)');
    });
};
