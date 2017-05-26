var dof = require('../components/deleteOldFiles.js');
var fs = require('fs');
var path = require('path');
var assert = require('assert');

var testDir = '/tmp/deleteOldFilesTest';

var filesAndData = [["A.txt", "This is some text file, I believe"],
                    ["B.dat", "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfadsfasdfadsfasdfasdf"],
                    ["C.d", "kattskit"]];

function makeTestFile(index, cb) {
  var x = filesAndData[index];
  fs.writeFile(path.join(testDir, x[0]), x[1], cb);
}

function prepare(cb) {
  fs.mkdir(testDir, function(err) {
    dof.pmap(makeTestFile, [0, 1, 2], function(data) {
      cb();
    });
  });
}

describe('delete old files', function() {
  it('try-fs', function() {
    fs.readdir('/tmp', function(err, files) {
      console.log("Got these files: %j", files);
      fs.stat(files[0], function(err, info) {
        console.log("Size of first files: %j", info);
      });
    });
  });

  it('try-clean', function(done) {
    prepare(function() {
      dof.fullReadDir(testDir, function(err, initialFiles) {
        assert(initialFiles.length == 3);
        dof.cleanFolder(
          testDir, 
          // Remove two files.
          function(files, cb) {cb(null, files.slice(0, 2));}, 
          function(err, output) {
            assert(!err);
            dof.fullReadDir(testDir, function(err, finalFiles) {
              assert(finalFiles.length == 1);
              done();
            });
          });        
      });
    });
  });

  it('compute-amount-remove-test', function() {
    assert(100 == dof.computeAmountToRemove(400, 200, 0.5));
    assert(-50 == dof.computeAmountToRemove(400, 200, 0.75));
  });

  it('file-removal', function() {
    var files = [{filename: "a.txt", size: 100, ctime: "2020"},
                 {filename: "b.txt", size: 100, ctime: "2029"},
                 {filename: "c.txt", size: 100, ctime: "2018"},
                 {filename: "d.txt", size: 100, ctime: "2010"}];
    var toRemove = dof.filesToRemoveByFreeFraction(files, 200, 0.5);
    assert(toRemove.length == 1);
    assert(toRemove[0].filename == "d.txt");
  });

  
});
