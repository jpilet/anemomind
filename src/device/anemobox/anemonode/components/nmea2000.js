// Emulate candump -L can0
var anemonode = require('../build/Release/anemonode');
var logger = require('./logger.js');
var nmea2000Source = new anemonode.Nmea2000Source();
var exec = require('child_process').exec;
var infrequent = require('./infrequent.js');
var canutils = require('./canutils.js');
var fork = require('child_process').fork;

var racc = infrequent.makeAcceptor(1000);
var verbose = true;

function logRawPacket(jsocket, msg) {
  var l = logger.getLogger();
  if (l) {
    var systemTime0 = 1000*msg.ts_sec + 0.001*msg.ts_usec;
    var systemTime1 = new Date().getTime();
    var monotonicTime1 = anemonode.currentTime();
  
    // monotonicTime1 - monotonicTime0 = systemTime1 - systemTime0
    var delay = systemTime1 - systemTime0;
    monotonicTime0 = monotonicTime1 - delay;
    
    if (verbose && racc()) {
      console.log("RAW DATA: %j", msg);
    }
    l.logRawNmea2000(
	monotonicTime0,
	msg.id, msg.data);
  }
  
  // This will, hopefully, trigger a packet to be
  // delivered, inside "nmea2000.js".
  jsocket.fetch();
}

var jacc = infrequent.makeAcceptor(1000);

function handlePacket(data, timestamp, srcName, pgn, priority, dstAddr, srcAddr) {
  nmea2000Source.process(data, timestamp, srcName, pgn, srcAddr);

  if (jacc()) {
    var str = "t:" + timestamp + " s:" + srcName + " pgn:" + pgn + " d:" + dstAddr;
    for (var i = 0; i < data.length; ++i) {
      str += " " + data[i].toString(16);
    }
    console.log(str);
  }
}

var src = null;
function start() {
  if (src) {
    console.log("Restarting CanSource...");
    src.start();
  } else {
    var j1939 = require('j1939socket').j1939;
    var jsocket = new j1939.J1939Socket("can0");
    jsocket.open(handlePacket);
    src = new CanSource(function(msg) {
       logRawPacket(jsocket, msg);
    });
    src.start();
  }
}

function CanSource(cb) {
  this.process = null;
  this.cb = cb;
}

function dispKeys(x) {
  for (var k in x) {
    console.log("Key: %j", k);
  }
}


// Returns a mapping transducer
function map(f) {
  return function(red) {
    return function(dst, x) {
      return red(dst, f(x));
    };
  };
}

// Returns a filtering transducer
function filter(f) {
  return function(red) {
    return function(dst, x) {
      return f(x)? red(dst, x) : dst;
    };
  };
}

function compose2(f, g) {
  return function(x) {
    return f(g(x));
  };
}

function compose() {
  var args = Array.prototype.slice.call(arguments);
  return args.reduce(compose2);
}

// Returns true if the message should be accepted (no error),
// otherwise displays the error, but not too often.
function messageAcceptor() {
  var acc = infrequent.makeAcceptor(1000);
  return function(msg) {
    if (msg.error) {
      if (acc()) {
        console.log("GOT ERROR from raw can source: ");
	console.log(msg);
      }
      return null;
    } else {
      return msg;
    }
  }
}

CanSource.prototype.stop = function() {
  if (this.process) {
    this.process.stdin.pause();
    this.process.kill();
  }
}

CanSource.prototype.start = function() {
  this.stop();
  this.process = fork('./components/cansource.js', [], {
    stdio: 'pipe',
    silent: true
  });
  if (!this.process) {
    throw new Error("Failed to instantiate child can source.");
  }
  
  var pipeline = compose(
  	canutils.catSplit("\n"), 
  	map(canutils.deserializeMessage),
	filter(messageAcceptor()));
  	
  var processData = pipeline(function(cb, x) {
    cb(x);
    return cb;
  });
  
  var self = this;
  this.process.stdout.on('data', function(data) {
    processData(self.cb, data);
  });
}

// HACK to reboot we hit SPI bug
function detectSPIBug(callback) {
  var timer = setInterval(function() {
    exec("ps -A -o '%C/%c' | grep kworker | sort -n -r | head -2 | sed 's#/.*##' | awk '{s+=$1} END {print s < 60}'",
         function(error, stdout, stderr) {
            if (stdout.trim() != "1") {
               clearInterval(timer);
               callback();
            }
         }
    );
  }
  , 10 * 1000);
};

module.exports.start = start;
module.exports.detectSPIBug = detectSPIBug;
module.exports.CanSource = CanSource;








