'use strict';

var app = require('../../../server'),
    supertest = require('supertest'),
    request = supertest(app);

describe('POST /api/upload', function() {
  it('should refuse unauthorized uploads', function(done) {
    request
      .post('/api/upload')
      .attach('text', 'test/server/upload/sample.txt')
      .expect(401, done);
  });

  it('should store authorized uploads', function(done) {
    var agent = supertest.agent(app);
    agent
      .post('/api/upload')
      .attach('text', 'test/server/upload/sample.txt')
      .expect(401, done);
  });
});