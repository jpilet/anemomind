'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');

describe('GET /api/upload', function() {

  it('should accept a file upload', function(done) {
    request(app)
      .post('/api/upload')
      .set('Content-Disposition', 'form-data; name="uploadedfile"; filename="testfile.txt"')
      .send({ testobject: true })
      .expect(201)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.equal('ok');
        done();
      });
  });
});
