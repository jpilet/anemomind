'use strict';

var should = require('should');
var backup = require('./index');
var fs = require('fs');
var config = require('../../config/environment');

describe('backupPhotos', function() {
  before(function(done) {
    fs.writeFile(config.uploadDir + '/photos/test_photo', "test",
      function(err) {
        if (err) {
          done(err);
        } else {
          fs.writeFile(config.uploadDir + '/anemologs/test_log', "test", done);
        }
      });
    }
  );

  function checkErrAndFile(err, file, done) {
    if (err) {
      done(err);
    }
    var path = config.backupDestination + '/' + file;
    fs.readFile(path, done);
  }

  it('Should save the content of uploads/photos', function(done) {
    backup.backupPhotos(function(err) {
      checkErrAndFile(err, 'photos/test_photo', done);
    });
  });
  it('Should save the log folder', function(done) {
    backup.pushLogFilesToProcessingServer(function(err) {
      checkErrAndFile(err, 'anemologs/test_log', done);
    });
  });

  after(function(done) {
    fs.unlinkSync(config.backupDestination + '/photos/test_photo');
    fs.unlinkSync(config.uploadDir + '/photos/test_photo');
    fs.unlinkSync(config.backupDestination + '/anemologs/test_log');
    done();
  });
});

