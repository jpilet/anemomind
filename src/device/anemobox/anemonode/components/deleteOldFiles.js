var fs = require('fs');
var path = require('path');
var assert = require('assert');
var exec = require('child_process').exec;
var diskspace = require('diskspace');

function pmapOneElement(index, f, data, results, cb) {
  f(data[index], function(x) {
    results[index] = x;
    cb();
  });
}

function pmap(f, data, cb) {
  assert(data instanceof Array);
  var counter = 0;
  var results = new Array(data.length);
  for (var i = 0; i < data.length; i++) {
    pmapOneElement(i, f, data, results, function() {
      counter++;
      if (counter == data.length) {
        cb(results);
      }
    });
  }
}

function withFileInfo(filename, cb) {
  fs.stat(filename, function(err, info) {
    if (err) {
      cb({err: err, filename: filename});
    } else {
      info.filename = filename;
      cb(info);
    }
  });
}

function withFullPath(base) {
  return function(filename) {
    return path.join(base, filename);
  }
}

function removeFile(file, cb) {
  fs.unlink(file.filename, function(err) {
    cb({err: err, file: file});
  });
}

function removeManyFiles(files, cb) {
  assert(files instanceof Array);
  pmap(removeFile, files, function(results) {
    console.log("results = %j", results);
    for (var i = 0; i < results.length; i++) {
      var x = results[i];
      if (x.err) {
        console.log("WARNING: Failed to remove '%s'", x.filename);
      }
    }
    cb(null, results);
  });
}

function noErr(x) {
  return !x.err;
}

function fullReadDir(folder, cb) {
  fs.readdir(folder, function(err, files) {
    if (err) {
      cb(err);
    } else {
      pmap(withFileInfo, files.map(withFullPath(folder)), function(files) {
        cb(null, files);
      });
    }
  });
}

// folder: A string path to a folder that should be scanned
//
// fileFilter: A function that takes as input an array of
//             files (as output from fs.readdir) and returns
//             all elements of that array that should be removed.
//             See below for such filters.
function cleanFolder(folder, filesToRemoveFilter, cb) {
  fullReadDir(folder, function(err, allFiles) {
    if (err) {
      cb(err);
    } else {
      filesToRemoveFilter(allFiles.filter(noErr), function(err, filesToRemove) {
        if (err) {
          cb(err);
        } else {
          console.log(
            "Remove %d/%d files in folder %s",
            filesToRemove.length, allFiles.length, folder);
          removeManyFiles(filesToRemove, cb);
        }
      });
    }
  });
}



///////////////////////////////////
///////// filters: Strategies for selecting what files to remove

/*

We know the amount of used space U of our files.
We know the amount of free space F on the disk.
We know a constant fraction k, e.g. k=0.5

We are looking for the amount R to remove so that 

   (U - R)/(U + F) = k

      <=>

   R = U - 0.5*(U + F)

 E.g. U=400, F=200, k=0.5 =>
  
   R = 400 - 0.5*(400 + 200) = 400 - 300 = 100
  

*/
function computeAmountToRemove(U, F, k) {
  return U - k*(U + F);
}

function add(a, b) {return a + b;}

function creationTime(file) {
  return file.ctime; // Is this value representative?
}

function byDate(fileA, fileB) {
  return creationTime(fileA) < creationTime(fileB)? -1 : 1;
}

// This function takes a list of files and returns the 
// files that we would like to remove. Fraction is 0 < fraction < 1
function filesToRemoveByFreeFraction(allFiles, freeSpaceBytes, fraction) {
  assert(0 < fraction);
  assert(fraction < 1);
  assert(typeof freeSpaceBytes == "number");
  var filesWithSize = allFiles.filter(function(f) {return f.size;}).sort(byDate);
  var totalSizeUsed = filesWithSize.map(function(f) {return f.size;}).reduce(add);
  var amountToRemove = computeAmountToRemove(totalSizeUsed, freeSpaceBytes, fraction);
  var toRemove = [];
  var bytesRemovedSoFar = 0;
  for (var i = 0; (i < filesWithSize.length) 
       && (bytesRemovedSoFar < amountToRemove); i++) {
    var f = filesWithSize[i];
    toRemove.push(f);
    bytesRemovedSoFar += f.size;
  }
  return toRemove;
}

// Constructs a filter that can be passed
// as second argument to clean folder.
function filterByFreeFraction(folder, fraction) {
  return function(files, cb) {
    diskspace.check(folder, function(err, info) {
      if (err) {
        cb(err);
      } else {
        var filtered = filesToRemoveByFreeFraction(
          files, info.free, fraction);
        cb(null, filtered);
      }
    });
  }
}

////// Wraps cleanFolder
function cleanFolderByFraction(folder, fraction, cb) {
  cleanFolder(folder, filterByFreeFraction(folder, fraction), cb);
}

function easyCleanFolder(folder, cb) {
  cleanFolderByFraction(folder, 0.5, cb);
}

module.exports.cleanFolder = cleanFolder;
module.exports.pmap = pmap;
module.exports.fullReadDir = fullReadDir;
module.exports.computeAmountToRemove = computeAmountToRemove;
module.exports.filesToRemoveByFreeFraction = filesToRemoveByFreeFraction;
module.exports.easyCleanFolder = easyCleanFolder;
