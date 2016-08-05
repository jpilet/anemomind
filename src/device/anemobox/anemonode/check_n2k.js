var socket = require('j1939socket');

setInterval(function() {
            console.log('NMEA2000: nothing received, or bad data');
            }, 3000);

// we expect the other side to do:
// ./bin/cansend can0 15fd0723#04c0166dff7fffff
//
socket.open(function (data, timestamp, srcName,
                      pgn, priority, dstAddr, srcAddr) {
  /*
  console.log("t:" + timestamp + " s:" + srcName + " pgn:" + pgn
              + "src: " + srcAddr + " d:" + dstAddr);
  console.warn(data);
  */
  if (pgn == 130311 && data.length == 8) {
    var expected = ['04', 'c0', '16', '6d', 'ff', '7f', 'ff', 'ff'];
    for (var i = 0; i < 8; ++i) {
      if (data[i] != parseInt(expected[i], 16)) {
        return;
      }
    }
    console.log('NMEA2000: test OK.');
    process.exit(0);
  }    
}
);

