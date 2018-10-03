'use strict';

const should = require('should');
const app = require('../../app');
const request = require('supertest');
const PerfStat = require('./perfstats.model');
const testUtils = require('../testUtils.spec');
const esaPolarLoader = require('../files/esapolar');
const mongoose = require('mongoose');

const filename = 'polarESA20170708_1629.ESA';
const esaPolarFile = __dirname + '/../../../../datasets/astradata/Regata/polarESA20170708_1244.ESA';
describe('ESA Polar', () => {
  let boatid;
  let testUser;
  let token;

  before((done) => {
    testUtils.addTestUser('test')
    .then(function(user) {
        testUser = user;
        return testUtils.addTestBoat({name: "writable boat", admins: [ user.id ]});
      })
    .then(function (boat) {
         boatid = boat._id + '';
         done();
     })
    .catch(function (err) {
      console.warn(err);
      done(err);
    });
  });

  it('should parse and upload an ESA file', (done) => {
    esaPolarLoader.readEsaPolar(esaPolarFile)
      .then((esaPolar) => {
        return esaPolarLoader.uploadEsaPolar(boatid, esaPolar);
      })
      .then((data) => { done(); })
      .catch(done);
  });

  it('should give the test user an auth token', function(done) {
    request(app)
      .post('/auth/local')
      .send({ email: 'test@test.anemomind.com', password: 'anemoTest' })
      .expect(200)
      .expect('Content-Type', /json/)
      .end(function (err, res) {
           token = res.body.token;
           if (err) return done(err);
           return done();
       });
  });

  it('should reply with a boat perf array', (done) => {
    request(app)
      .get('/api/perfstat/' + boatid)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end((err, res) => {
        if (err) return done(err);
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(1);
        res.body[0].should.have.property('name');
        res.body[0].name.should.equal(filename);
        res.body[0].should.have.property('boat');
        res.body[0].boat.should.equal(boatid);
        done();
      });
  });

  it('should reply with a single boat perf', (done) => {
    request(app)
      .get('/api/perfstat/' + boatid + '/' + filename)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end((err, res) => {
        if (err) return done(err);
        res.body.should.have.property('name');
        res.body.name.should.equal(filename);
        res.body.should.have.property('boat');
        res.body.boat.should.equal(boatid);
        done();
      });
  });

  after((done) => {
    testUtils.cleanup();

    PerfStat.remove({
      boat: boatid,
      name: filename
    }, () => { done() });
  });
});

