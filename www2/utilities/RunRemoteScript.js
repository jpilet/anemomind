var common = require('../server/api/boxexec/remoteOps');

common.init();

function sentCallback(err, reqCode) {
  if (err) {
    console.log('Failed to send script to box because');
    console.log(err);
    process.exit(1);
  } else {
    console.log('Successfully posted script to box for remote execution.');
    console.log('You can view it by calling\n');
    console.log('  node ViewRemoteScript.js ' + reqCode);
    console.log('');
    process.exit(0);
  }
}

var boatId = process.argv[2];
console.log('Boat id:         ' + boatId);

/*

  Two different calling formats:

  (i)
  node RemoteScript.js boatId scriptFilename

     where scriptFilename should end with either .js or .sh

  or

  (ii)
  node RemoteScript.js boatId scriptType scriptData

     where scriptType is 'js' or 'sh', and
     scriptData is the script. If it is 'js', it
     should, when evaled, return a function that
     takes a single callback to which it passes its result.
*/

if (process.argv[4]) {
  var scriptType = process.argv[3];
  var scriptData = process.argv[4];
  console.log('Script type:     ' + scriptType);
  console.log('Script:          ' + scriptData);
  common.sendScriptToBox(boatId, scriptType, scriptData, sentCallback);
} else {
  var scriptFilename = process.argv[3];
  console.log('Script filename: ' + scriptFilename);
  common.sendScriptFileToBox(boatId, scriptFilename, sentCallback);
}
