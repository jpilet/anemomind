var bdw = require('../server/components/boxdatawatch');
var assert = require('assert');
var common = require('../utilities/common.js');
var mkdirp = require('mkdirp');
var removeBoat = true;
var withTestBoat = require('./testboat.js');
var path = require('path');
var fs = require('fs');
var rmdir = require('rmdir')



describe('boxdatawatch', function() {
  it('Extract boat id from path', function() {
    assert.equal('119', bdw.getBoatIdFromPath('/tmp/boat119/a/b/rulle.txt'));
    assert.equal(null, bdw.getBoatIdFromPath('/tmp/box119/a/b/rulle.txt'));
    assert.equal(null, bdw.getBoatIdFromPath('/tmp/b/rulle.txt'));
  });

  it('watch', function(done) {
    var root = '/tmp/watched_logfiles';
    rmdir(root, function(err) {
      withTestBoat(function(boatId, cbDone) {
        logpath = path.join(root, 'boat' + boatId);
        var filename = path.join(logpath, "boat.dat");
        var otherFilename = path.join(logpath, "otherfile.dat");
        
        mkdirp(logpath, 0755, function(err) {
          assert(!err);

          bdw.startWatchForFiles(root, {"boat.dat": "boat.dat"}, function(err, info) {
            assert(!err);
            assert.equal(info.src, filename);
            assert.equal(info.boatId, boatId);
            assert.equal(info.dst, "boat.dat");
            cbDone();
          });

          fs.writeFile(
            otherFilename, "Some data that we are not interested in.",
            function(err) {
              assert(!err);
              fs.writeFile(filename, "This is the log file data", function(err) {
                assert(!err);
                // At this point, the watch should discover the file and post it for the box.
              });
            });
        });
      }, done);
    });
  });
});
