var ep = require('endpoint/endpoint.sqlite.js');
var epstate = require('endpoint/epstate.js');
var fs = require('fs');

function openExistingEndpoint(filename, cb) {
  fs.readFile(filename, function(err) {
    if (err) {
      cb(err);
    } else {
      ep.tryMakeEndpoint(filename, 'unknownname', cb);  
    }
  });
}

function report(err) {
  console.log("Got error " + err);
}

function summary(filename) {
  openExistingEndpoint(filename, function(err, ep) {
    if (err) {return report(err);}
    epstate.getEndpointSummary(ep, function(err, data) {
      console.log('Summary of ' + filename + ":\n" 
                  + JSON.stringify(data, null, 2));
    });
  });
}

function reset(filename) {
  openExistingEndpoint(filename, function(err, ep) {
    ep.reset(function(err) {
      if (err) {return report(err);}
      console.log("Successfully reset endpoint.");
    })
  });
}





////// Command line interface
args = process.argv.slice(2)
opkey = args[0]
filename = args[1]

var ops = {
  summary: summary,
  reset: reset
};

if (!(opkey in ops)) {
  console.log("No such operation: " + opkey);
  console.log("Available ops: ");
  for (var op in ops) {
    console.log('  * ' + op)
  }
  console.log('USAGE: node eptool.js [op] [filename]')
} else {
  ops[opkey](filename)
}
