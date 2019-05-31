var anemonode = require('../build/Release/anemonode');

var minResendTime = 490; // ms
var maxVmgTargetAgeMs = 5000;

var subscriptions = undefined;
var callback = function() { };
var outputFormat = undefined;
var updateInterval;

var lastSentTime;
var lastEmptySent;

var lastActionTime = {};

function rateLimitedAction(name, dt_ms) {
  var now = anemonode.currentTime().getTime();
  if (!lastActionTime[name] || (now - lastActionTime[name]) > dt_ms) {
    lastActionTime[name] = now;
    return true;
  }
  return false;
}

function getPerf() {
  var now = anemonode.currentTime();

  if (lastSentTime != undefined
      && (now - lastSentTime) < minResendTime) {
    return;
  }
  
  var vmgDispData = anemonode.dispatcher.values.vmg;
  if (!vmgDispData) {
    return;
  }
  var targetVmgDispData = anemonode.dispatcher.values.targetVmg;
  if (!targetVmgDispData) {
    return;
  }

  if ((lastSentTime != undefined
       && vmgDispData.time() < lastSentTime) // Nothing new to send?
      || targetVmgDispData.time() < (now - maxVmgTargetAgeMs)) {  // too old
    return;
  }

  var targetVmg = targetVmgDispData.value();

  if (targetVmg > 0) {
    lastSentTime = now;
    return {
      'vmg': vmgDispData.value(),
      'targetVmg': targetVmg
    };
  }
}

function nmeaPacket(data) {
  var sum = 0;
  for (var i = 0; i < data.length; ++i) {
    sum ^= data.charCodeAt(i);
  }
  var hexsum = sum.toString(16);
  if (hexsum.length == 1) {
    hexsum = "0" + hexsum;
  }
  hexsum = hexsum.toUpperCase();

  return "$" + data + "*" + hexsum + "\r\n";
}

/* NKE specific code */
function nkeDynamic(data2CharCode, channelNumber, format, value, dataLabel, unitLabel) {
  return nmeaPacket(
      [
      'PNKEA',
      data2CharCode,
      channelNumber,
      format,
      value,
      dataLabel,
      unitLabel].join(','));
}

function nkePerfSentences(perf, targetVmg) {
  return (
      nkeDynamic(
         'PV', 1, 4 /* XXX_X 0 to 999.9 number*/,
         Math.round(perf * 10),
         'VMG Perf', 'Prcent')
      + nkeDynamic(
          'TV', 2, 0, /* xx_xx 0.00 to 99.99 number */
          Math.round(targetVmg * 100),
          'VMG Target', 'Nds'));
}


function tacktickCustomPage(pageno, header, footer) {
  if ((pageno < 1) || (pageno > 4)) {
    throw new Error("tacktick page out of range");
  }

  return nmeaPacket("PTAK,FFP" + pageno + "," + header + "," + footer);
}

function tacktickPageData(pageno, data) {
  return nmeaPacket("PTAK,FFD" + pageno + "," + data);
}

function tacktickWindCorrect(twaDeltaDeg, twsDeltaKnots) {
  return nmeaPacket("PTAK,TWC,"
   + twsDeltaKnots.toFixed(1) + ","
   + twaDeltaDeg.toFixed(0));
}

function update() {
  var perf = getPerf();
  var nmea = "";
  var perfPercent = 0;

  if (perf && Math.abs(perf.targetVmg) > 0) {
    perfPercent = 100 * perf.vmg / perf.targetVmg;
  }

  if (outputFormat == 'NKE') {
    if (rateLimitedAction('sendNKE', 990)) {
      if (perf) {
        nmea = nkePerfSentences(perfPercent, perf.targetVmg)
      } else {
        nmea = nkePerfSentences(0, 0);
      }
    }
  } else if (outputFormat == 'TackTick') {
    if (perf && rateLimitedAction('sendTackTickData', 1000)) {
      nmea += tacktickPageData(1, perfPercent.toFixed(0));
      nmea += tacktickPageData(2, perf.targetVmg.toFixed(1));
    }
    if (rateLimitedAction('sendCustomPage', 20000)) {
      nmea += tacktickCustomPage(1, "VMGPRF", "PERCENT") +
        tacktickCustomPage(2, "VMGTGT", "KNOTS");
    }
  }
  if (nmea) {
    callback(nmea);
  }
}

var acceptedFormats = { NKE: true, TackTick: true };

function activateNmea0183PerfOutput(nmea0183cb, format) {
  if (!acceptedFormats[format]) {
    nmea0183cb = undefined;
  }

  if (nmea0183cb) {
    callback = nmea0183cb;
    outputFormat = format;
    if (subscriptions == undefined) {
      subscriptions = {
        vmg: anemonode.dispatcher.values.vmg.subscribe(update),
        targetVmg: anemonode.dispatcher.values.targetVmg.subscribe(update),
      };
    }
    if (updateInterval == undefined) {
      updateInterval = setInterval(update, 1000);
    }
  } else {
    callback = function() {};
    outputFormat = undefined;
    if (subscriptions != undefined) {
      anemonode.dispatcher.values.vmg.unsubscribe(subscriptions.vmg);
      anemonode.dispatcher.values.targetVmg.unsubscribe(subscriptions.targetVmg);
      subscriptions = undefined;
    }
    if (updateInterval != undefined) {
      clearInterval(updateInterval);
      updateInterval = undefined;
    }
  }
}

module.exports.activateNmea0183PerfOutput = activateNmea0183PerfOutput;
module.exports.nkeDynamic = nkeDynamic;
module.exports.nmeaPacket = nmeaPacket;
