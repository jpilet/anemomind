var exec = require('child_process').exec;

var anemoId;

function getAnemoId(cb) {
  if (anemoId) {
    cb(anemoId);
  } else {
    exec("ifconfig wlan0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'",
      function (error, stdout, stderr) {
        if (stdout) {
          anemoId = stdout;
        } else {
          console.log("wlan0 not found, anemoID is: " + '78:4b:87:a1:f2:61');
          anemoId = '78:4b:87:a1:f2:61';
        }
        cb(anemoId);
      }
    );
  }
}

getAnemoId(function(id){});

module.exports.getAnemoId = getAnemoId;
