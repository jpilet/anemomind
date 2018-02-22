var pgntable = require('./pgntable.js');
var utils = require('./utils.js');

var splitRegEx = /\$.*\*[0-9A-F]{2}\n/g;

function parseAngle(str, digits, neg) {
  if (typeof str != 'string' || str.length < digits+2) {
    return undefined;
  }

  var angle = parseFloat(str.substr(0, digits))
    + parseFloat(str.substr(digits)) / 60;

  if (neg == 'S' || neg == 'W') {
    angle = - angle;
  }
  return angle;
}

function parseLatitude(arg1, arg2) { return parseAngle(arg1, 2, arg2); }
function parseLongitude(arg1, arg2) { return parseAngle(arg1, 3, arg2); }

function getAndSplitSentence(nmea, sentence) {
  var re = new RegExp('\\$(..)' + sentence + '(,.*)+\\*([0-9A-F]{2})');
  var r = nmea.match(re);
  if (!r) {
    return;
  }
  return (r[1] + r[2]).split(',');
}

function dateFromRMC(rmc) {
  if (!rmc) {
    return;
  }

  var time = rmc[1];
  var date = rmc[9];

  return new Date(Date.UTC(
    2000 + parseInt(date.substr(4,6)),
    parseInt(date.substr(2, 4)) - 1,
    parseInt(date.substr(0, 2)),
    parseInt(time.substr(0, 2)),
    parseInt(time.substr(2, 4)),
    parseInt(time.substr(4, 6))));
}

function satsUsedForFix(nmea) {
  var gsas = nmea.match(/\$GNGSA.*\*[0-9A-F]{2}/g);
  var r = [];
  for (var i in gsas) {
    var gsa = getAndSplitSentence(gsas[i], 'GSA');
    for (var i = 3; i < gsa.length - 3; i++) {
      if (!gsa[i] || gsa[i] == '') break;
      r.push({
             referenceStationType: 0, // GPS
             referenceStationId: parseInt(gsa[i]),
             ageOfDgnssCorrections: undefined
      });
    }
  }
  return r;
}

function floatOrUndefined(f) {
  return f ? parseFloat(f) : undefined;
}

var sid = 0;

function makePackets(nmea) {
  var gga = getAndSplitSentence(nmea, 'GGA');
  var rmc = getAndSplitSentence(nmea, 'RMC');
  var gsv = getAndSplitSentence(nmea, 'GSV');
  var gsa = getAndSplitSentence(nmea, 'GSA');

  if (!gga || !rmc || !gsv || !gsa) {
    return;
  }

  var date = dateFromRMC(rmc);
  var secondsInDay = 60 * 60 * 24;
  var days = Math.floor(date.getTime() / (1000 * secondsInDay));
  var seconds = Math.floor(date.getTime() / 1000) - days * secondsInDay;

  sid = utils.nextSid(sid);
  var deviceIndex = 1;// GPS virtual device

  return [{
    pgn: pgntable.cogSogRapidUpdate,
    sid: sid,
    deviceIndex: deviceIndex,
    cogReference: 0,
    cog: utils.tag(floatOrUndefined(rmc[6]), "deg"),
    sog: utils.tag(floatOrUndefined(rmc[5]), "knots")
  }, {
    pgn: pgntable.gnssPositionData,
    sid: sid,
    deviceIndex: deviceIndex,
    date: days,
    time: seconds,
    latitude: parseLatitude(gga[2], gga[3]),
    longitude: parseLongitude(gga[4], gga[5]),
    altitude: floatOrUndefined(gga[9]),
    gnssType: 0,
    method: parseInt(gga[6]),
    integrity: rmc[2] == 'A' ? 1 : 2,
    numberOfSvs: parseInt(gsv[3]),
    hdop: floatOrUndefined(gga[8]),
    pdop: floatOrUndefined(gsa[15]),
    geoidalSeparation: utils.tag(floatOrUndefined(gga[11]), "m"),
    referenceStations: [] // I am not sure what we should send here.
  }];
}

module.exports.makePackets = makePackets;

