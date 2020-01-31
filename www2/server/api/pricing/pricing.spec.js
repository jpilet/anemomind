'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');
const controller = require('./pricing.controller');
var testUtils = require('../testUtils.spec');
var Q = require('q');

describe('Pricing  /api/pricing', function () {

    var testUser, boatid;
    var writableBoat, publicBoat;

    before(function (done) {
        testUtils.addTestUser('test')
            .then(function (user) {
                testUser = user;
                return testUtils.addTestBoat({ name: "writable boat", admins: [user.id] });
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

    var server = request(app);
    var token;

    it('should give the test user an auth token', function (done) {
        server
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

    it('get all plans', function (done) {
        request(app)
            .get('/api/pricing/getAllPlans')
            .expect(200)
            .end(function (err, res) {
                if (err) {
                    console.warn(err);
                    return done(err);
                }
                res.body.should.be.instanceof(Array);
                done();
            });
    });


    it('get all countries', function (done) {
        let countries = controller.getCountriesData();
        if (countries instanceof (Array)) {
            done();
        }
        else {
            let err = "error getting contries details";
            countries.errorMessage = err;
            done(countries)
        }
    });

    it('subscribe user to a plan', function (done) {
        request(app)
            .post('/api/pricing/subscribe/' + boatid)
            .set('Authorization', 'Bearer ' + token)
            // the name does not match, anemobox is missing. The test should fail.
            .send({
                stripeSource = "src_1FGs7eKcpwNgG6BNWXw6vaNf",
                email = testUser.email,
                country = "CHF",
                plan = "NA.PR",
                boatId = boatid,
            })
            .expect(200)
            .end(function (err, res) {
                if (err) {
                    console.warn(err);
                    return done(err);
                }
                res.body.should.be.instanceof(Array);
                done();
            });
    });


    it('get prorations for a subscription', function (done) {
        var subId = 'sub_FTtzxve542kq19';
        request(app)
            .get('/api/pricing/getProrations/' + subId)
            .expect(200)
            .end(function (err, res) {
                if (err) {
                    console.warn(err);
                    return done(err);
                }
                res.body.should.be.instanceof(Array);
                done();
            });
    });

    // TODO: Update subscription

});
