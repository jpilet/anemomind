'use strict';

var app = require(__dirname + '/../../../server');
var supertest = require('supertest');
var request = supertest(app);

describe('POST /api/upload', function() {
  it('should refuse unauthorized uploads', function(done) {
    request
      .post('/api/upload')
      .attach('text', 'test/server/upload/sample.txt')
      .expect(401, done);
  });
});
