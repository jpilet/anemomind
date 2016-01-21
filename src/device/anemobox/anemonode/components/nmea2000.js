var j1939socket = require('j1939socket');

function handlePacket(data, timestamp, srcName, pgn, priority, dstAddr) {
  if (false) {
    var str = "t:" + timestamp + " s:" + srcName + " pgn:" + pgn + " d:" + dstAddr;

    for (var i = 0; i < data.length; ++i) {
      str += " " + data[i].toString(16);
    }
    console.log(str);
  }
}

j1939socket.open(handlePacket);

