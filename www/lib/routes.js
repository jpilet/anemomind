'use strict';

var index = require('./controllers');
var users = require('./controllers/users');
var upload = require('./controllers/upload');
var session = require('./controllers/session');
var races = require('./controllers/races');
var tilesGeoJSON = require('./controllers/tilesGeoJSON');

var middleware = require('./middleware');

/**
 * Application routes
 */
module.exports = function(app) {

  // Server API Routes
  app.post('/api/users', users.create);
  app.put('/api/users', middleware.auth, users.changePreferences);
  app.get('/api/users/me', users.me);
  app.get('/api/users/:id', users.show);

  app.post('/api/upload', middleware.auth, upload.upload);
  app.post('/api/upload/store', upload.storeData);

  app.post('/api/session', session.login);
  app.del('/api/session', session.logout);

  app.get('/api/races', middleware.auth, races.list);
  app.get('/api/races/:id', middleware.auth, races.raceDetail);

  app.get('/api/tilesGeoJSON/:scale/:x/:y/:boat/:startsAfter?/:endsBefore?', tilesGeoJSON.retrieve);

  // All undefined api routes should return a 404
  app.get('/api/*', function(req, res) {
    res.send(404);
  });
  
  // All other routes to use Angular routing in app/scripts/app.js
  app.get('/partials/*', index.partials);
  app.get('/*', middleware.setUserCookie, index.index);
};
