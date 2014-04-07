'use strict';

var app = require('../../../server'),
    supertest = require('supertest'),
    request = supertest(app),
    mongoose = require('mongoose');

describe('POST /api/upload', function() {
  it('should refuse unauthorized uploads', function(done) {
    request(app)
      .post('/api/upload')
      .attach('text', 'test/server/upload/sample.txt')
      .expect(401, done);
  });

  it('should store authorized uploads', function(done) {
    request(app)
      .post('/api/upload')
      .attach('text', 'test/server/upload/sample.txt')
      .expect(401, done);
  });
});



var supertest = require('supertest');
var app = require('../lib/your_app_location');

var request = supertest(app);

describe('when user not logged in', function() {
    describe('POST /api/posts', function() {
        var agent1 = supertest.agent(app);

        agent1
            .post(API.url('posts'))
            .set('Accept', 'application/json')
            .send(post: data)
            .(end(function(err, res) {
                should.not.exist(err);
                res.should.have.status(401);
                should.exist(res.headers['set-cookie']);
                done();
            }));
    });
});