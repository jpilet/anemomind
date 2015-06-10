var exec = require('child_process').exec;

var anemoId;
var pending = [];

function getAnemoId(cb) {
  if (anemoId) {
    cb(anemoId);
  } else {
    pending.push(cb);
  }
}

exec("ifconfig wlan0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'",
  function (error, stdout, stderr) {
    if (stdout) {
      anemoId = stdout;
    } else {
      console.log("wlan0 not found, using default anemoID.");
      anemoId = '78:4b:87:a1:f2:61';
    }
    anemoId = anemoId.replace(/(:)|(\s)/g,'');

    console.log("Anemobox ID: " + anemoId);

    for (var i in pending) {
      pending[i](anemoId);
    }
    pending = undefined;
  }
);

module.exports.getAnemoId = getAnemoId;

