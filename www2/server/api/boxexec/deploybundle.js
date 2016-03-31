// NOTE: The code in this file should be self-contained
// and depend on nothing but the Node standard library,
// because it will be deployed on the box to install a
// bundle.

var path = require('path');
var fs = require('fs');
var exec = require('child_process').exec;
var branch = 'master';

// Simplified exec with string errors for easy packing.
function exec2(cmd, cb) {
  exec(cmd, function(err, stdout, stderr) {
    if (err) {
      var commonErrorMessage = 'Failed to execute "' + cmd + '" because "';
      if (stderr == null  || stderr.length == 0) {
        cb(commonErrorMessage + err + '". ');
      } else {
        cb(commonErrorMessage + stderr + '". ');
      }
    } else {
      cb(null, {stdout: stdout, stderr: stderr});
    }
  });
}

function makeLocalCopy(directory, suffix, cb) {
  var dir2 = directory + suffix;
  exec2('rm ' + dir2 + ' -rf; cp ' + directory + ' ' + dir2 + ' -r',
        function(err, out) {
          if (err) {
            exec2('rm ' + dir2 + ' -rf', function(err2, out2) {
              cb(err + err2);
            });
          } else {
            cb(null, dir2);
          }
        });
}

function move(src, dst, cb) {
  fs.rename(src, dst, function(err) {
    if (err) {
      cb('Failed to rename "' + src + '" to "' + dst + '"');
    } else {
      cb();
    }
  });
}

function readCurrentSetupInfo(dstPath, cb) {

  // TODO: What do we want to know?
  exec2('git -C ' + dstPath + ' rev-parse HEAD', cb);
}

function backupOriginalAndReplace(updatePath, dstPath, cb) {
  var backup = dstPath + '_backup';
  exec2('rm ' + backup + ' -rf', function(err, out) {

    // First make a backup by moving the original directory to 
    // a new location.
    move(dstPath, backup, function(err, out) {
      if (err) {
        cb(err);
      } else {

        // Now replace the current installation by an updated one.
        move(updatePath, dstPath, function(err, out) {
          if (err) {
            
            // Attempt to restore the previous state.
            move(backup, dstPath, function(err2, out) {
              if (err) {
                cb('Fatal: Failed to finalize update, '
                   + 'and failed to restore previous state. '
                   + 'System may be in an inconsistent state: '
                   + err + err2);
              } else {
                cb('Failed to finalize update, but restored previous state: ' + err);
              }
            });
            
          } else {

            // Seems like we succeeded with the update.
            // Remains to remove the backup directory. Removing the backup
            // directory could fail, but at this stage, we have successfully deployed
            // the new version, so if there is an error removing the backup directory, 
            // we might not want to treat it as such, but nevertheless report it for 
            // later analysis.
            exec('rm ' + backup + ' -rf', function(err, value) {
              cb(null, err);
            });

          }
        });
      }
    });
  });
}





function finalizeUpdate(updatePath, dstPath, err, cb) {

  // At this stage, we have attempted to update the local directory
  // and either failed or succeeded.
  if (err) {
    exec2('rm ' + updatePath + ' -rf', function(errRemove) {
      cb(err + errRemove);
    });
  } else {
    // Now remains taking a backup of our initial directory,
    // and replace it by the updated one.
    backupOriginalAndReplace(updatePath, dstPath, function(err, rmBackupError) {
      if (err) {
        cb(err);
      } else {
        readCurrentSetupInfo(dstPath, function(err, out) {
          if (err) {
            cb(err);
          } else {
            if (rmBackupError) {
              out.stderr = '' + out.stderr + rmBackupError;
            }
            cb(null, out);
          }
        });
      }
    });
  }
}

function performUpdate(bp, up, cb) {
  exec2('git -C ' + up + ' bundle verify ' + bp + ' && git -C ' 
        + up + ' pull ' + bp 
        + ' master && git -C ' + up + ' checkout master', cb);
}

function deploy(localBundleFilename, dstPath, cb) {
  // First ensure that there is a bundle file
  fs.access(localBundleFilename, fs.F_OK, function(err) {
    if (err) {
      cb('Could not find local bundle file: ' + localBundleFilename);
    } else {

      // We are going to perform the update on a copy of the repository
      makeLocalCopy(
        dstPath, '_updated', 

        function(err, updatePath) {
          if (err) {
            cb(err);
          } else {
            performUpdate(
              localBundleFilename, updatePath,
              function(err) {

                finalizeUpdate(updatePath, dstPath, err, cb);

              });
          }
        });
    }
  });
}

function makeMainFunction(localBundleFilename, dstPath) {
  return function(cb) {
    try {

      deploy(localBundleFilename, dstPath, function(err, output) {

        if (err) {
          cb(null, {err: err});
        } else {
          cb(null, output);
        }

      });

    } catch (e) {
      cb(e);
    }
  };
}

module.exports.deploy = deploy;
